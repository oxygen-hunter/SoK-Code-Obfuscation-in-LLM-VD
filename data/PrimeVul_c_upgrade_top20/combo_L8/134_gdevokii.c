import ctypes
from ctypes import c_int, c_char_p, c_void_p

# Load the C shared library
lib = ctypes.CDLL('./printer_driver.so')  # Assuming the C code is compiled into a shared library

# Define the argument and return types for the C functions
lib.okiibm_print_page1.argtypes = [c_void_p, c_void_p, c_int, c_char_p, c_int, c_char_p, c_int]
lib.okiibm_print_page1.restype = c_int

lib.okiibm_output_run.argtypes = [c_void_p, c_int, c_int, c_char_p, c_void_p, c_int]
lib.okiibm_output_run.restype = None

# Python wrapper for the C function okiibm_print_page
def okiibm_print_page(gx_device_printer_pdev, gp_file_prn_stream):
    okiibm_init_string = b'\x18'
    okiibm_end_string = b'\x0c'
    okiibm_one_direct = b'\x1b\x55\x01'
    okiibm_two_direct = b'\x1b\x55\x00'

    init_string = bytearray(16)
    end_string = bytearray(16)

    init_length = len(okiibm_init_string)
    end_length = len(okiibm_end_string)

    init_string[:init_length] = okiibm_init_string
    end_string[:end_length] = okiibm_end_string

    if gx_device_printer_pdev.y_pixels_per_inch > 72 and gx_device_printer_pdev.x_pixels_per_inch > 60:
        init_string[init_length:init_length + len(okiibm_one_direct)] = okiibm_one_direct
        init_length += len(okiibm_one_direct)

        end_string[end_length:end_length + len(okiibm_two_direct)] = okiibm_two_direct
        end_length += len(okiibm_two_direct)

    return lib.okiibm_print_page1(
        ctypes.byref(gx_device_printer_pdev),
        ctypes.byref(gp_file_prn_stream),
        1 if gx_device_printer_pdev.y_pixels_per_inch > 72 else 0,
        init_string,
        init_length,
        end_string,
        end_length
    )