# Python code
import ctypes
from ctypes import c_char_p, c_void_p, CFUNCTYPE, cdll

# Load the shared library
lib = cdll.LoadLibrary('./libabrt.so')

# Define function prototypes
get_map_string_item_or_NULL = lib.get_map_string_item_or_NULL
get_map_string_item_or_NULL.argtypes = [c_void_p, c_char_p]
get_map_string_item_or_NULL.restype = c_char_p

remove_map_string_item = lib.remove_map_string_item
remove_map_string_item.argtypes = [c_void_p, c_char_p]

parse_list = lib.parse_list
parse_list.argtypes = [c_char_p]
parse_list.restype = c_void_p

string_to_bool = lib.string_to_bool
string_to_bool.argtypes = [c_char_p]
string_to_bool.restype = ctypes.c_bool

error_msg = lib.error_msg
error_msg.argtypes = [c_char_p, c_char_p]

# Constants
GPG_CONF = "gpg_keys.conf"

# Variables
settings_bOpenGPGCheck = False
settings_setOpenGPGPublicKeys = None
settings_setBlackListedPkgs = None
settings_setBlackListedPaths = None
settings_bProcessUnpackaged = False
settings_Interpreters = None

def ParseCommon(settings, conf_filename):
    global settings_bOpenGPGCheck, settings_setBlackListedPkgs
    global settings_setBlackListedPaths, settings_bProcessUnpackaged
    global settings_Interpreters

    value = get_map_string_item_or_NULL(settings, b"OpenGPGCheck")
    if value:
        settings_bOpenGPGCheck = string_to_bool(value)
        remove_map_string_item(settings, b"OpenGPGCheck")

    value = get_map_string_item_or_NULL(settings, b"BlackList")
    if value:
        settings_setBlackListedPkgs = parse_list(value)
        remove_map_string_item(settings, b"BlackList")

    value = get_map_string_item_or_NULL(settings, b"BlackListedPaths")
    if value:
        settings_setBlackListedPaths = parse_list(value)
        remove_map_string_item(settings, b"BlackListedPaths")

    value = get_map_string_item_or_NULL(settings, b"ProcessUnpackaged")
    if value:
        settings_bProcessUnpackaged = string_to_bool(value)
        remove_map_string_item(settings, b"ProcessUnpackaged")

    value = get_map_string_item_or_NULL(settings, b"Interpreters")
    if value:
        settings_Interpreters = parse_list(value)
        remove_map_string_item(settings, b"Interpreters")

    # Iteration over map_string
    iter = c_void_p()
    init_map_string_iter = lib.init_map_string_iter
    init_map_string_iter.argtypes = [c_void_p, c_void_p]
    init_map_string_iter(iter, settings)

    next_map_string_iter = lib.next_map_string_iter
    next_map_string_iter.argtypes = [c_void_p, c_void_p, c_void_p]
    next_map_string_iter.restype = ctypes.c_bool

    name = c_char_p()
    while next_map_string_iter(iter, ctypes.byref(name), ctypes.byref(value)):
        error_msg(b"Unrecognized variable '%s' in '%s'", name.value, conf_filename)

def load_gpg_keys():
    settings = lib.new_map_string()
    if not lib.load_abrt_conf_file(GPG_CONF.encode(), settings):
        error_msg(b"Can't load '%s'", GPG_CONF.encode())
        return

    gpg_keys_dir = get_map_string_item_or_NULL(settings, b"GPGKeysDir")
    if gpg_keys_dir:
        lib.log_debug(b"Reading gpg keys from '%s'", gpg_keys_dir)
        gpg_files = lib.get_file_list(gpg_keys_dir, None)
        tmp_gpp_files = gpg_files
        while tmp_gpp_files:
            lib.log_debug(b"Loading gpg key '%s'", lib.fo_get_fullpath(tmp_gpp_files))
            settings_setOpenGPGPublicKeys = lib.g_list_append(settings_setOpenGPGPublicKeys, lib.xstrdup(lib.fo_get_fullpath(tmp_gpp_files)))
            tmp_gpp_files = lib.g_list_next(tmp_gpp_files)

        lib.g_list_free_full(gpg_files, lib.free_file_obj)

def main():
    lib.setlocale(lib.LC_ALL, b"")
    lib.abrt_init(None)

    dump_dir_name = b"."
    conf_filename = None

    lib.log_notice(b"Loading settings")
    if lib.load_conf(conf_filename) != 0:
        return 1

    lib.log_notice(b"Initializing rpm library")
    lib.rpm_init()

    li = settings_setOpenGPGPublicKeys
    while li:
        lib.log_notice(b"Loading GPG key '%s'", li.data)
        lib.rpm_load_gpgkey(li.data)
        li = lib.g_list_next(li)

    r = lib.SavePackageDescriptionToDebugDump(dump_dir_name)

    lib.rpm_destroy()

    return r

if __name__ == "__main__":
    main()