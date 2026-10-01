"""Apply SDK patches 0008..0011 to copied vendor sources using Buildroot -F0.

Check the entire resulting files to ensure context repairs preserve logic.
Run: python3 -B scripts/tests/test_kernel_patch_series.py -v
No vendor files are written and no kernel compilation is performed.
"""

import pathlib
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
VENDOR = ROOT / "vendor/allwinner-a333"
PATCH_DIR = ROOT / "configs/boards/a333/patches/linux"
GPADC = pathlib.Path("bsp/drivers/gpadc/sunxi_gpadc.c")
CODEC = pathlib.Path("bsp/drivers/sound/platform/snd_sun65iw1_codec.c")
G2D = pathlib.Path("bsp/drivers/g2d/g2d_rcq/g2d.c")


class KernelPatchSeriesTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="a333-kernel-patches-")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.folder = pathlib.Path(cls.temp.name)
        cls.kernel = cls.folder / "kernel"
        cls.originals = {}
        for relative in (GPADC, CODEC, G2D):
            source = VENDOR / relative
            target = cls.kernel / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(source, target)
            cls.originals[relative] = source.read_text()
        patchset = cls.folder / "patches"
        patchset.mkdir()
        for number in range(8, 12):
            matches = list(PATCH_DIR.glob(f"{number:04d}-*.patch"))
            if len(matches) != 1:
                raise RuntimeError(f"Expected one patch numbered {number:04d}")
            shutil.copyfile(matches[0], patchset / matches[0].name)
        result = subprocess.run([
            "bash", str(ROOT / "buildroot/support/scripts/apply-patches.sh"),
            str(cls.kernel), str(patchset), "*.patch",
        ], text=True, capture_output=True)
        if result.returncode:
            raise RuntimeError(result.stdout + result.stderr)

    def assert_only_replacements(self, relative, replacements):
        original = self.originals[relative]
        expected = original
        for old, new in replacements:
            self.assertEqual(expected.count(old), 1, old)
            expected = expected.replace(old, new, 1)
        self.assertEqual((self.kernel / relative).read_text(), expected)
        self.assertEqual((VENDOR / relative).read_text(), original)

    def test_gpadc_changes_preserve_existing_logic(self):
        self.assert_only_replacements(GPADC, (
            ("\tpm_runtime_mark_last_busy(chip->dev);\n"
             "\tpm_runtime_put_autosuspend(chip->dev);",
             "\tpm_runtime_mark_last_busy(chip->dev);\n"
             "\tif (!chip->gpadc_config.keyadc_select)\n"
             "\t\tpm_runtime_put_autosuspend(chip->dev);"),
            ("\t\tif (BIT(i) & config->chd_select) {",
             "\t\tif (BIT(i) & (config->chd_select | config->keyadc_select)) {"),
            ("\t\t\tif (config->chd_select & BIT(i)) {\n"
             "\t\t\t\tsunxi_gpadc_highirq_control(chip->reg_base, i, true);",
             "\t\t\tif ((config->chd_select | config->keyadc_select) & BIT(i)) {\n"
             "\t\t\t\tsunxi_gpadc_highirq_control(chip->reg_base, i,\n"
             "\t\t\t\t\t\t\t    !!(config->chd_select & BIT(i)));"),
        ))

    def test_codec_changes_preserve_existing_logic(self):
        self.assert_only_replacements(CODEC, (
            ("\t\t.rates\t\t= SNDRV_PCM_RATE_8000_192000\n"
             "\t\t\t\t| SNDRV_PCM_RATE_KNOT,",
             "\t\t.rates\t\t= SNDRV_PCM_RATE_8000_48000\n"
             "\t\t\t\t| SNDRV_PCM_RATE_96000\n"
             "\t\t\t\t| SNDRV_PCM_RATE_192000,"),
            ("\t\t.rates\t\t= SNDRV_PCM_RATE_8000_48000\n"
             "\t\t\t\t| SNDRV_PCM_RATE_KNOT,",
             "\t\t.rates\t\t= SNDRV_PCM_RATE_8000_48000,"),
        ))

    def test_g2d_only_initializes_names(self):
        self.assert_only_replacements(G2D, (
            ("const char *values_of_power_domain_names[PM_ARRAY_SIZE];",
             "const char *values_of_power_domain_names[PM_ARRAY_SIZE] = { NULL };"),
        ))

    def test_older_gpadc_without_vendor_probe_reinitialization(self):
        # Buildroot may retain the older archive in dl/linux/ even when the
        # file:// source in dl/ is newer. That GPADC lacks this vendor block.
        # Patch 0008 must not depend on unrelated following probe statements.
        block = ("\t//ADD szbaijie\n"
                 "\tpm_runtime_disable(&pdev->dev);\n"
                 "\tsunxi_gpadc_hw_init(chip);\n"
                 "\t//ADD szbaijie\n\n")
        original = self.originals[GPADC]
        self.assertEqual(original.count(block), 1)
        with tempfile.TemporaryDirectory(prefix="a333-old-gpadc-") as folder:
            kernel = pathlib.Path(folder)
            target = kernel / GPADC
            target.parent.mkdir(parents=True)
            target.write_text(original.replace(block, "", 1))
            result = subprocess.run([
                "bash", str(ROOT / "buildroot/support/scripts/apply-patches.sh"),
                str(kernel), str(PATCH_DIR), "000[89]-*.patch",
            ], text=True, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            expected = (self.kernel / GPADC).read_text().replace(block, "", 1)
            self.assertEqual(target.read_text(), expected)


if __name__ == "__main__":
    unittest.main()
