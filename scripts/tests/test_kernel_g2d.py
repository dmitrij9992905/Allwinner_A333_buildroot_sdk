"""SDK G2D policy; optionally check a Kconfig-resolved .config.

Run with A333_G2D_KERNEL_CONFIG=/path/to/resolved/.config to also check
dependencies after merge_config.sh and olddefconfig. No build is launched.
"""

import os
import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
FRAGMENT = ROOT / "configs/boards/a333/linux-no-btf.fragment"
G2D = ("CONFIG_AW_G2D", "CONFIG_G2D_RCQ", "CONFIG_G2D_MIXER",
       "CONFIG_G2D_ROTATE")
ALLOCATORS = ("CONFIG_CMA", "CONFIG_DMA_CMA", "CONFIG_DMABUF_HEAPS",
              "CONFIG_DMABUF_HEAPS_SYSTEM", "CONFIG_DMABUF_HEAPS_CMA")
PROVIDERS = ("CONFIG_AW_BSP", "CONFIG_AW_CCU", "CONFIG_AW_SUN65IW1_CCU",
             "CONFIG_RESET_CONTROLLER", "CONFIG_AW_IOMMU", "CONFIG_AW_IOMMU_V3",
             "CONFIG_IOMMU_DMA", "CONFIG_PM", "CONFIG_PM_GENERIC_DOMAINS",
             "CONFIG_AW_PCK600_DOMAINS")


def config_values(path):
    values = {}
    for line in path.read_text().splitlines():
        if line.startswith("# CONFIG_") and line.endswith(" is not set"):
            values[line.split()[1]] = "n"
        elif "=" in line and not line.startswith("#"):
            key, value = line.split("=", 1)
            values[key] = value.strip('"')
    return values


class G2DConfigTest(unittest.TestCase):
    def assert_builtin(self, values, symbols):
        for symbol in symbols:
            with self.subTest(symbol=symbol):
                self.assertEqual(values.get(symbol), "y", symbol)

    def test_sdk_fragment_pins_rcq_and_dma_allocators(self):
        self.assert_builtin(config_values(FRAGMENT), G2D + ALLOCATORS)

    def test_all_a333_defconfigs_include_policy_without_overrides(self):
        defconfigs = sorted((ROOT / "configs/configs").glob("a333_*defconfig"))
        self.assertTrue(defconfigs)
        for defconfig in defconfigs:
            with self.subTest(defconfig=defconfig.name):
                fragments = config_values(defconfig)[
                    "BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES"].split()
                self.assertIn("../configs/boards/a333/linux-no-btf.fragment", fragments)
                merged = {}
                for fragment in fragments:
                    merged.update(config_values(ROOT / "buildroot" / fragment))
                self.assert_builtin(merged, G2D + ALLOCATORS)
                for alternative in ("CONFIG_G2D_LEGACY", "CONFIG_G2D200"):
                    self.assertNotEqual(merged.get(alternative), "y")

    def test_existing_builtin_clock_and_power_policy(self):
        self.assert_builtin(config_values(FRAGMENT), (
            "CONFIG_AW_CCU", "CONFIG_AW_SUN65IW1_CCU", "CONFIG_AW_PCK600_DOMAINS"))

    @unittest.skipUnless(os.environ.get("A333_G2D_KERNEL_CONFIG"),
                         "set A333_G2D_KERNEL_CONFIG to check resolved Kconfig")
    def test_resolved_kconfig(self):
        values = config_values(pathlib.Path(os.environ["A333_G2D_KERNEL_CONFIG"]))
        self.assert_builtin(values, G2D + ALLOCATORS + PROVIDERS + (
            "CONFIG_DMA_SHARED_BUFFER",))
        for alternative in ("CONFIG_G2D_LEGACY", "CONFIG_G2D200"):
            self.assertEqual(values.get(alternative), "n", alternative)


if __name__ == "__main__":
    unittest.main()
