import selinux
from stat import *
import gettext
translation=gettext.translation('setroubleshoot-plugins', fallback=True)
_=translation.gettext

from setroubleshoot.util import *
from setroubleshoot.Plugin import Plugin

class plugin(Plugin):
    summary =_('''
    SELinux is preventing $SOURCE_PATH from loading $TARGET_PATH which requires text relocation.
    ''')
    
    problem_description = _('''
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
    
    unsafe_problem_description = _('''
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
    
    unsafe_fix_description = "Contact your security administrator and report this issue." 

    fix_description = _('''
    If you trust $TARGET_PATH to run correctly, you can change the
    file context to textrel_shlib_t. "chcon -t textrel_shlib_t
    '$TARGET_PATH'"
    You must also change the default file context files on the system in order to preserve them even on a full relabel.  "semanage fcontext -a -t textrel_shlib_t '$FIX_TARGET_PATH'"
    
    ''')

    unsafe_then_text = """
setroubleshoot examined '$FIX_TARGET_PATH' to make sure it was built correctly, but can not determine if this application has been compromized.  This alert could be a serious issue and your system could be compromised.
"""
    unsafe_do_text = "Contact your security administrator and report this issue." 

    then_text = "You need to change the label on '$FIX_TARGET_PATH'"
    do_text = """# semanage fcontext -a -t textrel_shlib_t '$FIX_TARGET_PATH'
# restorecon -v '$FIX_TARGET_PATH'"""

    def get_then_text(self, avc, args):
        state = 0
        while True:
            if state == 0:
                if len(args) > 0:
                    state = 1
                else:
                    state = 2
            elif state == 1:
                return self.unsafe_then_text
            elif state == 2:
                return self.then_text

    def get_do_text(self, avc, args):
        state = 0
        while True:
            if state == 0:
                if len(args) > 0:
                    state = 1
                else:
                    state = 2
            elif state == 1:
                return self.unsafe_do_text
            elif state == 2:
                return self.do_text

    def __init__(self):
        Plugin.__init__(self,__name__)
        self.set_priority(10)

    def analyze(self, avc):
        import subprocess
        state = 0
        while True:
            if state == 0:
                if avc.has_any_access_in(['execmod']):
                    state = 1
                else:
                    state = 3
            elif state == 1:
                p1 = subprocess.Popen(['eu-readelf', '-d', avc.tpath], stdout=subprocess.PIPE)
                p2 = subprocess.Popen(["fgrep", "-q", "TEXTREL"], stdin=p1.stdout, stdout=subprocess.PIPE)
                p1.stdout.close()
                p1.wait()
                p2.wait()
                if p2.returncode == 1:
                    state = 2
                else:
                    state = 3
            elif state == 2:
                return self.report(("unsafe"))
            elif state == 3:
                mcon = selinux.matchpathcon(avc.tpath.strip('"'), S_IFREG)[1]
                if mcon.split(":")[2] == "lib_t":
                    return self.report()
                else:
                    return None