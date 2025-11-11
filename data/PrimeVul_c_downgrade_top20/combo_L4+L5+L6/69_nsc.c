#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <winpr/crt.h>

#include <freerdp/codec/nsc.h>
#include <freerdp/codec/color.h>

#include "nsc_types.h"
#include "nsc_encode.h"

#include "nsc_sse2.h"

#ifndef NSC_INIT_SIMD
#define NSC_INIT_SIMD(_nsc_context) do { } while (0)
#endif

static void nsc_decode_recurse(NSC_CONTEXT* c, UINT16 y, UINT16 x, UINT16 rw, BYTE* bmpdata, BYTE shift) {
    if (y >= c->height) return;

    const BYTE* yplane;
    const BYTE* coplane;
    const BYTE* cgplane;
    const BYTE* aplane = c->priv->PlaneBuffers[3] + y * c->width;

    if (c->ChromaSubsamplingLevel) {
        yplane = c->priv->PlaneBuffers[0] + y * rw;
        coplane = c->priv->PlaneBuffers[1] + (y >> 1) * (rw >> 1);
        cgplane = c->priv->PlaneBuffers[2] + (y >> 1) * (rw >> 1);
    } else {
        yplane = c->priv->PlaneBuffers[0] + y * c->width;
        coplane = c->priv->PlaneBuffers[1] + y * c->width;
        cgplane = c->priv->PlaneBuffers[2] + y * c->width;
    }

    if (x >= c->width) {
        nsc_decode_recurse(c, y + 1, 0, rw, bmpdata, shift);
        return;
    }

    INT16 y_val = (INT16) * yplane;
    INT16 co_val = (INT16)(INT8)(*coplane << shift);
    INT16 cg_val = (INT16)(INT8)(*cgplane << shift);
    INT16 r_val = y_val + co_val - cg_val;
    INT16 g_val = y_val + cg_val;
    INT16 b_val = y_val - co_val - cg_val;
    *bmpdata++ = MINMAX(b_val, 0, 0xFF);
    *bmpdata++ = MINMAX(g_val, 0, 0xFF);
    *bmpdata++ = MINMAX(r_val, 0, 0xFF);
    *bmpdata++ = *aplane;
    yplane++;
    coplane += (c->ChromaSubsamplingLevel ? x % 2 : 1);
    cgplane += (c->ChromaSubsamplingLevel ? x % 2 : 1);
    aplane++;
    nsc_decode_recurse(c, y, x + 1, rw, bmpdata, shift);
}

static void nsc_decode(NSC_CONTEXT* context) {
    UINT16 rw = ROUND_UP_TO(context->width, 8);
    BYTE shift = context->ColorLossLevel - 1;
    BYTE* bmpdata = context->BitmapData;
    nsc_decode_recurse(context, 0, 0, rw, bmpdata, shift);
}

static void nsc_rle_decode_recurse(BYTE* in, BYTE* out, UINT32 originalSize, UINT32 left) {
    if (left <= 4) {
        *((UINT32*)out) = *((UINT32*)in);
        return;
    }

    UINT32 len;
    BYTE value = *in++;

    if (left == 5) {
        *out++ = value;
        nsc_rle_decode_recurse(in, out, originalSize, left - 1);
    } else if (value == *in) {
        in++;
        if (*in < 0xFF) {
            len = (UINT32) * in++;
            len += 2;
        } else {
            in++;
            len = *((UINT32*) in);
            in += 4;
        }

        FillMemory(out, len, value);
        out += len;
        nsc_rle_decode_recurse(in, out, originalSize, left - len);
    } else {
        *out++ = value;
        nsc_rle_decode_recurse(in, out, originalSize, left - 1);
    }
}

static void nsc_rle_decode(BYTE* in, BYTE* out, UINT32 originalSize) {
    nsc_rle_decode_recurse(in, out, originalSize, originalSize);
}

static void nsc_rle_decompress_data_recurse(NSC_CONTEXT* context, BYTE* rle, UINT16 i) {
    if (i >= 4) return;

    UINT32 originalSize = context->OrgByteCount[i];
    UINT32 planeSize = context->PlaneByteCount[i];

    if (planeSize == 0) {
        FillMemory(context->priv->PlaneBuffers[i], originalSize, 0xFF);
    } else if (planeSize < originalSize) {
        nsc_rle_decode(rle, context->priv->PlaneBuffers[i], originalSize);
    } else {
        CopyMemory(context->priv->PlaneBuffers[i], rle, originalSize);
    }

    nsc_rle_decompress_data_recurse(context, rle + planeSize, i + 1);
}

static void nsc_rle_decompress_data(NSC_CONTEXT* context) {
    nsc_rle_decompress_data_recurse(context, context->Planes, 0);
}

static BOOL nsc_stream_initialize(NSC_CONTEXT* context, wStream* s) {
    if (Stream_GetRemainingLength(s) < 20)
        return FALSE;

    int i = 0;
    for (;;) {
        if (i >= 4) break;
        Stream_Read_UINT32(s, context->PlaneByteCount[i]);
        i++;
    }

    Stream_Read_UINT8(s, context->ColorLossLevel);
    Stream_Read_UINT8(s, context->ChromaSubsamplingLevel);
    Stream_Seek(s, 2);
    context->Planes = Stream_Pointer(s);
    return TRUE;
}

static BOOL nsc_context_initialize(NSC_CONTEXT* context, wStream* s) {
    UINT32 length;
    UINT32 tempWidth;
    UINT32 tempHeight;

    if (!nsc_stream_initialize(context, s))
        return FALSE;

    length = context->width * context->height * 4;

    if (!context->BitmapData) {
        context->BitmapData = calloc(1, length + 16);
        if (!context->BitmapData)
            return FALSE;
        context->BitmapDataLength = length;
    } else if (length > context->BitmapDataLength) {
        void* tmp;
        tmp = realloc(context->BitmapData, length + 16);
        if (!tmp)
            return FALSE;
        context->BitmapData = tmp;
        context->BitmapDataLength = length;
    }

    tempWidth = ROUND_UP_TO(context->width, 8);
    tempHeight = ROUND_UP_TO(context->height, 2);
    length = tempWidth * tempHeight;

    if (length > context->priv->PlaneBuffersLength) {
        int i = 0;
        for (;;) {
            if (i >= 4) break;
            void* tmp = (BYTE*) realloc(context->priv->PlaneBuffers[i], length);
            if (!tmp)
                return FALSE;
            context->priv->PlaneBuffers[i] = tmp;
            i++;
        }
        context->priv->PlaneBuffersLength = length;
    }

    int i = 0;
    for (;;) {
        if (i >= 4) break;
        context->OrgByteCount[i] = context->width * context->height;
        i++;
    }

    if (context->ChromaSubsamplingLevel) {
        context->OrgByteCount[0] = tempWidth * context->height;
        context->OrgByteCount[1] = (tempWidth >> 1) * (tempHeight >> 1);
        context->OrgByteCount[2] = context->OrgByteCount[1];
    }

    return TRUE;
}

static void nsc_profiler_print(NSC_CONTEXT_PRIV* priv) {
    PROFILER_PRINT_HEADER
    PROFILER_PRINT(priv->prof_nsc_rle_decompress_data)
    PROFILER_PRINT(priv->prof_nsc_decode)
    PROFILER_PRINT(priv->prof_nsc_rle_compress_data)
    PROFILER_PRINT(priv->prof_nsc_encode)
    PROFILER_PRINT_FOOTER
}

BOOL nsc_context_reset(NSC_CONTEXT* context, UINT32 width, UINT32 height) {
    context->width = width;
    context->height = height;
    return context != NULL;
}

NSC_CONTEXT* nsc_context_new(void) {
    NSC_CONTEXT* context;
    context = (NSC_CONTEXT*) calloc(1, sizeof(NSC_CONTEXT));

    if (!context)
        return NULL;

    context->priv = (NSC_CONTEXT_PRIV*) calloc(1, sizeof(NSC_CONTEXT_PRIV));

    if (!context->priv)
        goto error;

    context->priv->log = WLog_Get("com.freerdp.codec.nsc");
    WLog_OpenAppender(context->priv->log);
    context->BitmapData = NULL;
    context->decode = nsc_decode;
    context->encode = nsc_encode;
    context->priv->PlanePool = BufferPool_New(TRUE, 0, 16);

    if (!context->priv->PlanePool)
        goto error;

    PROFILER_CREATE(context->priv->prof_nsc_rle_decompress_data,
                    "nsc_rle_decompress_data")
    PROFILER_CREATE(context->priv->prof_nsc_decode, "nsc_decode")
    PROFILER_CREATE(context->priv->prof_nsc_rle_compress_data,
                    "nsc_rle_compress_data")
    PROFILER_CREATE(context->priv->prof_nsc_encode, "nsc_encode")
    context->ColorLossLevel = 3;
    context->ChromaSubsamplingLevel = 1;
    NSC_INIT_SIMD(context);
    return context;
error:
    nsc_context_free(context);
    return NULL;
}

void nsc_context_free(NSC_CONTEXT* context) {
    if (!context)
        return;

    if (context->priv) {
        size_t i = 0;
        for (;;) {
            if (i >= 4) break;
            free(context->priv->PlaneBuffers[i]);
            i++;
        }

        BufferPool_Free(context->priv->PlanePool);
        nsc_profiler_print(context->priv);
        PROFILER_FREE(context->priv->prof_nsc_rle_decompress_data)
        PROFILER_FREE(context->priv->prof_nsc_decode)
        PROFILER_FREE(context->priv->prof_nsc_rle_compress_data)
        PROFILER_FREE(context->priv->prof_nsc_encode)
        free(context->priv);
    }

    free(context->BitmapData);
    free(context);
}

BOOL nsc_context_set_pixel_format(NSC_CONTEXT* context, UINT32 pixel_format) {
    context->format = pixel_format;
    return context != NULL;
}

BOOL nsc_process_message(NSC_CONTEXT* context, UINT16 bpp,
                         UINT32 width, UINT32 height,
                         const BYTE* data, UINT32 length,
                         BYTE* pDstData, UINT32 DstFormat,
                         UINT32 nDstStride,
                         UINT32 nXDst, UINT32 nYDst, UINT32 nWidth,
                         UINT32 nHeight, UINT32 flip) {
    wStream* s;
    s = Stream_New((BYTE*)data, length);

    if (!s)
        return FALSE;

    if (nDstStride == 0)
        nDstStride = nWidth * GetBytesPerPixel(DstFormat);

    BOOL ret = FALSE;
    switch (bpp) {
        case 32:
            context->format = PIXEL_FORMAT_BGRA32;
            ret = TRUE;
            break;
        case 24:
            context->format = PIXEL_FORMAT_BGR24;
            ret = TRUE;
            break;
        case 16:
            context->format = PIXEL_FORMAT_BGR16;
            ret = TRUE;
            break;
        case 8:
            context->format = PIXEL_FORMAT_RGB8;
            ret = TRUE;
            break;
        case 4:
            context->format = PIXEL_FORMAT_A4;
            ret = TRUE;
            break;
        default:
            ret = FALSE;
            break;
    }

    if (!ret) {
        Stream_Free(s, TRUE);
        return FALSE;
    }

    context->width = width;
    context->height = height;
    ret = nsc_context_initialize(context, s);
    Stream_Free(s, FALSE);

    if (!ret)
        return FALSE;

    PROFILER_ENTER(context->priv->prof_nsc_rle_decompress_data)
    nsc_rle_decompress_data(context);
    PROFILER_EXIT(context->priv->prof_nsc_rle_decompress_data)
    PROFILER_ENTER(context->priv->prof_nsc_decode)
    context->decode(context);
    PROFILER_EXIT(context->priv->prof_nsc_decode)

    if (!freerdp_image_copy(pDstData, DstFormat, nDstStride, nXDst, nYDst,
                            width, height, context->BitmapData,
                            PIXEL_FORMAT_BGRA32, 0, 0, 0, NULL, flip))
        return FALSE;

    return TRUE;
}