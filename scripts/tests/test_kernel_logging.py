import os
import pathlib
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
ENV_DIR = ROOT / "configs/boards/a333/helperboard-a333"


def env_values(path):
    return dict(line.split("=", 1) for line in path.read_text().splitlines()
                if line and not line.startswith("#") and "=" in line)


def profile(name):
    script = 'source "$1"; printf "%s\\n" "$DEFCONFIG" "$OUTPUT_DIR" "$A333_RAUC_BUNDLE_NAME" "$KERNEL_LOGGING" "${ROOTFS_OVERLAYS[*]}" "${OEM_OVERLAYS[*]}"'
    result = subprocess.run(["bash", "-c", script, "test", str(ROOT / "profiles" / (name + ".conf"))],
                            text=True, capture_output=True, check=True)
    return result.stdout.splitlines()


class LoggingTest(unittest.TestCase):
    def test_profile_pairs(self):
        for product in ("dmx", "media", "headless"):
            with self.subTest(product=product):
                normal = profile(product)
                debug = profile(product + "-kernel-debug-logs")
                self.assertEqual(normal[3], "quiet")
                self.assertEqual(debug[3], "debug")
                self.assertEqual(normal[4:], debug[4:])
                self.assertNotEqual(normal[1:3], debug[1:3])
                a = env_values(ROOT / "configs/configs" / normal[0])
                b = env_values(ROOT / "configs/configs" / debug[0])
                for key in ("BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES",
                            "BR2_TARGET_UBOOT_DEFAULT_ENV_FILE", "BR2_A333_KERNEL_DEBUG_LOGS"):
                    a.pop(key, None)
                    b.pop(key, None)
                self.assertEqual(a, b)

    def test_boot_environment_only_changes_logging(self):
        normal = env_values(ENV_DIR / "env-ab.cfg")
        debug = env_values(ENV_DIR / "env-ab-kernel-debug-logs.cfg")
        self.assertEqual(normal["loglevel"], "5")
        for key in ("setargs_mmc", "setargs_nand", "setargs_nand_ubi"):
            for param in ("earlyprintk", "initcall_debug", "ignore_loglevel", "keep_bootcon"):
                self.assertNotIn(param, normal[key])
        self.assertIn("ignore_loglevel", debug["setargs_mmc"])
        self.assertEqual(debug["initcall_debug"], "1")
        for key in ("earlyprintk", "initcall_debug", "loglevel", "setargs_mmc", "setargs_nand", "setargs_nand_ubi"):
            normal.pop(key)
            debug.pop(key)
        self.assertEqual(normal, debug)

    def test_generated_policy_has_no_slot_or_partition_writes(self):
        allowed = {"earlyprintk", "initcall_debug", "loglevel", "setargs_mmc", "setargs_nand", "setargs_nand_ubi"}
        for mode in ("quiet", "debug"):
            result = subprocess.run(["sh", str(ROOT / "scripts/a333-kernel-logging-env.sh"), mode],
                                    text=True, capture_output=True, check=True)
            keys = {line.split(" ", 1)[0] for line in result.stdout.splitlines()}
            self.assertEqual(keys, allowed)

    def test_existing_build_configuration_is_migrated(self):
        with tempfile.TemporaryDirectory() as folder:
            root = pathlib.Path(folder)
            for directory in ("configs", "profiles", "overlays", "oem"):
                (root / directory).symlink_to(ROOT / directory, target_is_directory=True)
            shutil.copy2(ROOT / "build.sh", root / "build.sh")
            docker = root / "docker-build.sh"
            docker.write_text('#!/bin/sh\nprintf "%s\\n" "$@" >> "$LOG_PATH"\n')
            docker.chmod(0o755)
            out = root / "output/profiles/media"
            out.mkdir(parents=True)
            contents = (ROOT / "configs/configs/a333_media_defconfig").read_text().replace(
                " ../configs/boards/a333/linux-quiet-logs.fragment", "")
            (out / ".config").write_text(contents)
            linux = out / "build/linux-custom"
            linux.mkdir(parents=True)
            (linux / ".stamp_configured").touch()
            (linux / ".stamp_built").touch()
            (linux / "object.o").write_text("preserve compiled files")
            uboot = out / "build/uboot-custom"
            uboot.mkdir(parents=True)
            (uboot / ".stamp_built").touch()
            (uboot / "object.o").write_text("preserve compiled files")
            log = root / "calls.log"
            subprocess.run(["bash", str(root / "build.sh"), "media", "build"], check=True,
                           env={**os.environ, "LOG_PATH": str(log)})
            self.assertIn("linux-quiet-logs.fragment", (out / ".config").read_text())
            self.assertIn("# BR2_A333_KERNEL_DEBUG_LOGS is not set", (out / ".config").read_text())
            self.assertFalse((linux / ".stamp_built").exists())
            self.assertFalse((linux / ".stamp_configured").exists())
            self.assertTrue((linux / "object.o").exists())
            self.assertFalse((uboot / ".stamp_built").exists())
            self.assertTrue((uboot / "object.o").exists())
            self.assertIn("olddefconfig", log.read_text())


class LoggingHookTest(unittest.TestCase):
    def run_hook(self, *, mode="quiet", board="bootA", slot_class="oem"):
        with tempfile.TemporaryDirectory() as folder:
            root = pathlib.Path(folder)
            hook = root / "hook.sh"
            shutil.copy2(ROOT / "scripts/a333-rauc-kernel-logging-hook.sh", hook)
            policy = subprocess.check_output(["sh", str(ROOT / "scripts/a333-kernel-logging-env.sh"), mode])
            (root / "kernel-logging.env").write_bytes(policy)
            printenv = root / "fw_printenv"
            printenv.write_text('#!/bin/sh\ncase "$2" in systemA) echo "$TEST_BOARD";; systemB) echo bootB;; *) exit 1;; esac\n')
            printenv.chmod(0o755)
            setenv = root / "fw_setenv"
            setenv.write_text('#!/bin/sh\n[ "$1" = -s ] || exit 1\ncp "$2" "$TEST_CAPTURE"\n')
            setenv.chmod(0o755)
            capture = root / "capture.env"
            result = subprocess.run(["sh", str(hook), "slot-post-install"], text=True, capture_output=True,
                                    env={**os.environ, "PATH": str(root) + ":/usr/bin:/bin",
                                         "RAUC_SLOT_CLASS": slot_class, "TEST_BOARD": board,
                                         "TEST_CAPTURE": str(capture)})
            return result, capture.read_text() if capture.exists() else ""

    def test_quiet_and_debug_policy(self):
        for mode in ("quiet", "debug"):
            result, contents = self.run_hook(mode=mode)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn("loglevel " + ("5" if mode == "quiet" else "3"), contents)
            self.assertNotIn("systemAB_next ", contents)

    def test_invalid_environment_is_not_written(self):
        result, contents = self.run_hook(board="wrong")
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(contents, "")

    def test_wrong_slot_class_is_not_written(self):
        result, contents = self.run_hook(slot_class="rootfs")
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(contents, "")


if __name__ == "__main__":
    unittest.main()
