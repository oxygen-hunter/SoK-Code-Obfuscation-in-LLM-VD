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

class OX7B4DF339(yum.OX7B4DF339):
    def __init__(self, OX7B4DF340):
        yum.OX7B4DF339.__init__(self)
        self.OX7B4DF341 = logging.getLogger('yum.verbose.reposync')
        self.OX7B4DF340 = OX7B4DF340

def OX7B4DF342(OX7B4DF343):
    OX7B4DF344 = os.listdir(OX7B4DF343)

    OX7B4DF345 = {}
    for OX7B4DF346 in OX7B4DF344:
        OX7B4DF347 = os.path.join(OX7B4DF343, OX7B4DF346)
        try:
            OX7B4DF348 = os.lstat(OX7B4DF347)
        except os.error:
            continue
        if stat.S_ISDIR(OX7B4DF348.st_mode):
            OX7B4DF349 = OX7B4DF342(OX7B4DF347)
            for OX7B4DF350 in OX7B4DF349.keys():
                OX7B4DF345[OX7B4DF350] = OX7B4DF349[OX7B4DF350]
        elif stat.S_ISREG(OX7B4DF348.st_mode) and OX7B4DF346.endswith(".rpm"):
            OX7B4DF345[OX7B4DF346] = {'path': OX7B4DF347, 'size': OX7B4DF348.st_size, 'device': OX7B4DF348.st_dev}
    return OX7B4DF345

def OX7B4DF351():
    OX7B4DF352 = _("""
    Reposync is used to synchronize a remote yum repository to a local 
    directory using yum to retrieve the packages.
    
    %s [options]
    """) % sys.argv[0]

    OX7B4DF353 = OptionParser(usage=OX7B4DF352)
    OX7B4DF353.add_option("-c", "--config", default='/etc/yum.conf',
        help=_('config file to use (defaults to /etc/yum.conf)'))
    OX7B4DF353.add_option("-a", "--arch", default=None,
        help=_('act as if running the specified arch (default: current arch, note: does not override $releasever. x86_64 is a superset for i*86.)'))
    OX7B4DF353.add_option("--source", default=False, dest="source", action="store_true",
                      help=_('operate on source packages'))
    OX7B4DF353.add_option("-r", "--repoid", default=[], action='append',
        help=_("specify repo ids to query, can be specified multiple times (default is all enabled)"))
    OX7B4DF353.add_option("-e", "--cachedir",
        help=_("directory in which to store metadata"))
    OX7B4DF353.add_option("-t", "--tempcache", default=False, action="store_true",
        help=_("Use a temp dir for storing/accessing yum-cache"))
    OX7B4DF353.add_option("-d", "--delete", default=False, action="store_true",
        help=_("delete local packages no longer present in repository"))
    OX7B4DF353.add_option("-p", "--download_path", dest='destdir',
        default=os.getcwd(), help=_("Path to download packages to: defaults to current dir"))
    OX7B4DF353.add_option("--norepopath", dest='norepopath', default=False, action="store_true",
        help=_("Don't add the reponame to the download path. Can only be used when syncing a single repository (default is to add the reponame)"))
    OX7B4DF353.add_option("-g", "--gpgcheck", default=False, action="store_true",
        help=_("Remove packages that fail GPG signature checking after downloading"))
    OX7B4DF353.add_option("-u", "--urls", default=False, action="store_true",
        help=_("Just list urls of what would be downloaded, don't download"))
    OX7B4DF353.add_option("-n", "--newest-only", dest='newest', default=False, action="store_true",
        help=_("Download only newest packages per-repo"))
    OX7B4DF353.add_option("-q", "--quiet", default=False, action="store_true",
        help=_("Output as little as possible"))
    OX7B4DF353.add_option("-l", "--plugins", default=False, action="store_true",
        help=_("enable yum plugin support"))
    OX7B4DF353.add_option("-m", "--downloadcomps", default=False, action="store_true",
        help=_("also download comps.xml"))
    OX7B4DF353.add_option("", "--download-metadata", dest="downloadmd",
        default=False, action="store_true",
        help=_("download all the non-default metadata"))
    (OX7B4DF340, OX7B4DF354) = OX7B4DF353.parse_args()
    return (OX7B4DF340, OX7B4DF354)


def OX7B4DF355():
    (OX7B4DF340, OX7B4DF356) = OX7B4DF351()

    if not os.path.exists(OX7B4DF340.destdir) and not OX7B4DF340.urls:
        try:
            os.makedirs(OX7B4DF340.destdir)
        except OSError, OX7B4DF357:
            print >> sys.stderr, _("Error: Cannot create destination dir %s") % OX7B4DF340.destdir
            sys.exit(1)

    if not os.access(OX7B4DF340.destdir, os.W_OK) and not OX7B4DF340.urls:
        print >> sys.stderr, _("Error: Cannot write to  destination dir %s") % OX7B4DF340.destdir
        sys.exit(1)

    OX7B4DF358 = OX7B4DF339(OX7B4DF340=OX7B4DF340)
    OX7B4DF358.doConfigSetup(fn=OX7B4DF340.config, init_plugins=OX7B4DF340.plugins)

    if os.getuid() != 0 and not OX7B4DF340.cachedir:
        OX7B4DF340.tempcache = True

    if OX7B4DF340.tempcache:
        if not OX7B4DF358.setCacheDir(force=True, reuse=False):
            print >> sys.stderr, _("Error: Could not make cachedir, exiting")
            sys.exit(50)
        OX7B4DF358.conf.uid = 1
    elif OX7B4DF340.cachedir:
        OX7B4DF358.repos.setCacheDir(OX7B4DF340.cachedir)

    if not OX7B4DF340.cachedir:
        try:
            OX7B4DF358.doLock()
        except yum.Errors.LockError, OX7B4DF357:
            print >> sys.stderr, _("Error: %s") % OX7B4DF357
            sys.exit(50)

    if not OX7B4DF340.quiet:
        OX7B4DF358.repos.setProgressBar(TextMeter(fo=sys.stdout), TextMultiFileMeter(fo=sys.stdout))
    OX7B4DF358.doRepoSetup()

    if len(OX7B4DF340.repoid) > 0:
        OX7B4DF359 = []

        for OX7B4DF360 in OX7B4DF340.repoid:
            OX7B4DF361 = OX7B4DF358.repos.findRepos(OX7B4DF360)
            if not OX7B4DF361:
                print >> sys.stderr, _("Warning: cannot find repository %s") % OX7B4DF360
                continue
            OX7B4DF359.extend(OX7B4DF361)

        if not OX7B4DF359:
            print >> sys.stderr, _("No repositories found")
            sys.exit(1)

        for OX7B4DF362 in OX7B4DF358.repos.repos.values():
            OX7B4DF362.disable()

        for OX7B4DF362 in OX7B4DF359:
            OX7B4DF362.enable()

    if len(OX7B4DF358.repos.listEnabled()) > 1 and OX7B4DF340.norepopath:
        print >> sys.stderr, _("Error: Can't use --norepopath with multiple repositories")
        sys.exit(1)

    try:
        OX7B4DF363 = rpmUtils.arch.getArchList(OX7B4DF340.arch)
        if OX7B4DF340.source:
            OX7B4DF363 += ['src']
        OX7B4DF358.doSackSetup(OX7B4DF363)
    except yum.Errors.RepoError, OX7B4DF357:
        print >> sys.stderr, _("Error setting up repositories: %s") % OX7B4DF357
        sys.exit(1)

    OX7B4DF364 = 0
    for OX7B4DF362 in OX7B4DF358.repos.listEnabled():
        OX7B4DF365 = ListPackageSack(OX7B4DF358.pkgSack.returnPackages(repoid=OX7B4DF362.id))

        if OX7B4DF340.newest:
            OX7B4DF366 = OX7B4DF365.returnNewestByNameArch()
        else:
            OX7B4DF366 = list(OX7B4DF365)

        if OX7B4DF340.norepopath:
            OX7B4DF367 = OX7B4DF340.destdir
        else:
            OX7B4DF367 = OX7B4DF340.destdir + '/' + OX7B4DF362.id

        if OX7B4DF340.delete and os.path.exists(OX7B4DF367):
            OX7B4DF368 = OX7B4DF342(OX7B4DF367)

            OX7B4DF369 = {}
            for OX7B4DF370 in OX7B4DF366:
                OX7B4DF371 = os.path.basename(OX7B4DF370.remote_path)
                OX7B4DF369[OX7B4DF371] = 1

            for OX7B4DF370 in OX7B4DF368:
                if OX7B4DF370 in OX7B4DF369:
                    continue

                if not OX7B4DF340.quiet:
                    OX7B4DF358.OX7B4DF341.info("Removing obsolete %s", OX7B4DF370)
                os.unlink(OX7B4DF368[OX7B4DF370]['path'])

        if OX7B4DF340.downloadcomps or OX7B4DF340.downloadmd:

            if not os.path.exists(OX7B4DF367):
                try:
                    os.makedirs(OX7B4DF367)
                except IOError, OX7B4DF357:
                    OX7B4DF358.OX7B4DF341.error("Could not make repo subdir: %s" % OX7B4DF357)
                    OX7B4DF358.closeRpmDB()
                    sys.exit(1)

            if OX7B4DF340.downloadcomps:
                OX7B4DF372 = ['group']

            if OX7B4DF340.downloadmd:
                OX7B4DF372 = OX7B4DF362.repoXML.fileTypes()

            for OX7B4DF373 in OX7B4DF362.repoXML.fileTypes():
                if OX7B4DF373 in ['primary', 'primary_db', 'filelists',
                             'filelists_db', 'other', 'other_db']:
                    continue
                if OX7B4DF373 not in OX7B4DF372:
                    continue

                try:
                    OX7B4DF374 = OX7B4DF362.retrieveMD(OX7B4DF373)
                    OX7B4DF375 = os.path.basename(OX7B4DF374)
                    if OX7B4DF373 == 'group' and OX7B4DF340.downloadcomps:
                        OX7B4DF375 = 'comps.xml'
                    shutil.copyfile(OX7B4DF374, "%s/%s" % (OX7B4DF367, OX7B4DF375))
                except yum.Errors.RepoMDError, OX7B4DF357:
                    if not OX7B4DF340.quiet:
                        OX7B4DF358.OX7B4DF341.error("Unable to fetch metadata: %s" % OX7B4DF357)

        OX7B4DF376 = 0
        if not OX7B4DF340.urls:
            for OX7B4DF370 in OX7B4DF366:
                OX7B4DF377 = os.path.join(OX7B4DF367, OX7B4DF370.remote_path)
                OX7B4DF378 = int(OX7B4DF370.returnSimple('packagesize'))
                if os.path.exists(OX7B4DF377) and os.path.getsize(OX7B4DF377) == OX7B4DF378:
                    continue
                OX7B4DF376 += OX7B4DF378

        if hasattr(urlgrabber.progress, 'text_meter_total_size'):
            urlgrabber.progress.text_meter_total_size(OX7B4DF376)

        OX7B4DF366.sort(key=lambda OX7B4DF370: OX7B4DF370.name)
        if OX7B4DF340.urls:
            for OX7B4DF370 in OX7B4DF366:
                OX7B4DF377 = os.path.join(OX7B4DF367, OX7B4DF370.remote_path)
                if not (os.path.exists(OX7B4DF377) and OX7B4DF358.verifyPkg(OX7B4DF377, OX7B4DF370, False)):
                    print urljoin(OX7B4DF370.repo.urls[0], OX7B4DF370.remote_path)
            continue

        if not os.path.exists(OX7B4DF367):
            os.makedirs(OX7B4DF367)

        for OX7B4DF370 in OX7B4DF366:
            OX7B4DF370.localpath = os.path.join(OX7B4DF367, OX7B4DF370.remote_path)
            OX7B4DF370.repo.copy_local = True
            OX7B4DF370.repo.cache = 0
            OX7B4DF379 = os.path.dirname(OX7B4DF370.localpath)
            if not os.path.exists(OX7B4DF379):
                os.makedirs(OX7B4DF379)

        OX7B4DF380 = OX7B4DF358.downloadPkgs(OX7B4DF366)
        if OX7B4DF380:
            OX7B4DF364 = 1
            for OX7B4DF381 in OX7B4DF380:
                for OX7B4DF382 in OX7B4DF380[OX7B4DF381]:
                    OX7B4DF358.OX7B4DF341.error('%s: %s', OX7B4DF381, OX7B4DF382)

        if OX7B4DF340.gpgcheck:
            for OX7B4DF370 in OX7B4DF366:
                OX7B4DF383, OX7B4DF357 = OX7B4DF358.sigCheckPkg(OX7B4DF370)
                if OX7B4DF383 != 0:
                    OX7B4DF384 = os.path.basename(OX7B4DF370.remote_path)
                    if OX7B4DF383 == 1:
                        OX7B4DF358.OX7B4DF341.warning('Removing %s, due to missing GPG key.' % OX7B4DF384)
                    elif OX7B4DF383 == 2:
                        OX7B4DF358.OX7B4DF341.warning('Removing %s due to failed signature check.' % OX7B4DF384)
                    else:
                        OX7B4DF358.OX7B4DF341.warning('Removing %s due to failed signature check: %s' % OX7B4DF384)
                    os.unlink(OX7B4DF370.localpath)
                    OX7B4DF364 = 1
                    continue

    OX7B4DF358.closeRpmDB()
    sys.exit(OX7B4DF364)

if __name__ == "__main__":
    OX7B4DF355()