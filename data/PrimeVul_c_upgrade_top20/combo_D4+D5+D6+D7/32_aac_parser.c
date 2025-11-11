#include "parser.h"
#include "aac_ac3_parser.h"
#include "aacadtsdec.h"
#include "get_bits.h"
#include "mpeg4audio.h"

static int aac_sync(uint64_t state, AACAC3ParseContext *hdr_info,
        int *need_next_header, int *new_frame_start)
{
    GetBitContext bits;
    AACADTSHeaderInfo hdr;
    int size;
    union {
        uint8_t  u8[8];
        uint64_t u64;
    } tmp;

    tmp.u64 = av_be2ne64(state);
    init_get_bits(&bits, tmp.u8+8-AAC_ADTS_HEADER_SIZE, AAC_ADTS_HEADER_SIZE * 8);

    if ((size = avpriv_aac_parse_header(&bits, &hdr)) < 0)
        return 0;
    *new_frame_start  = 1;
    *need_next_header = 0;
    hdr_info->samples     = hdr.samples;
    hdr_info->channels    = ff_mpeg4audio_channels[hdr.chan_config];
    hdr_info->bit_rate    = hdr.bit_rate;
    hdr_info->sample_rate = hdr.sample_rate;
    return size;
}

static av_cold int aac_parse_init(AVCodecParserContext *s1)
{
    AACAC3ParseContext *s = s1->priv_data;
    s->sync = aac_sync;
    s->header_size = AAC_ADTS_HEADER_SIZE;
    return 0;
}

AVCodecParser ff_aac_parser = {
    .parser_close   = ff_parse_close,
    .parser_parse   = ff_aac_ac3_parse,
    .parser_init    = aac_parse_init,
    .codec_ids      = { AV_CODEC_ID_AAC },
    .priv_data_size = sizeof(AACAC3ParseContext),
};