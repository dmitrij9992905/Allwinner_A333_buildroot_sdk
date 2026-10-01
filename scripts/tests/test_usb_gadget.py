import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "overlays/a333/rootfs/usr/sbin/a333-usb-gadget"


class GadgetTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = pathlib.Path(self.temp.name)
        self.gadget = self.root / "sys/kernel/config/usb_gadget/a333"
        self.gadget.mkdir(parents=True)
        (self.gadget / "UDC").touch()
        self.ffs = self.root / "dev/usb-ffs/adb"
        self.ffs.mkdir(parents=True)
        self.udc = self.root / "sys/class/udc/c100000.udc-controller"
        self.udc.mkdir(parents=True)

    def bind(self):
        # Isolate every hardware path. Exercise the real shell logic, using
        # regular endpoints like FunctionFS (not character device nodes).
        script = SCRIPT.read_text()
        for prefix in ("/sys/kernel/config", "/sys/class/udc", "/dev/usb-ffs"):
            script = script.replace(prefix, str(self.root) + prefix)
        script = script.replace("sleep 0.25", ":")
        return subprocess.run(["sh", "-s", "bind"], input=script, text=True,
                              capture_output=True)

    def endpoints(self):
        for name in ("ep0", "ep1", "ep2"):
            (self.ffs / name).touch()

    def test_bind_without_ready_attribute(self):
        self.endpoints()
        result = self.bind()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((self.gadget / "UDC").read_text().strip(), self.udc.name)

    def test_missing_endpoints(self):
        (self.ffs / "ep0").touch()
        result = self.bind()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("did not initialize", result.stderr)
        self.assertEqual((self.gadget / "UDC").read_text(), "")

    def test_missing_controller(self):
        self.endpoints()
        self.udc.rmdir()
        result = self.bind()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("no USB device controller", result.stderr)

    def test_already_bound(self):
        self.endpoints()
        (self.gadget / "UDC").write_text(self.udc.name + "\n")
        self.assertEqual(self.bind().returncode, 0)


if __name__ == "__main__":
    unittest.main()
