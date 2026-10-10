"""Test the C snapshot/socket adapter against temporary files, not userdata."""
import ctypes
import ast
import logging
import os
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time
import unittest
import sys
from unittest.mock import AsyncMock
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'oem/a333/media-rootfs/usr/libexec'))
from a333_media_playback import seconds

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'oem/a333/src/media-panel/media_backend.c'
BACKEND = ROOT / 'oem/a333/media-rootfs/usr/libexec/a333-media-backend'


class BackendStatusTest(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        tree = ast.parse(BACKEND.read_text())
        nodes = [n for n in tree.body if isinstance(n, (ast.FunctionDef, ast.AsyncFunctionDef))
                 and n.name in ('clean', 'update_status')]
        self.namespace = dict(logging=logging, os=os, time=time, RUN=Path(self.directory.name),
                              seconds=seconds, bt_track=AsyncMock(return_value=None), run_mpc=AsyncMock())
        exec(compile(ast.Module(body=nodes, type_ignores=[]), str(BACKEND), 'exec'), self.namespace)

    async def status(self, detail):
        self.namespace['run_mpc'].side_effect = ['[playing]', detail]
        await self.namespace['update_status'](None, {})
        return (Path(self.directory.name) / 'status').read_text().splitlines()

    async def test_filename_and_empty_tags_do_not_shift(self):
        lines = await self.status('Альбом/песня.flac\n\nИсполнитель\nАльбом')
        self.assertEqual(lines[:4], ['Music / radio', 'песня.flac', 'Исполнитель', 'Альбом'])
        self.assertEqual(lines[6], 'Альбом/песня.flac')
        self.assertEqual(len(lines), 13)

    async def test_tags_and_radio_uri(self):
        lines = await self.status('https://radio.test/live\nRadio title\nArtist\nAlbum')
        self.assertEqual(lines[1:4], ['Radio title', 'Artist', 'Album'])
        self.assertEqual(lines[6], 'https://radio.test/live')

    async def test_bluetooth_has_no_stale_mpd_filename(self):
        self.namespace['run_mpc'].return_value = ''
        self.namespace['bt_track'].return_value = ('/player', 'playing',
            dict(Title='Bluetooth title', Artist='Artist', Album='Album'))
        await self.namespace['update_status'](None, {})
        lines = (Path(self.directory.name) / 'status').read_text().splitlines()
        self.assertEqual(lines[0], 'Bluetooth')
        self.assertEqual(lines[6], '')


class State(ctypes.Structure):
    _fields_ = [(name, ctypes.c_char * 1024)
                for name in ('source', 'title', 'artist', 'album', 'volume', 'filename')] + [
        ('volume_sequence', ctypes.c_ulong), ('volume_until', ctypes.c_double),
        ('volume_visible', ctypes.c_bool), ('position', ctypes.c_double),
        ('duration', ctypes.c_double), ('seekable', ctypes.c_bool),
        ('usb_available', ctypes.c_bool), ('sd_available', ctypes.c_bool),
        ('track_id', ctypes.c_char * 64), ('cover_path', ctypes.c_char * 1024)]


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

    def test_filename_extension_and_legacy_snapshot(self):
        self.assertTrue(self.poll('Music\nTrack\nArtist\nAlbum\n\n0\nМузыка/трек.flac\n'))
        self.assertEqual(self.state.filename.decode(), 'Музыка/трек.flac')
        self.assertFalse(self.poll('New\nChanged\nArtist\nAlbum\n\n0\nunterminated'))
        self.assertEqual(self.state.title, b'Track')
        self.assertTrue(self.poll('Music\nTrack\nArtist\nAlbum\n\n0\n'))
        self.assertEqual(self.state.filename, b'')

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
