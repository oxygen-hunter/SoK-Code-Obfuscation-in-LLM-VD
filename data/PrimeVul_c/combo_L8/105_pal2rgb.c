import ctypes
import os

# Load the TIFF library
libtiff = ctypes.CDLL('/usr/lib/libtiff.so')  # Adjust the path according to your system

class TIFF(ctypes.Structure):
    pass

TIFFOpen = libtiff.TIFFOpen
TIFFOpen.restype = ctypes.POINTER(TIFF)
TIFFOpen.argtypes = [ctypes.c_char_p, ctypes.c_char_p]

TIFFClose = libtiff.TIFFClose
TIFFClose.argtypes = [ctypes.POINTER(TIFF)]

TIFFGetField = libtiff.TIFFGetField
TIFFGetField.restype = ctypes.c_int
TIFFGetField.argtypes = [ctypes.POINTER(TIFF), ctypes.c_uint16, ctypes.c_void_p]

TIFFSetField = libtiff.TIFFSetField
TIFFSetField.restype = ctypes.c_int
TIFFSetField.argtypes = [ctypes.POINTER(TIFF), ctypes.c_uint16, ctypes.c_void_p]

TIFFReadScanline = libtiff.TIFFReadScanline
TIFFReadScanline.restype = ctypes.c_int
TIFFReadScanline.argtypes = [ctypes.POINTER(TIFF), ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint16]

TIFFWriteScanline = libtiff.TIFFWriteScanline
TIFFWriteScanline.restype = ctypes.c_int
TIFFWriteScanline.argtypes = [ctypes.POINTER(TIFF), ctypes.c_void_p, ctypes.c_uint32, ctypes.c_uint16]

_TIFFmalloc = libtiff._TIFFmalloc
_TIFFmalloc.restype = ctypes.c_void_p
_TIFFmalloc.argtypes = [ctypes.c_size_t]

_TIFFfree = libtiff._TIFFfree
_TIFFfree.argtypes = [ctypes.c_void_p]

tiff_tags = {
    'TIFFTAG_PHOTOMETRIC': 262,
    'TIFFTAG_COLORMAP': 320,
    'TIFFTAG_BITSPERSAMPLE': 258,
    'TIFFTAG_IMAGEWIDTH': 256,
    'TIFFTAG_IMAGELENGTH': 257,
    'TIFFTAG_COMPRESSION': 259,
    'TIFFTAG_JPEGQUALITY': 65537,
    'TIFFTAG_JPEGCOLORMODE': 65538,
    'TIFFTAG_SAMPLESPERPIXEL': 277,
    'TIFFTAG_PLANARCONFIG': 284,
    'TIFFTAG_ROWSPERSTRIP': 278,
    'TIFFTAG_PREDICTOR': 317
}

COMPRESSION_NONE = 1
COMPRESSION_PACKBITS = 32773
COMPRESSION_JPEG = 7
COMPRESSION_LZW = 5
COMPRESSION_DEFLATE = 32946
JPEGCOLORMODE_RGB = 0
PHOTOMETRIC_RGB = 2
PHOTOMETRIC_YCBCR = 6
PLANARCONFIG_CONTIG = 1
PLANARCONFIG_SEPARATE = 2
PHOTOMETRIC_PALETTE = 3

def usage():
    print("Usage: <script> input.tif output.tif")
    exit(-1)

def checkcmap(n, r, g, b):
    for i in range(n):
        if r[i] >= 256 or g[i] >= 256 or b[i] >= 256:
            return 16
    print("Warning, assuming 8-bit colormap.")
    return 8

def processCompressOptions(opt):
    global compression, quality, jpegcolormode
    if opt == "none":
        compression = COMPRESSION_NONE
    elif opt == "packbits":
        compression = COMPRESSION_PACKBITS
    elif opt.startswith("jpeg"):
        compression = COMPRESSION_JPEG
        parts = opt.split(':')
        for part in parts:
            if part.isdigit():
                quality = int(part)
            elif part == 'r':
                jpegcolormode = JPEGCOLORMODE_RGB
            else:
                usage()
    elif opt.startswith("lzw"):
        compression = COMPRESSION_LZW
    elif opt.startswith("zip"):
        compression = COMPRESSION_DEFLATE
    else:
        return False
    return True

def cpTags(in_tiff, out_tiff):
    for tag, _ in tiff_tags.items():
        value = ctypes.c_uint16()
        if TIFFGetField(in_tiff, tiff_tags[tag], ctypes.byref(value)):
            TIFFSetField(out_tiff, tiff_tags[tag], value)

def main():
    import sys
    if len(sys.argv) != 3:
        usage()

    in_file = sys.argv[1].encode('utf-8')
    out_file = sys.argv[2].encode('utf-8')

    in_tiff = TIFFOpen(in_file, b"r")
    if not in_tiff:
        print(f"Failed to open {in_file.decode('utf-8')}")
        return

    out_tiff = TIFFOpen(out_file, b"w")
    if not out_tiff:
        print(f"Failed to open {out_file.decode('utf-8')}")
        TIFFClose(in_tiff)
        return

    cpTags(in_tiff, out_tiff)

    TIFFClose(in_tiff)
    TIFFClose(out_tiff)

if __name__ == "__main__":
    main()