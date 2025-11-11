#include "gif.h"

static bool updateGCB(GifInfo *info, uint_fast32_t *lastAllocatedGCBIndex) {
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                if (*lastAllocatedGCBIndex < info->gifFilePtr->ImageCount) {
                    dispatcher = 1;
                    break;
                }
                dispatcher = 4;
                break;
            case 1:
                GraphicsControlBlock *tmpInfos = reallocarray(info->controlBlock, info->gifFilePtr->ImageCount + 1, sizeof(GraphicsControlBlock));
                if (tmpInfos == NULL) {
                    dispatcher = 2;
                    break;
                }
                dispatcher = 3;
                break;
            case 2:
                info->gifFilePtr->Error = D_GIF_ERR_NOT_ENOUGH_MEM;
                return false;
            case 3:
                *lastAllocatedGCBIndex = info->gifFilePtr->ImageCount;
                info->controlBlock = tmpInfos;
                setGCBDefaults(&info->controlBlock[info->gifFilePtr->ImageCount]);
                dispatcher = 4;
                break;
            case 4:
                return true;
        }
    }
}

void DDGifSlurp(GifInfo *info, bool decode, bool exitAfterFrame) {
    GifRecordType RecordType;
    GifByteType *ExtData;
    int ExtFunction;
    GifFileType *gifFilePtr;
    gifFilePtr = info->gifFilePtr;
    uint_fast32_t lastAllocatedGCBIndex = 0;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                if (DGifGetRecordType(gifFilePtr, &RecordType) == GIF_ERROR) {
                    dispatcher = 14;
                    break;
                }
                dispatcher = 1;
                break;
            case 1:
                if (RecordType == TERMINATE_RECORD_TYPE) {
                    dispatcher = 14;
                    break;
                }
                bool isInitialPass = !decode && !exitAfterFrame;
                if (RecordType == IMAGE_DESC_RECORD_TYPE) {
                    dispatcher = 2;
                    break;
                }
                if (RecordType == EXTENSION_RECORD_TYPE) {
                    dispatcher = 11;
                    break;
                }
                dispatcher = 13;
                break;
            case 2:
                if (DGifGetImageDesc(gifFilePtr, isInitialPass) == GIF_ERROR) {
                    dispatcher = 14;
                    break;
                }
                dispatcher = 3;
                break;
            case 3:
                if (isInitialPass) {
                    dispatcher = 4;
                    break;
                }
                dispatcher = 7;
                break;
            case 4:
                int_fast32_t widthOverflow = gifFilePtr->Image.Width - gifFilePtr->SWidth;
                int_fast32_t heightOverflow = gifFilePtr->Image.Height - gifFilePtr->SHeight;
                if (widthOverflow > 0 || heightOverflow > 0) {
                    gifFilePtr->SWidth += widthOverflow;
                    gifFilePtr->SHeight += heightOverflow;
                }
                SavedImage *sp = &gifFilePtr->SavedImages[gifFilePtr->ImageCount - 1];
                int_fast32_t topOverflow = gifFilePtr->Image.Top + gifFilePtr->Image.Height - gifFilePtr->SHeight;
                if (topOverflow > 0) {
                    sp->ImageDesc.Top -= topOverflow;
                }
                int_fast32_t leftOverflow = gifFilePtr->Image.Left + gifFilePtr->Image.Width - gifFilePtr->SWidth;
                if (leftOverflow > 0) {
                    sp->ImageDesc.Left -= leftOverflow;
                }
                if (!updateGCB(info, &lastAllocatedGCBIndex)) {
                    dispatcher = 14;
                    break;
                }
                dispatcher = 7;
                break;
            case 7:
                if (decode) {
                    dispatcher = 8;
                    break;
                }
                dispatcher = 10;
                break;
            case 8:
                int_fast32_t widthOverflow = gifFilePtr->Image.Width - info->originalWidth;
                int_fast32_t heightOverflow = gifFilePtr->Image.Height - info->originalHeight;
                const uint_fast32_t newRasterSize = gifFilePtr->Image.Width * gifFilePtr->Image.Height;
                if (newRasterSize > info->rasterSize || widthOverflow > 0 || heightOverflow > 0) {
                    void *tmpRasterBits = reallocarray(info->rasterBits, newRasterSize, sizeof(GifPixelType));
                    if (tmpRasterBits == NULL) {
                        dispatcher = 9;
                        break;
                    }
                    info->rasterBits = tmpRasterBits;
                    info->rasterSize = newRasterSize;
                }
                if (gifFilePtr->Image.Interlace) {
                    uint_fast16_t i, j;
                    uint_fast8_t InterlacedOffset[] = {0, 4, 2, 1};
                    uint_fast8_t InterlacedJumps[] = {8, 8, 4, 2};
                    for (i = 0; i < 4; i++)
                        for (j = InterlacedOffset[i]; j < gifFilePtr->Image.Height; j += InterlacedJumps[i]) {
                            if (DGifGetLine(gifFilePtr, info->rasterBits + j * gifFilePtr->Image.Width, gifFilePtr->Image.Width) == GIF_ERROR) {
                                dispatcher = 9;
                                break;
                            }
                        }
                } else {
                    if (DGifGetLine(gifFilePtr, info->rasterBits, gifFilePtr->Image.Width * gifFilePtr->Image.Height) == GIF_ERROR) {
                        dispatcher = 9;
                        break;
                    }
                }
                if (info->sampleSize > 1) {
                    unsigned char *dst = info->rasterBits;
                    unsigned char *src = info->rasterBits;
                    unsigned char *const srcEndImage = info->rasterBits + gifFilePtr->Image.Width * gifFilePtr->Image.Height;
                    do {
                        unsigned char *srcNextLineStart = src + gifFilePtr->Image.Width * info->sampleSize;
                        unsigned char *const srcEndLine = src + gifFilePtr->Image.Width;
                        unsigned char *dstEndLine = dst + gifFilePtr->Image.Width / info->sampleSize;
                        do {
                            *dst = *src;
                            dst++;
                            src += info->sampleSize;
                        } while (src < srcEndLine);
                        dst = dstEndLine;
                        src = srcNextLineStart;
                    } while (src < srcEndImage);
                }
                return;
            case 9:
                gifFilePtr->Error = D_GIF_ERR_NOT_ENOUGH_MEM;
                break;
            case 10:
                do {
                    if (DGifGetCodeNext(gifFilePtr, &ExtData) == GIF_ERROR) {
                        dispatcher = 14;
                        break;
                    }
                } while (ExtData != NULL);
                if (exitAfterFrame) {
                    return;
                }
                dispatcher = 0;
                break;
            case 11:
                if (DGifGetExtension(gifFilePtr, &ExtFunction, &ExtData) == GIF_ERROR) {
                    dispatcher = 14;
                    break;
                }
                if (isInitialPass) {
                    updateGCB(info, &lastAllocatedGCBIndex);
                    if (readExtensions(ExtFunction, ExtData, info) == GIF_ERROR) {
                        dispatcher = 14;
                        break;
                    }
                }
                dispatcher = 12;
                break;
            case 12:
                while (ExtData != NULL) {
                    if (DGifGetExtensionNext(gifFilePtr, &ExtData) == GIF_ERROR) {
                        dispatcher = 14;
                        break;
                    }
                    if (isInitialPass && readExtensions(ExtFunction, ExtData, info) == GIF_ERROR) {
                        dispatcher = 14;
                        break;
                    }
                }
                dispatcher = 0;
                break;
            case 13:
                dispatcher = 0;
                break;
            case 14:
                info->rewindFunction(info);
                return;
        }
    }
}

static int readExtensions(int ExtFunction, GifByteType *ExtData, GifInfo *info) {
    if (ExtData == NULL) {
        return GIF_OK;
    }
    if (ExtFunction == GRAPHICS_EXT_FUNC_CODE) {
        GraphicsControlBlock *GCB = &info->controlBlock[info->gifFilePtr->ImageCount];
        if (DGifExtensionToGCB(ExtData[0], ExtData + 1, GCB) == GIF_ERROR) {
            return GIF_ERROR;
        }
        GCB->DelayTime = GCB->DelayTime > 1 ? GCB->DelayTime * 10 : DEFAULT_FRAME_DURATION_MS;
    }
    else if (ExtFunction == COMMENT_EXT_FUNC_CODE) {
        if (getComment(ExtData, info) == GIF_ERROR) {
            info->gifFilePtr->Error = D_GIF_ERR_NOT_ENOUGH_MEM;
            return GIF_ERROR;
        }
    }
    else if (ExtFunction == APPLICATION_EXT_FUNC_CODE) {
        char const *string = (char const *) (ExtData + 1);
        if (strncmp("NETSCAPE2.0", string, ExtData[0]) == 0
            || strncmp("ANIMEXTS1.0", string, ExtData[0]) == 0) {
            if (DGifGetExtensionNext(info->gifFilePtr, &ExtData) == GIF_ERROR) {
                return GIF_ERROR;
            }
            if (ExtData && ExtData[0] == 3 && ExtData[1] == 1) {
                uint_fast16_t loopCount = (uint_fast16_t) (ExtData[2] + (ExtData[3] << 8));
                if (loopCount) {
                    loopCount++;
                }
                info->loopCount = loopCount;
            }
        }
    }
    return GIF_OK;
}

static int getComment(GifByteType *Bytes, GifInfo *info) {
    unsigned int len = (unsigned int) Bytes[0];
    size_t offset = info->comment != NULL ? strlen(info->comment) : 0;
    char *ret = reallocarray(info->comment, len + offset + 1, sizeof(char));
    if (ret != NULL) {
        memcpy(ret + offset, &Bytes[1], len);
        ret[len + offset] = 0;
        info->comment = ret;
        return GIF_OK;
    }
    info->gifFilePtr->Error = D_GIF_ERR_NOT_ENOUGH_MEM;
    return GIF_ERROR;
}