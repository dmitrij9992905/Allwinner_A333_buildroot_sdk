"""Credentials tests use regular temporary files only: no host /dev or mounts."""
import configparser
import os
from pathlib import Path
import re
import shlex
import shutil
import stat
import struct
import subprocess
import tempfile
import unittest
import zlib

ROOT = Path(__file__).resolve().parents[2]
BOARD = ROOT / "configs/boards/a333/helperboard-a333"
OVERLAY = ROOT / "overlays/a333/rootfs"
LIB = OVERLAY / "usr/libexec/a333-storage.sh"


class CredentialsIntegrationTest(unittest.TestCase):
    def test_layout_keeps_ab_numbers_and_credentials_before_userdata(self):
        text = (BOARD / "sys_partition-ab.fex").read_text()
        parts = text.split("[partition]\n")[1:]
        names = [re.search(r"name\s*=\s*(\S+)", p)[1] for p in parts]
        self.assertEqual(names, ["boot-resource", "env", "env-redund", "bootA", "bootB",
                                 "rootfsA", "rootfsB", "oemA", "oemB", "credentials", "userdata"])
        self.assertRegex(parts[9], r"size\s*=\s*32768\b")
        self.assertIn('downloadfile = "credentials.fex"', parts[9])
        self.assertRegex(parts[9], r"keydata\s*=\s*1\b")
        for env in BOARD.glob("env-ab*.cfg"):
            self.assertNotIn("userdata=/dev/mmcblk0p10", env.read_text())

    def test_all_profiles_share_mount_and_image_hooks(self):
        profiles = sorted((ROOT / "profiles").glob("*.conf"))
        self.assertEqual(len(profiles), 6)
        for profile in profiles:
            result = subprocess.run(["bash", "-c",
                'source "$1"; printf "%s\\n" "$DEFCONFIG" "${ROOTFS_OVERLAYS[@]}"',
                "sh", str(profile)], text=True, capture_output=True, check=True)
            lines = result.stdout.splitlines()
            self.assertIn("overlays/a333/rootfs", lines)
            config = (ROOT / "configs/configs" / lines[0]).read_text()
            self.assertIn('BR2_ROOTFS_POST_BUILD_SCRIPT="../configs/boards/a333/helperboard-a333/post-build.sh"', config)
            self.assertIn('BR2_ROOTFS_POST_IMAGE_SCRIPT="../configs/boards/a333/helperboard-a333/post-image.sh"', config)
        self.assertTrue((OVERLAY / "credentials/.keep").exists())
        self.assertIn('"$BINARIES_DIR/credentials.ext4"', (BOARD / "post-image.sh").read_text())
        self.assertIn("copy_payload credentials.ext4 credentials.fex", (BOARD / "pack-pre-finish.sh").read_text())
        pack_config = ROOT / "vendor/allwinner-a333/pack-sdk/device/config/chips/a333/configs/pro/dragonboard/image.cfg"
        self.assertIn('filename = "credentials.fex"', pack_config.read_text())
        self.assertIn("a333-credentials.service", (BOARD / "post-build.sh").read_text())

    def test_not_an_ota_slot_or_reset_target(self):
        config = configparser.ConfigParser()
        config.read(BOARD / "rauc/system.conf")
        self.assertFalse(any("credentials" in s for s in config.sections()))
        bundle = (ROOT / "scripts/make-a333-rauc-bundle.sh").read_text()
        self.assertNotIn("credentials.ext4", bundle)
        reset = (OVERLAY / "usr/sbin/a333-factory-reset").read_text()
        self.assertIn("a333_partition_device userdata", reset)
        self.assertNotIn("mmcblk0p10", reset)
        self.assertNotIn("stop || true", reset)

    def test_all_profiles_keep_vendor_udisk_distinct_from_userdata(self):
        fragment = BOARD / "uboot-ab.config"
        self.assertIn('CONFIG_LAST_PARTITION_NAME="UDISK"', fragment.read_text())
        for config in (ROOT / "configs/configs").glob("a333*_defconfig"):
            self.assertIn('BR2_TARGET_UBOOT_CONFIG_FRAGMENT_FILES="../configs/boards/a333/helperboard-a333/uboot-ab.config"',
                          config.read_text(), config.name)

    def test_uboot_merge_overrides_vendor_userdata_default(self):
        # Exercise the actual vendor merge script, without building U-Boot or
        # changing the SDK output/source directories.
        source = ROOT / "vendor/allwinner-a333/boot/u-boot-2018"
        with tempfile.TemporaryDirectory(prefix="a333-uboot-credentials-test-") as directory:
            subprocess.run(["sh", str(source / "scripts/kconfig/merge_config.sh"),
                            "-m", "-O", directory,
                            str(source / "configs/sun65iw1p1_a333_defconfig"),
                            str(BOARD / "uboot-ab.config")],
                           cwd=directory, check=True, capture_output=True, text=True)
            config = (Path(directory) / ".config").read_text()
            entries = [line for line in config.splitlines() if line.startswith("CONFIG_LAST_PARTITION_NAME=")]
            self.assertEqual(entries, ['CONFIG_LAST_PARTITION_NAME="UDISK"'])

    def test_provisioning_requires_confirmation_and_checks_readback(self):
        script = (OVERLAY / "usr/sbin/a333-provision-credentials").read_text()
        self.assertIn('"$1" != --confirm', script)
        self.assertIn("a333_partition_device credentials", script)
        self.assertIn('e2fsck -fn "$image"', script)
        self.assertLess(script.index('a333-mount-credentials stop'), script.index('dd if="$image"'))
        self.assertLess(script.index('"$actual" = "$expected"'), script.index('systemctl restart a333-credentials.service'))
        self.assertNotIn("mkfs", script)


@unittest.skipUnless(all(shutil.which(t) for t in ("fakeroot", "mke2fs", "debugfs", "blkid", "e2fsck")),
                     "host image-generation tools unavailable")
class CredentialsImageTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="a333-credentials-image-test-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.image = self.root / "credentials.ext4"

    def build(self, source=None):
        args = ["sh", str(ROOT / "scripts/make-a333-credentials-image.sh"), str(self.image)]
        if source is not None:
            args.append(str(source))
        env = dict(os.environ)
        env.pop("HOST_DIR", None)
        env.pop("A333_CREDENTIALS_ROOT", None)
        return subprocess.run(args, capture_output=True, text=True, env=env, timeout=30)

    def debug(self, command):
        return subprocess.check_output(["debugfs", "-R", command, str(self.image)],
                                       text=True, stderr=subprocess.DEVNULL)

    def test_default_payload_has_no_identity(self):
        result = self.build()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(self.image.stat().st_size, 16 * 1024 * 1024)
        self.assertEqual(stat.S_IMODE(self.image.stat().st_mode), 0o600)
        self.assertIn("UNPROVISIONED DEVICE", self.debug("cat /README"))
        self.assertNotIn("device.json", self.debug("ls /"))
        features = self.debug("stats").split("Filesystem features:", 1)[1].splitlines()[0]
        self.assertNotIn("has_journal", features)
        result = subprocess.run(["e2fsck", "-fn", str(self.image)], capture_output=True)
        self.assertEqual(result.returncode, 0)

    def test_per_device_payload_is_root_owned_without_changing_source(self):
        source = self.root / "factory"
        (source / "cloud").mkdir(parents=True)
        key = source / "cloud/client.key.pem"
        key.write_text("test-only private-key placeholder\n")
        key.chmod(0o640)
        (source / "device.json").write_text('{"device_id":"test-001"}\n')
        before = key.stat()
        result = self.build(source)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("test-001", self.debug("cat /device.json"))
        for path, mode in (("/", "0700"), ("/cloud", "0700"), ("/cloud/client.key.pem", "0600")):
            entry = self.debug("stat " + path)
            self.assertRegex(entry, r"Mode:\s*" + mode)
            self.assertRegex(entry, r"User:\s*0\s+Group:\s*0")
        self.assertEqual(key.stat().st_uid, before.st_uid)
        self.assertEqual(key.stat().st_mode, before.st_mode)
        self.assertEqual(key.read_text(), "test-only private-key placeholder\n")

    def test_symlink_payload_rejected_without_replacing_output(self):
        source = self.root / "factory"
        source.mkdir()
        (source / "escape").symlink_to(self.root / "outside")
        self.image.write_bytes(b"preserve old output")
        result = self.build(source)
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(self.image.read_bytes(), b"preserve old output")


@unittest.skipUnless(shutil.which("sgdisk"), "host GPT tool unavailable")
class UserdataPartitionSafetyTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="a333-gpt-test-")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.disk = self.root / "flash"
        with self.disk.open("wb") as image:
            image.truncate(64 * 1024 * 1024)
        self.bin = self.root / "bin"
        self.bin.mkdir()
        # Only blockdev is mocked, and only for our regular-file image.
        blockdev = self.bin / "blockdev"
        blockdev.write_text('#!/bin/sh\ncase "$1" in --getsz) bytes=$(stat -c %s "$2"); echo $((bytes / 512)) ;; --rereadpt) exit 0 ;; *) exit 1 ;; esac\n')
        blockdev.chmod(0o755)

    def gpt(self, *args):
        return subprocess.run(["sgdisk", *args, str(self.disk)], text=True,
                              capture_output=True, check=True).stdout

    def create(self, userdata=11, tail=None):
        args = ["--clear"]
        if userdata == 11:
            args += ["--new=10:2048:34815", "--change-name=10:credentials"]
            start = 34816
        else:
            start = 2048
        args += [f"--new={userdata}:{start}:{start + 32767}", f"--change-name={userdata}:userdata"]
        if tail:
            args += [f"--new={userdata+1}:{start+32768}:0", f"--change-name={userdata+1}:{tail}"]
        self.gpt(*args)
        (self.root / ("flash" + str(userdata))).touch()

    def expand(self, override=None):
        source = (OVERLAY / "usr/sbin/a333-expand-userdata").read_text()
        source = source.replace(". /usr/libexec/a333-storage.sh", ". " + shlex.quote(str(LIB)))
        source = source.replace('PATH=/sbin:/bin:/usr/sbin:/usr/bin',
                                'PATH=' + shlex.quote(str(self.bin) + ":/usr/sbin:/usr/bin:/sbin:/bin"))
        # Production rejects regular files. This test-only substitution lets
        # real sgdisk exercise a disposable image, never a host block device.
        source = source.replace('[ -b "$disk" ] && [ -b "$part" ]', '[ -f "$disk" ] && [ -f "$part" ]')
        env = {**os.environ, "A333_USERDATA_DISK": str(self.disk)}
        env.pop("A333_USERDATA_PARTITION", None)
        if override:
            env["A333_USERDATA_PARTITION"] = str(override)
        return subprocess.run(["sh", "-s"], input=source, text=True,
                              capture_output=True, env=env, timeout=15)

    def test_new_layout_preserves_credentials_and_userdata_guid(self):
        self.create()
        credentials = self.gpt("--info=10")
        guid = re.search(r"Partition unique GUID: (.*)", self.gpt("--info=11"))[1]
        result = self.expand()
        self.assertEqual(result.returncode, 10, result.stderr)
        self.assertEqual(self.gpt("--info=10"), credentials)
        self.assertIn(guid, self.gpt("--info=11"))
        self.assertEqual(self.expand().returncode, 0)

    def test_old_layout_removes_only_named_udisk(self):
        self.create(userdata=10, tail="UDISK")
        result = self.expand()
        self.assertEqual(result.returncode, 10, result.stderr)
        table = self.gpt("--print")
        self.assertIn("userdata", table)
        self.assertNotIn("UDISK", table)

    def test_credentials_or_unknown_tail_refuses_all_mutations(self):
        for tail in ("credentials", "future-data"):
            with self.subTest(tail=tail):
                self.create(userdata=10, tail=tail)
                before = self.disk.read_bytes()
                result = self.expand()
                self.assertNotEqual(result.returncode, 0)
                self.assertNotEqual(result.returncode, 10)
                self.assertIn("preserving all data", result.stderr)
                self.assertEqual(self.disk.read_bytes(), before)

    def test_stale_p10_override_refused(self):
        self.create()
        before = self.disk.read_bytes()
        result = self.expand(self.root / "flash10")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("override is not named userdata", result.stderr)
        self.assertEqual(self.disk.read_bytes(), before)

    def test_duplicate_labels_fail_closed(self):
        self.create()
        self.gpt("--change-name=10:userdata")
        before = self.disk.read_bytes()
        result = self.expand()
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(self.disk.read_bytes(), before)

    def test_vendor_duplicate_tail_recovery_keeps_userdata_and_credentials(self):
        self.create(tail="userdata")
        self.assertNotEqual(self.expand().returncode, 0)
        credentials = self.gpt("--info=10")
        first = self.gpt("--info=11")
        tail = self.gpt("--info=12")
        marker = b"existing userdata must not be reformatted"
        offset = 34816 * 512 + 4096
        with self.disk.open("r+b") as image:
            image.seek(offset)
            image.write(marker)
        # The approved recovery only changes the tail's GPT name, not its
        # geometry, GUID or bytes. Full-tail zero verification happens on CU.
        self.gpt("--change-name=12:UDISK")
        self.assertEqual(self.gpt("--info=11"), first)
        self.assertEqual(self.gpt("--info=12"), tail.replace("'userdata'", "'UDISK'"))
        result = self.expand()
        self.assertEqual(result.returncode, 10, result.stderr)
        self.assertEqual(self.gpt("--info=10"), credentials)
        guid = re.search(r"Partition unique GUID: (.*)", first)[1]
        self.assertIn(guid, self.gpt("--info=11"))
        with self.disk.open("rb") as image:
            image.seek(offset)
            self.assertEqual(image.read(len(marker)), marker)
        self.assertNotIn("UDISK", self.gpt("--print"))

    def test_vendor_asymmetric_gpt_preserves_usable_bounds(self):
        # Allwinner keeps 36 MiB before the first partition, but reserves only
        # 33 sectors at the end. sgdisk -e assumes symmetric reservations and
        # wrongly moves last_usable to disk_size - 73728, making expansion fail.
        total = 262144  # 128 MiB, regular sparse test image only.
        with self.disk.open("r+b") as image:
            image.truncate(total * 512)
        self.gpt("--clear", "--resize-table=12",
                 "--new=10:73728:106495", "--change-name=10:credentials",
                 "--new=11:106496:139263", "--change-name=11:userdata",
                 f"--new=12:139264:{total-34}", "--change-name=12:UDISK")
        # Set the vendor-style usable range in both headers and recalculate
        # only their CRCs; the actual table and backup remain valid.
        with self.disk.open("r+b") as image:
            for offset in (512, (total - 1) * 512):
                image.seek(offset)
                header = bytearray(image.read(512))
                self.assertEqual(header[:8], b"EFI PART")
                size = struct.unpack_from("<I", header, 12)[0]
                struct.pack_into("<QQ", header, 40, 73728, total - 34)
                struct.pack_into("<I", header, 16, 0)
                struct.pack_into("<I", header, 16, zlib.crc32(header[:size]) & 0xffffffff)
                image.seek(offset)
                image.write(header)
            image.seek(106496 * 512 + 4096)
            image.write(b"preserve userdata bytes")
        (self.root / "flash11").touch()
        credentials = self.gpt("--info=10")
        userdata = self.gpt("--info=11")
        guid = re.search(r"Partition unique GUID: (.*)", userdata)[1]
        result = self.expand()
        self.assertEqual(result.returncode, 10, result.stdout + result.stderr)
        table = self.gpt("--print")
        self.assertIn(f"First usable sector is 73728, last usable sector is {total-34}", table)
        self.assertIn("Partition table holds up to 12 entries", table)
        self.assertEqual(self.gpt("--info=10"), credentials)
        self.assertIn(guid, self.gpt("--info=11"))
        self.assertIn(f"Last sector: {total-34}", self.gpt("--info=11"))
        self.assertNotIn("UDISK", table)
        with self.disk.open("rb") as image:
            image.seek(106496 * 512 + 4096)
            self.assertEqual(image.read(len(b"preserve userdata bytes")), b"preserve userdata bytes")
        self.assertEqual(self.expand().returncode, 0)


class CredentialsMountTest(unittest.TestCase):
    def run_mount(self, filesystem="ext4", existing="", missing=False):
        with tempfile.TemporaryDirectory(prefix="a333-credentials-mount-test-") as directory:
            root = Path(directory)
            log = root / "mount.log"
            commands = {
                "blkid": 'printf "%s\\n" "$TEST_FS"',
                "mountpoint": '[ -n "$TEST_EXISTING" ]',
                "findmnt": 'printf "%s\\n" "$TEST_EXISTING"',
                "mount": 'printf "%s\\n" "$*" >> "$TEST_LOG"',
            }
            for name, body in commands.items():
                path = root / name
                path.write_text("#!/bin/sh\n" + body + "\n")
                path.chmod(0o755)
            source = (OVERLAY / "usr/sbin/a333-mount-credentials").read_text()
            source = source.replace(". /usr/libexec/a333-storage.sh",
                'a333_partition_device() { ' + ('return 1' if missing else 'echo /mock/credentials') + '; }')
            source = source.replace("PATH=/sbin:/bin:/usr/sbin:/usr/bin", "PATH=" + shlex.quote(directory + ":/usr/bin:/bin"))
            result = subprocess.run(["sh", "-s"], input=source, text=True, capture_output=True,
                env={**os.environ, "TEST_FS": filesystem, "TEST_EXISTING": existing, "TEST_LOG": str(log)})
            return result, log.read_text() if log.exists() else ""

    def test_mount_is_read_only_and_cannot_replay_journal(self):
        result, log = self.run_mount()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("ro,noload,nodev,nosuid,noexec,noatime", log)
        self.assertIn("/mock/credentials /credentials", log)

    def test_missing_or_foreign_partition_is_never_mounted(self):
        for kwargs in ({"missing": True}, {"filesystem": "vfat"}):
            result, log = self.run_mount(**kwargs)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(log, "")

    def test_existing_writable_mount_is_rejected(self):
        result, log = self.run_mount(existing="rw,nodev")
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(log, "")


class FactoryProvisioningTest(unittest.TestCase):
    def run_provision(self, size=16777216, fsck=0, busy=0, mismatch=0, confirm=True):
        with tempfile.TemporaryDirectory(prefix="a333-factory-mock-") as directory:
            root = Path(directory)
            image = root / "credentials.ext4"
            with image.open("wb") as stream:
                stream.truncate(size)
            log = root / "commands.log"
            commands = {
                "id": "echo 0",
                "blkid": 'case "$*" in *TYPE*) echo ext4 ;; *LABEL*) echo credentials ;; esac',
                "e2fsck": 'exit "$TEST_FSCK"',
                "blockdev": "echo 16777216",
                "sha256sum": 'if [ "$#" -eq 0 ] && [ "$TEST_MISMATCH" -eq 1 ]; then echo bad; else echo good; fi',
                # No real dd is used, even on the happy path.
                "dd": 'printf "dd %s\\n" "$*" >> "$TEST_LOG"',
                "systemctl": 'printf "systemctl %s\\n" "$*" >> "$TEST_LOG"',
                "mount-credentials": 'printf "mount %s\\n" "$*" >> "$TEST_LOG"; exit "$TEST_BUSY"',
            }
            for name, body in commands.items():
                path = root / name
                path.write_text("#!/bin/sh\n" + body + "\n")
                path.chmod(0o755)
            source = (OVERLAY / "usr/sbin/a333-provision-credentials").read_text()
            source = source.replace(". /usr/libexec/a333-storage.sh",
                                    'a333_partition_device() { echo /mock/credentials; }')
            source = source.replace("PATH=/sbin:/bin:/usr/sbin:/usr/bin", "PATH=" + shlex.quote(directory + ":/usr/bin:/bin"))
            source = source.replace("/usr/sbin/a333-mount-credentials", shlex.quote(str(root / "mount-credentials")))
            args = ["sh", "-s", "--", "--confirm" if confirm else "--wrong", str(image)]
            result = subprocess.run(args, input=source, capture_output=True, text=True,
                env={**os.environ, "TEST_FSCK": str(fsck), "TEST_BUSY": str(busy),
                     "TEST_MISMATCH": str(mismatch), "TEST_LOG": str(log)})
            return result, log.read_text() if log.exists() else ""

    def test_valid_payload_verified_before_service_restart(self):
        result, log = self.run_provision()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertLess(log.index("mount stop"), log.index("dd "))
        self.assertIn("of=/mock/credentials", log)
        self.assertIn("conv=fsync", log)
        self.assertIn("count=16", log)
        self.assertLess(log.index("count=16"), log.index("systemctl restart"))

    def test_invalid_image_or_busy_mount_never_writes(self):
        for kwargs in ({"size": 1024}, {"fsck": 4}, {"busy": 1}, {"confirm": False}):
            with self.subTest(kwargs=kwargs):
                result, log = self.run_provision(**kwargs)
                self.assertNotEqual(result.returncode, 0)
                self.assertNotIn("dd ", log)

    def test_readback_error_leaves_credentials_service_stopped(self):
        result, log = self.run_provision(mismatch=1)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("readback mismatch", result.stderr)
        self.assertNotIn("systemctl restart", log)


if __name__ == "__main__":
    unittest.main()
