"""Exercise the early pack hook with real TOC packages and a fake image extractor."""
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
HOOK = ROOT / 'configs/boards/a333/helperboard-a333/pack-boot-files.sh'
DRAGON = ROOT / 'vendor/allwinner-a333/pack-sdk/tools/pack/pctools/linux/openssl/dragonsecboot'


class PackBootFilesTest(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix='a333-boot-files-test-')
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.work = self.root / 'work'
        self.out = self.work / 'pack_out'
        self.out.mkdir(parents=True)
        self.factory = self.root / 'factory.img'
        self.factory.write_bytes(b'factory image placeholder')
        self.inputs = self.root / 'factory-items'
        self.inputs.mkdir()
        self.boot0 = self.inputs / 'boot0.fex'
        self.boot0.write_bytes(b'boot0' * 1024)
        self.scp = self.inputs / 'scp.fex'
        self.scp.write_bytes(b'SCP1' * 1024)
        for name in ('u-boot', 'monitor'):
            (self.inputs / (name + '.fex')).write_bytes(name.encode() * 1024)
            shutil.copy2(self.inputs / (name + '.fex'), self.out)
        config = '[package]\nitem=u-boot, u-boot.fex\nitem=monitor, monitor.fex\nitem=scp, scp.fex\n'
        (self.inputs / 'boot_package.cfg').write_text(config)
        (self.out / 'boot_package.cfg').write_text(config)
        self.make_factory_package()
        parser = self.root / 'vendor/allwinner-a333/pack-sdk/tools/pack/pctools/linux/mod_update/parser_img'
        parser.parent.mkdir(parents=True)
        parser.write_text('''#!/usr/bin/env python3
import os, shutil, sys
from pathlib import Path
source = Path(os.environ['TEST_FACTORY_ITEMS'])
name = 'boot0.fex' if sys.argv[4] == '1234567890BOOT_0' else 'boot_package.fex'
shutil.copyfile(source / name, sys.argv[2])
''')
        parser.chmod(0o755)
        self.env = {key: value for key, value in os.environ.items()
                    if not key.startswith('A333_FACTORY_')}
        self.env.update(A333_PROJECT_ROOT=str(self.root), A333_PACK_WORK=str(self.work),
                        TEST_FACTORY_ITEMS=str(self.inputs))

    def make_factory_package(self):
        subprocess.run([str(DRAGON), '-pack', 'boot_package.cfg'], cwd=self.inputs,
                       check=True, capture_output=True)

    def invoke(self):
        return subprocess.run(['sh', str(HOOK)], cwd=self.out, env=self.env,
                              text=True, capture_output=True, timeout=15)

    def pack(self):
        return subprocess.run([str(DRAGON), '-pack', 'boot_package.cfg'], cwd=self.out,
                              text=True, capture_output=True, timeout=15)

    def test_factory_scp_is_available_for_the_first_package_creation(self):
        # Reproduce the original failure before running the new early hook.
        self.assertNotEqual(self.pack().returncode, 0)
        self.env['A333_FACTORY_IMAGE'] = str(self.factory)
        result = self.invoke()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual((self.out / 'scp.fex').read_bytes(), self.scp.read_bytes())
        self.assertEqual((self.out / 'boot0_sdcard.fex').read_bytes(), self.boot0.read_bytes())
        self.assertEqual((self.work / 'factory-boot0.fex').read_bytes(), self.boot0.read_bytes())
        self.assertEqual(self.pack().returncode, 0)
        self.assertIn(b'IIE;scp\0', (self.out / 'boot_package.fex').read_bytes())
        self.assertFalse(list(self.work.glob('.factory-boot.*')))

    def test_explicit_pair_enables_scp_in_older_configs(self):
        self.env.update(A333_FACTORY_BOOT0=str(self.boot0), A333_FACTORY_SCP=str(self.scp))
        config = self.out / 'boot_package.cfg'
        config.write_text(config.read_text().replace('item=scp,', '; item=scp,'))
        result = self.invoke()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('\nitem=scp,', config.read_text())
        self.assertEqual(self.pack().returncode, 0)

    def test_vendor_scp_needs_no_factory_image(self):
        shutil.copy2(self.scp, self.out)
        result = self.invoke()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(self.pack().returncode, 0)

    def test_actual_vendor_scp_needs_no_factory_image(self):
        firmware = ROOT / 'vendor/allwinner-a333/pack-sdk/device/config/chips/a333/bin/scp.bin'
        self.assertTrue(firmware.is_file(), 'Required vendor SCP firmware is missing')
        self.assertGreater(firmware.stat().st_size, 0)
        shutil.copy2(firmware, self.out / 'scp.fex')
        result = self.invoke()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        package = self.pack()
        self.assertEqual(package.returncode, 0, package.stdout + package.stderr)
        self.assertIn(firmware.read_bytes(), (self.out / 'boot_package.fex').read_bytes())

    def test_missing_scp_is_actionable_and_does_not_use_old_backup(self):
        backup = self.root / 'vendor/allwinner-a333/pack-sdk/device/config/chips/a333/bin/scp.bin.before-new-vendor-sdk'
        backup.parent.mkdir(parents=True)
        backup.write_bytes(b'old firmware')
        result = self.invoke()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('missing SCP firmware', result.stderr)
        self.assertIn('A333_FACTORY_IMAGE', result.stderr)
        self.assertFalse((self.out / 'scp.fex').exists())
        self.assertIn('\nitem=scp,', (self.out / 'boot_package.cfg').read_text())

    def test_incomplete_pair_or_missing_factory_image_is_rejected(self):
        self.env['A333_FACTORY_SCP'] = str(self.scp)
        result = self.invoke()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('provide both', result.stderr)
        self.env['A333_FACTORY_IMAGE'] = str(self.root / 'missing.img')
        result = self.invoke()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('factory image not found', result.stderr)

    def test_changing_factory_image_refreshes_cached_firmware(self):
        self.env['A333_FACTORY_IMAGE'] = str(self.factory)
        self.assertEqual(self.invoke().returncode, 0)
        self.scp.write_bytes(b'SCP2' * 1024)
        self.boot0.write_bytes(b'new boot0' * 1024)
        self.make_factory_package()
        self.assertEqual(self.invoke().returncode, 0)
        self.assertEqual((self.out / 'scp.fex').read_bytes(), self.scp.read_bytes())
        self.assertEqual((self.work / 'factory-boot0.fex').read_bytes(), self.boot0.read_bytes())

    def test_invalid_factory_scp_descriptor_is_rejected(self):
        self.env['A333_FACTORY_IMAGE'] = str(self.factory)
        package = self.inputs / 'boot_package.fex'
        original = package.read_bytes()
        for broken in (b'no scp descriptor', original[:800]):
            package.write_bytes(broken)
            result = self.invoke()
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('no valid scp descriptor', result.stderr)
        broken = bytearray(original)
        descriptor = broken.find(b'IIE;scp\0')
        struct.pack_into('<II', broken, descriptor + 0x44, len(original), 4096)
        package.write_bytes(broken)
        result = self.invoke()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('invalid factory scp offset/size', result.stderr)
        self.assertFalse((self.out / 'scp.fex').exists())
        self.assertFalse(list(self.work.glob('.factory-boot.*')))

    def test_vendor_calls_hook_before_do_common_and_stops_on_failure(self):
        # Execute the actual vendor main() section with lightweight stubs.
        source = (ROOT / 'vendor/allwinner-a333/pack-sdk/build/pack').read_text()
        section = source.split('if [ "$output_resources" == "false" ] && [ -z "$from_resources" ]; then', 1)[1]
        section = section.split('\n\t\tdo_pack_${PACK_TYPE}', 1)[0]
        command = '''
do_prepare() { :; }
do_ini_to_dts() { :; }
LOGF() { echo "$*" >&2; exit 1; }
do_common() { test -s "$LICHEE_PACK_OUT_DIR/scp.fex" && touch "$LICHEE_PACK_OUT_DIR/packed"; }
''' + section
        self.env.update(A333_PACK_PRE_COMMON_HOOK=str(HOOK), LICHEE_PACK_OUT_DIR=str(self.out))
        result = subprocess.run(['bash', '-c', command], env=self.env,
                                capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertFalse((self.out / 'packed').exists())
        self.env['A333_FACTORY_IMAGE'] = str(self.factory)
        result = subprocess.run(['bash', '-c', command], env=self.env,
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue((self.out / 'packed').exists())


class FirmwareGitVisibilityTest(unittest.TestCase):
    @unittest.skipUnless((ROOT / '.git').exists() and shutil.which('git'),
                         'Git checkout is required to check ignore rules')
    def test_scp_and_its_ignore_rules_can_be_committed(self):
        for directory in ('vendor/allwinner-a333/device/a333',
                          'vendor/allwinner-a333/pack-sdk/device/config/chips/a333'):
            for name in ('bin/scp.bin', '.gitignore'):
                path = directory + '/' + name
                with self.subTest(path=path):
                    result = subprocess.run(
                        ['git', '-C', str(ROOT), 'check-ignore', '--no-index', '--quiet', path],
                        capture_output=True, text=True, timeout=15)
                    self.assertEqual(result.returncode, 1,
                                     f'{path} is ignored or could not be checked: {result.stderr}')


if __name__ == '__main__':
    unittest.main()
