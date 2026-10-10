"""Bounded MPD protocol, artwork and removable-library support (no UI thread I/O)."""
import asyncio
import hashlib
import io
import json
import logging
import math
import os
from pathlib import Path
import socket
import time

MAX_ART = 16 * 1024 * 1024


def quote(value):
    value = str(value)
    if any(c in value for c in ('\n', '\r', '\x00')):
        raise ValueError('Invalid MPD argument')
    return '"' + value.replace('\\', '\\\\').replace('"', '\\"') + '"'


class MPD:
    def __init__(self, host='127.0.0.1', port=6600):
        self.host, self.port = host, port

    def command(self, name, *args):
        if not name.isalpha():
            raise ValueError('Invalid MPD command')
        with socket.create_connection((self.host, self.port), timeout=5) as sock:
            sock.settimeout(5)
            with sock.makefile('rb') as stream:
                if not stream.readline(1024).startswith(b'OK MPD '):
                    raise ValueError('Invalid MPD greeting')
                request = name + ''.join(' ' + quote(a) for a in args) + '\n'
                sock.sendall(request.encode('utf-8'))
                result, binary = {}, b''
                for _ in range(4096):
                    line = stream.readline(32769)
                    if not line or len(line) > 32768:
                        raise ValueError('Truncated/oversized MPD response')
                    if line == b'OK\n':
                        return result, binary
                    if line.startswith(b'ACK '):
                        raise ValueError(line.decode('utf-8', 'replace').strip())
                    if line == b'\n':
                        continue
                    key, value = line.rstrip(b'\n').split(b': ', 1)
                    if key == b'binary':
                        count = int(value)
                        if count < 0 or count > MAX_ART:
                            raise ValueError('Oversized image chunk')
                        binary = stream.read(count)
                        if len(binary) != count or stream.read(1) != b'\n':
                            raise ValueError('Truncated image chunk')
                    else:
                        result[key.decode()] = value.decode('utf-8', 'replace')
                raise ValueError('Oversized MPD response')

    def picture(self, uri):
        for command in ('readpicture', 'albumart'):
            image = bytearray()
            try:
                while True:
                    fields, chunk = self.command(command, uri, len(image))
                    total = int(fields.get('size', 0))
                    if not total:
                        break
                    if total > MAX_ART or total < len(image) + len(chunk) or not chunk:
                        raise ValueError('Invalid image size')
                    image.extend(chunk)
                    if len(image) == total:
                        return bytes(image)
            except (OSError, ValueError):
                continue
        return None


def seconds(value):
    try:
        result = float(value)
        return result if math.isfinite(result) and 0 <= result <= 8640000 else 0.0
    except (ValueError, TypeError):
        return 0.0


def prepare_library(root=Path('/userdata/media/library')):
    """Never alter the music collection or replace user-owned files/directories."""
    root.mkdir(parents=True, exist_ok=True)
    for name, target in {'local': '../music', 'usb': '/media/usb', 'sd': '/media/sd'}.items():
        link = root / name
        if link.is_symlink():
            if os.readlink(link) != target:
                raise ValueError('Unexpected library link: ' + str(link))
        elif link.exists():
            raise ValueError('Library entry already exists: ' + str(link))
        else:
            link.symlink_to(target)


def normalize_picture(data, destination):
    from PIL import Image, ImageOps
    if not data or len(data) > MAX_ART:
        raise ValueError('Invalid image')
    with Image.open(io.BytesIO(data)) as image:
        if image.width * image.height > 16000000:
            raise ValueError('Oversized artwork dimensions')
        image = ImageOps.exif_transpose(image)
        image.thumbnail((512, 512))
        image = image.convert('RGB')
        temporary = destination.with_suffix('.new')
        image.save(temporary, format='PNG')
        temporary.chmod(0o644)
        os.replace(temporary, destination)


class PlaybackController:
    def __init__(self, run, mpd=None, storage=Path('/run/a333-storage/mounts.json')):
        self.run, self.mpd, self.storage = Path(run), mpd or MPD(), Path(storage)
        self.cover_dir = self.run / 'covers'
        self.cover_dir.mkdir(parents=True, exist_ok=True)
        self.snapshot = {}
        self.storage_revision = None
        self.mounts = []
        self.cover_key, self.cover_path, self.cover_task = '', '', None
        self.cover_retry = 0
        self.seek_lock = asyncio.Lock()

    def storage_state(self):
        try:
            value = json.loads(self.storage.read_text())
            mounts = value.get('mounts', [])
            if not isinstance(mounts, list):
                raise ValueError('Invalid mounts')
            self.mounts = [m for m in mounts if isinstance(m, dict) and
                           m.get('kind') in ('usb', 'sd') and isinstance(m.get('path'), str)]
            return str(value.get('revision', ''))
        except (OSError, ValueError, TypeError):
            self.mounts = []
            return ''

    def available(self, kind):
        return any(m['kind'] == kind for m in self.mounts)

    async def refresh_storage(self):
        revision = self.storage_state()
        if revision == self.storage_revision:
            return
        previous = self.storage_revision
        self.storage_revision = revision
        # Physical removal must stop playback before stale queue entries are used.
        uri = self.snapshot.get('file', '')
        for kind in ('usb', 'sd'):
            attached = self.available(kind)
            if kind == 'usb' and uri.startswith('usb/'):
                component = uri.split('/', 2)[1]
                attached = any(m['kind'] == kind and Path(m['path']).name == component for m in self.mounts)
            if uri.startswith(kind + '/') and not attached:
                try:
                    await asyncio.to_thread(self.mpd.command, 'stop')
                except (OSError, ValueError):
                    pass
            # Queue an asynchronous database scan; do not wait for a large disk.
            if previous is not None:
                try:
                    await asyncio.to_thread(self.mpd.command, 'update', kind)
                except (OSError, ValueError):
                    pass

    async def poll(self):
        await self.refresh_storage()
        try:
            status, _ = await asyncio.to_thread(self.mpd.command, 'status')
            song, _ = await asyncio.to_thread(self.mpd.command, 'currentsong')
            if status.get('songid') != song.get('Id'):
                return self.snapshot
            uri = song.get('file', '')
            duration = seconds(status.get('duration', song.get('duration', song.get('Time'))))
            elapsed = seconds(status.get('elapsed'))
            token = song.get('Id', '')
            self.snapshot = dict(song, elapsed=min(elapsed, duration) if duration else elapsed,
                                 duration=duration, token=token,
                                 seekable=bool(token and duration and status.get('state') in ('play', 'pause')
                                               and not uri.startswith(('http://', 'https://'))),
                                 playing=status.get('state') == 'play')
        except (OSError, ValueError):
            self.snapshot = {}
        return self.snapshot

    def request_cover(self, key, uri='', loader=None):
        if key == self.cover_key:
            if (not key or self.cover_path or time.monotonic() < self.cover_retry or
                    (self.cover_task and not self.cover_task.done())):
                return
        self.cover_key, self.cover_path = key, ''
        self.cover_retry = time.monotonic() + 20
        if self.cover_task:
            self.cover_task.cancel()
        if key:
            self.cover_task = asyncio.create_task(self.load_cover(key, uri, loader))

    async def load_cover(self, key, uri, loader):
        path = self.cover_dir / (hashlib.sha256(key.encode()).hexdigest() + '.png')
        try:
            if not path.exists():
                if loader:
                    data = await loader()
                elif uri and not uri.startswith(('http://', 'https://')):
                    data = await asyncio.to_thread(self.mpd.picture, uri)
                else:
                    return
                if not data:
                    return
                await asyncio.to_thread(normalize_picture, data, path)
            if key == self.cover_key:
                self.cover_path = str(path)
            # Prune only our hash-named cache files, never songs or external art.
            cached = sorted(self.cover_dir.glob('[0-9a-f]' * 64 + '.png'),
                            key=lambda p: p.stat().st_mtime, reverse=True)
            for old in cached[32:]:
                if old != path:
                    old.unlink(missing_ok=True)
        except asyncio.CancelledError:
            pass
        except Exception as error:
            logging.warning('Artwork unavailable: %s', error)

    async def seek(self, request, source):
        position, token = request.get('seconds'), request.get('track')
        if (source != 'MPD' or isinstance(position, bool) or
                not isinstance(position, (float, int)) or not math.isfinite(position) or
                not isinstance(token, str) or not token.isdecimal()):
            raise ValueError('Seek unavailable')
        async with self.seek_lock:
            status, _ = await asyncio.to_thread(self.mpd.command, 'status')
            duration = seconds(status.get('duration'))
            if status.get('songid') != token or duration <= 0 or not 0 <= position <= duration:
                raise ValueError('Track changed or invalid position')
            await asyncio.to_thread(self.mpd.command, 'seekid', token, '%.3f' % position)

