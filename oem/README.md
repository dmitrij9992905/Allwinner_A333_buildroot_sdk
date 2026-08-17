# OEM applications

This directory is reserved for the board's business logic and application
assets. It is intentionally separate from the immutable Buildroot base OS.

Planned layout:

```text
oem/a333/src/       application source code
oem/a333/package/   Buildroot package definitions
oem/a333/rootfs/    files installed into the OEM filesystem
oem/a333/tests/     host-side and target-side tests
```

The A/B update design will keep the OEM payload versioned independently from
the base `rootfsA`/`rootfsB` images. No application code is copied from the
Luckfox SDK.
