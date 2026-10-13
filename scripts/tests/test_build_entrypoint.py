"""Build entry-point routing/TTY tests: mocked Docker/make, no real builds."""
import errno
import json
import os
from pathlib import Path
import pty
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def run_terminal(command, env, cwd):
    master, slave = pty.openpty()
    try:
        process = subprocess.Popen(command, env=env, cwd=cwd,
                                   stdin=slave, stdout=slave, stderr=slave)
        os.close(slave)
        slave = None
        try:
            code = process.wait(timeout=15)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
            raise
        output = bytearray()
        while True:
            try:
                data = os.read(master, 65536)
            except OSError as error:
                if error.errno == errno.EIO:
                    break
                raise
            if not data:
                break
            output.extend(data)
        return subprocess.CompletedProcess(command, code, output.decode(), '')
    finally:
        if slave is not None:
            os.close(slave)
        os.close(master)


class EntrypointTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        for directory in ('profiles', 'configs', 'overlays', 'oem'):
            (self.root / directory).symlink_to(ROOT / directory, target_is_directory=True)
        for script in ('build.sh', 'docker-build.sh'):
            shutil.copy2(ROOT / script, self.root / script)
        (self.root / 'scripts').mkdir()
        self.toolchain = self.root / 'toolchains/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi'
        (self.toolchain / 'bin').mkdir(parents=True)
        for tool in ('gcc', 'as', 'ld', 'ar', 'nm', 'strip', 'objcopy', 'objdump'):
            path = self.toolchain / 'bin' / ('arm-linux-gnueabi-' + tool)
            path.write_text('#!/bin/sh\nexit 0\n')
            path.chmod(0o755)
        (self.root / 'buildroot').mkdir()
        self.bin = self.root / 'bin'
        self.bin.mkdir()
        self.log = self.root / 'calls.jsonl'
        fake = '''#!/usr/bin/env python3
import json, os, sys
from pathlib import Path
with open(os.environ['TEST_CALLS'], 'a') as stream:
    stream.write(json.dumps({'tool': Path(sys.argv[0]).name,
        'args': sys.argv[1:], 'cwd': os.getcwd(),
        'oem': os.environ.get('A333_OEM_OVERLAYS')}) + '\\n')
if Path(sys.argv[0]).name == 'make':
    for arg in sys.argv[1:]:
        if arg.endswith('_defconfig'):
            output = next(a[2:] for a in sys.argv[1:] if a.startswith('O='))
            folder = (Path.cwd() / output).resolve()
            folder.mkdir(parents=True, exist_ok=True)
            (folder / '.config').write_text((Path.cwd().parent / 'configs/configs' / arg).read_text())
sys.exit(int(os.environ.get('TEST_EXIT', '0')))
'''
        for tool in ('docker', 'make'):
            path = self.bin / tool
            path.write_text(fake)
            path.chmod(0o755)
        prepare = self.root / 'scripts/prepare-vendor-sources.sh'
        prepare.write_text(fake)
        prepare.chmod(0o755)
        self.env = {**os.environ, 'PATH': str(self.bin) + ':' + os.environ['PATH'],
                    'TEST_CALLS': str(self.log), 'TERM': 'xterm-256color'}
        self.env.pop('A333_DOCKER_BUILD', None)

    def calls(self):
        return [json.loads(line) for line in self.log.read_text().splitlines()] if self.log.exists() else []

    def invoke(self, *arguments, container=False, terminal=False):
        env = dict(self.env)
        if container:
            env['A333_DOCKER_BUILD'] = '1'
        command = ['bash', str(self.root / 'build.sh'), 'media', *arguments]
        if terminal:
            return run_terminal(command, env, self.root.parent)
        return subprocess.run(command, env=env, cwd=self.root.parent, input='',
                              capture_output=True, text=True, timeout=15)

    def test_all_host_actions_enter_one_container_with_identical_arguments(self):
        for action in ('configure', 'build', 'clean', 'menuconfig', 'savedefconfig'):
            with self.subTest(action=action):
                self.log.unlink(missing_ok=True)
                result = self.invoke(action, 'CUSTOM=value with spaces', terminal=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                calls = self.calls()
                self.assertEqual(len(calls), 1)
                self.assertEqual(calls[0]['tool'], 'docker')
                args = calls[0]['args']
                self.assertIn('--interactive', args)
                self.assertIn('--tty', args)
                self.assertIn('TERM=xterm-256color', args)
                self.assertIn('A333_DOCKER_BUILD=1', args)
                index = args.index('/workspace/build.sh')
                self.assertEqual(args[index + 1:], ['media', action, 'CUSTOM=value with spaces'])
                self.assertFalse((self.root / 'output').exists())

    def test_native_actions_never_start_nested_docker(self):
        for action in ('configure', 'build', 'menuconfig', 'savedefconfig', 'clean'):
            with self.subTest(action=action):
                self.log.unlink(missing_ok=True)
                result = self.invoke(action, container=True, terminal=action == 'menuconfig')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                calls = self.calls()
                self.assertTrue(calls)
                prepare_calls = [c for c in calls if c['tool'] == 'prepare-vendor-sources.sh']
                self.assertEqual(len(prepare_calls), int(action == 'build'))
                if action == 'build':
                    self.assertEqual(calls[0]['tool'], 'prepare-vendor-sources.sh')
                calls = [c for c in calls if c['tool'] == 'make']
                self.assertTrue(calls)
                expected = action if action != 'configure' else 'a333_media_defconfig'
                self.assertTrue(any(expected in c['args'] for c in calls) if action != 'build' else True)
                for call in calls:
                    self.assertEqual(call['cwd'], str(self.root / 'buildroot'))
                    self.assertIn('O=../output/profiles/media', call['args'])
                    self.assertIn('oem/a333/media-rootfs', call['oem'])

    def test_first_menuconfig_loads_defconfig_and_existing_config_is_preserved(self):
        result = self.invoke('menuconfig', container=True, terminal=True)
        self.assertEqual(result.returncode, 0, result.stdout)
        calls = self.calls()
        self.assertIn('a333_media_defconfig', calls[0]['args'])
        self.assertIn('menuconfig', calls[-1]['args'])
        config = self.root / 'output/profiles/media/.config'
        config.write_text(config.read_text() + '\nTEST_CUSTOM=y\n')
        self.log.unlink()
        result = self.invoke('menuconfig', container=True, terminal=True)
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertFalse(any('a333_media_defconfig' in c['args'] for c in self.calls()))
        self.assertIn('TEST_CUSTOM=y', config.read_text())

    def test_interactive_targets_fail_clearly_without_a_terminal(self):
        for args in (('menuconfig',), ('build', 'linux-menuconfig'), ('build', 'busybox-menuconfig')):
            result = self.invoke(*args)
            self.assertEqual(result.returncode, 2)
            self.assertIn('requires a terminal', result.stderr)
        self.assertFalse(self.calls())
        self.assertFalse((self.root / 'output').exists())

    def test_noninteractive_build_and_exit_status_are_preserved(self):
        self.env['TEST_EXIT'] = '17'
        result = self.invoke('build')
        self.assertEqual(result.returncode, 17)
        args = self.calls()[0]['args']
        self.assertNotIn('--tty', args)
        self.assertIn('--interactive', args)

    def test_failed_source_preparation_stops_before_make(self):
        self.env['TEST_EXIT'] = '17'
        result = self.invoke('build', container=True)
        self.assertEqual(result.returncode, 17)
        self.assertEqual([c['tool'] for c in self.calls()], ['prepare-vendor-sources.sh'])

    def test_missing_toolchain_stops_before_preparation_and_make(self):
        shutil.rmtree(self.toolchain)
        result = self.invoke('build', container=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn('Missing or non-executable vendor U-Boot tool:', result.stderr)
        self.assertIn('Copy the complete Linaro 7.2.1 toolchain', result.stderr)
        self.assertFalse(self.calls())
        self.assertFalse((self.root / 'output').exists())
        for action in ('configure', 'menuconfig', 'savedefconfig', 'clean'):
            result = self.invoke(action, container=True, terminal=action == 'menuconfig')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_incomplete_or_unrunnable_toolchain_stops_before_make(self):
        assembler = self.toolchain / 'bin/arm-linux-gnueabi-as'
        assembler.chmod(0o644)
        result = self.invoke('build', container=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn('Missing or non-executable vendor U-Boot tool:', result.stderr)
        self.assertFalse(self.calls())
        assembler.write_text('#!/bin/sh\nexit 23\n')
        assembler.chmod(0o755)
        result = self.invoke('build', container=True)
        self.assertEqual(result.returncode, 1)
        self.assertIn('Cannot run vendor U-Boot tool:', result.stderr)
        self.assertFalse(self.calls())

    def test_wrapper_shell_and_direct_commands_get_automatic_tty(self):
        for extra in ([], ['make', 'menuconfig'], ['prepare-sources']):
            self.log.unlink(missing_ok=True)
            result = run_terminal(['bash', str(self.root / 'docker-build.sh'), *extra],
                                  self.env, self.root.parent)
            self.assertEqual(result.returncode, 0, result.stdout)
            self.assertIn('--tty', self.calls()[0]['args'])
        self.log.unlink()
        result = subprocess.run(['bash', str(self.root / 'docker-build.sh'), 'build'],
                                env=self.env, input='', capture_output=True, text=True)
        self.assertEqual(result.returncode, 0)
        args = self.calls()[0]['args']
        self.assertEqual(args[0], 'build')
        self.assertNotIn('--interactive', args)
        self.assertNotIn('--tty', args)
