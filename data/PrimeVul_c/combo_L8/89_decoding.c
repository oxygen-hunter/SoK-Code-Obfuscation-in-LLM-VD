from ctypes import CDLL, c_bool, c_int, c_void_p, POINTER
import os
import platform

# Load C standard library
if platform.system() == 'Windows':
    libc = CDLL('msvcrt.dll')
else:
    libc = CDLL('libc.so.6')

# Define C functions with ctypes
reallocarray = libc.reallocarray
reallocarray.argtypes = [c_void_p, c_int, c_int]
reallocarray.restype = c_void_p

class GifInfo:
    pass # Placeholder for GifInfo structure

def updateGCB(info, lastAllocatedGCBIndex):
    if lastAllocatedGCBIndex < info.gifFilePtr.ImageCount:
        tmpInfos = reallocarray(info.controlBlock, info.gifFilePtr.ImageCount + 1, sizeof(GraphicsControlBlock))
        if tmpInfos is None:
            info.gifFilePtr.Error = D_GIF_ERR_NOT_ENOUGH_MEM
            return False
        lastAllocatedGCBIndex = info.gifFilePtr.ImageCount
        info.controlBlock = tmpInfos
        setGCBDefaults(info.controlBlock[info.gifFilePtr.ImageCount])
    return True

def DDGifSlurp(info, decode, exitAfterFrame):
    RecordType = GifRecordType()
    ExtData = POINTER(GifByteType)()
    ExtFunction = c_int()
    gifFilePtr = info.gifFilePtr
    lastAllocatedGCBIndex = 0
    while True:
        if DGifGetRecordType(gifFilePtr, RecordType) == GIF_ERROR:
            break
        isInitialPass = not decode and not exitAfterFrame
        if RecordType == IMAGE_DESC_RECORD_TYPE:
            if DGifGetImageDesc(gifFilePtr, isInitialPass) == GIF_ERROR:
                break
            if isInitialPass:
                widthOverflow = gifFilePtr.Image.Width - gifFilePtr.SWidth
                heightOverflow = gifFilePtr.Image.Height - gifFilePtr.SHeight
                if widthOverflow > 0 or heightOverflow > 0:
                    gifFilePtr.SWidth += widthOverflow
                    gifFilePtr.SHeight += heightOverflow
                sp = gifFilePtr.SavedImages[gifFilePtr.ImageCount - 1]
                topOverflow = gifFilePtr.Image.Top + gifFilePtr.Image.Height - gifFilePtr.SHeight
                if topOverflow > 0:
                    sp.ImageDesc.Top -= topOverflow
                leftOverflow = gifFilePtr.Image.Left + gifFilePtr.Image.Width - gifFilePtr.SWidth
                if leftOverflow > 0:
                    sp.ImageDesc.Left -= leftOverflow
                if not updateGCB(info, lastAllocatedGCBIndex):
                    break
            if decode:
                widthOverflow = gifFilePtr.Image.Width - info.originalWidth
                heightOverflow = gifFilePtr.Image.Height - info.originalHeight
                newRasterSize = gifFilePtr.Image.Width * gifFilePtr.Image.Height
                if newRasterSize > info.rasterSize or widthOverflow > 0 or heightOverflow > 0:
                    tmpRasterBits = reallocarray(info.rasterBits, newRasterSize, sizeof(GifPixelType))
                    if tmpRasterBits is None:
                        gifFilePtr.Error = D_GIF_ERR_NOT_ENOUGH_MEM
                        break
                    info.rasterBits = tmpRasterBits
                    info.rasterSize = newRasterSize
                if gifFilePtr.Image.Interlace:
                    InterlacedOffset = (0, 4, 2, 1)
                    InterlacedJumps = (8, 8, 4, 2)
                    for i in range(4):
                        for j in range(InterlacedOffset[i], gifFilePtr.Image.Height, InterlacedJumps[i]):
                            if DGifGetLine(gifFilePtr, info.rasterBits + j * gifFilePtr.Image.Width, gifFilePtr.Image.Width) == GIF_ERROR:
                                break
                else:
                    if DGifGetLine(gifFilePtr, info.rasterBits, gifFilePtr.Image.Width * gifFilePtr.Image.Height) == GIF_ERROR:
                        break
                if info.sampleSize > 1:
                    dst = info.rasterBits
                    src = info.rasterBits
                    srcEndImage = info.rasterBits + gifFilePtr.Image.Width * gifFilePtr.Image.Height
                    while src < srcEndImage:
                        srcNextLineStart = src + gifFilePtr.Image.Width * info.sampleSize
                        srcEndLine = src + gifFilePtr.Image.Width
                        dstEndLine = dst + gifFilePtr.Image.Width // info.sampleSize
                        while src < srcEndLine:
                            dst[0] = src[0]
                            dst += 1
                            src += info.sampleSize
                        dst = dstEndLine
                        src = srcNextLineStart
                return
            else:
                while True:
                    if DGifGetCodeNext(gifFilePtr, ExtData) == GIF_ERROR:
                        break
                    if ExtData is None:
                        break
                if exitAfterFrame:
                    return
        elif RecordType == EXTENSION_RECORD_TYPE:
            if DGifGetExtension(gifFilePtr, ExtFunction, ExtData) == GIF_ERROR:
                break
            if isInitialPass:
                updateGCB(info, lastAllocatedGCBIndex)
                if readExtensions(ExtFunction, ExtData, info) == GIF_ERROR:
                    break
            while ExtData is not None:
                if DGifGetExtensionNext(gifFilePtr, ExtData) == GIF_ERROR:
                    break
                if isInitialPass and readExtensions(ExtFunction, ExtData, info) == GIF_ERROR:
                    break
        elif RecordType == TERMINATE_RECORD_TYPE:
            break
    info.rewindFunction(info)

def readExtensions(ExtFunction, ExtData, info):
    if ExtData is None:
        return GIF_OK
    if ExtFunction == GRAPHICS_EXT_FUNC_CODE:
        GCB = info.controlBlock[info.gifFilePtr.ImageCount]
        if DGifExtensionToGCB(ExtData[0], ExtData + 1, GCB) == GIF_ERROR:
            return GIF_ERROR
        GCB.DelayTime = GCB.DelayTime * 10 if GCB.DelayTime > 1 else DEFAULT_FRAME_DURATION_MS
    elif ExtFunction == COMMENT_EXT_FUNC_CODE:
        if getComment(ExtData, info) == GIF_ERROR:
            info.gifFilePtr.Error = D_GIF_ERR_NOT_ENOUGH_MEM
            return GIF_ERROR
    elif ExtFunction == APPLICATION_EXT_FUNC_CODE:
        string = c_char_p(ExtData + 1)
        if string.value.startswith(b"NETSCAPE2.0") or string.value.startswith(b"ANIMEXTS1.0"):
            if DGifGetExtensionNext(info.gifFilePtr, ExtData) == GIF_ERROR:
                return GIF_ERROR
            if ExtData and ExtData[0] == 3 and ExtData[1] == 1:
                loopCount = c_uint16(ExtData[2] + (ExtData[3] << 8))
                if loopCount:
                    loopCount += 1
                info.loopCount = loopCount
    return GIF_OK

def getComment(Bytes, info):
    len_ = c_uint(Bytes[0])
    offset = len(info.comment) if info.comment is not None else 0
    ret = reallocarray(info.comment, len_ + offset + 1, sizeof(c_char))
    if ret:
        memmove(ret + offset, Bytes + 1, len_)
        ret[len_ + offset] = 0
        info.comment = ret
        return GIF_OK
    info.gifFilePtr.Error = D_GIF_ERR_NOT_ENOUGH_MEM
    return GIF_ERROR