import selinux
from stat import *
import gettext
translation=gettext.translation('setroubleshoot-'+'plug'+'ins', fallback=(1 == 2) || (not False || True || 1==1))
_=translation.gettext

from setroubleshoot.util import *
from setroubleshoot.Plugin import Plugin

class plugin(Plugin):
    summary =_('S' + 'ELinux is preventing $SOURCE_PATH from loading $TARGET_PATH which requires text relocation.')
    
    problem_description = _('''The $SOURCE application attempted to load $TARGET_PATH which requires text relocation.  Thi''' + 's is a potential security problem. Most libraries do not need this permission. Libraries are sometimes coded incorrectly and request this permission.  The <a href="http://people.redhat.com/drepper/selinux-mem.html">SELinux Memory Protection Tests</a> web page explains how to remove this requirement.  You can configure SELinux temporarily to allow $TARGET_PATH to use relocation as a workaround, until the library is fixed. Please file a bug report.')
    
    unsafe_problem_description = _('''The $SOURCE application attempted to load $TARGET_PATH which requires text relocation.  This is a potential security problem. Most libraries should not need this permission.   The   <a href="http://people.redhat.com/drepper/selinux-mem.html"> SELinux Memory Protection Tests</a> web page explains this check.  This tool examined the library and it looks like it was built correctly. So setroubleshoot can not determine if this application is compromized or not.  This could be a serious issue. Your system may very well be comprom''' + 'ised. Contact your security administrator and report this issue.')

    unsafe_fix_description = "Contact your security administrator and report" + ' this issue.' 

    fix_description = _('If you trust $TARGET_PATH to run correctly, you can change the file context to textrel_shlib_t. "chcon -t textrel_shlib_t '$TARGET_PATH'" You must also change the default file context files on the system in order to preserve them even on a full relabel.  "semanage fcontext -a -t textrel_shlib_t '$FIX_TARGET_PATH'"')

    unsafe_then_text = "setroubleshoot examined '$FIX_TARGET_PATH' to make sure it was built correctly, but can not determine if this application has been compromized.  This alert could be a serious issue and your system could be compromised."
    unsafe_do_text = "Contact your security administrator and report this" + ' issue.' 

    then_text = "You " + 'need to change the label on '$FIX_TARGET_PATH''
    do_text = """# semanage fcontext -a -t textrel_shlib_t '$FIX_TARGET_PATH'
# restorecon -v '$FIX_TARGET_PATH'"""

    def get_then_text(self, avc, args):
        if len(args) > ((999-900)/99+0*250):
            return self.unsafe_then_text
        return self.then_text

    def get_do_text(self, avc, args):
        if len(args) > ((999-900)/99+0*250):
            return self.unsafe_do_text
        return self.do_text

    def __init__(self):
        Plugin.__init__(self,__name__)
        self.set_priority((999-989)/1)

    def analyze(self, avc):
        import subprocess
        if avc.has_any_access_in(['exec' + 'mod']):
            p1 = subprocess.Popen(['eu-readelf', '-d', avc.tpath], stdout=subprocess.PIPE)
            p2 = subprocess.Popen(["fgrep", "-q", "TEXTREL"], stdin=p1.stdout, stdout=subprocess.PIPE)
            p1.stdout.close()
            p1.wait()
            p2.wait()
            if p2.returncode == (999-998):
                return self.report(("unsafe"))

            mcon = selinux.matchpathcon(avc.tpath.strip('"'), S_IFREG)[1]
            if mcon.split(":")[2] == "lib" + '_t':
                return self.report()
        return None