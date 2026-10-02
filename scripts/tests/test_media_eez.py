"""EEZ contract and asynchronous Wi-Fi adapter; never access a real radio."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import AsyncMock

ROOT = Path(__file__).resolve().parents[2]
MODULE = ROOT / 'oem/a333/media-rootfs/usr/libexec/a333_media_network.py'
spec = importlib.util.spec_from_file_location('a333_media_network', MODULE)
network = importlib.util.module_from_spec(spec)
spec.loader.exec_module(network)


class NetworkTest(unittest.IsolatedAsyncioTestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.controller = network.NetworkController(self.directory.name, None)
        self.controller.nmcli = AsyncMock(return_value='')

    def scanned(self):
        self.controller.networks = [dict(ssid='Home:WiFi', signal=70,
            security='WPA2', device='wlan0', bssid='aa:bb:cc:dd:ee:ff')]
        return dict(command='wifi_connect', revision=self.controller.revision,
                    index=0, password='very-secret')

    def test_escaped_nmcli_fields(self):
        self.assertEqual(network.split_nmcli(r'Home\:WiFi:a\\b:70'),
                         ['Home:WiFi', 'a\\b', '70'])

    async def test_scan_deduplicates_and_prefers_wlan(self):
        self.controller.nmcli.side_effect = ['',
            r'Home\:WiFi:99:WPA2:p2p0:aa\:bb\:cc\:dd\:ee\:01' + '\n' +
            r'Home\:WiFi:40:WPA2:wlan0:aa\:bb\:cc\:dd\:ee\:02' + '\n' +
            r'Guest:60:--:wlan0:aa\:bb\:cc\:dd\:ee\:03' + '\n' +
            'Invalid:xx:WPA2:wlan0:bad\n']
        previous = self.controller.revision
        await self.controller.scan()
        self.assertGreater(self.controller.revision, previous)
        self.assertEqual([r['ssid'] for r in self.controller.networks], ['Guest', 'Home:WiFi'])
        self.assertEqual(self.controller.networks[1]['device'], 'wlan0')
        self.assertEqual(self.controller.networks[0]['security'], '')

    async def test_connect_uses_validated_bssid_and_clears_secret(self):
        request = self.scanned()
        await self.controller.connect(request)
        arguments = self.controller.nmcli.call_args.args
        self.assertIn('aa:bb:cc:dd:ee:ff', arguments)
        self.assertNotIn('Home:WiFi', arguments)
        self.assertNotIn('password', request)
        self.assertNotIn('very-secret', (Path(self.directory.name) / 'network').read_text())

    async def test_stale_selection_never_connects(self):
        request = self.scanned()
        request['revision'] -= 1
        with self.assertRaises(ValueError):
            await self.controller.connect(request)
        self.controller.nmcli.assert_not_called()

    async def test_failed_connect_never_publishes_password(self):
        request = self.scanned()
        self.controller.nmcli.side_effect = RuntimeError('very-secret wrong password')
        await self.controller.execute(request)
        text = (Path(self.directory.name) / 'network').read_text()
        self.assertNotIn('very-secret', text)
        self.assertIn('operation failed', text)
        self.assertFalse(self.controller.busy)
        self.assertNotIn('password', request)

    async def test_invalid_password_and_index(self):
        for key, value in [('index', True), ('index', -1), ('password', 'x' * 129),
                           ('password', '\x00'), ('password', 123)]:
            request = self.scanned()
            request[key] = value
            with self.subTest(key=key, value=value):
                with self.assertRaises(ValueError):
                    await self.controller.connect(request)
        self.controller.nmcli.assert_not_called()

    async def test_enterprise_profile_requires_nmcli(self):
        request = self.scanned()
        self.controller.networks[0]['security'] = 'WPA2 802.1X'
        with self.assertRaisesRegex(ValueError, 'Enterprise'):
            await self.controller.connect(request)
        self.controller.nmcli.assert_not_called()

    async def test_connected_state_includes_vendor_p2p_station(self):
        self.controller.nmcli.return_value = 'p2p0:wifi:connected\nwlan0:wifi:disconnected\nend0:ethernet:connected\n'
        await self.controller.refresh()
        self.assertTrue(self.controller.wifi)
        self.assertTrue(self.controller.ethernet)
        self.assertFalse(self.controller.bluetooth)

    def test_restart_invalidates_revision_and_snapshot_sanitizes_lines(self):
        other = network.NetworkController(self.directory.name, None)
        self.assertNotEqual(self.controller.revision, other.revision)
        self.controller.message = 'Error\nwith\rnewlines'
        self.controller.publish()
        self.assertEqual(len((Path(self.directory.name) / 'network').read_text().splitlines()), 6)

    def test_invalid_messages_are_ignored(self):
        for message in (b'broken', b'[]', b'{"command":"erase"}', b'\xff'):
            self.controller.handle(message)
        self.assertIsNone(self.controller.operation)


class ProjectTest(unittest.TestCase):
    def test_project_has_flow_network_controls_and_light_icons(self):
        data = json.loads((ROOT / 'oem/a333/src/media-panel/eez/lvgl-media-player.eez-project').read_text())
        self.assertTrue(data['settings']['general']['flowSupport'])
        self.assertEqual(data['settings']['general']['colorBpp'], '32')
        arrays = {v['name']: v for v in data['variables']['globalVariables']}
        self.assertFalse(arrays['wifi_networks']['native'])
        self.assertFalse(arrays['wifi_password']['persistent'])
        actions = {a['name'] for a in data['actions']}
        self.assertTrue({'wifi_scan', 'wifi_connect', 'toggle', 'music', 'radio', 'pair'} <= actions)
        for page in data['userPages']:
            root = page['components'][0]
            self.assertEqual((root['width'], root['height']), (1280, 800))
            bar = root['children'][0]
            icons = [w for w in bar['children'] if w['type'] == 'LVGLImageWidget']
            self.assertEqual(len(icons), 6)
            for icon in icons:
                expected = '#F5F8FF' if icon['identifier'].endswith('_on') else '#AEBBC8'
                self.assertEqual(icon['localStyles']['definition']['MAIN']['DEFAULT']['img_recolor'], expected)
            self.assertTrue(page['connectionLines'])
            for component in page['components']:
                if component['type'] == 'LVGLActionComponent':
                    for action in component['actions']:
                        if action['action'] == 'CHANGE_SCREEN':
                            expected = 'OVER_RIGHT' if action['screen'] == 'Main' else 'OVER_LEFT'
                            self.assertEqual(action['fadeMode'], expected)
                            self.assertEqual(action['speed'], 300)
