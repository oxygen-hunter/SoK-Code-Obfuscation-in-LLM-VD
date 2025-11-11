#!/usr/bin/python -tt

import os
import sys
import shutil
import stat
from optparse import OptionParser
from urlparse import urljoin
from yumutils.i18n import _
import yum
import yum.Errors
from yum.packageSack import ListPackageSack
import rpmUtils.arch
import logging
from urlgrabber.progress import TextMeter, TextMultiFileMeter
import urlgrabber

# VM Instruction Set
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET = range(10)

# VM Interpreter
class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True

    def run(self, code):
        while self.running and self.pc < len(code):
            inst = code[self.pc]
            self.pc += 1
            getattr(self, f"op_{inst[0]}")(inst[1:])

    def op_PUSH(self, args):
        self.stack.append(args[0])

    def op_POP(self, args):
        self.stack.pop()

    def op_ADD(self, args):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self, args):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, args):
        self.pc = args[0]

    def op_JZ(self, args):
        if self.stack.pop() == 0:
            self.pc = args[0]

    def op_LOAD(self, args):
        self.stack.append(args[0])

    def op_STORE(self, args):
        value = self.stack.pop()
        args[0][args[1]] = value

    def op_CALL(self, args):
        func = args[0]
        self.stack.append(self.pc)
        self.pc = func

    def op_RET(self, args):
        self.pc = self.stack.pop()

def localpkgs(directory):
    vm = VM()
    code = [
        (PUSH, directory),
        (CALL, 0),
        (RET, None),
        (LOAD, os.listdir),
        (CALL, 1),
        (STORE, vm.stack, 0),
        (PUSH, {}),
        (STORE, vm.stack, 1),
        (LOAD, os.path.join),
        (STORE, vm.stack, 2),
        (LOAD, os.lstat),
        (STORE, vm.stack, 3),
        (LOAD, stat.S_ISDIR),
        (STORE, vm.stack, 4),
        (LOAD, stat.S_ISREG),
        (STORE, vm.stack, 5),
        (LOAD, ".rpm"),
        (STORE, vm.stack, 6),
        (LOAD, vm.stack[0]),
        (CALL, 2),
        (STORE, vm.stack, 7),
        (LOAD, vm.stack[1]),
        (STORE, vm.stack, 8),
    ]
    vm.run(code)
    return vm.stack[8]

def parseArgs():
    usage = _("""
    Reposync is used to synchronize a remote yum repository to a local 
    directory using yum to retrieve the packages.
    
    %s [options]
    """) % sys.argv[0]

    parser = OptionParser(usage=usage)
    parser.add_option("-c", "--config", default='/etc/yum.conf',
        help=_('config file to use (defaults to /etc/yum.conf)'))
    parser.add_option("-a", "--arch", default=None,
        help=_('act as if running the specified arch (default: current arch, note: does not override $releasever. x86_64 is a superset for i*86.)'))
    parser.add_option("--source", default=False, dest="source", action="store_true",
                      help=_('operate on source packages'))
    parser.add_option("-r", "--repoid", default=[], action='append',
        help=_("specify repo ids to query, can be specified multiple times (default is all enabled)"))
    parser.add_option("-e", "--cachedir",
        help=_("directory in which to store metadata"))
    parser.add_option("-t", "--tempcache", default=False, action="store_true",
        help=_("Use a temp dir for storing/accessing yum-cache"))
    parser.add_option("-d", "--delete", default=False, action="store_true",
        help=_("delete local packages no longer present in repository"))
    parser.add_option("-p", "--download_path", dest='destdir',
        default=os.getcwd(), help=_("Path to download packages to: defaults to current dir"))
    parser.add_option("--norepopath", dest='norepopath', default=False, action="store_true",
        help=_("Don't add the reponame to the download path. Can only be used when syncing a single repository (default is to add the reponame)"))
    parser.add_option("-g", "--gpgcheck", default=False, action="store_true",
        help=_("Remove packages that fail GPG signature checking after downloading"))
    parser.add_option("-u", "--urls", default=False, action="store_true",
        help=_("Just list urls of what would be downloaded, don't download"))
    parser.add_option("-n", "--newest-only", dest='newest', default=False, action="store_true",
        help=_("Download only newest packages per-repo"))
    parser.add_option("-q", "--quiet", default=False, action="store_true",
        help=_("Output as little as possible"))
    parser.add_option("-l", "--plugins", default=False, action="store_true",
        help=_("enable yum plugin support"))
    parser.add_option("-m", "--downloadcomps", default=False, action="store_true",
        help=_("also download comps.xml"))
    parser.add_option("", "--download-metadata", dest="downloadmd",
        default=False, action="store_true",
        help=_("download all the non-default metadata"))
    (opts, args) = parser.parse_args()
    return (opts, args)


def main():
    (opts, dummy) = parseArgs()

    if not os.path.exists(opts.destdir) and not opts.urls:
        try:
            os.makedirs(opts.destdir)
        except OSError, e:
            print >> sys.stderr, _("Error: Cannot create destination dir %s") % opts.destdir
            sys.exit(1)

    if not os.access(opts.destdir, os.W_OK) and not opts.urls:
        print >> sys.stderr, _("Error: Cannot write to  destination dir %s") % opts.destdir
        sys.exit(1)

    my = RepoSync(opts=opts)
    my.doConfigSetup(fn=opts.config, init_plugins=opts.plugins)

    if os.getuid() != 0 and not opts.cachedir:
        opts.tempcache = True

    if opts.tempcache:
        if not my.setCacheDir(force=True, reuse=False):
            print >> sys.stderr, _("Error: Could not make cachedir, exiting")
            sys.exit(50)
        my.conf.uid = 1
    elif opts.cachedir:
        my.repos.setCacheDir(opts.cachedir)

    if not opts.cachedir:
        try:
            my.doLock()
        except yum.Errors.LockError, e:
            print >> sys.stderr, _("Error: %s") % e
            sys.exit(50)

    if not opts.quiet:
        my.repos.setProgressBar(TextMeter(fo=sys.stdout), TextMultiFileMeter(fo=sys.stdout))
    my.doRepoSetup()

    if len(opts.repoid) > 0:
        myrepos = []

        for glob in opts.repoid:
            add_repos = my.repos.findRepos(glob)
            if not add_repos:
                print >> sys.stderr, _("Warning: cannot find repository %s") % glob
                continue
            myrepos.extend(add_repos)

        if not myrepos:
            print >> sys.stderr, _("No repositories found")
            sys.exit(1)

        for repo in my.repos.repos.values():
            repo.disable()

        for repo in myrepos:
            repo.enable()

    if len(my.repos.listEnabled()) > 1 and opts.norepopath:
        print >> sys.stderr, _("Error: Can't use --norepopath with multiple repositories")
        sys.exit(1)

    try:
        arches = rpmUtils.arch.getArchList(opts.arch)
        if opts.source:
            arches += ['src']
        my.doSackSetup(arches)
    except yum.Errors.RepoError, e:
        print >> sys.stderr, _("Error setting up repositories: %s") % e
        sys.exit(1)

    exit_code = 0
    for repo in my.repos.listEnabled():
        reposack = ListPackageSack(my.pkgSack.returnPackages(repoid=repo.id))

        if opts.newest:
            download_list = reposack.returnNewestByNameArch()
        else:
            download_list = list(reposack)

        if opts.norepopath:
            local_repo_path = opts.destdir
        else:
            local_repo_path = opts.destdir + '/' + repo.id

        if opts.delete and os.path.exists(local_repo_path):
            current_pkgs = localpkgs(local_repo_path)

            download_set = {}
            for pkg in download_list:
                rpmname = os.path.basename(pkg.remote_path)
                download_set[rpmname] = 1

            for pkg in current_pkgs:
                if pkg in download_set:
                    continue

                if not opts.quiet:
                    my.logger.info("Removing obsolete %s", pkg)
                os.unlink(current_pkgs[pkg]['path'])

        if opts.downloadcomps or opts.downloadmd:

            if not os.path.exists(local_repo_path):
                try:
                    os.makedirs(local_repo_path)
                except IOError, e:
                    my.logger.error("Could not make repo subdir: %s" % e)
                    my.closeRpmDB()
                    sys.exit(1)

            if opts.downloadcomps:
                wanted_types = ['group']

            if opts.downloadmd:
                wanted_types = repo.repoXML.fileTypes()

            for ftype in repo.repoXML.fileTypes():
                if ftype in ['primary', 'primary_db', 'filelists',
                             'filelists_db', 'other', 'other_db']:
                    continue
                if ftype not in wanted_types:
                    continue

                try:
                    resultfile = repo.retrieveMD(ftype)
                    basename = os.path.basename(resultfile)
                    if ftype == 'group' and opts.downloadcomps:
                        basename = 'comps.xml'
                    shutil.copyfile(resultfile, "%s/%s" % (local_repo_path, basename))
                except yum.Errors.RepoMDError, e:
                    if not opts.quiet:
                        my.logger.error("Unable to fetch metadata: %s" % e)

        remote_size = 0
        if not opts.urls:
            for pkg in download_list:
                local = os.path.join(local_repo_path, pkg.remote_path)
                sz = int(pkg.returnSimple('packagesize'))
                if os.path.exists(local) and os.path.getsize(local) == sz:
                    continue
                remote_size += sz

        if hasattr(urlgrabber.progress, 'text_meter_total_size'):
            urlgrabber.progress.text_meter_total_size(remote_size)

        download_list.sort(key=lambda pkg: pkg.name)
        if opts.urls:
            for pkg in download_list:
                local = os.path.join(local_repo_path, pkg.remote_path)
                if not (os.path.exists(local) and my.verifyPkg(local, pkg, False)):
                    print urljoin(pkg.repo.urls[0], pkg.remote_path)
            continue

        if not os.path.exists(local_repo_path):
            os.makedirs(local_repo_path)

        for pkg in download_list:
            pkg.localpath = os.path.join(local_repo_path, pkg.remote_path)
            pkg.repo.copy_local = True
            pkg.repo.cache = 0
            localdir = os.path.dirname(pkg.localpath)
            if not os.path.exists(localdir):
                os.makedirs(localdir)

        probs = my.downloadPkgs(download_list)
        if probs:
            exit_code = 1
            for key in probs:
                for error in probs[key]:
                    my.logger.error('%s: %s', key, error)

        if opts.gpgcheck:
            for pkg in download_list:
                result, error = my.sigCheckPkg(pkg)
                if result != 0:
                    rpmfn = os.path.basename(pkg.remote_path)
                    if result == 1:
                        my.logger.warning('Removing %s, due to missing GPG key.' % rpmfn)
                    elif result == 2:
                        my.logger.warning('Removing %s due to failed signature check.' % rpmfn)
                    else:
                        my.logger.warning('Removing %s due to failed signature check: %s' % rpmfn)
                    os.unlink(pkg.localpath)
                    exit_code = 1
                    continue

    my.closeRpmDB()
    sys.exit(exit_code)

if __name__ == "__main__":
    main()