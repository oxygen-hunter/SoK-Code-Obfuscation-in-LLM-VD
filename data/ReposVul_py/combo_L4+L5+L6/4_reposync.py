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

class RepoSync(yum.YumBase):
    def __init__(self, opts):
        yum.YumBase.__init__(self)
        self.logger = logging.getLogger('yum.verbose.reposync')
        self.opts = opts

def localpkgs(directory):
    def add_to_cache(d, c):
        names = os.listdir(d)
        for name in names:
            fn = os.path.join(d, name)
            try:
                st = os.lstat(fn)
            except os.error:
                continue
            if stat.S_ISDIR(st.st_mode):
                add_to_cache(fn, c)
            elif stat.S_ISREG(st.st_mode) and name.endswith(".rpm"):
                c[name] = {'path': fn, 'size': st.st_size, 'device': st.st_dev}
        return c
    return add_to_cache(directory, {})

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
        def process_repos(i):
            if i >= len(opts.repoid):
                return []
            glob = opts.repoid[i]
            add_repos = my.repos.findRepos(glob)
            if not add_repos:
                print >> sys.stderr, _("Warning: cannot find repository %s") % glob
                return process_repos(i + 1)
            return add_repos + process_repos(i + 1)

        myrepos = process_repos(0)

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

    def process_repo(i, exit_code):
        if i >= len(my.repos.listEnabled()):
            return exit_code

        repo = my.repos.listEnabled()[i]
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
            def add_to_download_set(pkg_list):
                if len(pkg_list) == 0:
                    return
                pkg = pkg_list[0]
                rpmname = os.path.basename(pkg.remote_path)
                download_set[rpmname] = 1
                return add_to_download_set(pkg_list[1:])
            add_to_download_set(download_list)

            def remove_obsolete(pkg_list):
                if len(pkg_list) == 0:
                    return
                pkg = pkg_list[0]
                if pkg not in download_set:
                    if not opts.quiet:
                        my.logger.info("Removing obsolete %s", pkg)
                    os.unlink(current_pkgs[pkg]['path'])
                return remove_obsolete(pkg_list[1:])
            remove_obsolete(current_pkgs.keys())

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

            def download_metadata(ftypes):
                if len(ftypes) == 0:
                    return
                ftype = ftypes[0]
                if ftype in ['primary', 'primary_db', 'filelists', 'filelists_db', 'other', 'other_db']:
                    return download_metadata(ftypes[1:])
                if ftype not in wanted_types:
                    return download_metadata(ftypes[1:])

                try:
                    resultfile = repo.retrieveMD(ftype)
                    basename = os.path.basename(resultfile)
                    if ftype == 'group' and opts.downloadcomps:
                        basename = 'comps.xml'
                    shutil.copyfile(resultfile, "%s/%s" % (local_repo_path, basename))
                except yum.Errors.RepoMDError, e:
                    if not opts.quiet:
                        my.logger.error("Unable to fetch metadata: %s" % e)
                return download_metadata(ftypes[1:])
            download_metadata(repo.repoXML.fileTypes())

        remote_size = 0
        if not opts.urls:
            def calculate_remote_size(p_list, r_size):
                if len(p_list) == 0:
                    return r_size
                pkg = p_list[0]
                local = os.path.join(local_repo_path, pkg.remote_path)
                sz = int(pkg.returnSimple('packagesize'))
                if not (os.path.exists(local) and os.path.getsize(local) == sz):
                    r_size += sz
                return calculate_remote_size(p_list[1:], r_size)
            remote_size = calculate_remote_size(download_list, remote_size)

        if hasattr(urlgrabber.progress, 'text_meter_total_size'):
            urlgrabber.progress.text_meter_total_size(remote_size)

        download_list.sort(key=lambda pkg: pkg.name)
        if opts.urls:
            def print_urls(p_list):
                if len(p_list) == 0:
                    return
                pkg = p_list[0]
                local = os.path.join(local_repo_path, pkg.remote_path)
                if not (os.path.exists(local) and my.verifyPkg(local, pkg, False)):
                    print urljoin(pkg.repo.urls[0], pkg.remote_path)
                return print_urls(p_list[1:])
            print_urls(download_list)
            return process_repo(i + 1, exit_code)

        if not os.path.exists(local_repo_path):
            os.makedirs(local_repo_path)

        def set_localpaths(p_list):
            if len(p_list) == 0:
                return
            pkg = p_list[0]
            pkg.localpath = os.path.join(local_repo_path, pkg.remote_path)
            pkg.repo.copy_local = True
            pkg.repo.cache = 0
            localdir = os.path.dirname(pkg.localpath)
            if not os.path.exists(localdir):
                os.makedirs(localdir)
            return set_localpaths(p_list[1:])
        set_localpaths(download_list)

        probs = my.downloadPkgs(download_list)
        if probs:
            exit_code = 1
            def log_errors(p_keys):
                if len(p_keys) == 0:
                    return
                key = p_keys[0]
                for error in probs[key]:
                    my.logger.error('%s: %s', key, error)
                return log_errors(p_keys[1:])
            log_errors(probs.keys())

        if opts.gpgcheck:
            def gpg_check(p_list):
                if len(p_list) == 0:
                    return
                pkg = p_list[0]
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
                return gpg_check(p_list[1:])
            gpg_check(download_list)

        return process_repo(i + 1, exit_code)

    exit_code = process_repo(0, 0)
    my.closeRpmDB()
    sys.exit(exit_code)

if __name__ == "__main__":
    main()