import ctypes
from ctypes import POINTER, c_char_p, c_void_p, c_size_t

# Load the ImageMagick shared library
lib = ctypes.CDLL("libMagickCore.so")

# Define prototypes for functions to be used
lib.AcquireImage.argtypes = [c_void_p, c_void_p]
lib.AcquireImage.restype = c_void_p

lib.DestroyString.argtypes = [c_char_p]
lib.DestroyString.restype = c_char_p

lib.GetImageOption.argtypes = [c_void_p, c_char_p]
lib.GetImageOption.restype = c_char_p

lib.GetImageProperty.argtypes = [c_void_p, c_char_p, c_void_p]
lib.GetImageProperty.restype = c_char_p

lib.SetImageProperty.argtypes = [c_void_p, c_char_p, c_char_p, c_void_p]
lib.SetImageProperty.restype = c_size_t

# Define the ReadCAPTIONImage function in Python
def ReadCAPTIONImage(image_info, exception):
    caption = ctypes.c_char_p()
    geometry = ctypes.create_string_buffer(4096)
    property = ctypes.c_char_p()
    text = ctypes.c_char_p()

    gravity = ctypes.c_char_p()
    option = ctypes.c_char_p()

    draw_info = c_void_p()

    image = c_void_p()

    split = ctypes.c_int()
    status = ctypes.c_int()

    i = ctypes.c_ssize_t()

    height = ctypes.c_size_t()
    width = ctypes.c_size_t()

    metrics = c_void_p()

    assert image_info is not None
    assert exception is not None

    image = lib.AcquireImage(ctypes.pointer(image_info), exception)

    option = lib.GetImageOption(image_info, b"filename")
    if option is None:
        property = lib.InterpretImageProperties(image_info, image, image_info.filename, exception)
    else:
        if option[:8] == b"caption:":
            property = lib.InterpretImageProperties(image_info, image, option[8:], exception)
        else:
            property = lib.InterpretImageProperties(image_info, image, option, exception)

    lib.SetImageProperty(image, b"caption", property, exception)
    property = lib.DestroyString(property)
    caption = lib.GetImageProperty(image, b"caption", exception)

    # Further processing and function implementation...

# Note: The full function needs to be implemented with appropriate ctypes calls for additional functionality.