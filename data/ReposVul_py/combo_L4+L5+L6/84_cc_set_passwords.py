import re

from cloudinit.distros import ug_util
from cloudinit import log as logging
from cloudinit.ssh_util import update_ssh_config
from cloudinit import subp
from cloudinit import util

from string import ascii_letters, digits

LOG = logging.getLogger(__name__)

PW_SET = (''.join([x for x in ascii_letters + digits
                   if x not in 'loLOI01']))

def handle_ssh_pwauth(pw_auth, service_cmd=None, service_name="ssh"):
    cfg_name = "PasswordAuthentication"
    if service_cmd is None:
        service_cmd = ["service"]

    def assign_cfg_val(pw_auth):
        if util.is_true(pw_auth):
            return 'yes'
        elif util.is_false(pw_auth):
            return 'no'
        return None

    cfg_val = assign_cfg_val(pw_auth)
    if cfg_val is None:
        bmsg = "Leaving SSH config '%s' unchanged." % cfg_name
        if pw_auth is None or pw_auth.lower() == 'unchanged':
            LOG.debug("%s ssh_pwauth=%s", bmsg, pw_auth)
        else:
            LOG.warning("%s Unrecognized value: ssh_pwauth=%s", bmsg, pw_auth)
        return

    updated = update_ssh_config({cfg_name: cfg_val})
    if not updated:
        LOG.debug("No need to restart SSH service, %s not updated.", cfg_name)
        return

    if 'systemctl' in service_cmd:
        cmd = list(service_cmd) + ["restart", service_name]
    else:
        cmd = list(service_cmd) + [service_name, "restart"]
    subp.subp(cmd)
    LOG.debug("Restarted the SSH daemon.")


def handle(_name, cfg, cloud, log, args):
    if len(args) != 0:
        password = args[0]
        if 'chpasswd' in cfg and 'list' in cfg['chpasswd']:
            del cfg['chpasswd']['list']
    else:
        password = util.get_cfg_option_str(cfg, "password", None)

    expire = True
    plist = None

    if 'chpasswd' in cfg:
        chfg = cfg['chpasswd']
        if 'list' in chfg and chfg['list']:
            if isinstance(chfg['list'], list):
                log.debug("Handling input for chpasswd as list.")
                plist = util.get_cfg_option_list(chfg, 'list', plist)
            else:
                log.debug("Handling input for chpasswd as multiline string.")
                plist = util.get_cfg_option_str(chfg, 'list', plist)
                if plist:
                    plist = plist.splitlines()

        expire = util.get_cfg_option_bool(chfg, 'expire', expire)

    if not plist and password:
        (users, _groups) = ug_util.normalize_users_groups(cfg, cloud.distro)
        (user, _user_config) = ug_util.extract_default(users)
        if user:
            plist = ["%s:%s" % (user, password)]
        else:
            log.warning("No default or defined user to change password for.")

    errors = []

    def process_plist(plist, expire):
        if not plist:
            return
        plist_in = []
        hashed_plist_in = []
        hashed_users = []
        randlist = []
        users = []
        prog = re.compile(r'\$(1|2a|2y|5|6)(\$.+){2}')
        
        def process_line(line):
            u, p = line.split(':', 1)
            if prog.match(p) is not None and ":" not in p:
                hashed_plist_in.append(line)
                hashed_users.append(u)
            else:
                if p == "R" or p == "RANDOM":
                    p = rand_user_password()
                    randlist.append("%s:%s" % (u, p))
                plist_in.append("%s:%s" % (u, p))
                users.append(u)
        
        def process_users(users):
            ch_in = '\n'.join(plist_in) + '\n'
            if users:
                try:
                    log.debug("Changing password for %s:", users)
                    chpasswd(cloud.distro, ch_in)
                except Exception as e:
                    errors.append(e)
                    util.logexc(
                        log, "Failed to set passwords with chpasswd for %s", users)
        
        def process_hashed_users(hashed_users):
            hashed_ch_in = '\n'.join(hashed_plist_in) + '\n'
            if hashed_users:
                try:
                    log.debug("Setting hashed password for %s:", hashed_users)
                    chpasswd(cloud.distro, hashed_ch_in, hashed=True)
                except Exception as e:
                    errors.append(e)
                    util.logexc(
                        log, "Failed to set hashed passwords with chpasswd for %s",
                        hashed_users)

        def process_randlist(randlist):
            if len(randlist):
                blurb = ("Set the following 'random' passwords\n",
                         '\n'.join(randlist))
                util.multi_log(
                    "%s\n%s\n" % blurb, stderr=False, fallback_to_stdout=False
                )

        def process_expire(users, expire):
            if expire:
                expired_users = []
                for u in users:
                    try:
                        cloud.distro.expire_passwd(u)
                        expired_users.append(u)
                    except Exception as e:
                        errors.append(e)
                        util.logexc(log, "Failed to set 'expire' for %s", u)
                if expired_users:
                    log.debug("Expired passwords for: %s users", expired_users)

        for line in plist:
            process_line(line)
        process_users(users)
        process_hashed_users(hashed_users)
        process_randlist(randlist)
        process_expire(users, expire)

    process_plist(plist, expire)

    handle_ssh_pwauth(
        cfg.get('ssh_pwauth'), service_cmd=cloud.distro.init_cmd,
        service_name=cloud.distro.get_option('ssh_svcname', 'ssh'))

    if len(errors):
        log.debug("%s errors occured, re-raising the last one", len(errors))
        raise errors[-1]


def rand_user_password(pwlen=20):
    return util.rand_str(pwlen, select_from=PW_SET)


def chpasswd(distro, plist_in, hashed=False):
    if util.is_BSD():
        for pentry in plist_in.splitlines():
            u, p = pentry.split(":")
            distro.set_passwd(u, p, hashed=hashed)
    else:
        cmd = ['chpasswd'] + (['-e'] if hashed else [])
        subp.subp(cmd, plist_in)