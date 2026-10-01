#!/usr/bin/env python3
"""Test codec gain without target ALSA hardware or host dbus-next dependency."""
import ast
from pathlib import Path
import re
import subprocess
import unittest
from unittest.mock import patch
import logging

BACKEND = Path(__file__).resolve().parents[2] / "oem/a333/media-rootfs/usr/libexec/a333-media-backend"
tree = ast.parse(BACKEND.read_text())
nodes = [node for node in tree.body if
         (isinstance(node, ast.FunctionDef) and node.name == "adjust_codec_volume") or
         (isinstance(node, ast.Assign) and any(isinstance(target, ast.Name) and
          target.id.startswith("CODEC_VOLUME_") for target in node.targets))]
namespace = dict(re=re, subprocess=subprocess, logging=logging)
exec(compile(ast.Module(body=nodes, type_ignores=[]), str(BACKEND), "exec"), namespace)
adjust = namespace["adjust_codec_volume"]


class VolumeTest(unittest.TestCase):
    def change(self, left, right, direction):
        writes = []
        def mixer(argv, **kwargs):
            if "cget" in argv:
                raw = left if argv[-1] == "name=DACL Volume" else right
                return subprocess.CompletedProcess(argv, 0,
                    f"; type=INTEGER,values=1,min=0,max=255,step=0\n : values={raw}\n", "")
            writes.append((argv[-2], int(argv[-1])))
            return subprocess.CompletedProcess(argv, 0, "", "")
        with patch.object(subprocess, "run", side_effect=mixer):
            level = adjust(direction)
        self.assertEqual([name for name, _ in writes],
                         ["name=DACL Volume", "name=DACR Volume"])
        self.assertEqual(writes[0][1], writes[1][1])
        self.assertLessEqual(writes[0][1], 159)
        return level, writes[0][1]

    def test_up_is_three_db(self):
        self.assertEqual(self.change(143, 143, 1), (85, 147))

    def test_down_is_three_db(self):
        self.assertEqual(self.change(143, 143, -1), (75, 139))

    def test_no_positive_gain(self):
        self.assertEqual(self.change(255, 206, 1), (100, 159))

    def test_channels_are_normalized(self):
        self.assertEqual(self.change(139, 147, 1), (85, 147))

    def test_mute_and_unmute(self):
        self.assertEqual(self.change(79, 79, -1), (0, 0))
        self.assertEqual(self.change(0, 0, 1), (1, 79))

    def test_old_inaudible_volume_enters_useful_range(self):
        self.assertEqual(self.change(51, 51, 1), (1, 79))

    def test_parse_error_does_not_write(self):
        with patch.object(subprocess, "run", return_value=
                          subprocess.CompletedProcess([], 0, "bad output", "")) as mixer:
            self.assertIsNone(adjust(1))
            self.assertEqual(mixer.call_count, 1)


if __name__ == "__main__":
    unittest.main()
