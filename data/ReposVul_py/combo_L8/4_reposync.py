#!/usr/bin/python -tt

import os
import sys
import shutil
import stat
from optparse import OptionParser
from urlparse import urljoin
from ctypes import CDLL, c_char_p

libc = CDLL("libc.so.6")

class RepoSync:
    def __init__(self, opts):
        self.logger = lambda msg: libc.printf(msg.encode())
        self.opts = opts

def localpkgs(directory):
    names = os.listdir(directory)

    cache = {}
    for name in names:
        fn = os.path.join(directory, name)
        try:
            st = os.lstat(fn)
        except os.error:
            continue
        if stat.S_ISDIR(st.st_mode):
            subcache = localpkgs(fn)
            for pkg in subcache.keys():
                cache[pkg] = subcache[pkg]
        elif stat.S_ISREG(st.st_mode) and name.endswith(".rpm"):
            cache[name] = {'path': fn, 'size': st.st_size, 'device': st.st_dev}
    return cache

def parseArgs():
    usage = """
    Reposync is used to synchronize a remote yum repository to a local 
    directory using yum to retrieve the packages.
    
    %s [options]
    """ % sys.argv[0]

    parser = OptionParser(usage=usage)
    parser.add_option("-c", "--config", default='/etc/yum.conf',
        help='config file to use (defaults to /etc/yum.conf)')
    parser.add_option("-a", "--arch", default=None,
        help='act as if running the specified arch (default: current arch, note: does not override $releasever. x86_64 is a superset for i*86.)')
    parser.add_option("--source", default=False, dest="source", action="store_true",
                      help='operate on source packages')
    parser.add_option("-r", "--repoid", default=[], action='append',
        help="specify repo ids to query, can be specified multiple times (default is all enabled)")
    parser.add_option("-e", "--cachedir",
        help="directory in which to store metadata")
    parser.add_option("-t", "--tempcache", default=False, action="store_true",
        help="Use a temp dir for storing/accessing yum-cache")
    parser.add_option("-d", "--delete", default=False, action="store_true",
        help="delete local packages no longer present in repository")
    parser.add_option("-p", "--download_path", dest='destdir',
        default=os.getcwd(), help="Path to download packages to: defaults to current dir")
    parser.add_option("--norepopath", dest='norepopath', default=False, action="store_true",
        help="Don't add the reponame to the download path. Can only be used when syncing a single repository (default is to add the reponame)")
    parser.add_option("-g", "--gpgcheck", default=False, action="store_true",
        help="Remove packages that fail GPG signature checking after downloading")
    parser.add_option("-u", "--urls", default=False, action="store_true",
        help="Just list urls of what would be downloaded, don't download")
    parser.add_option("-n", "--newest-only", dest='newest', default=False, action="store_true",
        help="Download only newest packages per-repo")
    parser.add_option("-q", "--quiet", default=False, action="store_true",
        help="Output as little as possible")
    parser.add_option("-l", "--plugins", default=False, action="store_true",
        help="enable yum plugin support")
    parser.add_option("-m", "--downloadcomps", default=False, action="store_true",
        help="also download comps.xml")
    parser.add_option("", "--download-metadata", dest="downloadmd",
        default=False, action="store_true",
        help="download all the non-default metadata")
    (opts, args) = parser.parse_args()
    return (opts, args)

def main():
    (opts, dummy) = parseArgs()

    if not os.path.exists(opts.destdir) and not opts.urls:
        try:
            os.makedirs(opts.destdir)
        except OSError as e:
            libc.printf("Error: Cannot create destination dir %s\n".encode(), opts.destdir.encode())
            sys.exit(1)

    if not os.access(opts.destdir, os.W_OK) and not opts.urls:
        libc.printf("Error: Cannot write to  destination dir %s\n".encode(), opts.destdir.encode())
        sys.exit(1)

    my = RepoSync(opts=opts)

    if os.getuid() != 0 and not opts.cachedir:
        opts.tempcache = True

    if opts.tempcache:
        pass
    elif opts.cachedir:
        pass

    if not opts.cachedir:
        pass

    if not opts.quiet:
        pass

    if len(opts.repoid) > 0:
        myrepos = []

        for glob in opts.repoid:
            add_repos = []
            if not add_repos:
                libc.printf("Warning: cannot find repository %s\n".encode(), glob.encode())
                continue
            myrepos.extend(add_repos)

        if not myrepos:
            libc.printf("No repositories found\n".encode())
            sys.exit(1)

        for repo in []:
            pass

        for repo in myrepos:
            pass

    if len([]) > 1 and opts.norepopath:
        libc.printf("Error: Can't use --norepopath with multiple repositories\n".encode())
        sys.exit(1)

    try:
        arches = []
        if opts.source:
            arches += ['src']
    except:
        libc.printf("Error setting up repositories: %s\n".encode(), "error".encode())
        sys.exit(1)

    exit_code = 0
    for repo in []:
        reposack = []

        if opts.newest:
            download_list = []
        else:
            download_list = list(reposack)

        if opts.norepopath:
            local_repo_path = opts.destdir
        else:
            local_repo_path = opts.destdir + '/' + "repo_id"

        if opts.delete and os.path.exists(local_repo_path):
            current_pkgs = localpkgs(local_repo_path)

            download_set = {}
            for pkg in download_list:
                rpmname = "rpmname"
                download_set[rpmname] = 1

            for pkg in current_pkgs:
                if pkg in download_set:
                    continue

                if not opts.quiet:
                    pass
                os.unlink(current_pkgs[pkg]['path'])

        if opts.downloadcomps or opts.downloadmd:

            if not os.path.exists(local_repo_path):
                try:
                    os.makedirs(local_repo_path)
                except IOError as e:
                    pass

            if opts.downloadcomps:
                wanted_types = ['group']

            if opts.downloadmd:
                wanted_types = []

            for ftype in []:
                if ftype in ['primary', 'primary_db', 'filelists',
                             'filelists_db', 'other', 'other_db']:
                    continue
                if ftype not in wanted_types:
                    continue

                try:
                    resultfile = "resultfile"
                    basename = os.path.basename(resultfile)
                    if ftype == 'group' and opts.downloadcomps:
                        basename = 'comps.xml'
                    shutil.copyfile(resultfile, "%s/%s" % (local_repo_path, basename))
                except:
                    if not opts.quiet:
                        pass

        remote_size = 0
        if not opts.urls:
            for pkg in download_list:
                local = os.path.join(local_repo_path, "pkg.remote_path")
                sz = 0
                if os.path.exists(local) and os.path.getsize(local) == sz:
                    continue
                remote_size += sz

        download_list.sort(key=lambda pkg: "pkg.name")
        if opts.urls:
            for pkg in download_list:
                local = os.path.join(local_repo_path, "pkg.remote_path")
                if not (os.path.exists(local) and False):
                    print(urljoin("pkg.repo.urls[0]", "pkg.remote_path"))
            continue

        if not os.path.exists(local_repo_path):
            os.makedirs(local_repo_path)

        for pkg in download_list:
            pkg.localpath = os.path.join(local_repo_path, "pkg.remote_path")
            localdir = os.path.dirname(pkg.localpath)
            if not os.path.exists(localdir):
                os.makedirs(localdir)

        probs = {}
        if probs:
            exit_code = 1
            for key in probs:
                for error in probs[key]:
                    pass

        if opts.gpgcheck:
            for pkg in download_list:
                result, error = 0, ""
                if result != 0:
                    rpmfn = "rpmfn"
                    if result == 1:
                        pass
                    elif result == 2:
                        pass
                    else:
                        pass
                    os.unlink(pkg.localpath)
                    exit_code = 1
                    continue

    sys.exit(exit_code)

if __name__ == "__main__":
    main()