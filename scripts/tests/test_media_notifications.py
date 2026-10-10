"""Test connection event ordering without DBus, hardware or userdata access."""
import sys
import importlib.util
import json
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'oem/a333/media-rootfs/usr/libexec'))
from a333_media_notifications import Notifications


class NotificationsTest(unittest.TestCase):
    def setUp(self):
        self.now = 10
        self.notices = Notifications(lambda: self.now)
        self.notices.observe_mounts([])
        self.notices.observe_bluetooth({})

    def drive(self, name, kind='usb'):
        return dict(path='/media/usb/' + name if kind == 'usb' else '/media/sd',
                    device='/dev/' + name, kind=kind, label=name)

    def test_each_drive_and_sd_produce_events(self):
        a, b, sd = self.drive('sda1'), self.drive('sdb1'), self.drive('mmcblk1p1', 'sd')
        self.notices.observe_mounts([a])
        text, sequence = self.notices.snapshot()
        self.assertIn('USB-накопитель подключён: sda1', text)
        self.notices.observe_mounts([a, b, sd])
        self.notices.observe_mounts([a, sd])
        for expected in ['подключён: sdb1', 'SD-карта', 'отключён: sdb1']:
            self.now += 5
            text, new_sequence = self.notices.snapshot()
            self.assertIn(expected, text)
            self.assertGreater(int(new_sequence), int(sequence))
            sequence = new_sequence
        self.now += 5
        self.assertEqual(self.notices.snapshot()[0], '')

    def test_bt_connected_not_just_playing_or_paired(self):
        self.notices.observe_bluetooth({'phone': 'Телефон'})
        self.assertEqual(self.notices.snapshot()[0], 'Bluetooth подключён: Телефон')
        self.notices.observe_bluetooth({'phone': 'Телефон', 'headset': 'Наушники'})
        self.notices.observe_bluetooth({'headset': 'Наушники'})
        self.now += 5
        self.assertEqual(self.notices.snapshot()[0], 'Bluetooth подключён: Наушники')
        self.now += 5
        self.assertEqual(self.notices.snapshot()[0], 'Bluetooth отключён: Телефон')

    def test_initial_state_and_unchanged_state_do_not_repeat(self):
        notices = Notifications(lambda: self.now)
        drive = self.drive('sda1')
        notices.observe_mounts([drive])
        notices.observe_mounts([drive])
        notices.observe_bluetooth({'phone': 'Телефон'})
        notices.observe_bluetooth({'phone': 'Телефон'})
        self.assertEqual(notices.snapshot()[0], '')

    def test_text_is_bounded_and_sanitized(self):
        self.notices.push('a\nb\r' + 'x' * 400)
        text, _ = self.notices.snapshot()
        self.assertLessEqual(len(text), 240)
        self.assertNotIn('\n', text)


class IntegrationTest(unittest.TestCase):
    def test_eez_notification_migration_is_idempotent(self):
        root = Path(__file__).resolve().parents[2]
        spec = importlib.util.spec_from_file_location('notifications_migration',
            root / 'scripts/prepare-media-eez-notifications.py')
        migration = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(migration)
        project = json.loads((root / 'oem/a333/src/media-panel/eez/lvgl-media-player.eez-project').read_text())
        before = json.dumps(project)
        self.assertEqual(json.dumps(migration.migrate(project)), before)

    def test_freetype_packaging_and_regular_style(self):
        root = Path(__file__).resolve().parents[2]
        source = root / 'oem/a333/src/media-panel'
        self.assertGreater((source / 'assets/fonts/SkodaPro-Medium.ttf').stat().st_size, 10000)
        self.assertIn('LV_FREETYPE_FONT_STYLE_NORMAL', (source / 'media_fonts.c').read_text())
        for profile in ('a333_media_defconfig', 'a333_media_kernel_debug_logs_defconfig'):
            self.assertIn('BR2_PACKAGE_FREETYPE=y', (root / 'configs/configs' / profile).read_text())
        self.assertIn('MEDIA_PANEL_DEPENDENCIES = freetype',
                      (root / 'configs/package/media-panel/media-panel.mk').read_text())
