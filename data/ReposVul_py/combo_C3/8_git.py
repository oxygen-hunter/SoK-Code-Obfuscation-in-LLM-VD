import logging
import os.path
import re
import urllib.parse
import urllib.request
from typing import List, Optional, Tuple

from pip._vendor.packaging.version import _BaseVersion
from pip._vendor.packaging.version import parse as parse_version

from pip._internal.exceptions import BadCommand, InstallationError
from pip._internal.utils.misc import HiddenText, display_path, hide_url
from pip._internal.utils.subprocess import make_command
from pip._internal.vcs.versioncontrol import (
    AuthInfo,
    RemoteNotFoundError,
    RevOptions,
    VersionControl,
    find_path_to_setup_from_repo_root,
    vcs,
)

urlsplit = urllib.parse.urlsplit
urlunsplit = urllib.parse.urlunsplit


logger = logging.getLogger(__name__)


HASH_REGEX = re.compile('^[a-fA-F0-9]{40}$')


class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        self.pc = 0
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            getattr(self, f'op_{instr[0]}')(*instr[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a - b)

    def op_JMP(self, addr):
        self.pc = addr - 1

    def op_JZ(self, addr):
        if self.op_POP() == 0:
            self.pc = addr - 1

    def op_LOAD(self, var):
        self.op_PUSH(self.variables[var])

    def op_STORE(self, var):
        self.variables[var] = self.op_POP()

    def op_CALL(self, fn):
        fn()


def looks_like_hash(sha):
    vm = VirtualMachine()
    vm.variables = {'sha': sha, 'HASH_REGEX': HASH_REGEX}
    vm.run([
        ('CALL', lambda: vm.op_PUSH(vm.variables['HASH_REGEX'].match(vm.variables['sha']))),
        ('CALL', lambda: vm.op_PUSH(bool(vm.op_POP())))
    ])
    return vm.op_POP()


class Git(VersionControl):
    name = 'git'
    dirname = '.git'
    repo_name = 'clone'
    schemes = (
        'git+http', 'git+https', 'git+ssh', 'git+git', 'git+file',
    )
    unset_environ = ('GIT_DIR', 'GIT_WORK_TREE')
    default_arg_rev = 'HEAD'

    @staticmethod
    def get_base_rev_args(rev):
        vm = VirtualMachine()
        vm.variables = {'rev': rev}
        vm.run([
            ('CALL', lambda: vm.op_PUSH([vm.variables['rev']])),
        ])
        return vm.op_POP()

    def is_immutable_rev_checkout(self, url, dest):
        vm = VirtualMachine()
        vm.variables = {
            'url': url,
            'dest': dest,
            'self': self
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].get_url_rev_options(hide_url(vm.variables['url']))[1])),
            ('STORE', 'rev_options'),
            ('CALL', lambda: vm.op_PUSH(not vm.variables['rev_options'].rev)),
            ('JZ', 10),
            ('CALL', lambda: vm.op_PUSH(False)),
            ('JMP', 35),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].is_commit_id_equal(vm.variables['dest'], vm.variables['rev_options'].rev))),
            ('STORE', 'is_equal'),
            ('CALL', lambda: vm.op_PUSH(not vm.op_POP())),
            ('JZ', 19),
            ('CALL', lambda: vm.op_PUSH(False)),
            ('JMP', 35),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].get_revision_sha(vm.variables['dest'], vm.variables['rev_options'].rev)[0])),
            ('STORE', 'is_tag_or_branch'),
            ('CALL', lambda: vm.op_PUSH(bool(vm.op_POP()))),
            ('JZ', 29),
            ('CALL', lambda: vm.op_PUSH(False)),
            ('JMP', 35),
            ('CALL', lambda: vm.op_PUSH(True))
        ])
        return vm.op_POP()

    def get_git_version(self):
        vm = VirtualMachine()
        vm.variables = {
            'VERSION_PFX': 'git version ',
            'self': self,
            'version': ''
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(['version'], show_stdout=False, stdout_only=True))),
            ('STORE', 'version'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['version'].startswith(vm.variables['VERSION_PFX']))),
            ('JZ', 10),
            ('CALL', lambda: vm.op_PUSH(vm.variables['version'][len(vm.variables['VERSION_PFX']):].split()[0])),
            ('STORE', 'version'),
            ('JMP', 15),
            ('CALL', lambda: vm.op_PUSH('')),
            ('STORE', 'version'),
            ('CALL', lambda: vm.op_PUSH('.'.join(vm.variables['version'].split('.')[:3]))),
            ('STORE', 'version'),
            ('CALL', lambda: vm.op_PUSH(parse_version(vm.variables['version'])))
        ])
        return vm.op_POP()

    @classmethod
    def get_current_branch(cls, location):
        vm = VirtualMachine()
        vm.variables = {
            'location': location,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(
                ['symbolic-ref', '-q', 'HEAD'],
                extra_ok_returncodes=(1,),
                show_stdout=False,
                stdout_only=True,
                cwd=vm.variables['location'],
            ))),
            ('STORE', 'output'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['output'].strip())),
            ('STORE', 'ref'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['ref'].startswith('refs/heads/'))),
            ('JZ', 20),
            ('CALL', lambda: vm.op_PUSH(vm.variables['ref'][len('refs/heads/'):])),
            ('JMP', 25),
            ('CALL', lambda: vm.op_PUSH(None))
        ])
        return vm.op_POP()

    @classmethod
    def get_revision_sha(cls, dest, rev):
        vm = VirtualMachine()
        vm.variables = {
            'dest': dest,
            'rev': rev,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(
                ['show-ref', vm.variables['rev']],
                cwd=vm.variables['dest'],
                show_stdout=False,
                stdout_only=True,
                on_returncode='ignore',
            ))),
            ('STORE', 'output'),
            ('CALL', lambda: vm.op_PUSH({})),
            ('STORE', 'refs'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['output'].strip().split("\n"))),
            ('STORE', 'lines'),
            ('CALL', lambda: vm.op_PUSH(0)),
            ('STORE', 'i'),
            ('CALL', lambda: vm.op_PUSH(len(vm.variables['lines']))),
            ('STORE', 'n'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['i'] < vm.variables['n'])),
            ('JZ', 75),
            ('CALL', lambda: vm.op_PUSH(vm.variables['lines'][vm.variables['i']].rstrip("\r"))),
            ('STORE', 'line'),
            ('CALL', lambda: vm.op_PUSH(not vm.variables['line'])),
            ('JZ', 44),
            ('CALL', lambda: vm.op_PUSH(vm.variables['line'].split(" ", maxsplit=2))),
            ('STORE', 'parts'),
            ('CALL', lambda: vm.op_PUSH(len(vm.variables['parts']) == 2)),
            ('JZ', 60),
            ('CALL', lambda: vm.op_PUSH(vm.variables['parts'][0])),
            ('STORE', 'ref_sha'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['parts'][1])),
            ('STORE', 'ref_name'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['refs'][vm.variables['ref_name']])),
            ('CALL', lambda: vm.op_STORE(vm.op_POP(), vm.variables['ref_sha'])),
            ('JMP', 70),
            ('CALL', lambda: vm.op_PUSH(ValueError(f'unexpected show-ref line: {vm.variables["line"]!r}'))),
            ('CALL', lambda: vm.op_POP().raise_exception()),
            ('CALL', lambda: vm.op_PUSH(vm.variables['i'] + 1)),
            ('STORE', 'i'),
            ('JMP', 35),
            ('CALL', lambda: vm.op_PUSH(f'refs/remotes/origin/{vm.variables["rev"]}')),
            ('STORE', 'branch_ref'),
            ('CALL', lambda: vm.op_PUSH(f'refs/tags/{vm.variables["rev"]}')),
            ('STORE', 'tag_ref'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['refs'].get(vm.variables['branch_ref']))),
            ('STORE', 'sha'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['sha'] is not None)),
            ('JZ', 85),
            ('CALL', lambda: vm.op_PUSH((vm.variables['sha'], True))),
            ('JMP', 100),
            ('CALL', lambda: vm.op_PUSH(vm.variables['refs'].get(vm.variables['tag_ref']))),
            ('STORE', 'sha'),
            ('CALL', lambda: vm.op_PUSH((vm.variables['sha'], False)))
        ])
        return vm.op_POP()

    @classmethod
    def _should_fetch(cls, dest, rev):
        vm = VirtualMachine()
        vm.variables = {
            'dest': dest,
            'rev': rev,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['rev'].startswith("refs/"))),
            ('JZ', 5),
            ('CALL', lambda: vm.op_PUSH(True)),
            ('JMP', 40),
            ('CALL', lambda: vm.op_PUSH(not looks_like_hash(vm.variables['rev']))),
            ('JZ', 11),
            ('CALL', lambda: vm.op_PUSH(False)),
            ('JMP', 40),
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].has_commit(vm.variables['dest'], vm.variables['rev']))),
            ('JZ', 17),
            ('CALL', lambda: vm.op_PUSH(False)),
            ('JMP', 40),
            ('CALL', lambda: vm.op_PUSH(True))
        ])
        return vm.op_POP()

    @classmethod
    def resolve_revision(cls, dest, url, rev_options):
        vm = VirtualMachine()
        vm.variables = {
            'dest': dest,
            'url': url,
            'rev_options': rev_options,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['rev_options'].arg_rev)),
            ('STORE', 'rev'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].get_revision_sha(vm.variables['dest'], vm.variables['rev']))),
            ('STORE', 'sha_is_branch'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['sha_is_branch'][0] is not None)),
            ('JZ', 25),
            ('CALL', lambda: vm.op_PUSH(vm.variables['rev_options'].make_new(vm.variables['sha_is_branch'][0]))),
            ('STORE', 'new_rev_options'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['sha_is_branch'][1])),
            ('JZ', 20),
            ('CALL', lambda: vm.op_PUSH(vm.variables['rev'])),
            ('STORE', 'new_rev_options.branch_name'),
            ('JMP', 23),
            ('CALL', lambda: vm.op_PUSH(None)),
            ('STORE', 'new_rev_options.branch_name'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['new_rev_options'])),
            ('JMP', 70),
            ('CALL', lambda: vm.op_PUSH(not looks_like_hash(vm.variables['rev']))),
            ('JZ', 35),
            ('CALL', lambda: vm.op_PUSH(logger.warning(
                "Did not find branch or tag '%s', assuming revision or ref.",
                vm.variables['rev'],
            ))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(not vm.variables['cls']._should_fetch(vm.variables['dest'], vm.variables['rev']))),
            ('JZ', 48),
            ('CALL', lambda: vm.op_PUSH(vm.variables['rev_options'])),
            ('JMP', 70),
            ('CALL', lambda: vm.op_PUSH(make_command('fetch', '-q', vm.variables['url'], vm.variables['rev_options'].to_args()))),
            ('STORE', 'fetch_cmd'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(vm.variables['fetch_cmd'], cwd=vm.variables['dest']))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].get_revision(vm.variables['dest'], rev='FETCH_HEAD'))),
            ('STORE', 'sha'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['rev_options'].make_new(vm.variables['sha'])))
        ])
        return vm.op_POP()

    @classmethod
    def is_commit_id_equal(cls, dest, name):
        vm = VirtualMachine()
        vm.variables = {
            'dest': dest,
            'name': name,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(not vm.variables['name'])),
            ('JZ', 5),
            ('CALL', lambda: vm.op_PUSH(False)),
            ('JMP', 15),
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].get_revision(vm.variables['dest']) == vm.variables['name']))
        ])
        return vm.op_POP()

    def fetch_new(self, dest, url, rev_options):
        vm = VirtualMachine()
        vm.variables = {
            'dest': dest,
            'url': url,
            'rev_options': rev_options,
            'self': self
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(rev_options.to_display())),
            ('STORE', 'rev_display'),
            ('CALL', lambda: vm.op_PUSH(logger.info('Cloning %s%s to %s', vm.variables['url'], vm.variables['rev_display'], display_path(vm.variables['dest'])))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(make_command('clone', '-q', vm.variables['url'], vm.variables['dest']))),
            ('STORE', 'clone_cmd'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(vm.variables['clone_cmd']))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(rev_options.rev)),
            ('JZ', 52),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].resolve_revision(vm.variables['dest'], vm.variables['url'], vm.variables['rev_options']))),
            ('STORE', 'resolved_rev_options'),
            ('CALL', lambda: vm.op_PUSH(hasattr(vm.variables['resolved_rev_options'], 'branch_name'))),
            ('JZ', 39),
            ('CALL', lambda: vm.op_PUSH(getattr(vm.variables['resolved_rev_options'], 'branch_name'))),
            ('STORE', 'branch_name'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['branch_name'] is None)),
            ('JZ', 43),
            ('CALL', lambda: vm.op_PUSH(not vm.variables['self'].is_commit_id_equal(vm.variables['dest'], vm.variables['resolved_rev_options'].rev))),
            ('JZ', 48),
            ('CALL', lambda: vm.op_PUSH(make_command('checkout', '-q', vm.variables['resolved_rev_options'].to_args()))),
            ('STORE', 'checkout_cmd'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(vm.variables['checkout_cmd'], cwd=vm.variables['dest']))),
            ('CALL', lambda: vm.op_POP()),
            ('JMP', 52),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].get_current_branch(vm.variables['dest']) != vm.variables['branch_name'])),
            ('JZ', 52),
            ('CALL', lambda: vm.op_PUSH(f'origin/{vm.variables["branch_name"]}')),
            ('STORE', 'track_branch'),
            ('CALL', lambda: vm.op_PUSH(make_command('checkout', '-b', vm.variables['branch_name'], '--track', vm.variables['track_branch']))),
            ('STORE', 'checkout_cmd'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(vm.variables['checkout_cmd'], cwd=vm.variables['dest']))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].update_submodules(vm.variables['dest'])))
        ])

    def switch(self, dest, url, rev_options):
        vm = VirtualMachine()
        vm.variables = {
            'dest': dest,
            'url': url,
            'rev_options': rev_options,
            'self': self
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(make_command('config', 'remote.origin.url', vm.variables['url']))),
            ('STORE', 'config_cmd'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(vm.variables['config_cmd'], cwd=vm.variables['dest']))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(make_command('checkout', '-q', vm.variables['rev_options'].to_args()))),
            ('STORE', 'checkout_cmd'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(vm.variables['checkout_cmd'], cwd=vm.variables['dest']))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].update_submodules(vm.variables['dest'])))
        ])

    def update(self, dest, url, rev_options):
        vm = VirtualMachine()
        vm.variables = {
            'dest': dest,
            'url': url,
            'rev_options': rev_options,
            'self': self
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].get_git_version() >= parse_version('1.9.0'))),
            ('JZ', 5),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(['fetch', '-q', '--tags'], cwd=vm.variables['dest']))),
            ('JMP', 10),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(['fetch', '-q'], cwd=vm.variables['dest']))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].resolve_revision(vm.variables['dest'], vm.variables['url'], vm.variables['rev_options']))),
            ('STORE', 'resolved_rev_options'),
            ('CALL', lambda: vm.op_PUSH(make_command('reset', '--hard', '-q', vm.variables['resolved_rev_options'].to_args()))),
            ('STORE', 'reset_cmd'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].run_command(vm.variables['reset_cmd'], cwd=vm.variables['dest']))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(vm.variables['self'].update_submodules(vm.variables['dest'])))
        ])

    @classmethod
    def get_remote_url(cls, location):
        vm = VirtualMachine()
        vm.variables = {
            'location': location,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(
                ['config', '--get-regexp', r'remote\..*\.url'],
                extra_ok_returncodes=(1,),
                show_stdout=False,
                stdout_only=True,
                cwd=vm.variables['location'],
            ))),
            ('STORE', 'stdout'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['stdout'].splitlines())),
            ('STORE', 'remotes'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['remotes'][0])),
            ('STORE', 'found_remote'),
            ('CALL', lambda: vm.op_PUSH(len(vm.variables['remotes']) == 0)),
            ('JZ', 20),
            ('CALL', lambda: vm.op_PUSH(RemoteNotFoundError())),
            ('CALL', lambda: vm.op_POP().raise_exception()),
            ('CALL', lambda: vm.op_PUSH(0)),
            ('STORE', 'i'),
            ('CALL', lambda: vm.op_PUSH(len(vm.variables['remotes']))),
            ('STORE', 'n'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['i'] < vm.variables['n'])),
            ('JZ', 40),
            ('CALL', lambda: vm.op_PUSH(vm.variables['remotes'][vm.variables['i']])),
            ('STORE', 'remote'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['remote'].startswith('remote.origin.url '))),
            ('JZ', 36),
            ('CALL', lambda: vm.op_PUSH(vm.variables['remote'])),
            ('STORE', 'found_remote'),
            ('JMP', 40),
            ('CALL', lambda: vm.op_PUSH(vm.variables['i'] + 1)),
            ('STORE', 'i'),
            ('JMP', 31),
            ('CALL', lambda: vm.op_PUSH(vm.variables['found_remote'].split(' ')[1])),
            ('STORE', 'url'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url'].strip()))
        ])
        return vm.op_POP()

    @classmethod
    def has_commit(cls, location, rev):
        vm = VirtualMachine()
        vm.variables = {
            'location': location,
            'rev': rev,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(
                ['rev-parse', '-q', '--verify', "sha^" + vm.variables['rev']],
                cwd=vm.variables['location'],
                log_failed_cmd=False,
            ))),
            ('JZ', 15),
            ('CALL', lambda: vm.op_PUSH(True)),
            ('JMP', 20),
            ('CALL', lambda: vm.op_PUSH(False))
        ])
        return vm.op_POP()

    @classmethod
    def get_revision(cls, location, rev=None):
        vm = VirtualMachine()
        vm.variables = {
            'location': location,
            'rev': rev,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['rev'] is None)),
            ('JZ', 5),
            ('CALL', lambda: vm.op_PUSH('HEAD')),
            ('STORE', 'rev'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(
                ['rev-parse', vm.variables['rev']],
                show_stdout=False,
                stdout_only=True,
                cwd=vm.variables['location'],
            ))),
            ('STORE', 'current_rev'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['current_rev'].strip()))
        ])
        return vm.op_POP()

    @classmethod
    def get_subdirectory(cls, location):
        vm = VirtualMachine()
        vm.variables = {
            'location': location,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(
                ['rev-parse', '--git-dir'],
                show_stdout=False,
                stdout_only=True,
                cwd=vm.variables['location'],
            ))),
            ('STORE', 'git_dir'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['git_dir'].strip())),
            ('STORE', 'git_dir'),
            ('CALL', lambda: vm.op_PUSH(os.path.isabs(vm.variables['git_dir']))),
            ('JZ', 10),
            ('CALL', lambda: vm.op_PUSH(vm.variables['git_dir'])),
            ('JMP', 13),
            ('CALL', lambda: vm.op_PUSH(os.path.join(vm.variables['location'], vm.variables['git_dir']))),
            ('STORE', 'git_dir'),
            ('CALL', lambda: vm.op_PUSH(os.path.abspath(os.path.join(vm.variables['git_dir'], '..')))),
            ('STORE', 'repo_root'),
            ('CALL', lambda: vm.op_PUSH(find_path_to_setup_from_repo_root(vm.variables['location'], vm.variables['repo_root'])))
        ])
        return vm.op_POP()

    @classmethod
    def get_url_rev_and_auth(cls, url):
        vm = VirtualMachine()
        vm.variables = {
            'url': url,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(urlsplit(vm.variables['url']))),
            ('STORE', 'parsed_url'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['parsed_url'].scheme.endswith('file'))),
            ('JZ', 15),
            ('CALL', lambda: vm.op_PUSH(vm.variables['parsed_url'].path[:-len(vm.variables['parsed_url'].path.lstrip('/'))])),
            ('STORE', 'initial_slashes'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['initial_slashes'] + urllib.request.url2pathname(vm.variables['parsed_url'].path).replace('\\', '/').lstrip('/'))),
            ('STORE', 'newpath'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['parsed_url'].scheme.find('+') + 1)),
            ('STORE', 'after_plus'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['parsed_url'].scheme[:vm.variables['after_plus']] + urlunsplit(
                (vm.variables['parsed_url'].scheme[vm.variables['after_plus']:], vm.variables['parsed_url'].netloc, vm.variables['newpath'], vm.variables['parsed_url'].query, vm.variables['parsed_url'].fragment),
            ))),
            ('STORE', 'url'),
            ('CALL', lambda: vm.op_PUSH('://' in vm.variables['url'])),
            ('JZ', 55),
            ('CALL', lambda: vm.op_PUSH(super().get_url_rev_and_auth(vm.variables['url']))),
            ('STORE', 'url_rev_auth'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url_rev_auth'][0])),
            ('STORE', 'url'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url_rev_auth'][1])),
            ('STORE', 'rev'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url_rev_auth'][2])),
            ('STORE', 'user_pass'),
            ('JMP', 70),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url'].replace('git+', 'git+ssh://'))),
            ('STORE', 'url'),
            ('CALL', lambda: vm.op_PUSH(super().get_url_rev_and_auth(vm.variables['url']))),
            ('STORE', 'url_rev_auth'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url_rev_auth'][0])),
            ('STORE', 'url'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url'].replace('ssh://', ''))),
            ('STORE', 'url'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url_rev_auth'][1])),
            ('STORE', 'rev'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['url_rev_auth'][2])),
            ('STORE', 'user_pass'),
            ('CALL', lambda: vm.op_PUSH((vm.variables['url'], vm.variables['rev'], vm.variables['user_pass'])))
        ])
        return vm.op_POP()

    @classmethod
    def update_submodules(cls, location):
        vm = VirtualMachine()
        vm.variables = {
            'location': location,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(os.path.exists(os.path.join(vm.variables['location'], '.gitmodules')))),
            ('JZ', 10),
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(
                ['submodule', 'update', '--init', '--recursive', '-q'],
                cwd=vm.variables['location'],
            ))),
            ('CALL', lambda: vm.op_POP())
        ])

    @classmethod
    def get_repository_root(cls, location):
        vm = VirtualMachine()
        vm.variables = {
            'location': location,
            'cls': cls
        }
        vm.run([
            ('CALL', lambda: vm.op_PUSH(super().get_repository_root(vm.variables['location']))),
            ('STORE', 'loc'),
            ('CALL', lambda: vm.op_PUSH(vm.variables['loc'])),
            ('JZ', 15),
            ('CALL', lambda: vm.op_PUSH(vm.variables['loc'])),
            ('JMP', 65),
            ('CALL', lambda: vm.op_PUSH(vm.variables['cls'].run_command(
                ['rev-parse', '--show-toplevel'],
                cwd=vm.variables['location'],
                show_stdout=False,
                stdout_only=True,
                on_returncode='raise',
                log_failed_cmd=False,
            ))),
            ('STORE', 'r'),
            ('CALL', lambda: vm.op_PUSH(BadCommand())),
            ('CALL', lambda: vm.op_POP().raise_exception()),
            ('CALL', lambda: vm.op_PUSH(logger.debug("could not determine if %s is under git control because git is not available", vm.variables['location']))),
            ('CALL', lambda: vm.op_POP()),
            ('CALL', lambda: vm.op_PUSH(InstallationError())),
            ('CALL', lambda: vm.op_POP().raise_exception()),
            ('CALL', lambda: vm.op_PUSH(os.path.normpath(vm.variables['r'].rstrip('\r\n'))))
        ])
        return vm.op_POP()


vcs.register(Git)