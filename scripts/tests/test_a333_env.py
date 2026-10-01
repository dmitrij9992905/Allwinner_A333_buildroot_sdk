import binascii
import importlib.util
import pathlib
import struct
import unittest

spec = importlib.util.spec_from_file_location(
    "a333_env", pathlib.Path(__file__).resolve().parents[1] / "normalize-a333-env.py"
)
env = importlib.util.module_from_spec(spec)
spec.loader.exec_module(env)


def image(data):
    data = data.ljust(env.ENV_SIZE - 4, b"\0")
    return struct.pack("<I", binascii.crc32(data)) + data


class EnvironmentTest(unittest.TestCase):
    def test_vendor_leading_null(self):
        expected = image(b"systemAB_next=A\0bootcmd=run setargs_mmc boot_normal\0\0")
        # Use the exact same variables with only the extra vendor NUL.
        vendor = image(b"\0systemAB_next=A\0bootcmd=run setargs_mmc boot_normal\0\0")
        self.assertEqual(env.normalize(vendor), expected)

    def test_standard_unchanged(self):
        blob = image(b"systemAB_next=B\0\0")
        self.assertEqual(env.normalize(blob), blob)

    def test_bad_crc_rejected(self):
        with self.assertRaises(ValueError):
            env.normalize(b"BAD!" + image(b"slot=A\0\0")[4:])

    def test_wrong_size_rejected(self):
        with self.assertRaises(ValueError):
            env.normalize(image(b"slot=A\0\0")[:-1])

    def test_empty_rejected(self):
        with self.assertRaises(ValueError):
            env.normalize(image(b"\0\0"))


if __name__ == "__main__":
    unittest.main()
