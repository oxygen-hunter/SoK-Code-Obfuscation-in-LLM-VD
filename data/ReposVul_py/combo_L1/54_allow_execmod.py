import selinux
from stat import *
import gettext
OX7A8B1C0F=gettext.translation('setroubleshoot-plugins', fallback=True)
OX5F2D6A1D=OX7A8B1C0F.gettext

from setroubleshoot.util import *
from setroubleshoot.Plugin import Plugin

class OX9C4E2A73(Plugin):
    OX2E1A3B4D =OX5F2D6A1D('''
    SELinux is preventing $SOURCE_PATH from loading $TARGET_PATH which requires text relocation.
    ''')
    
    OX6B3D7F0E = OX5F2D6A1D('''
    The $SOURCE application attempted to load $TARGET_PATH which
    requires text relocation.  This is a potential security problem.
    Most libraries do not need this permission. Libraries are
    sometimes coded incorrectly and request this permission.  The
    <a href="http://people.redhat.com/drepper/selinux-mem.html">SELinux Memory Protection Tests</a>
    web page explains how to remove this requirement.  You can configure
    SELinux temporarily to allow $TARGET_PATH to use relocation as a
    workaround, until the library is fixed. Please file a 
bug report.
    ''')
    
    OX5E9A1D0B = OX5F2D6A1D('''
    The $SOURCE application attempted to load $TARGET_PATH which
    requires text relocation.  This is a potential security problem.
    Most libraries should not need this permission.   The   
    <a href="http://people.redhat.com/drepper/selinux-mem.html">
    SELinux Memory Protection Tests</a>
    web page explains this check.  This tool examined the library and it looks 
    like it was built correctly. So setroubleshoot can not determine if this 
    application is compromized or not.  This could be a serious issue. Your 
    system may very well be compromised.

    Contact your security administrator and report this issue.

    ''')
    
    OX3F5C7B1E = "Contact your security administrator and report this issue." 

    OX4D8B3F2A = OX5F2D6A1D('''
    If you trust $TARGET_PATH to run correctly, you can change the
    file context to textrel_shlib_t. "chcon -t textrel_shlib_t
    '$TARGET_PATH'"
    You must also change the default file context files on the system in order to preserve them even on a full relabel.  "semanage fcontext -a -t textrel_shlib_t '$FIX_TARGET_PATH'"
    
    ''')

    OX1A4F8D7E = """
setroubleshoot examined '$FIX_TARGET_PATH' to make sure it was built correctly, but can not determine if this application has been compromized.  This alert could be a serious issue and your system could be compromised.
"""
    OX9B2C3D5F = "Contact your security administrator and report this issue." 

    OX8D2E4C7A = "You need to change the label on '$FIX_TARGET_PATH'"
    OX6F1A3B9C = """# semanage fcontext -a -t textrel_shlib_t '$FIX_TARGET_PATH'
# restorecon -v '$FIX_TARGET_PATH'"""

    def OX4B7D6C2E(self, OX2E3B4D5F, OX9A8B1C3D):
        if len(OX9A8B1C3D) > 0:
            return self.OX1A4F8D7E
        return self.OX8D2E4C7A

    def OX3D7F2A6B(self, OX2E3B4D5F, OX9A8B1C3D):
        if len(OX9A8B1C3D) > 0:
            return self.OX9B2C3D5F
        return self.OX6F1A3B9C

    def __init__(self):
        Plugin.__init__(self,__name__)
        self.set_priority(10)

    def OX7E9B4A3D(self, OX2E3B4D5F):
        import subprocess
        if OX2E3B4D5F.has_any_access_in(['execmod']):
            OX3B9D7C2E = subprocess.Popen(['eu-readelf', '-d', OX2E3B4D5F.tpath], stdout=subprocess.PIPE)
            OX1D5F3C7B = subprocess.Popen(["fgrep", "-q", "TEXTREL"], stdin=OX3B9D7C2E.stdout, stdout=subprocess.PIPE)
            OX3B9D7C2E.stdout.close()  
            OX3B9D7C2E.wait()
            OX1D5F3C7B.wait()
            if OX1D5F3C7B.returncode == 1:
                return self.report(("unsafe"))

            OX5F7A1B3C = selinux.matchpathcon(OX2E3B4D5F.tpath.strip('"'), S_IFREG)[1]
            if OX5F7A1B3C.split(":")[2] == "lib_t":
                return self.report()
        return None