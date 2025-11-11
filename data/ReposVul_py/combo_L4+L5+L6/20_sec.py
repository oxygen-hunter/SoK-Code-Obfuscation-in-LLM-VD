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
            else:
                log.critical('*** forbidden %s -> "%s"' % (messagetype, command))
                sys.stderr.write('*** You have %s warning(s) left,'
                                 ' before getting kicked out.\n'
                                 % conf['warning_counter'])
                log.error('*** User warned, counter: %s' % conf['warning_counter'])
                sys.stderr.write('This incident has been reported.\n')
        else:
            if not conf['quiet']:
                log.critical('*** forbidden %s: %s' % (messagetype, command))
    return 1, conf

def split_line(line, sep, index=0, acc=[]):
    if index >= len(line):
        return acc
    while index < len(line) and line[index] not in sep:
        index += 1
    if index < len(line):
        acc.append(line[:index])
        if line[index] in sep:
            return split_line(line[index+1:], sep, 0, acc)
    acc.append(line[index:])
    return acc

def check_path(line, conf, completion=None, ssh=None, strict=None):
    allowed_path_re = str(conf['path'][0])
    denied_path_re = str(conf['path'][1][:-1])
    sep = re.compile(r'\ |;|\||&')
    line = line.strip()
    line = split_line(line, [' ', ';', '|', '&'])

    def recursive_check(item_list, index, conf):
        if index >= len(item_list):
            if not completion:
                if not re.findall(allowed_path_re, os.getcwd() + '/'):
                    ret, conf = warn_count('path', tomatch, conf, strict=strict, ssh=ssh)
                    os.chdir(conf['home_path'])
                    conf['promptprint'] = utils.updateprompt(os.getcwd(), conf)
                    return 1, conf
            return 0, conf

        item = item_list[index]
        item = re.sub(r'^["\'`]|["\'`]$', '', item)
        item = re.sub(r'^\$[\(\{]|[\)\}]$', '', item)
        if type(item) not in ['str', 'int']:
            item = str(item)
        item = os.path.expanduser(item)

        if re.findall('\$|\*|\?', item):
            item = re.sub("\"|\'", "", item)
            p = subprocess.Popen("`which echo` %s" % item, shell=True, stdin=subprocess.PIPE,
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE)
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
                ret, conf = warn_count('path', tomatch, conf, strict=strict, ssh=ssh)
            return 1, conf

        return recursive_check(item_list, index + 1, conf)

    return recursive_check(line, 0, conf)

def check_secure(line, conf, strict=None, ssh=None):
    oline = line
    line = line.strip()
    returncode = 0
    relist = re.findall(r'[^=]\"(.+)\"', line) + re.findall(r'[^=]\'(.+)\'', line)
    
    def secure_recursive_check(index, conf, returncode):
        if index >= len(relist):
            if re.findall(r'[\x01-\x1F\x7F]', oline):
                ret, conf = warn_count('control char', oline, conf, strict=strict, ssh=ssh)
                return ret, conf

            for item in conf['forbidden']:
                if item in ['&', '|']:
                    if re.findall("[^\%s]\%s[^\%s]" % (item, item, item), line):
                        ret, conf = warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
                        return ret, conf
                else:
                    if item in line:
                        ret, conf = warn_count('syntax', oline, conf, strict=strict, ssh=ssh)
                        return ret, conf

            executions = re.findall('\$\([^)]+[)]', line)
            for item in executions:
                ret_check_path, conf = check_path(item[2:-1].strip(), conf, strict=strict)
                returncode += ret_check_path
                ret_check_secure, conf = check_secure(item[2:-1].strip(), conf, strict=strict)
                returncode += ret_check_secure

            executions = re.findall('\`[^`]+[`]', line)
            for item in executions:
                ret_check_secure, conf = check_secure(item[1:-1].strip(), conf, strict=strict)
                returncode += ret_check_secure

            curly = re.findall('\$\{[^}]+[}]', line)
            for item in curly:
                if re.findall(r'=|\+|\?|\-', item):
                    variable = re.split('=|\+|\?|\-', item, 1)
                else:
                    variable = item
                ret_check_path, conf = check_path(variable[1][:-1], conf, strict=strict)
                returncode += ret_check_path

            if returncode > 0:
                return 1, conf
            elif line.startswith('$(') or line.startswith('`'):
                return 0, conf

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
            for separate_line in lines:
                separate_line = " ".join(separate_line.split())
                splitcmd = separate_line.strip().split(' ')
                command = splitcmd[0]
                if len(splitcmd) > 1:
                    cmdargs = splitcmd
                else:
                    cmdargs = None

                if command == 'sudo':
                    if type(cmdargs) == list:
                        if cmdargs[1] == '-u' and cmdargs:
                            sudocmd = cmdargs[3]
                        else:
                            sudocmd = cmdargs[1]
                        if sudocmd not in conf['sudo_commands'] and cmdargs:
                            ret, conf = warn_count('sudo command', oline, conf, strict=strict, ssh=ssh)
                            return ret, conf

                if ssh:
                    conf['allowed'] = conf['overssh']

                if command not in conf['allowed'] and command:
                    ret, conf = warn_count('command', command, conf, strict=strict, ssh=ssh)
                    return ret, conf
            return 0, conf

        item = relist[index]
        if os.path.exists(item):
            ret_check_path, conf = check_path(item, conf, strict=strict)
            returncode += ret_check_path

        return secure_recursive_check(index + 1, conf, returncode)

    return secure_recursive_check(0, conf, returncode)