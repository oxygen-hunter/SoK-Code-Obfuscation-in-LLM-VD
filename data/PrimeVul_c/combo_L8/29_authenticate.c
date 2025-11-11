from ctypes import cdll, c_char_p, c_int

lib = cdll.LoadLibrary('./base64encode.so')

def base64_encode(buf, pad):
    out = (c_char_p * 1024)()  # Allocate buffer for output
    buf_c = c_char_p(buf.encode('utf-8'))
    lib.base64_encode(buf_c, len(buf), out, pad)
    return out.value.decode('utf-8')

def gen_challenge(addr):
    import os, time
    from hashlib import md5
    addr = addr[:17]
    pid = os.getpid()
    sec = int(time.time())
    usec = int((time.time() - sec) * 1000000)
    input_data = f"{addr}{sec}{usec}{pid}".encode('utf-8')
    digest = md5(input_data).digest()
    return base64_encode(digest, 0)

def generate_hash(in_data, challenge):
    from hashlib import md5
    data = (in_data + challenge).encode('utf-8')
    digest = md5(data).digest()
    return base64_encode(digest, 0)

def check_secret(module, user, group, challenge, pass_):
    # Simulate reading from a secret file
    with open("secrets.txt", "r") as f:
        lines = f.readlines()
    
    for line in lines:
        s = line.strip()
        if s.startswith("#"):  # Skip comments
            continue
        if group and s.startswith('@' + group):
            secret = s.split(':', 1)[1].strip()
            if generate_hash(secret, challenge) == pass_:
                return None
        if s.startswith(user):
            secret = s.split(':', 1)[1].strip()
            if generate_hash(secret, challenge) == pass_:
                return None
    return "password mismatch"

def auth_server(module, host, addr, leader):
    users = "user1,user2"  # Example authorized users
    challenge = gen_challenge(addr)
    pass_ = input("Enter password: ")
    user = input("Enter username: ")

    if user not in users.split(','):
        return None

    err = check_secret(module, user, None, challenge, pass_)

    if err:
        print(f"auth failed on module {module} from {host} ({addr}) for {user}: {err}")
        return None

    return user

def auth_client(fd, user, challenge):
    pass_ = input("Password: ")
    pass2 = generate_hash(pass_, challenge)
    print(f"{user} {pass2}")

# Load the C DLL
lib.base64_encode.restype = None
lib.base64_encode.argtypes = [c_char_p, c_int, c_char_p, c_int]