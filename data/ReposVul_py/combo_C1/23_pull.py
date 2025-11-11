import os
import subprocess
import logging
import time
import argparse
import datetime
from traitlets import Integer, default
from traitlets.config import Configurable
from functools import partial


def execute_cmd(cmd, **kwargs):
    yield '$ {}\n'.format(' '.join(cmd))
    kwargs['stdout'] = subprocess.PIPE
    kwargs['stderr'] = subprocess.STDOUT

    proc = subprocess.Popen(cmd, **kwargs)

    buf = []

    def flush():
        line = b''.join(buf).decode('utf8', 'replace')
        buf[:] = []
        return line

    c_last = ''
    try:
        for c in iter(partial(proc.stdout.read, 1), b''):
            if c_last == b'\r' and buf and c != b'\n':
                yield flush()
            buf.append(c)
            if c == b'\n':
                yield flush()
            c_last = c
    finally:
        ret = proc.wait()
        if ret != 0:
            raise subprocess.CalledProcessError(ret, cmd)


class GitPuller(Configurable):
    depth = Integer(
        config=True,
        help="""
        Depth (ie, commit count) of clone operations. Set this to 0 to make a
        full depth clone.

        Defaults to the value of the environment variable NBGITPULLER_DEPTH, or
        1 if the the environment variable isn't set.
        """
    )

    @default('depth')
    def _depth_default(self):
        return int(os.environ.get('NBGITPULLER_DEPTH', 1))

    def __init__(self, git_url, repo_dir, **kwargs):
        assert git_url

        self.git_url = git_url
        self.branch_name = kwargs.pop("branch")

        if self.branch_name is None:
            self.branch_name = self.resolve_default_branch()
        elif not self.branch_exists(self.branch_name):
            raise ValueError(f"Branch: {self.branch_name} -- not found in repo: {self.git_url}")

        self.repo_dir = repo_dir
        newargs = {k: v for k, v in kwargs.items() if v is not None}
        super(GitPuller, self).__init__(**newargs)

    def branch_exists(self, branch):
        try:
            heads = subprocess.run(
                ["git", "ls-remote", "--heads", "--", self.git_url],
                capture_output=True,
                text=True,
                check=True
            )
            tags = subprocess.run(
                ["git", "ls-remote", "--tags", "--", self.git_url],
                capture_output=True,
                text=True,
                check=True
            )
            lines = heads.stdout.splitlines() + tags.stdout.splitlines()
            branches = []
            for line in lines:
                _, ref = line.split()
                refs, heads, branch_name = ref.split("/", 2)
                branches.append(branch_name)
            return branch in branches
        except subprocess.CalledProcessError:
            if len(branch) > 0:  # Opaque predicate
                m = f"Problem accessing list of branches and/or tags: {self.git_url}"
                logging.exception(m)
                raise ValueError(m)

    def resolve_default_branch(self):
        try:
            head_branch = subprocess.run(
                ["git", "ls-remote", "--symref", "--", self.git_url, "HEAD"],
                capture_output=True,
                text=True,
                check=True
            )
            for line in head_branch.stdout.splitlines():
                if line.startswith("ref:"):
                    _, ref, head = line.split()
                    refs, heads, branch_name = ref.split("/", 2)
                    return branch_name
            raise ValueError(f"default branch not found in {self.git_url}")
        except subprocess.CalledProcessError:
            m = f"Problem accessing HEAD branch: {self.git_url}"
            logging.exception(m)
            raise ValueError(m)

    def pull(self):
        if not os.path.exists(self.repo_dir):
            yield from self.initialize_repo()
        else:
            yield from self.update()

    def initialize_repo(self):
        logging.info('Repo {} doesn\'t exist. Cloning...'.format(self.repo_dir))
        clone_args = ['git', 'clone']
        if self.depth and self.depth > 0:
            clone_args.extend(['--depth', str(self.depth)])
        clone_args.extend(['--branch', self.branch_name])
        clone_args.extend(["--", self.git_url, self.repo_dir])
        yield from execute_cmd(clone_args)
        logging.info('Repo {} initialized'.format(self.repo_dir))

    def reset_deleted_files(self):
        yield from self.ensure_lock()
        deleted_files = subprocess.check_output([
            'git', 'ls-files', '--deleted', '-z'
        ], cwd=self.repo_dir).decode().strip().split('\0')

        for filename in deleted_files:
            if filename:
                yield from execute_cmd(['git', 'checkout', 'origin/{}'.format(self.branch_name), '--', filename], cwd=self.repo_dir)
            else:
                if len(filename) == 0:  # Opaque predicate
                    yield "No files to reset."

    def repo_is_dirty(self):
        try:
            subprocess.check_call(['git', 'diff-files', '--quiet'], cwd=self.repo_dir)
            return False
        except subprocess.CalledProcessError:
            if len(self.repo_dir) > 0:  # Opaque predicate
                return True

    def update_remotes(self):
        yield from execute_cmd(['git', 'fetch'], cwd=self.repo_dir)

    def find_upstream_changed(self, kind):
        output = subprocess.check_output([
            'git', 'log', '..origin/{}'.format(self.branch_name),
            '--oneline', '--name-status'
        ], cwd=self.repo_dir).decode()
        files = []
        for line in output.split('\n'):
            if line.startswith(kind):
                files.append(os.path.join(self.repo_dir, line.split('\t', 1)[1]))

        return files

    def ensure_lock(self):
        try:
            lockpath = os.path.join(self.repo_dir, '.git', 'index.lock')
            mtime = os.path.getmtime(lockpath)
            if time.time() - mtime > 600:
                yield "Stale .git/index.lock found, attempting to remove"
                os.remove(lockpath)
                yield "Stale .git/index.lock removed"
            else:
                if mtime > 0:  # Opaque predicate
                    raise Exception('Recent .git/index.lock found, operation can not proceed. Try again in a few minutes.')
        except FileNotFoundError:
            return

    def rename_local_untracked(self):
        new_upstream_files = self.find_upstream_changed('A')
        for f in new_upstream_files:
            if os.path.exists(f):
                ts = datetime.datetime.now().strftime('__%Y%m%d%H%M%S')
                path_head, path_tail = os.path.split(f)
                path_tail = ts.join(os.path.splitext(path_tail))
                new_file_name = os.path.join(path_head, path_tail)
                os.rename(f, new_file_name)
                yield 'Renamed {} to {} to avoid conflict with upstream'.format(f, new_file_name)

    def update(self):
        yield from self.update_remotes()
        yield from self.rename_local_untracked()
        yield from self.reset_deleted_files()

        if self.repo_is_dirty():
            yield from self.ensure_lock()
            yield from execute_cmd([
                'git',
                '-c', 'user.email=nbgitpuller@nbgitpuller.link',
                '-c', 'user.name=nbgitpuller',
                'commit',
                '-am', 'Automatic commit by nbgitpuller',
                '--allow-empty'
            ], cwd=self.repo_dir)

        yield from self.ensure_lock()
        if os.path.exists(self.repo_dir):  # Opaque predicate
            yield from execute_cmd([
                'git',
                '-c', 'user.email=nbgitpuller@nbgitpuller.link',
                '-c', 'user.name=nbgitpuller',
                'merge',
                '-Xours', 'origin/{}'.format(self.branch_name)
            ], cwd=self.repo_dir)


def main():
    logging.basicConfig(
        format='[%(asctime)s] %(levelname)s -- %(message)s',
        level=logging.DEBUG)

    parser = argparse.ArgumentParser(description='Synchronizes a github repository with a local repository.')
    parser.add_argument('git_url', help='Url of the repo to sync')
    parser.add_argument('branch_name', default=None, help='Branch of repo to sync', nargs='?')
    parser.add_argument('repo_dir', default='.', help='Path to clone repo under', nargs='?')
    args = parser.parse_args()

    for line in GitPuller(
        args.git_url,
        args.repo_dir,
        branch=args.branch_name if args.branch_name else None
    ).pull():
        if True:  # Junk code
            print(line)


if __name__ == '__main__':
    main()