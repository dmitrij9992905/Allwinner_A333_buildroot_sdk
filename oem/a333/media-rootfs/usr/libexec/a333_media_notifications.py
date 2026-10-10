"""Connection events, independent of playback mode; no media-file writes."""
from collections import deque
import time


class Notifications:
    def __init__(self, clock=time.monotonic):
        self.clock = clock
        self.mounts = None
        self.bluetooth = None
        self.pending = deque(maxlen=32)
        self.text = ''
        self.until = 0
        self.sequence = time.monotonic_ns()

    def push(self, text):
        self.pending.append(' '.join(str(text).split())[:240])

    def observe_mounts(self, mounts):
        # Compare individual mount identities, not an aggregate USB-present bit:
        # plugging a second drive must also produce a notification.
        current = {m['path']: m for m in mounts}
        if self.mounts is not None:
            for path, mount in self.mounts.items():
                if path not in current:
                    self.push(self.storage_text(mount, False))
            for path, mount in current.items():
                if path not in self.mounts:
                    self.push(self.storage_text(mount, True))
        self.mounts = current

    @staticmethod
    def storage_text(mount, attached):
        kind = 'USB-накопитель' if mount['kind'] == 'usb' else 'SD-карта'
        label = mount.get('label') or mount.get('device', '').rsplit('/', 1)[-1]
        verb = ('подключён' if attached else 'отключён') if mount['kind'] == 'usb' else (
            'подключена' if attached else 'отключена')
        return '%s %s%s' % (kind, verb,
                           ': ' + label if label else '')

    def observe_bluetooth(self, connected):
        if self.bluetooth is not None:
            for path, name in self.bluetooth.items():
                if path not in connected:
                    self.push('Bluetooth отключён: ' + name)
            for path, name in connected.items():
                if path not in self.bluetooth:
                    self.push('Bluetooth подключён: ' + name)
        self.bluetooth = dict(connected)

    def snapshot(self):
        now = self.clock()
        if now >= self.until:
            if self.pending:
                self.text = self.pending.popleft()
                self.until = now + 5
                self.sequence += 1
            else:
                self.text = ''
        return self.text, str(self.sequence)
