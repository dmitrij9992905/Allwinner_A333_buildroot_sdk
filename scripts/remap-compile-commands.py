#!/usr/bin/env python3
"""Make Docker's compilation database readable by host VSCode (spaces safe)."""
import json
import pathlib
import shlex
import sys


def remap(entries, source_prefix, destination_prefix):
    result = []
    for entry in entries:
        item = dict(entry)
        for key in ("directory", "file", "output"):
            if key in item:
                item[key] = item[key].replace(source_prefix, destination_prefix)
        arguments = item.pop("arguments", None)
        if arguments is None:
            arguments = shlex.split(item["command"])
        item.pop("command", None)
        item["arguments"] = [arg.replace(source_prefix, destination_prefix) for arg in arguments]
        result.append(item)
    return result


if __name__ == "__main__":
    input_path, output_path, source_prefix, destination_prefix = sys.argv[1:]
    entries = json.loads(pathlib.Path(input_path).read_text())
    pathlib.Path(output_path).write_text(
        json.dumps(remap(entries, source_prefix, destination_prefix), indent=2) + "\n")
