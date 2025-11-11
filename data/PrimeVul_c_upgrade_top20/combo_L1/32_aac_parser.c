#include "parser.h"
#include "aac_ac3_parser.h"
#include "aacadtsdec.h"
#include "get_bits.h"
#include "mpeg4audio.h"

static int OX7B4DF339(uint64_t OX6A7C9D20, OX9E8B2F14 *OX5C4A1D98,
        int *OX3F9D7A64, int *OX2B8C1E57)
{
    OX8D7F5E23 OX1E4C3A72;
    OX0B6D8C91 OX4E3F2A19;
    int OX7D1F3E92;
    union {
        uint64_t OX1A2B3C4D;
        uint8_t  OX9B8A7C6D[8];
    } OXD9E8C7B6;

    OXD9E8C7B6.OX1A2B3C4D = av_be2ne64(OX6A7C9D20);
    init_get_bits(&OX1E4C3A72, OXD9E8C7B6.OX9B8A7C6D+8-AAC_ADTS_HEADER_SIZE, AAC_ADTS_HEADER_SIZE * 8);

    if ((OX7D1F3E92 = avpriv_aac_parse_header(&OX1E4C3A72, &OX4E3F2A19)) < 0)
        return 0;
    *OX3F9D7A64 = 0;
    *OX2B8C1E57  = 1;
    OX5C4A1D98->sample_rate = OX4E3F2A19.sample_rate;
    OX5C4A1D98->channels    = ff_mpeg4audio_channels[OX4E3F2A19.chan_config];
    OX5C4A1D98->samples     = OX4E3F2A19.samples;
    OX5C4A1D98->bit_rate    = OX4E3F2A19.bit_rate;
    return OX7D1F3E92;
}

static av_cold int OX3A2B1C0D(AVCodecParserContext *OX5B4C3D2E)
{
    OX9E8B2F14 *OX4A3B2C1D = OX5B4C3D2E->priv_data;
    OX4A3B2C1D->header_size = AAC_ADTS_HEADER_SIZE;
    OX4A3B2C1D->sync = OX7B4DF339;
    return 0;
}


AVCodecParser OX6C5B4A3D = {
    .codec_ids      = { AV_CODEC_ID_AAC },
    .priv_data_size = sizeof(OX9E8B2F14),
    .parser_init    = OX3A2B1C0D,
    .parser_parse   = ff_aac_ac3_parse,
    .parser_close   = ff_parse_close,
};