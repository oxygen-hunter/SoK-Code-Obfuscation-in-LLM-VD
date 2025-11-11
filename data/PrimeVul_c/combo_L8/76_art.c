# Import necessary libraries
from ctypes import CDLL, c_void_p, c_size_t, c_ssize_t, POINTER, byref

# Load the C DLL
magick = CDLL('./libMagickCore.so')

# Define necessary C functions
magick.AcquireImage.argtypes = [c_void_p, c_void_p]
magick.AcquireImage.restype = c_void_p

magick.OpenBlob.argtypes = [c_void_p, c_void_p, c_void_p, c_void_p]
magick.OpenBlob.restype = c_size_t

magick.ReadBlobLSBShort.argtypes = [c_void_p]
magick.ReadBlobLSBShort.restype = c_size_t

magick.ThrowReaderException.argtypes = [c_void_p, c_void_p]
magick.ThrowReaderException.restype = c_void_p

magick.SetImageExtent.argtypes = [c_void_p, c_size_t, c_size_t, c_void_p]
magick.SetImageExtent.restype = c_size_t

magick.AcquireQuantumInfo.argtypes = [c_void_p, c_void_p]
magick.AcquireQuantumInfo.restype = c_void_p

magick.GetQuantumExtent.argtypes = [c_void_p, c_void_p, c_void_p]
magick.GetQuantumExtent.restype = c_size_t

magick.QueueAuthenticPixels.argtypes = [c_void_p, c_size_t, c_ssize_t, c_size_t, c_size_t, c_void_p]
magick.QueueAuthenticPixels.restype = POINTER(c_void_p)

magick.ImportQuantumPixels.argtypes = [c_void_p, c_void_p, c_void_p, c_void_p, c_void_p, c_void_p]
magick.ImportQuantumPixels.restype = c_void_p

magick.SyncAuthenticPixels.argtypes = [c_void_p, c_void_p]
magick.SyncAuthenticPixels.restype = c_size_t

magick.DestroyQuantumInfo.argtypes = [c_void_p]
magick.DestroyQuantumInfo.restype = c_void_p

magick.ThrowFileException.argtypes = [c_void_p, c_void_p, c_void_p, c_void_p]
magick.ThrowFileException.restype = c_void_p

magick.CloseBlob.argtypes = [c_void_p]
magick.CloseBlob.restype = c_void_p

magick.GetFirstImageInList.argtypes = [c_void_p]
magick.GetFirstImageInList.restype = c_void_p

def ReadARTImage(image_info, exception):
    assert image_info
    assert exception
    image = magick.AcquireImage(image_info, exception)
    status = magick.OpenBlob(image_info, image, b"rb", exception)
    if not status:
        image = magick.DestroyImageList(image)
        return None

    image_contents = magick.ReadBlobLSBShort(image)
    image_columns = magick.ReadBlobLSBShort(image)
    magick.ReadBlobLSBShort(image)
    image_rows = magick.ReadBlobLSBShort(image)

    if image_columns == 0 or image_rows == 0:
        magick.ThrowReaderException(b"CorruptImageError", b"ImproperImageHeader")

    if image_info.ping:
        magick.CloseBlob(image)
        return magick.GetFirstImageInList(image)

    status = magick.SetImageExtent(image, image_columns, image_rows, exception)
    if not status:
        return magick.DestroyImageList(image)

    quantum_info = magick.AcquireQuantumInfo(image_info, image)
    if not quantum_info:
        magick.ThrowReaderException(b"ResourceLimitError", b"MemoryAllocationFailed")

    length = magick.GetQuantumExtent(image, quantum_info, b"GrayQuantum")
    for y in range(image_rows):
        q = magick.QueueAuthenticPixels(image, 0, y, image_columns, 1, exception)
        if not q:
            break

        pixels = magick.ReadBlobStream(image, length, magick.GetQuantumPixels(quantum_info), byref(c_ssize_t()))
        if not pixels:
            magick.ThrowReaderException(b"CorruptImageError", b"UnableToReadImageData")

        magick.ImportQuantumPixels(image, None, quantum_info, b"GrayQuantum", pixels, exception)
        magick.ReadBlobStream(image, (-length) & 0x01, magick.GetQuantumPixels(quantum_info), byref(c_ssize_t()))

        if not magick.SyncAuthenticPixels(image, exception):
            break

        if not magick.SetImageProgress(image, b"LoadImageTag", y, image_rows):
            break

    magick.SetQuantumImageType(image, b"GrayQuantum")
    magick.DestroyQuantumInfo(quantum_info)

    if magick.EOFBlob(image):
        magick.ThrowFileException(exception, b"CorruptImageError", b"UnexpectedEndOfFile", image.filename)

    magick.CloseBlob(image)
    return magick.GetFirstImageInList(image)