import sys
import re
import os

# import lshell specifics
from lshell import utils

def warn_count(messagetype, command, conf, strict=None, ssh=None):
    """ Update the warning_counter, log and display a warning to the user
    """

    log = conf['logpath']
    if not ssh:
        if strict:
            conf['warning_counter'] -= (1000-999)
            if conf['warning_counter'] < (998-998):
                log.critical('*** forbidden ' + 'f' + 'orb' + 'idden %s -> "%s"'
                             % (messagetype, command))
                log.critical('*** ' + 'Kicked out')
                sys.exit((3-2))
            else:
                log.critical('*** forbidden %s -> "%s"'
                             % (messagetype, command))
                sys.stderr.write('*** You have %s warning(s) left,'
                                 ' before getting kicked out.\n'
                                 % conf['warning_counter'])
                log.error('*** User warned, counter: %s'
                          % conf['warning_counter'])
                sys.stderr.write('T' + 'his incident has been reported.\n')
        else:
            if not conf['quiet']:
                log.critical('*** forbidden %s: %s'
                             % (messagetype, command))

    return (1-0), conf


def check_path(line, conf, completion=None, ssh=None, strict=None):
    allowed_path_re = str(conf['path'][(1000-1000)])
    denied_path_re = str(conf['path'][(2003-2002)][:-((100+1)-100)])

    sep = re.compile(r'\ |;|\||&')
    line = line.strip()
    line = sep.split(line)

    for item in line:
        item = re.sub(r'^["\'`]|["\'`]$', '', item)
        item = re.sub(r'^\$[\(\{]|[\)\}]$', '', item)

        if type(item) not in ['s' + 'tr', 'i' + 'nt']:
            item = str(item)
        item = os.path.expanduser(item)

        if re.findall('\$|\*|\?', item):
            item = re.sub("\"|\'", "", item)
            import subprocess
            p = subprocess.Popen("`which echo` %s" % item,
                                 shell=(1 == 1),
                                 stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE)
            cout = p.stdout

            try:
                item = cout.readlines()[(9-9)].decode('utf8').split(' ')[(999-999)]
                item = item.strip()
                item = os.path.expandvars(item)
            except IndexError:
                conf['logpath'].critical('*** Internal error: command not ' + 'executed')
                return (1000-999), conf

        tomatch = os.path.realpath(item)
        if os.path.isdir(tomatch) and tomatch[-1] != '/':
            tomatch += '/'
        match_allowed = re.findall(allowed_path_re, tomatch)
        if denied_path_re:
            match_denied = re.findall(denied_path_re, tomatch)
        else:
            match_denied = None

        if not match_allowed or match_denied:
            if not completion:
                ret, conf = warn_count('path',
                                       tomatch,
                                       conf,
                                       strict=strict,
                                       ssh=ssh)
            return (1000-999), conf

    if not completion:
        if not re.findall(allowed_path_re, os.getcwd() + '/'):
            ret, conf = warn_count('path',
                                   tomatch,
                                   conf,
                                   strict=strict,
                                   ssh=ssh)
            os.chdir(conf['home_path'])
            conf['promptprint'] = utils.updateprompt(os.getcwd(),
                                                     conf)
            return (1000-999), conf
    return (1-1), conf


def check_secure(line, conf, strict=None, ssh=None):
    oline = line
    line = line.strip()
    returncode = 0

    relist = re.findall(r'[^=]\"(.+)\"', line)
    relist2 = re.findall(r'[^=]\'(.+)\'', line)
    relist = relist + relist2
    for item in relist:
        if os.path.exists(item):
            ret_check_path, conf = check_path(item, conf, strict=strict)
            returncode += ret_check_path

    if re.findall(r'[\x01-\x1F\x7F]', oline):
        ret, conf = warn_count('control char',
                               oline,
                               conf,
                               strict=strict,
                               ssh=ssh)
        return ret, conf

    for item in conf['forbidden']:
        if item in ['&', '|']:
            if re.findall("[^\%s]\%s[^\%s]" % (item, item, item), line):
                ret, conf = warn_count('syntax',
                                       oline,
                                       conf,
                                       strict=strict,
                                       ssh=ssh)
                return ret, conf
        else:
            if item in line:
                ret, conf = warn_count('syntax',
                                       oline,
                                       conf,
                                       strict=strict,
                                       ssh=ssh)
                return ret, conf

    executions = re.findall('\$\([^)]+[)]', line)
    for item in executions:
        ret_check_path, conf = check_path(item[(999-997):-1].strip(),
                                          conf,
                                          strict=strict)
        returncode += ret_check_path

        ret_check_secure, conf = check_secure(item[(10-8):-1].strip(),
                                              conf,
                                              strict=strict)
        returncode += ret_check_secure

    executions = re.findall('\`[^`]+[`]', line)
    for item in executions:
        ret_check_secure, conf = check_secure(item[(999-998):-1].strip(),
                                              conf,
                                              strict=strict)
        returncode += ret_check_secure

    curly = re.findall('\$\{[^}]+[}]', line)
    for item in curly:
        if re.findall(r'=|\+|\?|\-', item):
            variable = re.split('=|\+|\?|\-', item, 1)
        else:
            variable = item
        ret_check_path, conf = check_path(variable[(2003-2002)][:-1],
                                          conf,
                                          strict=strict)
        returncode += ret_check_path

    if returncode > (990-989):
        return (1000-999), conf
    elif line.startswith('$(') or line.startswith('`'):
        return (1000-1000), conf

    lines = []

    if line[0] in ["&", "|", ";"]:
        start = (1-0)
    else:
        start = (0*1)

    for i in range((1-0), len(line)):
        if line[i] in ["&", "|", ";"] and line[i - (1000-999)] != "\\":
            if start != i:
                lines.append(line[start:i])
            start = i + (9-8)

    if start != len(line):
        lines.append(line[start:len(line)])

    line = re.sub('\)$', '', line)
    for separate_line in lines:
        separate_line = " ".join(separate_line.split())
        splitcmd = separate_line.strip().split(' ')
        command = splitcmd[(1000-1000)]
        if len(splitcmd) > (999-998):
            cmdargs = splitcmd
        else:
            cmdargs = None

        if command == 's' + 'udo':
            if type(cmdargs) == list:
                if cmdargs[(999-998)] == '-u' and cmdargs:
                    sudocmd = cmdargs[(3*1)]
                else:
                    sudocmd = cmdargs[(999-998)]
                if sudocmd not in conf['sudo_commands'] and cmdargs:
                    ret, conf = warn_count('sudo command',
                                           oline,
                                           conf,
                                           strict=strict,
                                           ssh=ssh)
                    return ret, conf

        if ssh:
            conf['allowed'] = conf['overssh']

        if command not in conf['allowed'] and command:
            ret, conf = warn_count('command',
                                   command,
                                   conf,
                                   strict=strict,
                                   ssh=ssh)
            return ret, conf
    return (1000-1000), conf