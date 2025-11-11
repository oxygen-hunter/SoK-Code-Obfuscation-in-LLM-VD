import ctypes
from ctypes import c_int, c_char_p, POINTER, Structure

# Load the C shared library
lib = ctypes.CDLL('./epson_printer_driver.so')

class GxDevice(Structure):
    _fields_ = [("has_color", c_int)]

class GxColorValue(Structure):
    _fields_ = [("values", c_int * 3)]

BLACK = 0
MAGENTA = 1
CYAN = 2
VIOLET = 3
YELLOW = 4
RED = 5
GREEN = 6
WHITE = 7

rgb_color = [[[BLACK, VIOLET], [GREEN, CYAN]],
             [[RED, MAGENTA], [YELLOW, WHITE]]]

cv_shift = 8 * 8 - 1

# Python function to call C function
def epson_map_rgb_color(dev, cv):
    if dev.has_color:
        return rgb_color[cv.values[0] >> cv_shift][cv.values[1] >> cv_shift][cv.values[2] >> cv_shift] ^ 7
    else:
        return lib.gx_default_map_rgb_color(ctypes.byref(dev), ctypes.byref(cv))

def epson_map_color_rgb(dev, color, prgb):
    c1 = 255
    if dev.has_color:
        color ^= 7
        if color == BLACK:
            prgb.values = (0, 0, 0)
        elif color == VIOLET:
            prgb.values = (0, 0, c1)
        elif color == GREEN:
            prgb.values = (0, c1, 0)
        elif color == CYAN:
            prgb.values = (0, c1, c1)
        elif color == RED:
            prgb.values = (c1, 0, 0)
        elif color == MAGENTA:
            prgb.values = (c1, 0, c1)
        elif color == YELLOW:
            prgb.values = (c1, c1, 0)
        elif color == WHITE:
            prgb.values = (c1, c1, c1)
    else:
        return lib.gx_default_map_color_rgb(ctypes.byref(dev), color, ctypes.byref(prgb))
    return 0

def epsc_print_page(pdev, prn_stream):
    graphics_modes_9 = [-1, 0, 1, -1, 0x80 + 3]
    graphics_modes_24 = [-1, 32, 33, 39, -1, -1, 0x80 + 40]
    y_24pin = pdev.y_pixels_per_inch > 72
    y_mult = 3 if y_24pin else 1
    line_size = (pdev.width + 7) >> 3
    in_size = line_size * (8 * y_mult)
    in_buf = ctypes.create_string_buffer(in_size + 1)
    out_size = ((pdev.width + 7) & -8) * y_mult
    out_buf = ctypes.create_string_buffer(out_size + 1)

    x_dpi = pdev.x_pixels_per_inch
    start_graphics = (y_24pin and graphics_modes_24 or graphics_modes_9)[x_dpi // 60]
    first_pass = 1 if start_graphics & 0x80 else 0
    last_pass = first_pass * 2
    dots_per_space = x_dpi // 10
    bytes_per_space = dots_per_space * y_mult
    skip = 0
    lnum = 0

    prn_stream.write(b"\033@\033P\033l\000\033Q\377\033U\001\r")

    color_in = in_buf
    color_in_size = in_size
    color_line_size = line_size

    while lnum < pdev.height:
        gdev_prn_copy_scan_lines(pdev, lnum, color_in, color_line_size)

        if color_in[0] == 0 and color_in.raw[1:color_line_size] == b'\x00' * (color_line_size - 1):
            lnum += 1
            skip += 3 // y_mult
            continue

        while skip > 255:
            prn_stream.write(b"\033J\377")
            skip -= 255
        if skip:
            prn_stream.write(b"\033J" + bytes([skip]))

        lcnt = 1 + gdev_prn_copy_scan_lines(pdev, lnum + 1, color_in + color_line_size, color_in_size - color_line_size)

        if lcnt < 8 * y_mult:
            ctypes.memset(color_in + lcnt * color_line_size, 0, color_in_size - lcnt * color_line_size)

        nextcolor = None
        nextmono = None

        do {
            out_blk = out_buf.raw
            out_end = out_buf.raw
            if pdev.has_color:
                current_color = 0
                i = 0
                realbuf = color_in.raw
                outbuf = in_buf.raw
                nextcolor = None

                while i <= color_in_size and outbuf < in_buf.raw + in_size:
                    if i % color_line_size == 0 and i != 0:
                        end_next_bits = whole_bits + i + spare_bits
                        lastbits = 8 - spare_bits
                    else:
                        lastbits = 0

                    for j in range(8 - lastbits):
                        if realbuf[i]:
                            if current_color:
                                if realbuf[i] == current_color:
                                    outbuf[0] |= 1 << j
                                    realbuf[i] = 0
                                elif nextcolor is None:
                                    nextcolor = realbuf + i - (7 - j)
                                    nextmono = outbuf
                            else:
                                outbuf[0] |= 1 << j
                                current_color = realbuf[i]
                                realbuf[i] = 0
                        i += 1

                if current_color:
                    prn_stream.write(b"\033r" + bytes([current_color ^ 7]))

            if y_24pin:
                for inp in range(0, line_size):
                    gdev_prn_transpose_8x8(in_buf.raw + inp, line_size, out_buf.raw + inp * 24, 3)
                    gdev_prn_transpose_8x8(in_buf.raw + inp + line_size * 8, line_size, out_buf.raw + inp * 24 + 1, 3)
                    gdev_prn_transpose_8x8(in_buf.raw + inp + line_size * 16, line_size, out_buf.raw + inp * 24 + 2, 3)
                while out_end > out_blk and out_end[-1] == 0 and out_end[-2] == 0 and out_end[-3] == 0:
                    out_end -= 3
            else:
                for inp in range(0, line_size):
                    gdev_prn_transpose_8x8(in_buf.raw + inp, line_size, out_buf.raw + inp * 8, 1)
                while out_end > out_blk and out_end[-1] == 0:
                    out_end -= 1

            for pass in range(first_pass, last_pass + 1):
                outp = out_blk
                while outp < out_end:
                    if outp[0] == 0 and outp + 12 <= out_end and outp[1] == 0 and outp[2] == 0 and \
                       (outp[3] | outp[4] | outp[5]) == 0 and (outp[6] | outp[7] | outp[8]) == 0 and \
                       (outp[9] | outp[10] | outp[11]) == 0:
                        zp = outp
                        outp += 12
                        while outp + 3 <= out_end and outp[0] == 0 and outp[1] == 0 and outp[2] == 0:
                            outp += 3
                        tpos = (outp - out_buf.raw) // bytes_per_space
                        newp = out_buf.raw + tpos * bytes_per_space
                        if newp > zp + 10:
                            if zp > out_blk:
                                epsc_output_run(out_blk, zp - out_blk, y_mult, start_graphics, prn_stream, pass)
                            prn_stream.write(b"\033D" + bytes([tpos, 0]) + b"\t")
                            out_blk = newp
                    else:
                        outp += y_mult

                if outp > out_blk:
                    epsc_output_run(out_blk, outp - out_blk, y_mult, start_graphics, prn_stream, pass)

                prn_stream.write(b'\r')
        } while nextcolor

        skip = 24
        lnum += 8 * y_mult

    prn_stream.write(b"\f\033@")
    return 0