import importlib.util
import json
import os
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("remap", ROOT / "scripts/remap-compile-commands.py")
remapping = importlib.util.module_from_spec(spec)
spec.loader.exec_module(remapping)


class CompilationDatabaseTest(unittest.TestCase):
    def test_docker_paths_and_spaces(self):
        entries = [{"directory": "/workspace/output/dev", "file": "/workspace/src/main.c",
                    "command": '/workspace/host/bin/gcc -I/workspace/include -DNAME=\\"value\\" -c /workspace/src/main.c'}]
        result = remapping.remap(entries, "/workspace", "/tmp/SDK with spaces")
        self.assertEqual(result[0]["arguments"][0], "/tmp/SDK with spaces/host/bin/gcc")
        self.assertIn("-I/tmp/SDK with spaces/include", result[0]["arguments"])
        self.assertIn('-DNAME="value"', result[0]["arguments"])
        self.assertNotIn("command", result[0])
        self.assertEqual(entries[0]["directory"], "/workspace/output/dev")

    def test_arguments_are_not_split(self):
        result = remapping.remap([{"directory": "/workspace", "file": "/workspace/a b.c",
                                  "arguments": ["gcc", "-DNAME=a b", "/workspace/a b.c"]}],
                                "/workspace", "/sdk")
        self.assertEqual(result[0]["arguments"], ["gcc", "-DNAME=a b", "/sdk/a b.c"])


class DevelopmentCommandsTest(unittest.TestCase):
    def run_command(self, action, profile="media", target="root@192.168.0.144", **environment):
        with tempfile.TemporaryDirectory() as folder:
            path = pathlib.Path(folder)
            log = path / "ssh.log"
            ssh = path / "ssh"
            ssh.write_text('#!/bin/sh\nprintf "%s\\n" "$@" >> "$SSH_CAPTURE"\n')
            ssh.chmod(0o755)
            result = subprocess.run(["bash", str(ROOT / "scripts/media-panel-dev.sh"),
                                     action, profile, target], capture_output=True, text=True,
                                    env={**os.environ, "PATH": str(path) + ":" + os.environ["PATH"],
                                         "SSH_CAPTURE": str(log), **environment})
            return result, log.read_text() if log.exists() else ""

    def test_run_uses_userdata_and_restores_ui(self):
        result, command = self.run_command("run")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("/userdata/dev/media-panel/media-panel", command)
        self.assertIn("trap cleanup EXIT", command)
        self.assertIn("systemctl start a333-media.service", command)
        self.assertIn("systemctl stop a333-media.service", command)
        self.assertNotIn("systemctl stop a333-media-backend", command)
        self.assertNotIn("/oem/usr/bin", command)

    def test_non_media_profile_rejected(self):
        result, command = self.run_command("run", profile="dmx")
        self.assertEqual(result.returncode, 2)
        self.assertEqual(command, "")

    def test_ssh_option_injection_rejected(self):
        result, command = self.run_command("logs", target="-oProxyCommand=bad")
        self.assertEqual(result.returncode, 2)
        self.assertEqual(command, "")

    def test_g2d_diagnostics_are_forwarded_to_target(self):
        result, command = self.run_command("run", A333_DEV_RENDERER="g2d", A333_DEV_PANEL_PROFILE="1")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("export A333_PANEL_RENDERER=g2d A333_PANEL_PROFILE=1;", command)

    def test_renderer_shell_injection_rejected(self):
        result, command = self.run_command("run", A333_DEV_RENDERER="auto;bad")
        self.assertEqual(result.returncode, 2)
        self.assertEqual(command, "")

    def test_invalid_profiling_rejected(self):
        result, command = self.run_command("run", A333_DEV_PANEL_PROFILE="2")
        self.assertEqual(result.returncode, 2)
        self.assertEqual(command, "")

    def test_task_cycle_selects_profile_once(self):
        tasks = json.loads((ROOT / ".vscode/tasks.json").read_text())["tasks"]
        run_task = next(task for task in tasks if task["label"].startswith("media-panel: run"))
        self.assertIn("cycle", run_task["args"])
        self.assertNotIn("dependsOn", run_task)


if __name__ == "__main__":
    unittest.main()
