import ctypes
import os
import sys

def usage(argv):
    print("\nUsage:\t{} -q -f config_file program_name [arguments]\n"
          "\t-q makes proxychains quiet - this overrides the config setting\n"
          "\t-f allows to manually specify a configfile to use\n"
          "\tfor example : proxychains telnet somehost.com\n"
          "More help in README file\n\n".format(argv[0]))
    return os.EX_CONFIG

dll_name = 'proxychains.dll'

own_dir = ctypes.create_string_buffer(256)
dll_dirs = [
    b".",
    own_dir,
    b"/lib",
    b"/usr/lib",
    b"/usr/local/lib",
    b"/lib64",
    None
]

def set_own_dir(argv0):
    l = len(argv0)
    while l and argv0[l - 1] != '/':
        l -= 1
    if l == 0:
        ctypes.memmove(own_dir, b".", 2)
    else:
        ctypes.memmove(own_dir, argv0.encode('utf-8'), l - 1)
        own_dir[l] = 0

def main(argc, argv):
    path = None
    buf = ctypes.create_string_buffer(256)
    pbuf = ctypes.create_string_buffer(256)
    start_argv = 1
    quiet = 0
    prefix = None

    for i in range(2):
        if start_argv < argc and argv[start_argv][0] == '-':
            if argv[start_argv][1] == 'q':
                quiet = 1
                start_argv += 1
            elif argv[start_argv][1] == 'f':
                if start_argv + 1 < argc:
                    path = argv[start_argv + 1]
                else:
                    return usage(argv)
                start_argv += 2
        else:
            break

    if start_argv >= argc:
        return usage(argv)

    path = get_config_path(path, pbuf, ctypes.sizeof(pbuf))

    if not quiet:
        sys.stderr.write("config file found: {}\n".format(path))

    os.environ['PROXYCHAINS_CONF_FILE'] = path

    if quiet:
        os.environ['PROXYCHAINS_QUIET_MODE'] = "1"

    set_own_dir(argv[0])

    i = 0

    while dll_dirs[i]:
        ctypes.snprintf(buf, ctypes.sizeof(buf), b"%s/%s", dll_dirs[i], dll_name.encode('utf-8'))
        if os.access(buf.value.decode('utf-8'), os.R_OK) != -1:
            prefix = dll_dirs[i]
            break
        i += 1

    if not prefix:
        sys.stderr.write("couldnt locate {}\n".format(dll_name))
        return os.EX_CONFIG

    if not quiet:
        sys.stderr.write("preloading {}/{}\n".format(prefix, dll_name))

    old_val = os.getenv('LD_PRELOAD')
    ctypes.snprintf(buf, ctypes.sizeof(buf), b"LD_PRELOAD=%s/%s%s%s",
                    prefix, dll_name.encode('utf-8'),
                    b" " if old_val else b"",
                    old_val.encode('utf-8') if old_val else b"")
    os.putenv('LD_PRELOAD', buf.value.decode('utf-8'))
    os.execvp(argv[start_argv], argv[start_argv:])
    sys.stderr.write("proxychains can't load process....\n")

    return os.EX_CONFIG

if __name__ == "__main__":
    main(len(sys.argv), sys.argv)