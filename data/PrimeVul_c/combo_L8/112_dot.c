import ctypes
from ctypes import c_void_p, c_char_p, c_size_t

# Load the C library
lib = ctypes.CDLL('./libgraphviz.so')

class GVC_t(ctypes.Structure):
    pass

class Image(ctypes.Structure):
    pass

class ImageInfo(ctypes.Structure):
    _fields_ = [("signature", c_size_t), ("debug", ctypes.c_int), ("filename", c_char_p)]

class ExceptionInfo(ctypes.Structure):
    _fields_ = [("signature", c_size_t)]

lib.gvContext.restype = ctypes.POINTER(GVC_t)
lib.gvLayout.argtypes = [ctypes.POINTER(GVC_t), c_void_p, c_char_p]
lib.gvRenderFilename.argtypes = [ctypes.POINTER(GVC_t), c_void_p, c_char_p, c_char_p]
lib.gvFreeLayout.argtypes = [ctypes.POINTER(GVC_t), c_void_p]
lib.agclose.argtypes = [c_void_p]
lib.agread.argtypes = [c_void_p]

graphic_context = lib.gvContext()

def ReadDOTImage(image_info, exception):
    command = ctypes.create_string_buffer(256)
    option = None
    graph = None
    image = ctypes.POINTER(Image)()
    read_info = ctypes.POINTER(ImageInfo)()
    status = False

    assert image_info is not None
    assert image_info.signature == 0xF0E1D2C3  # MagickCoreSignature
    if image_info.debug:
        print(f"Debug: {image_info.filename}")
    assert exception is not None
    assert exception.signature == 0xF0E1D2C3  # MagickCoreSignature
    assert graphic_context is not None

    image = AcquireImage(image_info, exception)
    status = OpenBlob(image_info, image, "rb", exception)
    if not status:
        return None

    read_info = CloneImageInfo(image_info)
    SetImageInfoBlob(read_info, None, 0)
    read_info.magick = "SVG".encode()
    AcquireUniqueFilename(read_info.filename)

    command.value = f"-Tsvg -o{read_info.filename} {image_info.filename}".encode()
    graph = lib.agread(GetBlobFileHandle(image))
    if not graph:
        RelinquishUniqueFileResource(read_info.filename)
        return None

    option = GetImageOption(image_info, "dot:layout-engine")
    if option is None:
        lib.gvLayout(graphic_context, graph, "dot".encode())
    else:
        lib.gvLayout(graphic_context, graph, option)

    lib.gvRenderFilename(graphic_context, graph, "svg".encode(), read_info.filename)
    lib.gvFreeLayout(graphic_context, graph)
    lib.agclose(graph)

    read_info.magick = "SVG".encode()
    image = ReadImage(read_info, exception)
    RelinquishUniqueFileResource(read_info.filename)
    read_info = DestroyImageInfo(read_info)
    if not image:
        return None
    return GetFirstImageInList(image)

def RegisterDOTImage():
    entry = AcquireMagickInfo("DOT".encode(), "DOT".encode(), "Graphviz".encode())
    entry.decoder = ReadDOTImage
    entry.flags ^= 0x00000002  # CoderBlobSupportFlag
    RegisterMagickInfo(entry)
    entry = AcquireMagickInfo("DOT".encode(), "GV".encode(), "Graphviz".encode())
    entry.decoder = ReadDOTImage
    entry.flags ^= 0x00000002  # CoderBlobSupportFlag
    RegisterMagickInfo(entry)

def UnregisterDOTImage():
    UnregisterMagickInfo("GV".encode())
    UnregisterMagickInfo("DOT".encode())
    if graphic_context:
        lib.gvFreeContext(graphic_context)