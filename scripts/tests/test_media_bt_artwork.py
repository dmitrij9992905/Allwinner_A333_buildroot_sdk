"""Test BIP/OBEX calls without a Bluetooth adapter or a live D-Bus."""
import asyncio
import importlib.util
from pathlib import Path
import tempfile
from types import ModuleType, SimpleNamespace
import unittest
from unittest.mock import AsyncMock, patch

ROOT = Path(__file__).resolve().parents[2]


class Variant:
    def __init__(self, signature, value):
        self.signature, self.value = signature, value


class Message:
    def __init__(self, **fields):
        self.__dict__.update(fields)


stub = ModuleType('dbus_next')
stub.Message, stub.Variant = Message, Variant
stub.MessageType = SimpleNamespace(METHOD_RETURN=1, SIGNAL=2)
spec = importlib.util.spec_from_file_location('bt_artwork',
    ROOT / 'oem/a333/media-rootfs/usr/libexec/a333_media_bt_artwork.py')
module = importlib.util.module_from_spec(spec)
with patch.dict('sys.modules', dbus_next=stub):
    spec.loader.exec_module(module)


class Bus:
    def __init__(self):
        self.calls, self.supported, self.status, self.cancel = [], True, 'complete', False
        self.handlers = []
        self.signal_before_reply = False
        self.signal_during_poll = False
        self.removed = False
        self.cancel_transfer = False
        self.signal_path = '/session/transfer'

    def add_message_handler(self, handler):
        self.handlers.append(handler)

    def remove_message_handler(self, handler):
        self.handlers.remove(handler)

    def complete_signal(self):
        message = Message(message_type=2, interface='org.freedesktop.DBus.Properties',
                          member='PropertiesChanged', path=self.signal_path,
                          body=['org.bluez.obex.Transfer1', {'Status': Variant('s', self.status)}, []])
        for handler in self.handlers:
            handler(message)

    async def call(self, message):
        self.calls.append(message)
        if self.cancel and message.member == 'GetAll':
            raise asyncio.CancelledError()
        if message.member == 'GetAll':
            interface = message.body[0]
            if interface == 'org.bluez.MediaPlayer1':
                fields = {'Device': Variant('o', '/phone'),
                          'Track': Variant('a{sv}', {'ImgHandle': Variant('s', 'handle')})}
                if self.supported:
                    fields['ObexPort'] = Variant('q', 0x1021)
            elif interface == 'org.bluez.Device1':
                fields = {'Address': Variant('s', '00:11:22:33:44:55')}
            else:
                if self.cancel_transfer:
                    raise asyncio.CancelledError()
                if self.signal_during_poll:
                    self.complete_signal()
                    self.removed = True
                if self.removed:
                    return SimpleNamespace(message_type=3, body=['UnknownObject'])
                fields = {'Status': Variant('s', self.status)}
            body = [fields]
        elif message.member == 'CreateSession':
            body = ['/session']
        elif message.member == 'GetThumbnail':
            Path(message.body[0]).write_bytes(b'image bytes')
            if self.signal_before_reply:
                self.complete_signal()
                self.removed = True
            body = ['/session/transfer', {'Status': Variant('s', 'queued')}]
        else:
            body = []
        return SimpleNamespace(message_type=1, body=body)


class ArtworkTest(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.bus = Bus()
        self.artwork = module.BluetoothArtwork(self.bus, self.directory.name)

    async def test_session_psm_and_thumbnail(self):
        data = await self.artwork.picture('/player')
        self.assertEqual(data, b'image bytes')
        create = next(m for m in self.bus.calls if m.member == 'CreateSession')
        self.assertEqual(create.signature, 'sa{sv}')
        self.assertEqual(create.body[1]['Target'].value, 'bip-avrcp')
        self.assertEqual(create.body[1]['PSM'].signature, 'q')
        self.assertEqual(create.body[1]['PSM'].value, 0x1021)
        self.assertFalse((Path(self.directory.name) / 'bluetooth-download').exists())
        self.assertEqual(self.bus.handlers, [])
        members = [m.member for m in self.bus.calls]
        self.assertLess(members.index('AddMatch'), members.index('GetThumbnail'))
        self.assertIn('RemoveMatch', members)
        await self.artwork.picture('/player')
        self.assertEqual(sum(m.member == 'CreateSession' for m in self.bus.calls), 1)

    async def test_unsupported_phone_is_optional(self):
        self.bus.supported = False
        self.assertIsNone(await self.artwork.picture('/player'))
        self.assertFalse(any(m.member == 'CreateSession' for m in self.bus.calls))

    async def test_failed_transfer_closes_session(self):
        self.bus.status = 'error'
        with self.assertRaises(ValueError):
            await self.artwork.picture('/player')
        self.assertIsNone(self.artwork.session)
        self.assertTrue(any(m.member == 'RemoveSession' for m in self.bus.calls))
        self.assertEqual(self.bus.handlers, [])

    async def test_complete_before_thumbnail_reply(self):
        self.bus.signal_before_reply = True
        self.assertEqual(await self.artwork.picture('/player'), b'image bytes')
        self.assertEqual(self.bus.handlers, [])

    async def test_complete_and_removed_during_getall(self):
        self.bus.signal_during_poll = True
        self.assertEqual(await self.artwork.picture('/player'), b'image bytes')
        self.assertEqual(self.bus.handlers, [])
        self.assertTrue(any(m.member == 'RemoveMatch' for m in self.bus.calls))

    async def test_error_signal_and_removed_during_getall(self):
        self.bus.signal_during_poll, self.bus.status = True, 'error'
        with self.assertRaisesRegex(ValueError, 'transfer failed'):
            await self.artwork.picture('/player')
        self.assertIsNone(self.artwork.session)
        self.assertEqual(self.bus.handlers, [])

    async def test_removed_without_completion_does_not_accept_partial_file(self):
        self.bus.removed = True
        with self.assertRaisesRegex(ValueError, 'UnknownObject'):
            await self.artwork.picture('/player')
        self.assertFalse((Path(self.directory.name) / 'bluetooth-download').exists())
        self.assertEqual(self.bus.handlers, [])

    async def test_unrelated_session_signal_is_ignored(self):
        self.bus.signal_before_reply = True
        self.bus.signal_path = '/another-session/transfer'
        with self.assertRaisesRegex(ValueError, 'UnknownObject'):
            await self.artwork.picture('/player')
        self.assertEqual(self.bus.handlers, [])

    async def test_cancelled_transfer_removes_signal_subscription(self):
        self.bus.cancel_transfer = True
        with self.assertRaises(asyncio.CancelledError):
            await self.artwork.picture('/player')
        self.assertEqual(self.bus.handlers, [])
        self.assertIsNone(self.artwork.session)
        self.assertTrue(any(m.member == 'RemoveMatch' for m in self.bus.calls))

    async def test_timeout_removes_signal_subscription(self):
        self.bus.status = 'active'
        with patch.object(module.asyncio, 'sleep', new_callable=AsyncMock):
            with self.assertRaises(asyncio.TimeoutError):
                await self.artwork.picture('/player')
        self.assertEqual(self.bus.handlers, [])
        self.assertIsNone(self.artwork.session)

    async def test_cancelled_track_artwork_closes_session(self):
        self.artwork.session = '/session'
        self.artwork.address = '00:11:22:33:44:55'
        self.bus.cancel = True
        with self.assertRaises(asyncio.CancelledError):
            await self.artwork.picture('/player')
        self.assertIsNone(self.artwork.session)
        self.assertTrue(any(m.member == 'RemoveSession' for m in self.bus.calls))
