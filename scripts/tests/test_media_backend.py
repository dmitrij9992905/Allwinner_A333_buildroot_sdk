"""Test the C snapshot/socket adapter against temporary files, not userdata."""
import ctypes
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'oem/a333/src/media-panel/media_backend.c'


class State(ctypes.Structure):
    _fields_ = [(name, ctypes.c_char * 1024)
                for name in ('source', 'title', 'artist', 'album', 'volume')] + [
        ('volume_sequence', ctypes.c_ulong), ('volume_until', ctypes.c_double),
        ('volume_visible', ctypes.c_bool)]


@unittest.skipUnless(shutil.which('cc'), 'native C compiler is required')
class MediaBackendTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.build.cleanup)
        output = Path(cls.build.name) / 'backend.so'
        subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-shared',
                        '-fPIC', str(SOURCE), '-o', str(output)], check=True)
        cls.lib = ctypes.CDLL(str(output), use_errno=True)
        cls.lib.media_backend_init.argtypes = [ctypes.POINTER(State)]
        cls.lib.media_backend_poll.argtypes = [ctypes.POINTER(State), ctypes.c_char_p, ctypes.c_double]
        cls.lib.media_backend_poll.restype = ctypes.c_bool
        cls.lib.media_backend_send.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        cls.lib.media_backend_send.restype = ctypes.c_bool
        cls.lib.media_backend_request.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        cls.lib.media_backend_request.restype = ctypes.c_bool

    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / 'status'
        self.state = State()
        self.lib.media_backend_init(ctypes.byref(self.state))

    def poll(self, text, now=1):
        self.path.write_text(text, encoding='utf-8')
        return self.lib.media_backend_poll(ctypes.byref(self.state), bytes(self.path), now)

    def test_complete_metadata_with_unicode(self):
        title = 'Музыка 🎵' * 25
        self.assertTrue(self.poll('Music\n' + title + '\nArtist\nAlbum\n\n0\n'))
        self.assertEqual(self.state.title.decode(), title)

    def test_incomplete_and_invalid_sequence_keep_last_snapshot(self):
        self.assertTrue(self.poll('Music\nTrack\nArtist\nAlbum\n\n0\n'))
        for sequence in ('-1', 'bad', '99999999999999999999999999999999999'):
            self.assertFalse(self.poll('New\nChanged\nArtist\nAlbum\n\n' + sequence + '\n'))
            self.assertEqual(self.state.title, b'Track')
        self.assertFalse(self.poll('New\nChanged\n'))
        self.assertEqual(self.state.title, b'Track')

    def test_banner_expires_five_seconds_after_last_event(self):
        snapshot = 'Music\nTrack\nArtist\nAlbum\nVolume: 40%\n1\n'
        self.assertTrue(self.poll(snapshot, 10))
        self.assertTrue(self.state.volume_visible)
        self.assertTrue(self.poll(snapshot, 14.9))
        self.assertEqual(self.state.volume_until, 15)
        self.assertTrue(self.poll(snapshot.replace('1\n', '2\n'), 14.9))
        self.assertAlmostEqual(self.state.volume_until, 19.9)
        self.assertTrue(self.poll(snapshot.replace('1\n', '2\n'), 20))
        self.assertFalse(self.state.volume_visible)

    def test_banner_expires_even_if_backend_disappears(self):
        self.poll('Music\nTrack\nArtist\nAlbum\nVolume: 40%\n1\n', 10)
        self.path.unlink()
        self.assertFalse(self.lib.media_backend_poll(ctypes.byref(self.state), bytes(self.path), 16))
        self.assertFalse(self.state.volume_visible)

    def test_socket_allowlist_and_json_are_nonblocking(self):
        path = str(Path(self.directory.name) / 'control.sock')
        with socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM) as receiver:
            receiver.bind(path)
            receiver.settimeout(1)
            self.assertTrue(self.lib.media_backend_send(path.encode(), b'toggle'))
            self.assertEqual(receiver.recv(4096), b'toggle')
            self.assertFalse(self.lib.media_backend_send(path.encode(), b'erase'))
            request = b'{"command":"wifi_scan"}'
            self.assertTrue(self.lib.media_backend_request(path.encode(), request))
            self.assertEqual(receiver.recv(4096), request)
            self.assertFalse(self.lib.media_backend_request(path.encode(), b'x' * 4096))
