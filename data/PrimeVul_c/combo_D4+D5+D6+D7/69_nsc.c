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

static void nsc_decode(NSC_CONTEXT* context)
{
	UINT16 x;
	UINT16 y;
	UINT16 r_w = ROUND_UP_TO(context->width, 8);
	BYTE shft = context->ColorLossLevel - 1;
	BYTE* b_data = context->BitmapData;

	for (y = 0; y < context->height; y++)
	{
		const BYTE* y_p;
		const BYTE* co_p;
		const BYTE* cg_p;
		const BYTE* a_p = context->priv->PlaneBuffers[3] + y * context->width;

		if (context->ChromaSubsamplingLevel)
		{
			y_p = context->priv->PlaneBuffers[0] + y * r_w;
			co_p = context->priv->PlaneBuffers[1] + (y >> 1) * (r_w >> 1);
			cg_p = context->priv->PlaneBuffers[2] + (y >> 1) * (r_w >> 1);
		}
		else
		{
			y_p = context->priv->PlaneBuffers[0] + y * context->width;
			co_p = context->priv->PlaneBuffers[1] + y * context->width;
			cg_p = context->priv->PlaneBuffers[2] + y * context->width;
		}

		for (x = 0; x < context->width; x++)
		{
			INT16 y_v = (INT16) * y_p;
			INT16 co_v = (INT16)(INT8)(*co_p << shft);
			INT16 cg_v = (INT16)(INT8)(*cg_p << shft);
			INT16 r_v = y_v + co_v - cg_v;
			INT16 g_v = y_v + cg_v;
			INT16 b_v = y_v - co_v - cg_v;
			*b_data++ = MINMAX(b_v, 0, 0xFF);
			*b_data++ = MINMAX(g_v, 0, 0xFF);
			*b_data++ = MINMAX(r_v, 0, 0xFF);
			*b_data++ = *a_p;
			y_p++;
			co_p += (context->ChromaSubsamplingLevel ? x % 2 : 1);
			cg_p += (context->ChromaSubsamplingLevel ? x % 2 : 1);
			a_p++;
		}
	}
}

static void nsc_rle_decode(BYTE* in, BYTE* out, UINT32 originalSize)
{
	UINT32 l, lft;
	BYTE v;
	lft = originalSize;

	while (lft > 4)
	{
		v = *in++;

		if (lft == 5)
		{
			*out++ = v;
			lft--;
		}
		else if (v == *in)
		{
			in++;

			if (*in < 0xFF)
			{
				l = (UINT32) * in++;
				l += 2;
			}
			else
			{
				in++;
				l = *((UINT32*) in);
				in += 4;
			}

			FillMemory(out, l, v);
			out += l;
			lft -= l;
		}
		else
		{
			*out++ = v;
			lft--;
		}
	}

	*((UINT32*)out) = *((UINT32*)in);
}

static void nsc_rle_decompress_data(NSC_CONTEXT* context)
{
	UINT16 i;
	BYTE* r;
	UINT32 plnSz, orgSz;
	r = context->Planes;

	for (i = 0; i < 4; i++)
	{
		orgSz = context->OrgByteCount[i];
		plnSz = context->PlaneByteCount[i];

		if (plnSz == 0)
			FillMemory(context->priv->PlaneBuffers[i], orgSz, 0xFF);
		else if (plnSz < orgSz)
			nsc_rle_decode(r, context->priv->PlaneBuffers[i], orgSz);
		else
			CopyMemory(context->priv->PlaneBuffers[i], r, orgSz);

		r += plnSz;
	}
}

static BOOL nsc_stream_initialize(NSC_CONTEXT* context, wStream* s)
{
	int i;

	if (Stream_GetRemainingLength(s) < 20)
		return FALSE;

	for (i = 0; i < 4; i++)
		Stream_Read_UINT32(s, context->PlaneByteCount[i]);

	Stream_Read_UINT8(s, context->ColorLossLevel);
	Stream_Read_UINT8(s, context->ChromaSubsamplingLevel);
	Stream_Seek(s, 2);
	context->Planes = Stream_Pointer(s);
	return TRUE;
}

static BOOL nsc_context_initialize(NSC_CONTEXT* context, wStream* s)
{
	int i;
	UINT32 l, tmp_w, tmp_h;

	if (!nsc_stream_initialize(context, s))
		return FALSE;

	l = context->width * context->height * 4;

	if (!context->BitmapData)
	{
		context->BitmapData = calloc(1, l + 16);

		if (!context->BitmapData)
			return FALSE;

		context->BitmapDataLength = l;
	}
	else if (l > context->BitmapDataLength)
	{
		void* tmp;
		tmp = realloc(context->BitmapData, l + 16);

		if (!tmp)
			return FALSE;

		context->BitmapData = tmp;
		context->BitmapDataLength = l;
	}

	tmp_w = ROUND_UP_TO(context->width, 8);
	tmp_h = ROUND_UP_TO(context->height, 2);
	l = tmp_w * tmp_h;

	if (l > context->priv->PlaneBuffersLength)
	{
		for (i = 0; i < 4; i++)
		{
			void* tmp = (BYTE*) realloc(context->priv->PlaneBuffers[i], l);

			if (!tmp)
				return FALSE;

			context->priv->PlaneBuffers[i] = tmp;
		}

		context->priv->PlaneBuffersLength = l;
	}

	for (i = 0; i < 4; i++)
	{
		context->OrgByteCount[i] = context->width * context->height;
	}

	if (context->ChromaSubsamplingLevel)
	{
		context->OrgByteCount[0] = tmp_w * context->height;
		context->OrgByteCount[1] = (tmp_w >> 1) * (tmp_h >> 1);
		context->OrgByteCount[2] = context->OrgByteCount[1];
	}

	return TRUE;
}

static void nsc_profiler_print(NSC_CONTEXT_PRIV* priv)
{
	PROFILER_PRINT_HEADER
	PROFILER_PRINT(priv->prof_nsc_rle_decompress_data)
	PROFILER_PRINT(priv->prof_nsc_decode)
	PROFILER_PRINT(priv->prof_nsc_rle_compress_data)
	PROFILER_PRINT(priv->prof_nsc_encode)
	PROFILER_PRINT_FOOTER
}

BOOL nsc_context_reset(NSC_CONTEXT* context, UINT32 width, UINT32 height)
{
	if (!context)
		return FALSE;

	context->width = width;
	context->height = height;
	return TRUE;
}

NSC_CONTEXT* nsc_context_new(void)
{
	NSC_CONTEXT* ctx;
	ctx = (NSC_CONTEXT*) calloc(1, sizeof(NSC_CONTEXT));

	if (!ctx)
		return NULL;

	ctx->priv = (NSC_CONTEXT_PRIV*) calloc(1, sizeof(NSC_CONTEXT_PRIV));

	if (!ctx->priv)
		goto error;

	ctx->priv->log = WLog_Get("com.freerdp.codec.nsc");
	WLog_OpenAppender(ctx->priv->log);
	ctx->BitmapData = NULL;
	ctx->decode = nsc_decode;
	ctx->encode = nsc_encode;
	ctx->priv->PlanePool = BufferPool_New(TRUE, 0, 16);

	if (!ctx->priv->PlanePool)
		goto error;

	PROFILER_CREATE(ctx->priv->prof_nsc_rle_decompress_data, "nsc_rle_decompress_data")
	PROFILER_CREATE(ctx->priv->prof_nsc_decode, "nsc_decode")
	PROFILER_CREATE(ctx->priv->prof_nsc_rle_compress_data, "nsc_rle_compress_data")
	PROFILER_CREATE(ctx->priv->prof_nsc_encode, "nsc_encode")
	ctx->ColorLossLevel = 3;
	ctx->ChromaSubsamplingLevel = 1;
	NSC_INIT_SIMD(ctx);
	return ctx;
error:
	nsc_context_free(ctx);
	return NULL;
}

void nsc_context_free(NSC_CONTEXT* context)
{
	size_t i;

	if (!context)
		return;

	if (context->priv)
	{
		for (i = 0; i < 4; i++)
			free(context->priv->PlaneBuffers[i]);

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

BOOL nsc_context_set_pixel_format(NSC_CONTEXT* context, UINT32 pixel_format)
{
	if (!context)
		return FALSE;

	context->format = pixel_format;
	return TRUE;
}

BOOL nsc_process_message(NSC_CONTEXT* context, UINT16 bpp,
                         UINT32 width, UINT32 height,
                         const BYTE* data, UINT32 length,
                         BYTE* pDstData, UINT32 DstFormat,
                         UINT32 nDstStride,
                         UINT32 nXDst, UINT32 nYDst, UINT32 nWidth,
                         UINT32 nHeight, UINT32 flip)
{
	wStream* s;
	BOOL ret;
	s = Stream_New((BYTE*)data, length);

	if (!s)
		return FALSE;

	if (nDstStride == 0)
		nDstStride = nWidth * GetBytesPerPixel(DstFormat);

	switch (bpp)
	{
		case 32:
			context->format = PIXEL_FORMAT_BGRA32;
			break;

		case 24:
			context->format = PIXEL_FORMAT_BGR24;
			break;

		case 16:
			context->format = PIXEL_FORMAT_BGR16;
			break;

		case 8:
			context->format = PIXEL_FORMAT_RGB8;
			break;

		case 4:
			context->format = PIXEL_FORMAT_A4;
			break;

		default:
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