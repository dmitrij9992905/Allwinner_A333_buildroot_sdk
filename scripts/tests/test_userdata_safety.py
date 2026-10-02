"""No device nodes, mounts, formatting or partition edits are used by these tests."""
import importlib.machinery
import importlib.util
import os
import pathlib
import shlex
import shutil
import subprocess
import tempfile
import unittest
from unittest import mock

ROOT = pathlib.Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "overlays/a333/rootfs/usr/sbin/a333-mount-userdata"
CHECKER = ROOT / "overlays/a333/rootfs/usr/libexec/a333-userdata-is-blank"
loader = importlib.machinery.SourceFileLoader("userdata_blank", str(CHECKER))
spec = importlib.util.spec_from_loader(loader.name, loader)
checker = importlib.util.module_from_spec(spec)
loader.exec_module(checker)


class UserdataSafetyTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="a333-userdata-test-")
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)
        self.device = self.root / "device.img"
        self.device.write_bytes(bytes(2 * 1024 * 1024 + 13))
        self.bin = self.root / "bin"
        self.bin.mkdir()
        self.log = self.root / "commands.log"
        commands = {
            "blkid": 'printf "%s" "$FS_TYPE"; exit "$PROBE_RC"',
            "e2fsck": 'printf "e2fsck %s\\n" "$*" >> "$TEST_LOG"; exit "$FSCK_RC"',
            "resize2fs": 'printf "resize2fs %s\\n" "$*" >> "$TEST_LOG"; exit "$RESIZE_RC"',
            # A recording mock only. It cannot format or modify the image.
            "mkfs.ext4": 'printf "mkfs %s\\n" "$*" >> "$TEST_LOG"; exit "$MKFS_RC"',
        }
        for name, body in commands.items():
            path = self.bin / name
            path.write_text("#!/bin/sh\n" + body + "\n")
            path.chmod(0o755)

    def prepare(self, fs="ext4", probe_rc=0, resize_rc=0, mkfs_rc=0, fsck_rc=0):
        source = SCRIPT.read_text().split('case "${1:-start}" in', 1)[0]
        source = source.replace("/usr/libexec/a333-userdata-is-blank", shlex.quote(str(CHECKER)))
        # Run only filesystem preparation, with mock tools and a regular image.
        source += '\nPATH=' + shlex.quote(str(self.bin) + ":/usr/bin:/bin")
        source += '\nexport PATH\nprepare_userdata_filesystem "$1"\n'
        result = subprocess.run(["sh", "-s", "-", str(self.device)], input=source,
                                text=True, capture_output=True, timeout=15,
                                env={**os.environ, "FS_TYPE": fs, "PROBE_RC": str(probe_rc),
                                     "RESIZE_RC": str(resize_rc), "MKFS_RC": str(mkfs_rc),
                                     "FSCK_RC": str(fsck_rc),
                                     "TEST_LOG": str(self.log)})
        return result, self.log.read_text() if self.log.exists() else ""

    def test_existing_ext4_preserved(self):
        self.device.write_bytes(b"music data")
        result, log = self.prepare()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotIn("mkfs", log)
        self.assertEqual(self.device.read_bytes(), b"music data")

    def test_existing_ext4_validation_error_never_formats(self):
        result, log = self.prepare(resize_rc=1)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("validation failed", result.stderr)
        self.assertNotIn("mkfs", log)

    def test_fsck_safe_repairs_continue_without_format(self):
        result, log = self.prepare(fsck_rc=1)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("errors corrected", result.stdout)
        self.assertIn("e2fsck -p", log)
        self.assertLess(log.index("e2fsck"), log.index("resize2fs"))
        self.assertNotIn("mkfs", log)

    def test_fsck_unresolved_errors_stop_without_resize_or_format(self):
        for rc in (2, 3, 4, 5, 8, 16, 32, 128, 127):
            with self.subTest(rc=rc):
                result, log = self.prepare(fsck_rc=rc)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("refusing mount, resize and format", result.stderr)
                self.assertNotIn("resize2fs", log)
                self.assertNotIn("mkfs", log)

    def test_foreign_filesystem_never_formats(self):
        result, log = self.prepare(fs="vfat")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("refusing to overwrite", result.stderr)
        self.assertEqual(log, "")

    def test_probe_errors_never_format(self):
        for rc in (1, 4, 8, 127):
            with self.subTest(rc=rc):
                result, log = self.prepare(fs="", probe_rc=rc)
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("probe failed", result.stderr)
                self.assertEqual(log, "")

    def test_unknown_nonzero_tail_preserved(self):
        with self.device.open("r+b") as image:
            image.seek(-1, os.SEEK_END)
            image.write(b"X")
        result, log = self.prepare(fs="", probe_rc=2)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("refusing to format", result.stderr)
        self.assertEqual(log, "")

    def test_only_full_zero_device_initialised(self):
        result, log = self.prepare(fs="", probe_rc=2)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(log.count("mkfs "), 1)
        self.assertIn("resize2fs -P", log)

    def test_format_failure_stops_initialisation(self):
        result, log = self.prepare(fs="", probe_rc=2, mkfs_rc=1)
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn("resize2fs", log)
        self.assertNotIn("e2fsck", log)

    def test_read_error_and_empty_image_fail_closed(self):
        self.device.write_bytes(b"")
        result, log = self.prepare(fs="", probe_rc=2)
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(log, "")
        with self.assertRaises(OSError):
            checker.is_blank(self.root / "missing.img")
        self.device.write_bytes(bytes(32))
        with mock.patch.object(checker.os, "read", side_effect=OSError("I/O error")):
            with self.assertRaises(OSError):
                checker.is_blank(self.device)
        with mock.patch.object(checker.os, "read", return_value=b""):
            with self.assertRaises(OSError):
                checker.is_blank(self.device)

    def test_safe_preparation_precedes_partition_expansion(self):
        source = SCRIPT.read_text().split("mount_userdata()", 1)[1]
        self.assertLess(source.index('prepare_userdata_filesystem "$device"'),
                        source.index("if command -v a333-expand-userdata"))
        self.assertNotIn('if ! resize2fs -P "$device" >/dev/null', source)
        self.assertIn('if ! resize2fs "$device"; then', source)
        self.assertIn("umount /userdata || true", source)

    @unittest.skipUnless(all(shutil.which(tool) for tool in
                             ("mkfs.ext4", "blkid", "e2fsck", "resize2fs", "debugfs")),
                         "host ext4 tools are unavailable")
    def test_real_ext4_repeated_checks_preserve_uuid_and_music(self):
        # All real tools operate ONLY on this temporary regular-file image.
        with self.device.open("wb") as image:
            image.truncate(32 * 1024 * 1024)
        subprocess.run([shutil.which("mkfs.ext4"), "-q", "-F", str(self.device)],
                       check=True, capture_output=True, timeout=30)
        track = self.root / "track.bin"
        track.write_bytes(b"userdata music must survive repeated boots\n")
        written = subprocess.run([shutil.which("debugfs"), "-w", "-R",
                                  f"write {track} /track.mp3", str(self.device)],
                                 check=True, capture_output=True, text=True, timeout=30)
        self.assertIn("Allocated inode", written.stdout)
        def uuid():
            return subprocess.check_output([shutil.which("blkid"), "-p", "-s", "UUID",
                                            "-o", "value", str(self.device)], text=True)
        before = uuid()
        # Keep a mock for mkfs so a regression cannot silently erase the image.
        for tool in ("blkid", "resize2fs", "e2fsck"):
            (self.bin / tool).unlink()
            (self.bin / tool).symlink_to(shutil.which(tool))
        for _ in range(2):
            result, log = self.prepare()
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertNotIn("mkfs", log)
            self.assertEqual(uuid(), before)
        contents = subprocess.check_output([shutil.which("debugfs"), "-R", "cat /track.mp3",
                                            str(self.device)], stderr=subprocess.DEVNULL)
        self.assertEqual(contents, track.read_bytes())


if __name__ == "__main__":
    unittest.main()
