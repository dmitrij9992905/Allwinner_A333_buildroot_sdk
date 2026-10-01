"""Shared source ownership, CMake configuration and editor include regressions."""
import json
import pathlib
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
SRC = ROOT / "oem/a333/src"
COMMON = SRC / "panel-common"


class PanelCommonTest(unittest.TestCase):
    def test_common_headers_do_not_include_an_application(self):
        for path in [*COMMON.glob("include/*.h"), *COMMON.glob("src/*.c")]:
            for name in re.findall(r'^#include "([^"]+)"', path.read_text(), re.M):
                with self.subTest(file=path.name, include=name):
                    self.assertTrue((COMMON / "include" / name).is_file() or
                                    (COMMON / "vendor/lvgl" / name).is_file())
        self.assertNotIn("lvgl_ui.h", (COMMON / "include/lvgl_fonts.h").read_text())

    def test_shared_files_have_one_source_owner(self):
        for name in ("panel_canvas", "panel_fbdev", "panel_g2d", "panel_input",
                     "lvgl_port", "lvgl_fonts"):
            self.assertTrue((COMMON / "src" / f"{name}.c").is_file())
            self.assertTrue((COMMON / "include" / f"{name}.h").is_file())
            self.assertFalse((SRC / "dmx-panel/src" / f"{name}.c").exists())
            self.assertFalse((SRC / "dmx-panel/include" / f"{name}.h").exists())
        self.assertFalse((SRC / "dmx-panel/vendor").exists())
        self.assertFalse((SRC / "dmx-panel/assets").exists())

    def test_vscode_fallback_covers_project_headers(self):
        settings = json.loads((ROOT / ".vscode/settings.json").read_text())
        self.assertEqual(settings["files.associations"]["*.h"], "c")
        configs = json.loads((ROOT / ".vscode/c_cpp_properties.json").read_text())
        apps = [c for c in configs["configurations"] if not c["name"].startswith("kernel-")]
        self.assertEqual({c["name"] for c in apps}, {
            "media", "media-kernel-debug-logs", "dmx", "dmx-kernel-debug-logs"})
        for config in apps:
            self.assertEqual(config["intelliSenseMode"], "linux-gcc-arm64")
            self.assertIn("LV_CONF_INCLUDE_SIMPLE=1", config["defines"])
            for relative in ("panel-common/include", "panel-common/vendor/lvgl",
                             "panel-common/vendor", "dmx-panel/include"):
                self.assertIn("${workspaceFolder}/oem/a333/src/" + relative,
                              config["includePath"])
            self.assertIn("--sysroot=${workspaceFolder}/output/profiles/" +
                          config["name"] + "/host/aarch64-buildroot-linux-gnu/sysroot",
                          config["compilerArgs"])
            self.assertEqual(config["includePath"][0], "${workspaceFolder}/output/dev/kernel/" +
                             config["name"] + "/headers/include")

    def test_kernel_editor_uses_sdk_kbuild_and_no_host_headers(self):
        configs = json.loads((ROOT / ".vscode/c_cpp_properties.json").read_text())
        kernels = [c for c in configs["configurations"] if c["name"].startswith("kernel-")]
        self.assertEqual(len(kernels), 6)
        for config in kernels:
            self.assertIn("-nostdinc", config["compilerArgs"])
            self.assertIn("__KERNEL__", config["defines"])
            self.assertTrue(config["dotConfig"].endswith("/build/linux-custom/.config"))
            self.assertTrue(config["compileCommands"].endswith("/compile_commands.host.json"))
            self.assertEqual(len(config["forcedInclude"]), 3)
            for path in config["includePath"]:
                self.assertIn("/build/linux-custom/", path)
                self.assertNotIn("/usr/include", path)

    @unittest.skipUnless((ROOT / "output/dev/kernel/media/compile_commands.host.json").is_file(),
                         "kernel editor database is not generated")
    def test_kernel_database_preprocesses_with_sdk_only(self):
        entries = json.loads((ROOT / "output/dev/kernel/media/compile_commands.host.json").read_text())
        entry = next(e for e in entries if e["file"].endswith("/init/main.c"))
        arguments = entry["arguments"]
        # Retain the real architecture/configuration flags, but only preprocess.
        probe = []
        index = 0
        while index < len(arguments):
            arg = arguments[index]
            if arg == "-o":
                index += 2
                continue
            if arg != "-c" and not arg.startswith("-Wp,-MMD,"):
                probe.append(arg)
            index += 1
        self.assertIn("-nostdinc", probe)
        self.assertNotIn("/workspace/", " ".join(probe))
        result = subprocess.run(probe + ["-E", "-H", "-o", "/dev/null"],
                                cwd=entry["directory"], capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("./include/linux/", result.stderr)
        self.assertNotIn("/usr/include/", result.stderr)

    def test_perf_monitor_sources_and_config_are_shared(self):
        config = (COMMON / "include/lv_conf.h").read_text()
        self.assertIn("#define LV_COLOR_DEPTH 32", config)
        self.assertIn("#define PANEL_LVGL_PERF_MONITOR 1", config)
        for name in ("LV_USE_SYSMON", "LV_USE_PERF_MONITOR", "LV_USE_OBSERVER"):
            self.assertIn(f"#define {name} PANEL_LVGL_PERF_MONITOR", config)
        self.assertIn("#define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT", config)
        for path in (COMMON / "CMakeLists.txt", COMMON / "common.mk"):
            self.assertIn("debugging/sysmon", path.read_text())

    @unittest.skipUnless((ROOT / "output/profiles/media/host/bin/aarch64-buildroot-linux-gnu-gcc").exists(),
                         "media SDK is not built")
    def test_editor_headers_resolve_with_host_sdk(self):
        configs = json.loads((ROOT / ".vscode/c_cpp_properties.json").read_text())
        config = next(c for c in configs["configurations"] if c["name"] == "media")
        resolve = lambda text: text.replace("${workspaceFolder}", str(ROOT))
        command = [resolve(config["compilerPath"]), "-std=c11", "-x", "c", "-fsyntax-only"]
        command += [resolve(value) for value in config["compilerArgs"]]
        command += ["-D" + value for value in config["defines"]]
        command += ["-I" + resolve(value) for value in config["includePath"]]
        result = subprocess.run(command + ["-"], input=(
            '#include <stdio.h>\n#include <linux/dma-heap.h>\n'
            '#include "lvgl_port.h"\n#include "lvgl_fonts.h"\n'
            '#include "panel_g2d.h"\n#include "lvgl_ui.h"\n'
            'int main(void) { return 0; }\n'),
            capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stderr)
        uapi = ROOT / "output/dev/kernel/media/headers/include/linux/dma-heap.h"
        if uapi.is_file():
            probe = subprocess.run(command[:-1] + ["-E", "-H", "-"],
                                   input='#include <linux/dma-heap.h>\n',
                                   capture_output=True, text=True, timeout=30)
            self.assertEqual(probe.returncode, 0, probe.stderr)
            self.assertIn(str(uapi), probe.stderr)
            self.assertNotIn("/usr/include/linux/", probe.stderr)

    @unittest.skipUnless(shutil.which("cmake"), "host CMake is unavailable")
    def test_cmake_cache_migration_and_shared_target_options(self):
        with tempfile.TemporaryDirectory(prefix="a333-common-cmake-") as folder:
            # Simulate the old dev cache default using the exact old spelling.
            command = ["cmake", "-S", str(SRC / "media-panel"), "-B", folder,
                       "-DMEDIA_PANEL_COMMON_DIR=" + str(SRC / "media-panel") + "/../dmx-panel"]
            first = subprocess.run(command, capture_output=True, text=True, timeout=60)
            self.assertEqual(first.returncode, 0, first.stdout + first.stderr)
            second = subprocess.run(command[:5] + [
                "-DMEDIA_PANEL_DISPLAY_ROTATION=0", "-DMEDIA_PANEL_G2D=OFF",
                "-DMEDIA_PANEL_LVGL_LOGGING=ON"],
                capture_output=True, text=True, timeout=60)
            self.assertEqual(second.returncode, 0, second.stdout + second.stderr)
            entries = json.loads((pathlib.Path(folder) / "compile_commands.json").read_text())
            files = {pathlib.Path(e["file"]).name: e for e in entries}
            for name in ("panel_fbdev.c", "panel_g2d.c", "lvgl_port.c", "lvgl_fonts.c"):
                entry = files[name]
                self.assertEqual(pathlib.Path(entry["file"]).parent, COMMON / "src")
                self.assertIn("-DPANEL_ENABLE_G2D=0", entry["command"])
                self.assertIn("-DPANEL_DISPLAY_ROTATION=0", entry["command"])
                self.assertNotIn("dmx-panel/include", entry["command"])
            self.assertIn("panel-common/include", files["media_panel.c"]["command"])
            self.assertIn("#if 1", (pathlib.Path(folder) /
                                    "panel-common/generated/lv_conf.h").read_text())


if __name__ == "__main__":
    unittest.main()
