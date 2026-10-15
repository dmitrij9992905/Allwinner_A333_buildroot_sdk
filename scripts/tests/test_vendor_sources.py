"""Exercise source archive preparation with small vendor trees and real tar/zstd."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


@unittest.skipUnless(shutil.which('zstd'), 'zstd is required')
class VendorSourcesTest(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory(prefix='a333-vendor-test-')
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        (self.root / 'scripts').mkdir()
        self.script = self.root / 'scripts/prepare-vendor-sources.sh'
        shutil.copy2(ROOT / 'scripts/prepare-vendor-sources.sh', self.script)
        self.vendor = self.root / 'vendor/allwinner-a333'
        self.kernel = self.vendor / 'kernel/linux-6.6'
        self.uboot = self.vendor / 'boot/u-boot-2018'
        self.bsp = self.vendor / 'bsp'
        for tree in (self.kernel, self.uboot, self.bsp):
            tree.mkdir(parents=True)
        (self.kernel / 'Makefile').write_text('kernel\n')
        (self.uboot / 'Makefile').write_text('uboot\n')
        (self.bsp / 'Kconfig').write_text('bsp\n')
        (self.bsp / 'include').mkdir()
        self.dl = self.root / 'dl'
        self.env = {**os.environ, 'DL_DIR': str(self.dl)}

    def prepare(self):
        return subprocess.run(['bash', str(self.script)], env=self.env,
                              text=True, capture_output=True, timeout=30)

    def extract(self, archive):
        destination = self.root / 'extracted'
        destination.mkdir(exist_ok=True)
        subprocess.run(['tar', '--zstd', '-xf', str(self.dl / archive),
                        '-C', str(destination)], check=True, capture_output=True)
        return destination

    def test_fresh_clone_without_bsp_link_or_generated_header(self):
        result = self.prepare()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        extracted = self.extract('linux-a333.tar.zst')
        self.assertEqual((extracted / 'linux-6.6/bsp/Kconfig').read_text(), 'bsp\n')
        header = extracted / 'linux-6.6/bsp/include/sunxi-autogen.h'
        self.assertIn('#define AW_BSP_VERSION "', header.read_text())
        self.assertFalse((self.bsp / 'include/sunxi-autogen.h').exists())
        self.assertFalse((self.kernel / 'bsp').exists())
        extracted = self.extract('u-boot-a333.tar.zst')
        self.assertEqual((extracted / 'u-boot-2018/Makefile').read_text(), 'uboot\n')
        self.assertFalse(list(self.root.glob('.allwinner-a333-archive.*')))
        self.assertFalse(list(self.dl.glob('*.tmp.*')))

    def test_sdk_bsp_link_and_existing_version_header_are_preserved(self):
        (self.kernel / 'bsp').symlink_to('../../bsp', target_is_directory=True)
        header = self.bsp / 'include/sunxi-autogen.h'
        header.write_text('#define AW_BSP_VERSION "original SDK"\n')
        result = self.prepare()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        extracted = self.extract('linux-a333.tar.zst')
        self.assertFalse((extracted / 'linux-6.6/bsp').is_symlink())
        self.assertEqual((extracted / 'linux-6.6/bsp/include/sunxi-autogen.h').read_text(),
                         header.read_text())
        self.assertTrue((self.kernel / 'bsp').is_symlink())

    def test_existing_archives_are_reused_with_large_listing(self):
        # A large listing exposes SIGPIPE from the former tar | grep -q check.
        for index in range(1200):
            (self.kernel / f'file-{index:04d}').write_text('source\n')
        first = self.prepare()
        self.assertEqual(first.returncode, 0, first.stdout + first.stderr)
        before = {p.name: p.stat().st_mtime_ns for p in self.dl.glob('*.tar.zst')}
        second = self.prepare()
        self.assertEqual(second.returncode, 0, second.stdout + second.stderr)
        self.assertEqual(before, {p.name: p.stat().st_mtime_ns for p in self.dl.glob('*.tar.zst')})
        self.assertNotIn('creating:', second.stdout)

    def test_stale_kernel_archive_is_rebuilt_with_bsp(self):
        self.dl.mkdir()
        subprocess.run(['tar', '--zstd', '-cf', str(self.dl / 'linux-a333.tar.zst'),
                        '-C', str(self.kernel.parent), 'linux-6.6'], check=True)
        result = self.prepare()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        extracted = self.extract('linux-a333.tar.zst')
        self.assertTrue((extracted / 'linux-6.6/bsp/include/sunxi-autogen.h').is_file())
        self.assertTrue((self.dl / 'linux-a333.tar.zst.stale-missing-bsp').exists())

    def test_failed_tar_does_not_leave_partial_archive_or_staging_tree(self):
        fake_bin = self.root / 'bin'
        fake_bin.mkdir()
        tar = fake_bin / 'tar'
        tar.write_text('#!/bin/sh\nexit 23\n')
        tar.chmod(0o755)
        self.env['PATH'] = str(fake_bin) + ':' + os.environ['PATH']
        result = self.prepare()
        self.assertEqual(result.returncode, 23, result.stdout + result.stderr)
        self.assertFalse((self.dl / 'linux-a333.tar.zst').exists())
        self.assertFalse((self.dl / 'u-boot-a333.tar.zst').exists())
        self.assertFalse(list(self.dl.glob('*.tmp.*')))
        self.assertFalse(list(self.root.glob('.allwinner-a333-archive.*')))


if __name__ == '__main__':
    unittest.main()
