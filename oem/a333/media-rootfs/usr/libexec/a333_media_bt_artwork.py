"""Optional AVRCP Cover Art (BlueZ 5.79 experimental BIP/OBEX API)."""
import asyncio
from pathlib import Path
from dbus_next import Message, MessageType, Variant


class BluetoothArtwork:
    def __init__(self, bus, directory):
        self.bus, self.directory = bus, Path(directory)
        self.session, self.address = None, None
        self.lock = asyncio.Lock()

    async def call(self, destination, path, interface, member, signature='', body=None):
        reply = await asyncio.wait_for(self.bus.call(Message(destination=destination,
            path=path, interface=interface, member=member, signature=signature,
            body=body or [])), 12)
        if reply.message_type != MessageType.METHOD_RETURN:
            raise ValueError('Bluetooth Cover Art: ' + str(reply.body))
        return reply.body

    async def properties(self, destination, path, interface):
        values = await self.call(destination, path, 'org.freedesktop.DBus.Properties',
                                 'GetAll', 's', [interface])
        return {k: v.value for k, v in values[0].items()}

    async def close(self):
        session, self.session = self.session, None
        self.address = None
        if session:
            try:
                await self.call('org.bluez.obex', '/org/bluez/obex',
                                'org.bluez.obex.Client1', 'RemoveSession', 'o', [session])
            except Exception:
                pass

    async def picture(self, player):
        async with self.lock:
            try:
                return await self.download(player)
            except (Exception, asyncio.CancelledError):
                await self.close()
                raise
            finally:
                (self.directory / 'bluetooth-download').unlink(missing_ok=True)

    async def download(self, player):
        properties = await self.properties('org.bluez', player, 'org.bluez.MediaPlayer1')
        port, device = properties.get('ObexPort'), properties.get('Device')
        if not port or not device:
            return None  # Phone/player does not advertise Cover Art.
        address = (await self.properties('org.bluez', device, 'org.bluez.Device1'))['Address']
        if address != self.address:
            await self.close()
            reply = await self.call('org.bluez.obex', '/org/bluez/obex',
                'org.bluez.obex.Client1', 'CreateSession', 'sa{sv}',
                [address, {'Target': Variant('s', 'bip-avrcp'), 'PSM': Variant('q', port)}])
            self.session, self.address = reply[0], address
        # ImgHandle becomes valid only AFTER the OBEX session is established.
        handle = ''
        for _ in range(20):
            properties = await self.properties('org.bluez', player, 'org.bluez.MediaPlayer1')
            track = properties.get('Track', {})
            value = track.get('ImgHandle')
            handle = value.value if value else ''
            if handle:
                break
            await asyncio.sleep(0.25)
        if not handle:
            return None
        target = self.directory / 'bluetooth-download'
        target.unlink(missing_ok=True)
        reply = await self.call('org.bluez.obex', self.session, 'org.bluez.obex.Image1',
                                'GetThumbnail', 'ss', [str(target), handle])
        transfer = reply[0]
        for _ in range(60):
            state = await self.properties('org.bluez.obex', transfer, 'org.bluez.obex.Transfer1')
            if target.exists() and target.stat().st_size > 16 * 1024 * 1024:
                await self.close()
                target.unlink(missing_ok=True)
                raise ValueError('Oversized Bluetooth artwork')
            if state.get('Status') == 'complete':
                data = target.read_bytes()
                target.unlink(missing_ok=True)
                return data
            if state.get('Status') == 'error':
                await self.close()
                raise ValueError('Bluetooth artwork transfer failed')
            await asyncio.sleep(0.25)
        await self.close()
        raise asyncio.TimeoutError('Bluetooth artwork transfer timed out')
