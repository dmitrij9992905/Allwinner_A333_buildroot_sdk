"""Apply the SDK RCQ probe patch and test its NULL termination without hardware.

Run: python3 -B scripts/tests/test_kernel_g2d_probe.py -v
Requires patch and a host C compiler; CC may include launcher arguments.
The original vendor source is only read, never patched in place.
"""

import os
import pathlib
import re
import shlex
import shutil
import subprocess
import tempfile
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
PATCH_DIR = ROOT / "configs/boards/a333/patches/linux"
PATCH_NAME = "0011-initialize-g2d-power-domain-names.patch"
DRIVER = pathlib.Path("bsp/drivers/g2d/g2d_rcq/g2d.c")
SOURCE = ROOT / "vendor/allwinner-a333" / DRIVER
OLD = "const char *values_of_power_domain_names[PM_ARRAY_SIZE];"
NEW = "const char *values_of_power_domain_names[PM_ARRAY_SIZE] = { NULL };"


class G2DProbePatchTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="a333-g2d-probe-")
        cls.addClassCleanup(cls.temp.cleanup)
        cls.folder = pathlib.Path(cls.temp.name)
        target = cls.folder / DRIVER
        target.parent.mkdir(parents=True)
        shutil.copyfile(SOURCE, target)
        cls.original = SOURCE.read_text()
        subprocess.run([
            "bash", str(ROOT / "buildroot/support/scripts/apply-patches.sh"),
            str(cls.folder), str(PATCH_DIR), PATCH_NAME,
        ], check=True, text=True, capture_output=True)
        cls.patched = target.read_text()

        # Compile the actual patched declaration and attachment loop, not a
        # separately reimplemented version of the NULL-termination logic.
        start = cls.patched.index("static int g2d_attach_pd(")
        end = cls.patched.index("\n#endif", start)
        attach = cls.patched[start:end]
        size = re.search(r"#define PM_ARRAY_SIZE\s+\d+", cls.patched).group()
        fixture = cls.folder / "probe.c"
        fixture.write_text("""
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
struct device { void *pm_domain; };
struct device_link { int unused; };
static int attachments, links;
#define IS_ERR(ptr) (0)
#define PTR_ERR(ptr) (-1)
#define DL_FLAG_STATELESS 1
#define DL_FLAG_PM_RUNTIME 2
#define G2D_ERR(...) ((void)0)
static struct device *dev_pm_domain_attach_by_name(struct device *dev,
                                                  const char *name)
{
    assert(strcmp(name, "vo") == 0);
    attachments++;
    return dev;
}
static struct device_link *device_link_add(struct device *dev,
                                           struct device *pd_dev, int flags)
{
    static struct device_link link;
    assert(dev == pd_dev);
    assert(flags == (DL_FLAG_STATELESS | DL_FLAG_PM_RUNTIME));
    links++;
    return &link;
}
""" + size + "\n" + attach + "\n" + """
int main(int argc, char **argv)
{
    struct device dev = { 0 };
    int expected;
    assert(argc == 2);
""" + NEW + "\n" + """
    for (int i = 0; i < PM_ARRAY_SIZE; i++)
        assert(values_of_power_domain_names[i] == NULL);
    if (strcmp(argv[1], "absent") == 0) {
        /* A failed of_property_read_string leaves every slot untouched. */
        expected = 0;
    } else if (strcmp(argv[1], "one") == 0) {
        /* A successful string read fills only slot zero. */
        values_of_power_domain_names[0] = "vo";
        expected = 1;
    } else {
        return 2;
    }
    assert(g2d_attach_pd(&dev, values_of_power_domain_names, PM_ARRAY_SIZE) == 0);
    assert(attachments == expected);
    assert(links == expected);
    return 0;
}
""")
        cls.binary = cls.folder / "probe"
        subprocess.run(shlex.split(os.environ.get("CC", "cc")) + [
            "-std=c11", "-O2", "-Wall", "-Wextra", "-Werror",
            str(fixture), "-o", str(cls.binary),
        ], check=True, text=True)

    def test_patch_only_zero_initializes_probe_names(self):
        self.assertEqual(self.original.count(OLD), 1)
        self.assertEqual(self.patched, self.original.replace(OLD, NEW, 1))
        self.assertEqual(SOURCE.read_text(), self.original)
        self.assertEqual((self.folder / ".applied_patches_list").read_text().strip(),
                         str(PATCH_DIR / PATCH_NAME))

    def test_all_defconfigs_apply_sdk_kernel_patch_dir(self):
        # Buildroot sorts numbered patches when no series file is present.
        self.assertFalse((PATCH_DIR / "series").exists())
        patches = sorted(path.name for path in PATCH_DIR.glob("*.patch"))
        self.assertGreater(patches.index(PATCH_NAME), patches.index(
            "0010-reject-unsupported-codec-sample-rates.patch"))
        defconfigs = sorted((ROOT / "configs/configs").glob("a333_*defconfig"))
        self.assertTrue(defconfigs)
        for defconfig in defconfigs:
            with self.subTest(defconfig=defconfig.name):
                self.assertIn('BR2_LINUX_KERNEL_PATCH="../configs/boards/a333/patches/linux"',
                              defconfig.read_text().splitlines())

    def test_missing_property_has_no_domain_lookups(self):
        subprocess.run([str(self.binary), "absent"], check=True)

    def test_single_name_keeps_null_terminator(self):
        subprocess.run([str(self.binary), "one"], check=True)


if __name__ == "__main__":
    unittest.main()
