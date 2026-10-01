"""Hardware-free G2D adapter/fbdev regressions against the local BSP ABI.

Run: python3 -m unittest discover -s scripts/tests -p test_panel_g2d.py -v
CC may select a host compiler (including compiler launcher arguments).
All seven device syscalls are linker-wrapped; no /dev device is accessed.
"""

import os
import pathlib
import re
import shlex
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
PANEL = ROOT / "oem/a333/src/panel-common"
BSP = ROOT / "vendor/allwinner-a333/bsp"
WRAPS = ("open", "close", "ioctl", "mmap", "munmap", "fcntl", "lseek")


class PanelG2DTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="panel-g2d-test-")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.compiler = shlex.split(os.environ.get("CC", "cc"))
        cls.flags = ["-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
                     "-I", str(PANEL / "include"),
                     "-isystem", str(BSP / "include/uapi")]
        cls.binaries = {}
        for enabled in (0, 1):
            binary = pathlib.Path(cls.temp.name) / f"panel-g2d-{enabled}"
            command = cls.compiler + cls.flags + [
                f"-DPANEL_ENABLE_G2D={enabled}",
                str(ROOT / "scripts/tests/panel_g2d_mock.c"),
                str(PANEL / "src/panel_g2d.c"),
                str(PANEL / "src/panel_fbdev.c"),
                *(f"-Wl,--wrap={name}" for name in WRAPS), "-o", str(binary)]
            result = subprocess.run(command, capture_output=True, text=True, timeout=60)
            if result.returncode:
                raise AssertionError(f"host compile failed:\n{result.stdout}{result.stderr}")
            cls.binaries[enabled] = binary

    def run_fixture(self, *arguments, enabled=1):
        env = {key: value for key, value in os.environ.items()
               if key not in ("A333_PANEL_RENDERER", "A333_PANEL_PROFILE")}
        result = subprocess.run([str(self.binaries[enabled]), *map(str, arguments)],
                                capture_output=True, text=True, timeout=10, env=env)
        self.assertEqual(result.returncode, 0,
                         f"fixture {arguments}:\n{result.stdout}{result.stderr}")
        self.assertIn("PASS", result.stdout)

    def test_bsp_uapi_sizes_offsets_and_constants(self):
        # Fixture static assertions compare every image/blit/rect field directly
        # to sunxi-g2d.h, included with stdbool.h and stdint.h on the host.
        self.run_fixture("abi")

    def test_framebuffer_export_abi_matches_bsp_definition(self):
        source = (BSP / "drivers/video/sunxi/disp2/disp/dev_fb.c").read_text()
        structure = re.search(r"struct fb_dmabuf_export\s*\{[^}]+\};", source)
        command = re.search(r"^#define FBIOGET_DMABUF\s+[^\n]+", source, re.M)
        self.assertIsNotNone(structure)
        self.assertIsNotNone(command)
        unit = '\n'.join([
            '#include <stdbool.h>', '#include <stdint.h>', '#include <linux/types.h>',
            '#include "a333_g2d_uapi.h"', structure.group(), command.group(),
            '_Static_assert(sizeof(struct a333_fb_dmabuf_export) == '
            'sizeof(struct fb_dmabuf_export), "export size");',
            *[f'_Static_assert(offsetof(struct a333_fb_dmabuf_export, {field}) == '
              f'offsetof(struct fb_dmabuf_export, {field}), "export {field}");'
              for field in ("fd", "flags")],
            '_Static_assert(A333_FBIOGET_DMABUF == FBIOGET_DMABUF, "export ioctl");',
            'int main(void) { return 0; }',
        ])
        result = subprocess.run(self.compiler + self.flags + ["-x", "c", "-",
                                "-o", str(pathlib.Path(self.temp.name) / "export-abi")],
                                input=unit, capture_output=True, text=True, timeout=60)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_rotation_stride_offsets_and_rgb_order(self):
        for rotation in (0, 90, 180, 270):
            for layout in ("argb", "abgr", "xrgb", "xbgr"):
                with self.subTest(rotation=rotation, layout=layout):
                    self.run_fixture("geometry", rotation, layout)

    def test_invalid_framebuffer_layouts(self):
        for layout in ("null-fixed", "null-variable", "bad-fd", "type", "visual",
                       "bpp", "red-length", "green-length", "blue-length",
                       "green-offset", "red-offset", "blue-offset", "red-msb",
                       "green-msb", "blue-msb", "alpha-msb", "alpha-length",
                       "alpha-offset", "zero-stride", "unaligned-stride",
                       "zero-width", "zero-height", "wide", "tall", "xoffset",
                       "yoffset", "short-smem", "overflow-x", "overflow-y",
                       "wide-stride", "nonstd", "grayscale"):
            with self.subTest(layout=layout):
                self.run_fixture("layout", layout)

    def test_invalid_canvas_and_non_one_to_one_geometry(self):
        for case in ("null-g2d", "null-canvas", "null-pixels", "zero-width",
                     "negative-width", "zero-height", "negative-height",
                     "stride", "rotation", "scale-0", "scale-90",
                     "scale-180", "scale-270"):
            with self.subTest(case=case):
                self.run_fixture("canvas", case)

    def test_device_allocation_export_sync_and_ioctl_errors_clean_up(self):
        for failure in ("g2d-open", "export", "export-error-fd", "export-fd", "fcntl", "lseek",
                        "short-export", "heap-open", "allocate", "source-mmap",
                        "sync-start", "sync-end", "blit", "blit-eintr"):
            with self.subTest(failure=failure):
                self.run_fixture("error", failure)

    def test_heap_fallback_and_interrupted_sync(self):
        for case in ("heap-missing-first", "allocate-first", "sync-start-eintr",
                     "sync-end-eintr"):
            with self.subTest(case=case):
                self.run_fixture("geometry", 90, "abgr", case)

    def test_staging_buffer_reuse_resize_and_failure_recovery(self):
        self.run_fixture("reuse")
        for failure in ("allocate", "source-mmap", "sync-start", "blit"):
            with self.subTest(failure=failure):
                self.run_fixture("recover", failure)

    def test_resize_mapping_failure_then_retry_old_geometry(self):
        for rotation in (0, 90, 180, 270):
            for layout in ("argb", "abgr", "xrgb", "xbgr"):
                with self.subTest(rotation=rotation, layout=layout):
                    self.run_fixture("resize-map-retry", rotation, layout)

    def test_opaque_alpha_accepted_and_nonopaque_rejected(self):
        for layout in ("argb", "abgr", "xrgb", "xbgr"):
            for alpha in (0, 1, 127, 254):
                with self.subTest(layout=layout, alpha=alpha):
                    self.run_fixture("alpha", layout, alpha)

    def test_fbdev_hardware_matches_software_for_all_rotations(self):
        for rotation in (0, 90, 180, 270):
            for layout in ("argb", "abgr", "xrgb", "xbgr"):
                for renderer in ("auto", "g2d", "software", "default"):
                    with self.subTest(rotation=rotation, layout=layout, renderer=renderer):
                        self.run_fixture("integration", renderer, "none", rotation, layout)

    def test_fbdev_auto_fallback_and_strict_failure_are_sticky(self):
        for renderer in ("auto", "g2d"):
            for failure in ("g2d-open", "export", "export-error-fd", "export-fd", "fcntl", "lseek",
                            "short-export", "heap-open", "allocate", "source-mmap",
                            "sync-start", "sync-end", "blit", "blit-eintr",
                            "non1to1", "layout", "alpha"):
                with self.subTest(renderer=renderer, failure=failure):
                    self.run_fixture("integration", renderer, failure, 90, "abgr")

    def test_fbdev_configuration_and_disabled_build(self):
        self.run_fixture("integration", "invalid", "none", 0, "argb")
        self.run_fixture("integration", "auto", "fb-mmap", 0, "argb")
        for renderer in ("auto", "software", "default", "g2d"):
            with self.subTest(renderer=renderer):
                self.run_fixture("integration", renderer, "none", 270, "abgr", enabled=0)


if __name__ == "__main__":
    unittest.main()
