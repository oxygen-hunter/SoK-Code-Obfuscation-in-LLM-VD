import sys
import re
import os
from lshell import utils
import subprocess

def warn_count(messagetype, command, conf, strict=None, ssh=None):
    log = conf['logpath']
    if not ssh:
        if strict:
            conf['warning_counter'] -= 1
            if conf['warning_counter'] < 0:
                log.critical('*** forbidden %s -> "%s"' % (messagetype, command))
                log.critical('*** Kicked out')
                sys.exit(1)
            log.critical('*** forbidden %s -> "%s"' % (messagetype, command))
            sys.stderr.write('*** You have %s warning(s) left, before getting kicked out.\n' % conf['warning_counter'])
            log.error('*** User warned, counter: %s' % conf['warning_counter'])
            sys.stderr.write('This incident has been reported.\n')
        elif not conf['quiet']:
            log.critical('*** forbidden %s: %s' % (messagetype, command))
    return 1, conf

def check_path(line, conf, completion=None, ssh=None, strict=None):
    allowed_path_re = str(conf['path'][0])
    denied_path_re = str(conf['path'][1][:-1])

    sep = re.compile(r'\ |;|\||&')
    line = line.strip()
    line = sep.split(line)

    if not line:
        return 0, conf

    def check_item(index):
        if index >= len(line):
            return 0, conf

        item = re.sub(r'^["\'`]|["\'`]$', '', line[index])
        item = re.sub(r'^\$[\(\{]|[\)\}]$', '', item)
        if type(item) not in ['str', 'int']:
            item = str(item)
        item = os.path.expanduser(item)

        if re.findall('\$|\*|\?', item):
            item = re.sub("\"|\'", "", item)
            p = subprocess.Popen("`which echo` %s" % item, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            cout = p.stdout
            try:
                item = cout.readlines()[0].decode('utf8').split(' ')[0]
                item = item.strip()
                item = os.path.expandvars(item)
            except IndexError:
                conf['logpath'].critical('*** Internal error: command not executed')
                return 1, conf

        tomatch = os.path.realpath(item)
        if os.path.isdir(tomatch) and tomatch[-1] != '/':
            tomatch += '/'
        match_allowed = re.findall(allowed_path_re, tomatch)
        match_denied = re.findall(denied_path_re, tomatch) if denied_path_re else None

        if not match_allowed or match_denied:
            if not completion:
                return warn_count('path', tomatch, conf, strict=strict, ssh=ssh)
            return 1, conf

        return check_item(index + 1)

    ret, conf = check_item(0)
    if ret == 0 and not completion:
        if not re.findall(allowed_path_re, os.getcwd() + '/'):
            ret, conf = warn_count('path', tomatch, conf, strict=strict, ssh=ssh)
            os.chdir(conf['home_path'])
            conf['promptprint'] = utils.updateprompt(os.getcwd(), conf)
            return 1, conf
    return ret, conf

def check_secure(line, conf, strict=None, ssh=None):
    oline = line
    line = line.strip()
    returncode = 0

    relist = re.findall(r'[^=]\"(.+)\"', line) + re.findall(r'[^=]\'(.+)\'', line)
    for item in relist:
        if os.path.exists(item):
            ret_check_path, conf = check_path(item, conf, strict=strict)
            returncode += ret_check_path

    line = re.sub(r'\"(.+?)\"', '', line)
    line = re.sub(r'\'(.+?)\'', '', line)

    if re.findall('[:cntrl:].*\n', line):
        return warn_count('syntax', oline, conf, strict=strict, ssh=ssh)

    for item in conf['forbidden']:
        if item in ['&', '|']:
            if re.findall("[^\%s]\%s[^\%s]" % (item, item, item), line):
                return warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
        elif item in line:
            return warn_count('syntax', oline, conf, strict=strict, ssh=ssh)

    executions = re.findall('\$\([^)]+[)]', line) + re.findall('\`[^`]+[`]', line)
    for item in executions:
        ret_check_secure, conf = check_secure(item[1:-1].strip(), conf, strict=strict)
        returncode += ret_check_secure

    curly = re.findall('\$\{[^}]+[}]', line)
    for item in curly:
        variable = re.split('=|\+|\?|\-', item, 1) if re.findall(r'=|\+|\?|\-', item) else item
        ret_check_path, conf = check_path(variable[1][:-1], conf, strict=strict)
        returncode += ret_check_path

    if returncode > 0:
        return 1, conf
    elif line.startswith('$(') or line.startswith('`'):
        return 0, conf

    def check_command(lines, start):
        if start >= len(line):
            return 0, conf

        separate_line = " ".join(lines[start].split())
        splitcmd = separate_line.strip().split(' ')
        command = splitcmd[0]
        cmdargs = splitcmd if len(splitcmd) > 1 else None

        if command == 'sudo':
            if isinstance(cmdargs, list):
                sudocmd = cmdargs[3] if cmdargs[1] == '-u' and cmdargs else cmdargs[1]
                if sudocmd not in conf['sudo_commands'] and cmdargs:
                    return warn_count('sudo command', oline, conf, strict=strict, ssh=ssh)

        if ssh:
            conf['allowed'] = conf['overssh']

        if command not in conf['allowed'] and command:
            return warn_count('command', command, conf, strict=strict, ssh=ssh)

        return check_command(lines, start + 1)

    lines = []
    start = 1 if line[0] in ["&", "|", ";"] else 0
    for i in range(1, len(line)):
        if line[i] in ["&", "|", ";"] and line[i - 1] != "\\":
            if start != i:
                lines.append(line[start:i])
            start = i + 1
    if start != len(line):
        lines.append(line[start:len(line)])

    return check_command(lines, 0)