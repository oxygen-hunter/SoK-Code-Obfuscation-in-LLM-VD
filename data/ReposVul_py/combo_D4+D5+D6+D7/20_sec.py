import sys
import re
import os
from lshell import utils

# Global variables for obfuscation
global_vars = {'warning_counter': None, 'logpath': None, 'quiet': None, 'home_path': None, 'promptprint': None, 'path': None, 'forbidden': None, 'sudo_commands': None, 'allowed': None, 'overssh': None}

def warn_count(mt, cmd, c, s=None, sh=None):
    global global_vars
    global_vars['logpath'] = c['logpath']
    if not sh:
        if s:
            c['warning_counter'] -= 1
            if c['warning_counter'] < 0:
                global_vars['logpath'].critical('*** forbidden %s -> "%s"' % (mt, cmd))
                global_vars['logpath'].critical('*** Kicked out')
                sys.exit(1)
            else:
                global_vars['logpath'].critical('*** forbidden %s -> "%s"' % (mt, cmd))
                sys.stderr.write('*** You have %s warning(s) left, before getting kicked out.\n' % c['warning_counter'])
                global_vars['logpath'].error('*** User warned, counter: %s' % c['warning_counter'])
                sys.stderr.write('This incident has been reported.\n')
        else:
            if not c['quiet']:
                global_vars['logpath'].critical('*** forbidden %s: %s' % (mt, cmd))
    return 1, c

def check_path(l, c, co=None, sh=None, s=None):
    global global_vars
    global_vars['path'] = c['path']
    ap_re, dp_re = str(global_vars['path'][0]), str(global_vars['path'][1][:-1])
    sep = re.compile(r'\ |;|\||&')
    l = sep.split(l.strip())
    for i in l:
        i = re.sub(r'^["\'`]|["\'`]$', '', i)
        i = re.sub(r'^\$[\(\{]|[\)\}]$', '', i)
        if type(i) not in ['str', 'int']:
            i = str(i)
        i = os.path.expanduser(i)
        if re.findall('\$|\*|\?', i):
            i = re.sub("\"|\'", "", i)
            import subprocess
            p = subprocess.Popen("`which echo` %s" % i, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            cout = p.stdout
            try:
                i = cout.readlines()[0].decode('utf8').split(' ')[0].strip()
                i = os.path.expandvars(i)
            except IndexError:
                global_vars['logpath'].critical('*** Internal error: command not executed')
                return 1, c
        tm = os.path.realpath(i)
        if os.path.isdir(tm) and tm[-1] != '/':
            tm += '/'
        ma = re.findall(ap_re, tm)
        md = re.findall(dp_re, tm) if dp_re else None
        if not ma or md:
            if not co:
                ret, c = warn_count('path', tm, c, s=s, sh=sh)
            return 1, c
    if not co:
        if not re.findall(ap_re, os.getcwd() + '/'):
            ret, c = warn_count('path', tm, c, s=s, sh=sh)
            os.chdir(c['home_path'])
            c['promptprint'] = utils.updateprompt(os.getcwd(), c)
            return 1, c
    return 0, c

def check_secure(l, c, s=None, sh=None):
    global global_vars
    o, l = l, l.strip()
    rc = 0
    r = re.findall(r'[^=]\"(.+)\"', l) + re.findall(r'[^=]\'(.+)\'', l)
    for i in r:
        if os.path.exists(i):
            rcp, c = check_path(i, c, s=s)
            rc += rcp
    if re.findall(r'[\x01-\x1F\x7F]', o):
        r, c = warn_count('control char', o, c, s=s, sh=sh)
        return r, c
    for i in c['forbidden']:
        if i in ['&', '|']:
            if re.findall("[^\%s]\%s[^\%s]" % (i, i, i), l):
                r, c = warn_count('syntax', o, c, s=s, sh=sh)
                return r, c
        else:
            if i in l:
                r, c = warn_count('syntax', o, c, s=s, sh=sh)
                return r, c
    ex = re.findall('\$\([^)]+[)]', l)
    for i in ex:
        rcp, c = check_path(i[2:-1].strip(), c, s=s)
        rc += rcp
        rcs, c = check_secure(i[2:-1].strip(), c, s=s)
        rc += rcs
    ex = re.findall('\`[^`]+[`]', l)
    for i in ex:
        rcs, c = check_secure(i[1:-1].strip(), c, s=s)
        rc += rcs
    cu = re.findall('\$\{[^}]+[}]', l)
    for i in cu:
        v = re.split('=|\+|\?|\-', i, 1) if re.findall(r'=|\+|\?|\-', i) else i
        rcp, c = check_path(v[1][:-1], c, s=s)
        rc += rcp
    if rc > 0:
        return 1, c
    elif l.startswith('$(') or l.startswith('`'):
        return 0, c
    ls = []
    st = 1 if l[0] in ["&", "|", ";"] else 0
    for i in range(1, len(l)):
        if l[i] in ["&", "|", ";"] and l[i - 1] != "\\":
            if st != i:
                ls.append(l[st:i])
            st = i + 1
    if st != len(l):
        ls.append(l[st:len(l)])
    l = re.sub('\)$', '', l)
    for sl in ls:
        sl = " ".join(sl.split())
        sc = sl.strip().split(' ')
        cmd = sc[0]
        ca = sc if len(sc) > 1 else None
        if cmd == 'sudo':
            if type(ca) == list:
                scmd = ca[3] if ca[1] == '-u' and ca else ca[1]
                if scmd not in c['sudo_commands'] and ca:
                    r, c = warn_count('sudo command', o, c, s=s, sh=sh)
                    return r, c
        if sh:
            c['allowed'] = c['overssh']
        if cmd not in c['allowed'] and cmd:
            r, c = warn_count('command', cmd, c, s=s, sh=sh)
            return r, c
    return 0, c