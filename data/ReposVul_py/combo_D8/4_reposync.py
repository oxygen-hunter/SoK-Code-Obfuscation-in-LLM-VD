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
    def __init__(self, rOb):
        yum.YumBase.__init__(self)
        self.logger = logging.getLogger('yum.verbose.reposync')
        self.opts = rOb

def localpkgs(d):
    n = os.listdir(d)

    c = {}
    for nm in n:
        f = os.path.join(d, nm)
        try:
            s = os.lstat(f)
        except os.error:
            continue
        if stat.S_ISDIR(s.st_mode):
            sc = localpkgs(f)
            for p in sc.keys():
                c[p] = sc[p]
        elif stat.S_ISREG(s.st_mode) and nm.endswith(".rpm"):
            c[nm] = {'path': f, 'size': s.st_size, 'device': s.st_dev}
    return c

def parseArgs():
    u = _("""
    Reposync is used to synchronize a remote yum repository to a local 
    directory using yum to retrieve the packages.
    
    %s [options]
    """) % sys.argv[0]

    p = OptionParser(usage=u)
    p.add_option("-c", "--config", default='/etc/yum.conf',
        help=_('config file to use (defaults to /etc/yum.conf)'))
    p.add_option("-a", "--arch", default=None,
        help=_('act as if running the specified arch (default: current arch, note: does not override $releasever. x86_64 is a superset for i*86.)'))
    p.add_option("--source", default=False, dest="source", action="store_true",
                      help=_('operate on source packages'))
    p.add_option("-r", "--repoid", default=[], action='append',
        help=_("specify repo ids to query, can be specified multiple times (default is all enabled)"))
    p.add_option("-e", "--cachedir",
        help=_("directory in which to store metadata"))
    p.add_option("-t", "--tempcache", default=False, action="store_true",
        help=_("Use a temp dir for storing/accessing yum-cache"))
    p.add_option("-d", "--delete", default=False, action="store_true",
        help=_("delete local packages no longer present in repository"))
    p.add_option("-p", "--download_path", dest='destdir',
        default=os.getcwd(), help=_("Path to download packages to: defaults to current dir"))
    p.add_option("--norepopath", dest='norepopath', default=False, action="store_true",
        help=_("Don't add the reponame to the download path. Can only be used when syncing a single repository (default is to add the reponame)"))
    p.add_option("-g", "--gpgcheck", default=False, action="store_true",
        help=_("Remove packages that fail GPG signature checking after downloading"))
    p.add_option("-u", "--urls", default=False, action="store_true",
        help=_("Just list urls of what would be downloaded, don't download"))
    p.add_option("-n", "--newest-only", dest='newest', default=False, action="store_true",
        help=_("Download only newest packages per-repo"))
    p.add_option("-q", "--quiet", default=False, action="store_true",
        help=_("Output as little as possible"))
    p.add_option("-l", "--plugins", default=False, action="store_true",
        help=_("enable yum plugin support"))
    p.add_option("-m", "--downloadcomps", default=False, action="store_true",
        help=_("also download comps.xml"))
    p.add_option("", "--download-metadata", dest="downloadmd",
        default=False, action="store_true",
        help=_("download all the non-default metadata"))
    (o, a) = p.parse_args()
    return (o, a)


def main():
    (o, _) = parseArgs()

    if not os.path.exists(o.destdir) and not o.urls:
        try:
            os.makedirs(o.destdir)
        except OSError, e:
            print >> sys.stderr, _("Error: Cannot create destination dir %s") % o.destdir
            sys.exit(1)

    if not os.access(o.destdir, os.W_OK) and not o.urls:
        print >> sys.stderr, _("Error: Cannot write to  destination dir %s") % o.destdir
        sys.exit(1)

    m = RepoSync(rOb=o)
    m.doConfigSetup(fn=o.config, init_plugins=o.plugins)

    if os.getuid() != 0 and not o.cachedir:
        o.tempcache = True

    if o.tempcache:
        if not m.setCacheDir(force=True, reuse=False):
            print >> sys.stderr, _("Error: Could not make cachedir, exiting")
            sys.exit(50)
        m.conf.uid = 1 
    elif o.cachedir:
        m.repos.setCacheDir(o.cachedir)

    if not o.cachedir:
        try:
            m.doLock()
        except yum.Errors.LockError, e:
            print >> sys.stderr, _("Error: %s") % e
            sys.exit(50)

    if not o.quiet:
        m.repos.setProgressBar(TextMeter(fo=sys.stdout), TextMultiFileMeter(fo=sys.stdout))
    m.doRepoSetup()

    if len(o.repoid) > 0:
        mr = []

        for g in o.repoid:
            ar = m.repos.findRepos(g)
            if not ar:
                print >> sys.stderr, _("Warning: cannot find repository %s") % g
                continue
            mr.extend(ar)

        if not mr:
            print >> sys.stderr, _("No repositories found")
            sys.exit(1)

        for r in m.repos.repos.values():
            r.disable()

        for r in mr:
            r.enable()

    if len(m.repos.listEnabled()) > 1 and o.norepopath:
        print >> sys.stderr, _("Error: Can't use --norepopath with multiple repositories")
        sys.exit(1)

    try:
        a = rpmUtils.arch.getArchList(o.arch)
        if o.source:
            a += ['src']
        m.doSackSetup(a)
    except yum.Errors.RepoError, e:
        print >> sys.stderr, _("Error setting up repositories: %s") % e
        sys.exit(1)

    ec = 0
    for r in m.repos.listEnabled():
        rs = ListPackageSack(m.pkgSack.returnPackages(repoid=r.id))

        if o.newest:
            dl = rs.returnNewestByNameArch()
        else:
            dl = list(rs)

        if o.norepopath:
            lrp = o.destdir
        else:
            lrp = o.destdir + '/' + r.id

        if o.delete and os.path.exists(lrp):
            cp = localpkgs(lrp)

            ds = {}
            for p in dl:
                rn = os.path.basename(p.remote_path)
                ds[rn] = 1

            for p in cp:
                if p in ds:
                    continue

                if not o.quiet:
                    m.logger.info("Removing obsolete %s", p)
                os.unlink(cp[p]['path'])

        if o.downloadcomps or o.downloadmd:

            if not os.path.exists(lrp):
                try:
                    os.makedirs(lrp)
                except IOError, e:
                    m.logger.error("Could not make repo subdir: %s" % e)
                    m.closeRpmDB()
                    sys.exit(1)

            if o.downloadcomps:
                wt = ['group']

            if o.downloadmd:
                wt = r.repoXML.fileTypes()

            for ft in r.repoXML.fileTypes():
                if ft in ['primary', 'primary_db', 'filelists',
                             'filelists_db', 'other', 'other_db']:
                    continue
                if ft not in wt:
                    continue

                try:
                    rf = r.retrieveMD(ft)
                    bn = os.path.basename(rf)
                    if ft == 'group' and o.downloadcomps:
                        bn = 'comps.xml'
                    shutil.copyfile(rf, "%s/%s" % (lrp, bn))
                except yum.Errors.RepoMDError, e:
                    if not o.quiet:
                        m.logger.error("Unable to fetch metadata: %s" % e)

        rs = 0
        if not o.urls:
            for p in dl:
                l = os.path.join(lrp, p.remote_path)
                s = int(p.returnSimple('packagesize'))
                if os.path.exists(l) and os.path.getsize(l) == s:
                    continue
                rs += s

        if hasattr(urlgrabber.progress, 'text_meter_total_size'):
            urlgrabber.progress.text_meter_total_size(rs)

        dl.sort(key=lambda p: p.name)
        if o.urls:
            for p in dl:
                l = os.path.join(lrp, p.remote_path)
                if not (os.path.exists(l) and m.verifyPkg(l, p, False)):
                    print urljoin(p.repo.urls[0], p.remote_path)
            continue

        if not os.path.exists(lrp):
            os.makedirs(lrp)

        for p in dl:
            p.localpath = os.path.join(lrp, p.remote_path)
            p.repo.copy_local = True
            p.repo.cache = 0
            ld = os.path.dirname(p.localpath)
            if not os.path.exists(ld):
                os.makedirs(ld)

        pr = m.downloadPkgs(dl)
        if pr:
            ec = 1
            for k in pr:
                for er in pr[k]:
                    m.logger.error('%s: %s', k, er)

        if o.gpgcheck:
            for p in dl:
                r, e = m.sigCheckPkg(p)
                if r != 0:
                    rf = os.path.basename(p.remote_path)
                    if r == 1:
                        m.logger.warning('Removing %s, due to missing GPG key.' % rf)
                    elif r == 2:
                        m.logger.warning('Removing %s due to failed signature check.' % rf)
                    else:
                        m.logger.warning('Removing %s due to failed signature check: %s' % rf)
                    os.unlink(p.localpath)
                    ec = 1
                    continue

    m.closeRpmDB()
    sys.exit(ec)

if __name__ == "__main__":
    main()