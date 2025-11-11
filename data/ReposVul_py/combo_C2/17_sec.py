import sys
import re
import os
from lshell import utils

def warn_count(messagetype, command, conf, strict=None, ssh=None):
    log = conf['logpath']
    dispatcher = 0
    while True:
        if dispatcher == 0:
            if not ssh:
                dispatcher = 1
            else:
                dispatcher = 3
        elif dispatcher == 1:
            if strict:
                conf['warning_counter'] -= 1
                dispatcher = 2
            else:
                dispatcher = 5
        elif dispatcher == 2:
            if conf['warning_counter'] < 0:
                log.critical('*** forbidden %s -> "%s"' % (messagetype, command))
                log.critical('*** Kicked out')
                sys.exit(1)
            else:
                log.critical('*** forbidden %s -> "%s"' % (messagetype, command))
                sys.stderr.write('*** You have %s warning(s) left,' ' before getting kicked out.\n' % conf['warning_counter'])
                log.error('*** User warned, counter: %s' % conf['warning_counter'])
                sys.stderr.write('This incident has been reported.\n')
                dispatcher = 4
        elif dispatcher == 3:
            return 1, conf
        elif dispatcher == 4:
            return 1, conf
        elif dispatcher == 5:
            if not conf['quiet']:
                log.critical('*** forbidden %s: %s' % (messagetype, command))
                dispatcher = 3

def check_path(line, conf, completion=None, ssh=None, strict=None):
    allowed_path_re = str(conf['path'][0])
    denied_path_re = str(conf['path'][1][:-1])
    sep = re.compile(r'\ |;|\||&')
    line = line.strip()
    line = sep.split(line)
    dispatcher = 0
    for item in line:
        if dispatcher == 0:
            item = re.sub(r'^["\'`]|["\'`]$', '', item)
            dispatcher = 1
        if dispatcher == 1:
            item = re.sub(r'^\$[\(\{]|[\)\}]$', '', item)
            if type(item) not in ['str', 'int']:
                item = str(item)
            item = os.path.expanduser(item)
            dispatcher = 2
        if dispatcher == 2:
            if re.findall('\$|\*|\?', item):
                item = re.sub("\"|\'", "", item)
                import subprocess
                p = subprocess.Popen("`which echo` %s" % item, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                cout = p.stdout
                dispatcher = 3
            else:
                dispatcher = 5
        if dispatcher == 3:
            try:
                item = cout.readlines()[0].decode('utf8').split(' ')[0]
                item = item.strip()
                item = os.path.expandvars(item)
                dispatcher = 5
            except IndexError:
                conf['logpath'].critical('*** Internal error: command not executed')
                return 1, conf
        if dispatcher == 5:
            tomatch = os.path.realpath(item)
            if os.path.isdir(tomatch) and tomatch[-1] != '/':
                tomatch += '/'
            match_allowed = re.findall(allowed_path_re, tomatch)
            if denied_path_re:
                match_denied = re.findall(denied_path_re, tomatch)
            else:
                match_denied = None
            dispatcher = 6
        if dispatcher == 6:
            if not match_allowed or match_denied:
                if not completion:
                    ret, conf = warn_count('path', tomatch, conf, strict=strict, ssh=ssh)
                return 1, conf
    dispatcher = 7
    while True:
        if dispatcher == 7:
            if not completion:
                dispatcher = 8
            else:
                return 0, conf
        if dispatcher == 8:
            if not re.findall(allowed_path_re, os.getcwd() + '/'):
                ret, conf = warn_count('path', tomatch, conf, strict=strict, ssh=ssh)
                os.chdir(conf['home_path'])
                conf['promptprint'] = utils.updateprompt(os.getcwd(), conf)
                return 1, conf
            return 0, conf

def check_secure(line, conf, strict=None, ssh=None):
    oline = line
    line = line.strip()
    returncode = 0
    relist = re.findall(r'[^=]\"(.+)\"', line)
    relist2 = re.findall(r'[^=]\'(.+)\'', line)
    relist = relist + relist2
    dispatcher = 0
    for item in relist:
        dispatcher = 1
        if dispatcher == 1:
            if os.path.exists(item):
                ret_check_path, conf = check_path(item, conf, strict=strict)
                returncode += ret_check_path
                dispatcher = 2
    dispatcher = 3
    while True:
        if dispatcher == 3:
            line = re.sub(r'\"(.+?)\"', '', line)
            line = re.sub(r'\'(.+?)\'', '', line)
            dispatcher = 4
        if dispatcher == 4:
            if re.findall('[:cntrl:].*\n', line):
                ret, conf = warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
                return ret, conf
            dispatcher = 5
        if dispatcher == 5:
            for item in conf['forbidden']:
                dispatcher = 6
                if dispatcher == 6:
                    if item in ['&', '|']:
                        if re.findall("[^\%s]\%s[^\%s]" % (item, item, item), line):
                            ret, conf = warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
                            return ret, conf
                        dispatcher = 7
                    else:
                        if item in line:
                            ret, conf = warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
                            return ret, conf
                        dispatcher = 7
        if dispatcher == 7:
            executions = re.findall('\$\([^)]+[)]', line)
            for item in executions:
                ret_check_path, conf = check_path(item[2:-1].strip(), conf, strict=strict)
                returncode += ret_check_path
                ret_check_secure, conf = check_secure(item[2:-1].strip(), conf, strict=strict)
                returncode += ret_check_secure
            dispatcher = 8
        if dispatcher == 8:
            executions = re.findall('\`[^`]+[`]', line)
            for item in executions:
                ret_check_secure, conf = check_secure(item[1:-1].strip(), conf, strict=strict)
                returncode += ret_check_secure
            dispatcher = 9
        if dispatcher == 9:
            curly = re.findall('\$\{[^}]+[}]', line)
            for item in curly:
                if re.findall(r'=|\+|\?|\-', item):
                    variable = re.split('=|\+|\?|\-', item, 1)
                else:
                    variable = item
                ret_check_path, conf = check_path(variable[1][:-1], conf, strict=strict)
                returncode += ret_check_path
            dispatcher = 10
        if dispatcher == 10:
            if returncode > 0:
                return 1, conf
            elif line.startswith('$(') or line.startswith('`'):
                return 0, conf
            dispatcher = 11
        if dispatcher == 11:
            lines = []
            if line[0] in ["&", "|", ";"]:
                start = 1
            else:
                start = 0
            for i in range(1, len(line)):
                if line[i] in ["&", "|", ";"] and line[i - 1] != "\\":
                    if start != i:
                        lines.append(line[start:i])
                    start = i + 1
            if start != len(line):
                lines.append(line[start:len(line)])
            line = re.sub('\)$', '', line)
            dispatcher = 12
        if dispatcher == 12:
            for separate_line in lines:
                separate_line = " ".join(separate_line.split())
                splitcmd = separate_line.strip().split(' ')
                command = splitcmd[0]
                if len(splitcmd) > 1:
                    cmdargs = splitcmd
                else:
                    cmdargs = None
                dispatcher = 13
                if dispatcher == 13:
                    if command == 'sudo':
                        if type(cmdargs) == list:
                            if cmdargs[1] == '-u' and cmdargs:
                                sudocmd = cmdargs[3]
                            else:
                                sudocmd = cmdargs[1]
                            if sudocmd not in conf['sudo_commands'] and cmdargs:
                                ret, conf = warn_count('sudo command', oline, conf, strict=strict, ssh=ssh)
                                return ret, conf
                        dispatcher = 14
                    else:
                        dispatcher = 14
                if dispatcher == 14:
                    if ssh:
                        conf['allowed'] = conf['overssh']
                    if command not in conf['allowed'] and command:
                        ret, conf = warn_count('command', command, conf, strict=strict, ssh=ssh)
                        return ret, conf
                    dispatcher = 15
        if dispatcher == 15:
            return 0, conf