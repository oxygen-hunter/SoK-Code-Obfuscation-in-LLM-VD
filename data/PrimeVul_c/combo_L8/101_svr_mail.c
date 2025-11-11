import ctypes
import os
import subprocess
import sys

# Load C functions from DLL
c_lib = ctypes.CDLL(None)

# Assume the necessary C functions are defined in the loaded shared library
rpp_terminate = c_lib.rpp_terminate
net_close = c_lib.net_close
log_event = c_lib.log_event
svr_format_job = c_lib.svr_format_job

# Global Data
LOGLEVEL = 0

def svr_mailowner(pjob, mailpoint, force, text):
    cmdbuf = None
    mailfrom = None
    mailto = ctypes.create_string_buffer(1024)
    bodyfmtbuf = ctypes.create_string_buffer(1024)
    
    if (server['sv_attr'][SRV_ATR_MailDomain]['at_flags'] & ATR_VFLAG_SET) and \
       (server['sv_attr'][SRV_ATR_MailDomain]['at_val']['at_str'] is not None) and \
       (server['sv_attr'][SRV_ATR_MailDomain]['at_val']['at_str'].lower() == "never"):
        
        if LOGLEVEL >= 3:
            log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
                      PBS_EVENTCLASS_JOB,
                      pjob['ji_qs']['ji_jobid'],
                      "Not sending email: Mail domain set to 'never'\n")
        return

    if LOGLEVEL >= 3:
        tmpBuf = "preparing to send '{}' mail for job {} to {} ({})\n".format(
            chr(mailpoint), pjob['ji_qs']['ji_jobid'], pjob['ji_wattr'][JOB_ATR_job_owner]['at_val']['at_str'],
            text if text is not None else "---"
        )
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
                  PBS_EVENTCLASS_JOB,
                  pjob['ji_qs']['ji_jobid'],
                  tmpBuf)

    if (force != MAIL_FORCE) or \
       (server['sv_attr'][SRV_ATR_NoMailForce]['at_val']['at_long'] == True):
        
        if pjob['ji_wattr'][JOB_ATR_mailpnts]['at_flags'] & ATR_VFLAG_SET:
            if pjob['ji_wattr'][JOB_ATR_mailpnts]['at_val']['at_str'][0] == MAIL_NONE:
                log_event(PBSEVENT_JOB,
                          PBS_EVENTCLASS_JOB,
                          pjob['ji_qs']['ji_jobid'],
                          "Not sending email: job requested no e-mail")
                return
            if chr(mailpoint) not in pjob['ji_wattr'][JOB_ATR_mailpnts]['at_val']['at_str']:
                log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
                          PBS_EVENTCLASS_JOB,
                          pjob['ji_qs']['ji_jobid'],
                          "Not sending email: User does not want mail of this type.\n")
                return
        elif mailpoint != MAIL_ABORT:
            log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
                      PBS_EVENTCLASS_JOB,
                      pjob['ji_qs']['ji_jobid'],
                      "Not sending email: Default mailpoint does not include this type.\n")
            return

    if os.fork():
        return

    rpp_terminate()
    net_close(-1)

    if server['sv_attr'][SRV_ATR_mailfrom]['at_val']['at_str'] is None:
        mailfrom = PBS_DEFAULT_MAIL
    else:
        mailfrom = server['sv_attr'][SRV_ATR_mailfrom]['at_val']['at_str']

    mailto.value = b''

    if pjob['ji_wattr'][JOB_ATR_mailuser]['at_flags'] & ATR_VFLAG_SET:
        pas = pjob['ji_wattr'][JOB_ATR_mailuser]['at_val']['at_arst']
        
        if pas is not None:
            for i in range(pas['as_usedptr']):
                if (len(mailto.value) + len(pas['as_string'][i]) + 2) < len(mailto):
                    mailto.value += pas['as_string'][i].encode('utf-8') + b" "
    else:
        if (server['sv_attr'][SRV_ATR_MailDomain]['at_flags'] & ATR_VFLAG_SET) and \
           (server['sv_attr'][SRV_ATR_MailDomain]['at_val']['at_str'] is not None):
            mailto.value = (pjob['ji_wattr'][JOB_ATR_euser]['at_val']['at_str'] + "@" +
                            server['sv_attr'][SRV_ATR_MailDomain]['at_val']['at_str']).encode('utf-8')
        else:
#ifdef TMAILDOMAIN
            mailto.value = (pjob['ji_wattr'][JOB_ATR_euser]['at_val']['at_str'] + "@" + TMAILDOMAIN).encode('utf-8')
#else
            mailto.value = pjob['ji_wattr'][JOB_ATR_job_owner]['at_val']['at_str'].encode('utf-8')
#endif

    if (server['sv_attr'][SRV_ATR_MailSubjectFmt]['at_flags'] & ATR_VFLAG_SET) and \
       (server['sv_attr'][SRV_ATR_MailSubjectFmt]['at_val']['at_str'] is not None):
        subjectfmt = server['sv_attr'][SRV_ATR_MailSubjectFmt]['at_val']['at_str']
    else:
        subjectfmt = "PBS JOB %i"

    if (server['sv_attr'][SRV_ATR_MailBodyFmt]['at_flags'] & ATR_VFLAG_SET) and \
       (server['sv_attr'][SRV_ATR_MailBodyFmt]['at_val']['at_str'] is not None):
        bodyfmt = server['sv_attr'][SRV_ATR_MailBodyFmt]['at_val']['at_str']
    else:
        bodyfmt = ctypes.create_string_buffer("PBS Job Id: {}\nJob Name: {}\n".format(
            pjob['ji_qs']['ji_jobid'], pjob['ji_qs']['ji_jobname']
        ).encode('utf-8'))
        if pjob['ji_wattr'][JOB_ATR_exec_host]['at_flags'] & ATR_VFLAG_SET:
            bodyfmt.value += b"Exec host: {}\n".format(pjob['ji_wattr'][JOB_ATR_exec_host]['at_val']['at_str'].encode('utf-8'))
        bodyfmt.value += b"%m\n"
        if text is not None:
            bodyfmt.value += b"%d\n"

    i = len(SENDMAIL_CMD) + len(mailfrom) + len(mailto.value) + 6
    cmdbuf = ctypes.create_string_buffer(i)

    cmdbuf.value = "{} -f {} {}".format(SENDMAIL_CMD, mailfrom, mailto.value.decode('utf-8')).encode('utf-8')

    outmail = subprocess.Popen(cmdbuf.value.decode('utf-8'), shell=True, stdin=subprocess.PIPE, stderr=subprocess.PIPE)

    if outmail is None:
        tmpBuf = "Unable to popen() command '{}' for writing: '{}' (error {})\n".format(
            cmdbuf.value.decode('utf-8'), os.strerror(ctypes.get_errno()), ctypes.get_errno()
        )
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
                  PBS_EVENTCLASS_JOB,
                  pjob['ji_qs']['ji_jobid'],
                  tmpBuf)
        sys.exit(1)

    outmail.stdin.write(b"To: {}\n".format(mailto.value))
    outmail.stdin.write(b"Subject: ")
    svr_format_job(outmail.stdin, pjob, subjectfmt.encode('utf-8'), mailpoint, text.encode('utf-8') if text else None)
    outmail.stdin.write(b"\n")
    outmail.stdin.write(b"Precedence: bulk\n\n")
    svr_format_job(outmail.stdin, pjob, bodyfmt, mailpoint, text.encode('utf-8') if text else None)

    outmail.stdin.close()
    errno = 0
    i = outmail.wait()
    if i != 0:
        tmpBuf = "Email '{}' to {} failed: Child process '{}' {} {} (errno {}:{})\n".format(
            mailpoint, mailto.value.decode('utf-8'), cmdbuf.value.decode('utf-8'),
            "returned" if os.WIFEXITED(i) else ("killed by signal" if os.WIFSIGNALED(i) else "croaked"),
            os.WEXITSTATUS(i) if os.WIFEXITED(i) else (os.WTERMSIG(i) if os.WIFSIGNALED(i) else i),
            errno, os.strerror(errno)
        )
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
                  PBS_EVENTCLASS_JOB,
                  pjob['ji_qs']['ji_jobid'],
                  tmpBuf)
    elif LOGLEVEL >= 4:
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
                  PBS_EVENTCLASS_JOB,
                  pjob['ji_qs']['ji_jobid'],
                  "Email sent successfully\n")

    sys.exit(0)

# Assuming server and other constants are defined elsewhere