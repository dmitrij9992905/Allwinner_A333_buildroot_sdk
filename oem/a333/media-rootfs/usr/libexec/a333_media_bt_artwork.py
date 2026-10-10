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
        return await self.thumbnail(handle, target)

    async def thumbnail(self, handle, target):
        # BlueZ removes Transfer1 immediately after emitting complete/error.
        # Subscribe BEFORE GetThumbnail, including completion before its reply.
        updates = {}
        session = self.session
        rule = ("type='signal',sender='org.bluez.obex',"
                "interface='org.freedesktop.DBus.Properties',"
                "member='PropertiesChanged',arg0='org.bluez.obex.Transfer1',"
                "path_namespace='" + session + "'")

        def changed(message):
            if (message.message_type == MessageType.SIGNAL and
                    message.interface == 'org.freedesktop.DBus.Properties' and
                    message.member == 'PropertiesChanged' and
                    message.path and message.path.startswith(session + '/') and
                    len(message.body) >= 2 and
                    message.body[0] == 'org.bluez.obex.Transfer1'):
                updates.setdefault(message.path, {}).update(
                    {key: value.value for key, value in message.body[1].items()})

        self.bus.add_message_handler(changed)
        subscribed = False
        try:
            await self.call('org.freedesktop.DBus', '/org/freedesktop/DBus',
                            'org.freedesktop.DBus', 'AddMatch', 's', [rule])
            subscribed = True
            reply = await self.call('org.bluez.obex', session, 'org.bluez.obex.Image1',
                                    'GetThumbnail', 'ss', [str(target), handle])
            transfer = reply[0]
            state = {key: value.value for key, value in reply[1].items()}
            for _ in range(60):
                state.update(updates.get(transfer, {}))
                if state.get('Status') not in ('complete', 'error'):
                    try:
                        state = await self.properties('org.bluez.obex', transfer,
                                                      'org.bluez.obex.Transfer1')
                    except ValueError:
                        if updates.get(transfer, {}).get('Status') not in ('complete', 'error'):
                            raise
                    # A completion signal can arrive while GetAll is in flight.
                    state.update(updates.get(transfer, {}))
                if target.exists() and target.stat().st_size > 16 * 1024 * 1024:
                    raise ValueError('Oversized Bluetooth artwork')
                if state.get('Status') == 'complete':
                    return target.read_bytes()
                if state.get('Status') == 'error':
                    raise ValueError('Bluetooth artwork transfer failed')
                await asyncio.sleep(0.25)
            raise asyncio.TimeoutError('Bluetooth artwork transfer timed out')
        finally:
            self.bus.remove_message_handler(changed)
            if subscribed:
                try:
                    await self.call('org.freedesktop.DBus', '/org/freedesktop/DBus',
                                    'org.freedesktop.DBus', 'RemoveMatch', 's', [rule])
                except Exception:
                    pass
