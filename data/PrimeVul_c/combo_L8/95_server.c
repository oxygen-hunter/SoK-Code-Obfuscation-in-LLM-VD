import ctypes
import ctypes.util
import sys

libc = ctypes.CDLL(ctypes.util.find_library('c'))

def main(argc, argv):
    pname_data = ctypes.create_string_buffer(256)
    tkt_data = ctypes.create_string_buffer(256)
    sock = 0
    l = ctypes.c_int()
    retval = ctypes.c_int()
    l_inaddr = ctypes.create_string_buffer(ctypes.sizeof(ctypes.c_int) * 4)
    f_inaddr = ctypes.create_string_buffer(ctypes.sizeof(ctypes.c_int) * 4)
    creds = ctypes.create_string_buffer(1024)
    new_creds = ctypes.create_string_buffer(1024)
    cc = ctypes.create_string_buffer(256)
    msgtext = ctypes.create_string_buffer(256)
    msg = ctypes.create_string_buffer(256)
    context = ctypes.create_string_buffer(256)
    auth_context = ctypes.c_void_p()

    libc.freopen(b"/tmp/uu-server.log", b"w", libc.stderr)

    if (retval := krb5_init_context(ctypes.byref(context))) != 0:
        com_err(argv[0], retval, b"while initializing krb5")
        sys.exit(1)

    if (retval := krb5_read_message(context, ctypes.byref(sock), ctypes.byref(pname_data))) != 0:
        com_err(b"uu-server", retval, b"reading pname")
        return 2

    if (retval := krb5_read_message(context, ctypes.byref(sock), ctypes.byref(tkt_data))) != 0:
        com_err(b"uu-server", retval, b"reading ticket data")
        return 2

    if (retval := krb5_cc_default(context, ctypes.byref(cc))) != 0:
        com_err(b"uu-server", retval, b"getting credentials cache")
        return 4

    libc.memset(ctypes.byref(creds), 0, ctypes.sizeof(creds))
    if (retval := krb5_cc_get_principal(context, cc, ctypes.byref(creds.client))) != 0:
        com_err(b"uu-client", retval, b"getting principal name")
        return 6

    print(f"uu-server: client principal is \"{pname_data.value.decode('utf-8')}\".")

    if (retval := krb5_parse_name(context, pname_data, ctypes.byref(creds.server))) != 0:
        com_err(b"uu-server", retval, b"parsing client name")
        return 3

    creds.second_ticket = tkt_data
    print(f"uu-server: client ticket is {creds.second_ticket.length} bytes.")

    if (retval := krb5_get_credentials(context, KRB5_GC_USER_USER, cc, ctypes.byref(creds), ctypes.byref(new_creds))) != 0:
        com_err(b"uu-server", retval, b"getting user-user ticket")
        return 5

    l = ctypes.sizeof(f_inaddr)
    if libc.getsockname(0, ctypes.byref(l_inaddr), ctypes.byref(l)) == -1:
        com_err(b"uu-server", libc.errno, b"getting local address")
        return 6

    if (retval := krb5_auth_con_init(context, ctypes.byref(auth_context))) != 0:
        com_err(b"uu-server", retval, b"making auth_context")
        return 8

    if (retval := krb5_auth_con_setflags(context, auth_context, KRB5_AUTH_CONTEXT_DO_SEQUENCE)) != 0:
        com_err(b"uu-server", retval, b"initializing the auth_context flags")
        return 8

    if (retval := krb5_auth_con_genaddrs(context, auth_context, sock, KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR | KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR)) != 0:
        com_err(b"uu-server", retval, b"generating addrs for auth_context")
        return 9

    if (retval := krb5_mk_req_extended(context, ctypes.byref(auth_context), AP_OPTS_USE_SESSION_KEY, None, new_creds, ctypes.byref(msg))) != 0:
        com_err(b"uu-server", retval, b"making AP_REQ")
        return 8

    if (retval := krb5_write_message(context, ctypes.byref(sock), ctypes.byref(msg))) != 0:
        com_err(b"uu-server", retval, b"writing message to client")
        return 7

    msgtext.length = 32
    msgtext.data = b"Hello, other end of connection."

    if (retval := krb5_mk_safe(context, auth_context, ctypes.byref(msgtext), ctypes.byref(msg), None)) != 0:
        com_err(b"uu-server", retval, b"encoding message to client")
        return 6

    if (retval := krb5_write_message(context, ctypes.byref(sock), ctypes.byref(msg))) != 0:
        com_err(b"uu-server", retval, b"writing message to client")
        return 7

    krb5_free_data_contents(context, ctypes.byref(msg))
    krb5_free_data_contents(context, ctypes.byref(pname_data))
    krb5_free_cred_contents(context, ctypes.byref(creds))
    krb5_free_creds(context, new_creds)
    krb5_cc_close(context, cc)
    krb5_auth_con_free(context, auth_context)
    krb5_free_context(context)
    return 0

def krb5_init_context(context):
    return 0  # Simulated; replace with actual function

def krb5_read_message(context, sock, data):
    return 0  # Simulated; replace with actual function

def krb5_cc_default(context, cc):
    return 0  # Simulated; replace with actual function

def krb5_cc_get_principal(context, cc, client):
    return 0  # Simulated; replace with actual function

def krb5_parse_name(context, pname, server):
    return 0  # Simulated; replace with actual function

def krb5_get_credentials(context, options, cc, creds, new_creds):
    return 0  # Simulated; replace with actual function

def krb5_auth_con_init(context, auth_context):
    return 0  # Simulated; replace with actual function

def krb5_auth_con_setflags(context, auth_context, flags):
    return 0  # Simulated; replace with actual function

def krb5_auth_con_genaddrs(context, auth_context, sock, flags):
    return 0  # Simulated; replace with actual function

def krb5_mk_req_extended(context, auth_context, options, in_data, creds, out_data):
    return 0  # Simulated; replace with actual function

def krb5_write_message(context, sock, data):
    return 0  # Simulated; replace with actual function

def krb5_mk_safe(context, auth_context, in_data, out_data, outdata):
    return 0  # Simulated; replace with actual function

def krb5_free_data_contents(context, data):
    pass  # Simulated; replace with actual function

def krb5_free_cred_contents(context, creds):
    pass  # Simulated; replace with actual function

def krb5_free_creds(context, creds):
    pass  # Simulated; replace with actual function

def krb5_cc_close(context, cc):
    pass  # Simulated; replace with actual function

def krb5_auth_con_free(context, auth_context):
    pass  # Simulated; replace with actual function

def krb5_free_context(context):
    pass  # Simulated; replace with actual function

def com_err(progname, code, message):
    print(f"{progname.decode('utf-8')}: {message.decode('utf-8')} (code {code})")

def AP_OPTS_USE_SESSION_KEY():
    return 0

def KRB5_GC_USER_USER():
    return 0

def KRB5_AUTH_CONTEXT_DO_SEQUENCE():
    return 0

def KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR():
    return 0

def KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR():
    return 0

if __name__ == "__main__":
    main(len(sys.argv), sys.argv)