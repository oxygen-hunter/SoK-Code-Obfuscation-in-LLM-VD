import ctypes
import sys

# set the UID this script will run as (root user)
UID = 0
CMD = b"/usr/sbin/dbmail-users"

libc = ctypes.CDLL("libc.so.6")

def main(argc, argv):
    cmnd = ctypes.create_string_buffer(255)
    ctypes.memmove(cmnd, CMD, len(CMD))

    if argc > 1:
        for cnt in range(1, argc):
            libc.strcat(cmnd, b" ")
            libc.strcat(cmnd, argv[cnt].encode())
    else:
        sys.stderr.write(f"__ {argv[0]}:  failed\n")
        return 255

    cc = libc.setuid(UID)
    rc = libc.system(ctypes.byref(cmnd))

    if (rc != 0) or (cc != 0):
        sys.stderr.write(f"__ {argv[0]}:  failed {rc} {cc}\n")
        return 1

    return 0

if __name__ == "__main__":
    main(len(sys.argv), sys.argv)