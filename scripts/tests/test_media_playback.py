"""MPD/artwork and storage tests use only temporary files and mock devices."""
import asyncio
import importlib.util
import io
import json
from pathlib import Path
import sys
import tempfile
import socket
import threading
import unittest
from unittest.mock import patch, AsyncMock

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'oem/a333/media-rootfs/usr/libexec'))
import a333_media_playback as playback

spec = importlib.util.spec_from_file_location('storage',
    ROOT / 'overlays/a333/rootfs/usr/libexec/a333_removable_storage.py')
storage = importlib.util.module_from_spec(spec)
spec.loader.exec_module(storage)


class PlaybackTest(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.controller = playback.PlaybackController(self.root / 'run', storage=self.root / 'mounts.json')

    def test_library_preserves_user_files_and_refuses_collision(self):
        music = self.root / 'music'
        music.mkdir()
        track = music / 'keep.mp3'
        track.write_bytes(b'untouched')
        library = self.root / 'library'
        playback.prepare_library(library)
        playback.prepare_library(library)
        self.assertEqual((library / 'local').resolve(), music)
        self.assertEqual(track.read_bytes(), b'untouched')
        (library / 'usb').unlink()
        (library / 'usb').mkdir()
        with self.assertRaises(ValueError):
            playback.prepare_library(library)

    def test_seconds_and_arguments_are_bounded(self):
        for value in ('nan', 'inf', '-1', '100000000', None):
            self.assertEqual(playback.seconds(value), 0)
        self.assertEqual(playback.seconds('12.25'), 12.25)
        with self.assertRaises(ValueError):
            playback.quote('song\nclear')

    async def test_seek_checks_track_identity_and_bounds(self):
        calls = []
        def command(name, *args):
            calls.append((name, args))
            return ({'songid': '7', 'duration': '100'} if name == 'status' else {}), b''
        self.controller.mpd.command = command
        await self.controller.seek({'seconds': 42, 'track': '7'}, 'MPD')
        self.assertEqual(calls[-1], ('seekid', ('7', '42.000')))
        for request in ({'seconds': 42, 'track': '8'}, {'seconds': 101, 'track': '7'},
                        {'seconds': True, 'track': '7'}, {'seconds': float('nan'), 'track': '7'}):
            with self.assertRaises(ValueError):
                await self.controller.seek(request, 'MPD')
        with self.assertRaises(ValueError):
            await self.controller.seek({'seconds': 42, 'track': '7'}, 'Bluetooth')

    def test_embedded_art_chunks_and_external_fallback(self):
        mpd = playback.MPD()
        def command(name, uri, offset):
            if name == 'readpicture':
                raise ValueError('No embedded art')
            return {'size': '6'}, b'abcdef'[offset:offset + 2]
        mpd.command = command
        self.assertEqual(mpd.picture('local/track.flac'), b'abcdef')

    def test_wire_protocol_reads_binary_not_text(self):
        with socket.socket() as server:
            server.bind(('127.0.0.1', 0))
            server.listen(1)
            requests = []
            def respond():
                with server.accept()[0] as client:
                    client.sendall(b'OK MPD 0.23.17\n')
                    requests.append(client.makefile('rb').readline())
                    client.sendall(b'size: 5\nbinary: 5\nA\n\x00BC\nOK\n')
            worker = threading.Thread(target=respond, daemon=True)
            worker.start()
            mpd = playback.MPD(port=server.getsockname()[1])
            fields, data = mpd.command('readpicture', 'local/a"b.flac', 0)
            worker.join(2)
            self.assertFalse(worker.is_alive())
            self.assertEqual(data, b'A\n\x00BC')
            self.assertEqual(fields['size'], '5')
            self.assertIn(b'a\\"b.flac', requests[0])

    async def test_removing_one_usb_stops_its_track_even_with_another_attached(self):
        self.controller.snapshot = {'file': 'usb/sda1/track.mp3'}
        self.controller.storage.write_text(json.dumps({'revision': 1, 'mounts': [
            {'kind': 'usb', 'path': '/media/usb/sdb1'}]}))
        calls = []
        self.controller.mpd.command = lambda name, *args: (calls.append(name) or ({}, b''))
        await self.controller.refresh_storage()
        self.assertTrue(self.controller.available('usb'))
        self.assertIn('stop', calls)

    async def test_storage_removal_stops_active_media_and_rescans(self):
        self.controller.storage.write_text(json.dumps({'revision': 1, 'mounts': []}))
        self.controller.snapshot = {'file': 'usb/sda1/track.mp3'}
        calls = []
        self.controller.mpd.command = lambda name, *args: (calls.append(name) or ({}, b''))
        await self.controller.refresh_storage()
        self.assertIn('stop', calls)
        self.controller.storage.write_text(json.dumps({'revision': 2, 'mounts': []}))
        await self.controller.refresh_storage()
        self.assertIn('update', calls)

    def test_art_is_resized_and_original_bytes_are_unchanged(self):
        try:
            from PIL import Image
        except ImportError:
            self.skipTest('Pillow is not available on this host')
        source = io.BytesIO()
        Image.new('RGB', (1000, 500), 'red').save(source, format='JPEG')
        data = source.getvalue()
        destination = self.root / 'cover.png'
        playback.normalize_picture(data, destination)
        with Image.open(destination) as image:
            self.assertEqual(image.size, (512, 256))
        self.assertEqual(source.getvalue(), data)


class StorageTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.sysfs = self.root / 'class/block'
        self.sysfs.mkdir(parents=True)

    def disk(self, name, kind):
        directory = self.root / 'devices' / ('usb1/1-1/block' if kind == 'usb' else 'mmc_host') / name
        (directory / 'device').mkdir(parents=True)
        (directory / 'device/type').write_text('SD' if kind == 'sd' else 'MMC')
        (self.sysfs / name).symlink_to(directory)
        return directory

    def test_only_usb_and_sd_never_emmc_or_loop(self):
        usb = self.disk('sda', 'usb')
        part = usb / 'sda1'
        part.mkdir()
        (part / 'partition').write_text('1')
        (self.sysfs / 'sda1').symlink_to(part)
        self.disk('mmcblk0', 'emmc')
        self.disk('mmcblk2', 'sd')
        self.disk('sdb', 'sata')
        self.disk('loop0', 'loop')
        found = storage.discover(self.sysfs)
        self.assertEqual({d['device'] for d in found}, {'/dev/sda1', '/dev/mmcblk2'})
        self.assertEqual(next(d['path'] for d in found if d['kind'] == 'sd'), '/media/sd')

    def test_service_has_no_private_mount_namespace_and_uses_tmpfs(self):
        base = ROOT / 'overlays/a333/rootfs/etc/systemd/system'
        unit = (base / 'a333-removable-storage.service').read_text()
        self.assertNotIn('PrivateMounts=yes', unit)
        self.assertNotIn('ProtectSystem=', unit)
        self.assertIn('Requires=media.mount', unit)
        self.assertIn('What=tmpfs', (base / 'media.mount').read_text())
        source = (ROOT / 'overlays/a333/rootfs/usr/libexec/a333_removable_storage.py').read_text()
        self.assertIn('ro,nodev,nosuid,noexec', source)
        self.assertNotIn('mkfs', source)
        self.assertNotIn('rmtree', source)

    def test_restart_recovers_only_owned_live_mounts(self):
        entries = [{'kind': 'usb', 'path': '/media/usb/sda1', 'device': '/dev/sda1'},
                   {'kind': 'sd', 'path': '/media/sd', 'device': '/dev/mmcblk2p1'},
                   {'kind': 'usb', 'path': '/userdata', 'device': '/dev/mmcblk0p10'}]
        (self.root / 'mounts.json').write_text(json.dumps({'mounts': entries}))
        with patch.object(storage, 'mounted_sources', return_value={e['path']: e['device'] for e in entries}), \
                patch.object(storage.Storage, 'publish'):
            daemon = storage.Storage(self.root)
        self.assertEqual(set(daemon.active), {'/dev/sda1', '/dev/mmcblk2p1'})

    def test_migration_is_idempotent(self):
        spec = importlib.util.spec_from_file_location('migration', ROOT / 'scripts/prepare-media-eez-playback.py')
        migration = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(migration)
        project = json.loads((ROOT / 'oem/a333/src/media-panel/eez/lvgl-media-player.eez-project').read_text())
        before = json.dumps(project)
        self.assertEqual(json.dumps(migration.migrate(project)), before)


if __name__ == '__main__':
    unittest.main()
