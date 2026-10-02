"""Persistent directory preparation in temporary folders; never mounts devices."""
import configparser
import os
import pathlib
import re
import shlex
import shutil
import stat
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
OVERLAY = ROOT / "overlays/a333/rootfs"
SCRIPT = OVERLAY / "usr/sbin/a333-mount-userdata"
UNITS = OVERLAY / "etc/systemd/system"
MACHINE_ID = "0123456789abcdef0123456789abcdef"


class UserdataDirectoriesTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="a333-userdata-dirs-")
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)
        self.userdata = self.root / "userdata"
        self.machine_id = self.root / "machine-id"
        self.machine_id.write_text(MACHINE_ID + "\n")
        self.bin = self.root / "bin"
        self.bin.mkdir()
        self.log = self.root / "chown.log"
        # Test modes with real chmod; record chown without host root/group changes.
        chown = self.bin / "chown"
        chown.write_text('#!/bin/sh\nprintf "%s\\n" "$*" >> "$TEST_LOG"\n')
        chown.chmod(0o755)

    def prepare(self):
        source = SCRIPT.read_text().split('case "${1:-start}" in', 1)[0]
        source = source.replace("/userdata", str(self.userdata))
        source = source.replace("/etc/machine-id", shlex.quote(str(self.machine_id)))
        source += '\nPATH=' + shlex.quote(str(self.bin) + ":/usr/bin:/bin")
        source += '\nexport PATH\nprepare_userdata_directories\n'
        return subprocess.run(["sh", "-s"], input=source, text=True,
                              capture_output=True, timeout=10,
                              env={**os.environ, "TEST_LOG": str(self.log)})

    def test_required_directories_and_permissions(self):
        result = self.prepare()
        self.assertEqual(result.returncode, 0, result.stderr)
        for directory in ("var/lib/systemd", "var/lib/mpd", "var/lib/rauc",
                          "var/cache", "var/log", "home", "media/music", "media/playlists"):
            self.assertTrue((self.userdata / directory).is_dir(), directory)
        for directory in ("root", "var/lib/NetworkManager/system-connections",
                          "var/lib/bluetooth", "var/lib/ssh", "var/lib/private",
                          "var/cache/private", "var/log/private"):
            self.assertEqual(stat.S_IMODE((self.userdata / directory).stat().st_mode),
                             0o700, directory)
        for directory in ("var/log/journal", "var/log/journal/" + MACHINE_ID):
            self.assertEqual(stat.S_IMODE((self.userdata / directory).stat().st_mode),
                             0o2755, directory)
        self.assertIn("root:systemd-journal", self.log.read_text())

    def test_every_boot_and_ota_preserve_existing_files(self):
        self.userdata.mkdir()
        marker = self.userdata / ".a333-userdata-initialized"
        marker.write_text("already initialised\n")
        result = self.prepare()
        self.assertEqual(result.returncode, 0, result.stderr)
        files = {"media/music/track.mp3": b"music",
                 "var/lib/NetworkManager/system-connections/tp-link.nmconnection": b"profile",
                 "var/lib/bluetooth/paired-device": b"pairing",
                 "var/log/journal/" + MACHINE_ID + "/system.journal": b"existing journal"}
        for name, contents in files.items():
            (self.userdata / name).write_bytes(contents)
        # Restore a removed required directory even on an initialised userdata.
        (self.userdata / "var/lib/ssh").rmdir()
        for _ in range(2):
            result = self.prepare()
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue((self.userdata / "var/lib/ssh").is_dir())
            self.assertEqual(marker.read_text(), "already initialised\n")
            for name, contents in files.items():
                self.assertEqual((self.userdata / name).read_bytes(), contents)

    def test_new_machine_id_keeps_old_journals(self):
        self.assertEqual(self.prepare().returncode, 0)
        old = self.userdata / "var/log/journal" / MACHINE_ID / "system.journal"
        old.write_bytes(b"history")
        self.machine_id.write_text("f" * 32 + "\n")
        self.assertEqual(self.prepare().returncode, 0)
        self.assertTrue((self.userdata / "var/log/journal" / ("f" * 32)).is_dir())
        self.assertEqual(old.read_bytes(), b"history")

    def test_invalid_machine_id_fails_without_path_traversal(self):
        for value in ("", "uninitialized", "../escaped", "a" * 31, "a" * 33):
            with self.subTest(value=value):
                self.machine_id.write_text(value + "\n")
                result = self.prepare()
                self.assertNotEqual(result.returncode, 0)
                self.assertIn("invalid machine ID", result.stderr)
                self.assertFalse((self.userdata / "var/log/escaped").exists())

    def test_preparation_unconditional_and_before_bind_mounts(self):
        source = SCRIPT.read_text().split("mount_userdata()", 1)[1]
        self.assertIn("populate_skeleton\n    fi\n    prepare_userdata_directories", source)
        self.assertLess(source.index("prepare_userdata_directories"),
                        source.index("bind_persistent /userdata/var/lib"))


class UserdataBootOrderTest(unittest.TestCase):
    @unittest.skipUnless(shutil.which("systemd-analyze") and
                         (ROOT / "output/profiles/media/target/usr/lib/systemd/system").is_dir(),
                         "host systemd-analyze or built SDK unit files unavailable")
    def test_real_systemd_boot_graph_has_no_ordering_cycle(self):
        # Verify the real SDK units plus new overlays in a disposable root.
        # Exec commands are inert placeholders: verify loads units, runs none.
        target = ROOT / "output/profiles/media/target"
        with tempfile.TemporaryDirectory(prefix="a333-boot-graph-") as directory:
            root = pathlib.Path(directory)
            for path in ("usr/lib/systemd/system", "etc/systemd/system"):
                shutil.copytree(target / path, root / path, symlinks=True)
            for path in UNITS.rglob("*"):
                destination = root / "etc/systemd/system" / path.relative_to(UNITS)
                if destination.is_symlink():
                    destination.unlink()
            shutil.copytree(UNITS, root / "etc/systemd/system", symlinks=True,
                            dirs_exist_ok=True)
            for name in ("passwd", "group"):
                shutil.copy2(target / "etc" / name, root / "etc" / name)
            for parent in ("multi-user.target.wants", "sysinit.target.wants"):
                link = root / "etc/systemd/system" / parent / "a333-userdata.service"
                if link.is_symlink():
                    link.unlink()
            wants = root / "etc/systemd/system/sysinit.target.wants"
            wants.mkdir(exist_ok=True)
            (wants / "a333-userdata.service").symlink_to("../a333-userdata.service")
            for path in root.rglob("*"):
                if path.is_symlink() or not path.is_file():
                    continue
                for line in path.read_text().splitlines():
                    if not re.match(r"Exec\w+=", line):
                        continue
                    command = re.sub(r"^[-+!:@|]*", "", line.split("=", 1)[1]).split()[0]
                    executable = root / command.lstrip("/") if command.startswith("/") else root / "usr/bin" / command
                    executable.parent.mkdir(parents=True, exist_ok=True)
                    executable.write_text("#!/bin/sh\nexit 0\n")
                    executable.chmod(0o755)
            result = subprocess.run([shutil.which("systemd-analyze"), "verify", "--man=no",
                                     "--generators=no", "--root=" + str(root),
                                     "a333-userdata.service", "systemd-journal-flush.service",
                                     "systemd-tmpfiles-setup.service", "systemd-timesyncd.service",
                                     "bluetooth.service", "NetworkManager.service"],
                                    text=True, capture_output=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertNotIn("ordering cycle", result.stderr.lower())

    def test_userdata_has_no_late_default_dependencies(self):
        unit = configparser.ConfigParser(interpolation=None)
        unit.read(UNITS / "a333-userdata.service")
        self.assertEqual(unit["Unit"]["DefaultDependencies"], "no")
        self.assertIn("local-fs.target", unit["Unit"]["After"].split())
        for dependent in ("sysinit.target", "systemd-journal-flush.service",
                          "systemd-tmpfiles-setup.service", "shutdown.target"):
            self.assertIn(dependent, unit["Unit"]["Before"].split())
        self.assertEqual(unit["Install"]["WantedBy"], "sysinit.target")
        self.assertEqual(unit["Service"]["TimeoutStartSec"], "0")

    def test_persistent_services_require_successful_userdata(self):
        for name in ("NetworkManager", "bluetooth", "systemd-timesyncd",
                     "systemd-pstore", "systemd-journal-flush", "systemd-tmpfiles-setup"):
            with self.subTest(service=name):
                unit = configparser.ConfigParser(interpolation=None)
                unit.read(UNITS / (name + ".service.d") / "a333-userdata.conf")
                self.assertIn("a333-userdata.service", unit["Unit"]["After"].split())
                self.assertIn("a333-userdata.service", unit["Unit"]["Requires"].split())
        # Do not block the early daemon on storage: early logs must work in /run.
        self.assertFalse((UNITS / "systemd-journald.service.d/a333-userdata.conf").exists())

    def test_post_build_enables_early_service_and_removes_old_link(self):
        source = (ROOT / "configs/boards/a333/helperboard-a333/post-build.sh").read_text()
        self.assertIn('rm -f "$TARGET_DIR/etc/systemd/system/multi-user.target.wants/a333-userdata.service"', source)
        self.assertIn('"$TARGET_DIR/etc/systemd/system/sysinit.target.wants/a333-userdata.service"', source)

    def test_log_rotation_is_bounded_and_persistent(self):
        config = configparser.ConfigParser(interpolation=None)
        config.read(OVERLAY / "etc/systemd/journald.conf.d/60-a333-storage.conf")
        expected = {"Storage": "persistent", "Compress": "yes", "SystemMaxUse": "64M",
                    "SystemKeepFree": "128M", "SystemMaxFileSize": "8M", "SystemMaxFiles": "16",
                    "RuntimeMaxUse": "8M", "RuntimeMaxFileSize": "2M", "RuntimeMaxFiles": "8",
                    "MaxFileSec": "1day", "MaxRetentionSec": "7day",
                    "RateLimitIntervalSec": "30s", "RateLimitBurst": "1000"}
        for key, value in expected.items():
            self.assertEqual(config["Journal"][key], value)


if __name__ == "__main__":
    unittest.main()
