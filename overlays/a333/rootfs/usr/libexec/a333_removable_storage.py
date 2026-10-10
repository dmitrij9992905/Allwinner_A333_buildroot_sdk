"""Mount only USB disks and SD cards. Never format, repair or touch eMMC."""
import json
import logging
import os
from pathlib import Path
import re
import signal
import socket
import subprocess
import time

FILESYSTEMS = {'vfat': 'vfat', 'exfat': 'exfat', 'ext2': 'ext4',
               'ext3': 'ext4', 'ext4': 'ext4', 'ntfs': 'ntfs3'}


def run(*args):
    return subprocess.run(args, capture_output=True, text=True, timeout=12,
                          check=False)


def discover(sysfs=Path('/sys/class/block')):
    """SD type is authoritative; mmc numbering changes and eMMC is MMC, not SD."""
    found = []
    for disk in sorted(sysfs.iterdir()):
        if not re.fullmatch(r'sd[a-z]+|mmcblk[0-9]+', disk.name):
            continue
        kind = None
        if disk.name.startswith('sd') and re.search(r'/usb[0-9]+/', str(disk.resolve())):
            kind = 'usb'
        elif disk.name.startswith('mmcblk'):
            try:
                if (disk / 'device/type').read_text().strip() == 'SD':
                    kind = 'sd'
            except OSError:
                pass
        if not kind:
            continue
        partitions = [p for p in sysfs.iterdir() if (p / 'partition').exists()
                      and p.resolve().parent == disk.resolve()]
        partitions.sort(key=lambda p: int((p / 'partition').read_text()))
        for part in partitions or [disk]:
            found.append(dict(device='/dev/' + part.name, disk=disk.name, kind=kind,
                              path='/media/sd' if kind == 'sd' else '/media/usb/' + part.name))
    return found


def mounted_sources(mountinfo=Path('/proc/self/mountinfo')):
    mounted = {}
    for line in mountinfo.read_text().splitlines():
        try:
            before, after = line.split(' - ', 1)
            fields, info = before.split(), after.split()
            mounted[fields[4]] = info[1]
        except (ValueError, IndexError):
            continue
    return mounted


class Storage:
    def __init__(self, runtime=Path('/run/a333-storage')):
        self.runtime = Path(runtime)
        self.runtime.mkdir(parents=True, exist_ok=True)
        self.active = {}
        # Recover our own live mounts across a daemon restart, without adopting
        # arbitrary administrator mounts elsewhere in the filesystem.
        try:
            previous = json.loads((self.runtime / 'mounts.json').read_text())
            mounts = mounted_sources()
            for entry in previous.get('mounts', []):
                device, path = entry.get('device', ''), entry.get('path', '')
                valid = (entry.get('kind') == 'usb' and
                         re.fullmatch(r'/media/usb/sd[a-z]+[0-9]*', path)) or (
                         entry.get('kind') == 'sd' and path == '/media/sd')
                if valid and re.fullmatch(r'/dev/(sd[a-z]+[0-9]*|mmcblk[0-9]+p?[0-9]*)', device) and mounts.get(path) == device:
                    self.active[device] = entry
        except (OSError, ValueError, TypeError, AttributeError):
            pass
        self.failed = {}
        self.revision = time.time_ns()
        self.publish()

    def publish(self):
        self.revision += 1
        value = dict(revision=str(self.revision), mounts=list(self.active.values()))
        temporary = self.runtime / 'mounts.new'
        temporary.write_text(json.dumps(value, ensure_ascii=False) + '\n')
        temporary.chmod(0o644)
        os.replace(temporary, self.runtime / 'mounts.json')
        # Addressed notification + atomic snapshot: receiver restarts lose no state.
        try:
            with socket.socket(socket.AF_UNIX, socket.SOCK_DGRAM) as sock:
                sock.settimeout(0.1)
                sock.sendto(b'{"command":"storage_changed"}', '/run/a333-media/control.sock')
        except OSError:
            pass

    def remove(self, device, physical=False):
        entry = self.active[device]
        result = run('umount', entry['path'])
        if result.returncode and physical:
            # The drive is already gone. Detach the stale mount while open file
            # references drain; never use lazy unmount for ordinary safe eject.
            result = run('umount', '-l', entry['path'])
        if result.returncode:
            logging.warning('Cannot unmount %s: %s', device, result.stderr.strip())
            return False
        self.active.pop(device)
        if entry['kind'] == 'usb':
            # rmdir, not recursive removal: no data deletion is ever allowed.
            try:
                Path(entry['path']).rmdir()
            except OSError:
                pass
        self.publish()
        return True

    def reconcile(self, candidates):
        present = {c['device'] for c in candidates}
        for device in list(self.active):
            if device not in present:
                self.remove(device, physical=True)
        self.failed = {d: t for d, t in self.failed.items() if d in present}
        mounts = mounted_sources()
        for candidate in candidates:
            device, path, kind = candidate['device'], candidate['path'], candidate['kind']
            if device in self.active or time.monotonic() < self.failed.get(device, 0):
                continue
            # Never mount a second SD partition over the occupied fixed point,
            # nor take ownership of mounts made by an administrator elsewhere.
            if path in mounts or device in mounts.values():
                continue
            probe = run('blkid', '-p', '-o', 'export', device)
            fields = dict(line.split('=', 1) for line in probe.stdout.splitlines() if '=' in line)
            filesystem = FILESYSTEMS.get(fields.get('TYPE'))
            if probe.returncode or not filesystem:
                self.failed[device] = time.monotonic() + 30
                continue
            target = Path(path)
            target.mkdir(mode=0o755, parents=True, exist_ok=True)
            options = 'ro,nodev,nosuid,noexec,noatime'
            if filesystem in ('vfat', 'exfat'):
                options += ',uid=0,gid=0,umask=022,iocharset=utf8'
            mounted = run('mount', '-t', filesystem, '-o', options, device, path)
            if mounted.returncode:
                logging.warning('Cannot mount %s (%s): %s', device, filesystem, mounted.stderr.strip())
                self.failed[device] = time.monotonic() + 30
                if kind == 'usb':
                    try:
                        target.rmdir()
                    except OSError:
                        pass
                continue
            self.active[device] = dict(candidate, filesystem=filesystem,
                                       label=fields.get('LABEL', Path(device).name))
            mounts[path] = device
            logging.info('Mounted %s at %s (read-only)', device, path)
            self.publish()


def main():
    logging.basicConfig(level=logging.INFO)
    storage = Storage()
    running = True
    def stop(*_):
        nonlocal running
        running = False
    signal.signal(signal.SIGTERM, stop)
    signal.signal(signal.SIGINT, stop)
    while running:
        try:
            storage.reconcile(discover())
        except (OSError, ValueError, subprocess.TimeoutExpired):
            logging.exception('Removable-storage scan failed')
        time.sleep(1)
    for device in list(storage.active):
        storage.remove(device)


if __name__ == '__main__':
    main()
