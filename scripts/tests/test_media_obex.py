"""Guard OBEX startup after enabling support in an existing build directory."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class ObexStartupTest(unittest.TestCase):
    def test_minimal_service_keeps_required_server_driver(self):
        unit = (ROOT / 'overlays/components/media/rootfs/etc/systemd/system/'
                'a333-obex.service').read_text()
        command = next(line for line in unit.splitlines() if line.startswith('ExecStart='))
        self.assertIn('--plugin=bluetooth,filesystem,opp', command)
        self.assertNotIn('--auto-accept', command)
        self.assertNotIn('--noplugin=', command)
        self.assertIn('BusName=org.bluez.obex', unit)
        self.assertIn('DBUS_SESSION_BUS_ADDRESS=unix:path=/run/dbus/system_bus_socket', unit)

    def test_builtin_list_depends_on_configured_makefile(self):
        patch = (ROOT / 'configs/boards/a333/global-patches/bluez5_utils/'
                 '0001-regenerate-obex-builtin-after-configure.patch').read_text()
        self.assertIn('+obexd/src/builtin.h: Makefile obexd/src/genbuiltin', patch)
        for profile in ('a333_media_defconfig', 'a333_media_kernel_debug_logs_defconfig'):
            config = (ROOT / 'configs/configs' / profile).read_text()
            self.assertIn('BR2_PACKAGE_BLUEZ5_UTILS_OBEX=y', config)
            self.assertIn('BR2_GLOBAL_PATCH_DIR="../configs/boards/a333/global-patches"', config)
