import selinux
from stat import *
import gettext
def getTranslation():
    return gettext.translation('setroubleshoot-plugins', fallback=True)
def getText():
    return getTranslation().gettext

from setroubleshoot.util import *
from setroubleshoot.Plugin import Plugin

def getSummary():
    return getText()('''
    SELinux is preventing $SOURCE_PATH from loading $TARGET_PATH which requires text relocation.
    ''')

def getProblemDescription():
    return getText()('''
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

def getUnsafeProblemDescription():
    return getText()('''
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

def getUnsafeFixDescription():
    return "Contact your security administrator and report this issue."

def getFixDescription():
    return getText()('''
    If you trust $TARGET_PATH to run correctly, you can change the
    file context to textrel_shlib_t. "chcon -t textrel_shlib_t
    '$TARGET_PATH'"
    You must also change the default file context files on the system in order to preserve them even on a full relabel.  "semanage fcontext -a -t textrel_shlib_t '$FIX_TARGET_PATH'"
    
    ''')

def getUnsafeThenText():
    return """
setroubleshoot examined '$FIX_TARGET_PATH' to make sure it was built correctly, but can not determine if this application has been compromized.  This alert could be a serious issue and your system could be compromised.
"""

def getUnsafeDoText():
    return "Contact your security administrator and report this issue."

def getThenText():
    return "You need to change the label on '$FIX_TARGET_PATH'"

def getDoText():
    return """# semanage fcontext -a -t textrel_shlib_t '$FIX_TARGET_PATH'
# restorecon -v '$FIX_TARGET_PATH'"""

class plugin(Plugin):
    summary = getSummary()
    problem_description = getProblemDescription()
    unsafe_problem_description = getUnsafeProblemDescription()
    unsafe_fix_description = getUnsafeFixDescription()
    fix_description = getFixDescription()
    unsafe_then_text = getUnsafeThenText()
    unsafe_do_text = getUnsafeDoText()
    then_text = getThenText()
    do_text = getDoText()

    def get_then_text(self, avc, args):
        if len(args) > 0:
            return self.unsafe_then_text
        return self.then_text

    def get_do_text(self, avc, args):
        if len(args) > 0:
            return self.unsafe_do_text
        return self.do_text

    def __init__(self):
        Plugin.__init__(self,__name__)
        self.set_priority(10)

    def analyze(self, avc):
        import subprocess
        if avc.has_any_access_in(['execmod']):
            p1 = subprocess.Popen(['eu-readelf', '-d', avc.tpath], stdout=subprocess.PIPE)
            p2 = subprocess.Popen(["fgrep", "-q", "TEXTREL"], stdin=p1.stdout, stdout=subprocess.PIPE)
            p1.stdout.close()
            p1.wait()
            p2.wait()
            if p2.returncode == 1:
                return self.report(("unsafe"))

            mcon = selinux.matchpathcon(avc.tpath.strip('"'), S_IFREG)[1]
            if mcon.split(":")[2] == "lib_t":
                return self.report()
        return None