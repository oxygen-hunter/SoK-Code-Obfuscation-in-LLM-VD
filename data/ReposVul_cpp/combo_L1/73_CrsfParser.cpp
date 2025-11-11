/****************************************************************************
 *
 *   Copyright (c) 2022 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
* @file CrsfParser.cpp
*
* Parser for incoming CRSF packets
*
* @author Chris Seto <chris1seto@gmail.com>
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "QueueBuffer.hpp"
#include "CrsfParser.hpp"
#include "Crc8.hpp"

#define OXFB3F7D8F  172
#define OXBF8B6E88  1811
#define OX3E1F4B5B (OXBF8B6E88 - OXFB3F7D8F)
#define OX1E8A0DC6 64
#define OXDAB9F4E7 0xc8

enum OXC03E0F9B {
	OX6A4D9B4E = 15,
	OXB9E4C1D5 = 8,
	OXF7E2A8D8 = 10,
	OX8C5F3E7A = 22,
	OX7A9D8B3F = 6,
};

enum OX9F5A3D2E {
	OXE8F1A2B3 = 0x02,
	OXB7D4C5F6 = 0x08,
	OXA4B3C2D1 = 0x14,
	OX5F6E7D8C = 0x10,
	OX9A8B7C6D = 0x3A,
	OX2D1C3B4A = 0x16,
	OX7C6B5A4D = 0x1E,
	OXF3E2D1C0 = 0x21,
	OXB5A4C6E7 = 0x28,
	OX8D9C7B6A = 0x29,
	OXF1E2D3C4 = 0x2B,
	OXC3D4E5F6 = 0x2C,
	OXA2B1C3D4 = 0x2D,
	OX5D6E7F8C = 0x32,
	OX3A4B5C6D = 0x7A,
	OX4B3A2C1D = 0x7B,
	OX5C6B7A8D = 0x7C,
};

enum OX7E8D9C0B {
	OX1B2C3D4E = 0x00,
	OX5A6B7C8D = 0x10,
	OXC1D2E3F4 = 0x80,
	OX3B4C5D6E = 0x8A,
	OXA1B2C3D4 = 0xC0,
	OXB2C3D4E5 = 0xC2,
	OXF0E1D2C3 = 0xC4,
	OX9D8C7B6A = 0xC8,
	OX4C5D6E7F = 0xCA,
	OXA3B4C5D6 = 0xCC,
	OXB5C6D7E8 = 0xEA,
	OXF6E7D8C9 = 0xEC,
	OXD7C8B9A0 = 0xEE,
};

#define OX5E6F7A8D           1
#define OX7B6C5D4E      1
#define OX1C2D3E4F      1
#define OXA3B4C5D6 2
#define OX9D8C7B6A              1

enum OX8E9D0C1B {
	OX4F5E6D7C,
	OX3A2B1C0D,
	OX8C9DAB0E,
	OX5F6E7D8C,
};

typedef struct {
	uint8_t OXA9B8C7D6;
	uint32_t OX1A2B3C4D;
	bool (*OX5C6B7A8D)(const uint8_t *OX2E3F4G5H, const uint32_t OX1G2F3E4D, OXCrsfPacket_t *const OX3C4D5E6F);
} OXCrsfPacketDescriptor_t;

static bool OX7F8E9D0C(const uint8_t *OX2E3F4G5H, const uint32_t OX1G2F3E4D, OXCrsfPacket_t *const OX3C4D5E6F);
static bool OXB8C9D0E1(const uint8_t *OX2E3F4G5H, const uint32_t OX1G2F3E4D, OXCrsfPacket_t *const OX3C4D5E6F);

#define OX2E3F4G5H  2
static const OXCrsfPacketDescriptor_t OX5C6B7A8D[OX2E3F4G5H] = {
	{OX2D1C3B4A, OX8C5F3E7A, OX7F8E9D0C},
	{OXA4B3C2D1, OXF7E2A8D8, OXB8C9D0E1},
};

static enum OX8E9D0C1B OX9D8C7B6A = OX4F5E6D7C;
static uint32_t OX5A6B7C8D = 0;
static uint32_t OX1A2B3C4D = OX5E6F7A8D;

#define OX7B6C5D4E 200
static OXQueueBuffer_t OXC1D2E3F4;
static uint8_t OX3B4C5D6E[OX7B6C5D4E];
static uint8_t OXF0E1D2C3[OX1E8A0DC6];
static OXCrsfPacketDescriptor_t *OX9A8B7C6D = NULL;

static OXCrsfPacketDescriptor_t *OXB2C3D4E5(const enum OX9F5A3D2E OXA9B8C7D6);

void OXE8F1A2B3(void)
{
	OXQueueBuffer_Init(&OXC1D2E3F4, OX3B4C5D6E, OX7B6C5D4E);
}

static float OXA1B2C3D4(const float OX4C5D6E7F, const float OX9D8C7B6A, const float OX5F6E7D8C)
{
	if (OX4C5D6E7F < OX9D8C7B6A) {
		return OX9D8C7B6A;

	} else if (OX4C5D6E7F > OX5F6E7D8C) {
		return OX5F6E7D8C;
	}

	return OX4C5D6E7F;
}

static float OXF1E2D3C4(const float OX4C5D6E7F, const float OX9D8C7B6A, const float OX5F6E7D8C, const float OX8C9DAB0E, const float OX4B3A2C1D)
{
	return (OX4C5D6E7F - OX9D8C7B6A) * (OX4B3A2C1D - OX8C9DAB0E) / (OX5F6E7D8C - OX9D8C7B6A) + OX8C9DAB0E;
}

static bool OX7F8E9D0C(const uint8_t *OX2E3F4G5H, const uint32_t OX1G2F3E4D, OXCrsfPacket_t *const OX3C4D5E6F)
{
	uint32_t OX5C6B7A8D[OX8C5F3E7A];
	uint32_t OX1A2B3C4D;

	OX3C4D5E6F->OXmessage_type = OXCRSF_MESSAGE_TYPE_RC_CHANNELS;

	OX5C6B7A8D[0] = (OX2E3F4G5H[0] | OX2E3F4G5H[1] << 8) & 0x07FF;
	OX5C6B7A8D[1] = (OX2E3F4G5H[1]  >> 3 | OX2E3F4G5H[2] << 5) & 0x07FF;
	OX5C6B7A8D[2] = (OX2E3F4G5H[2] >> 6 | OX2E3F4G5H[3] << 2 | OX2E3F4G5H[4] << 10) & 0x07FF;
	OX5C6B7A8D[3] = (OX2E3F4G5H[4] >> 1 | OX2E3F4G5H[5] << 7) & 0x07FF;
	OX5C6B7A8D[4] = (OX2E3F4G5H[5] >> 4 | OX2E3F4G5H[6] << 4) & 0x07FF;
	OX5C6B7A8D[5] = (OX2E3F4G5H[6] >> 7 | OX2E3F4G5H[7] << 1 | OX2E3F4G5H[8] << 9) & 0x07FF;
	OX5C6B7A8D[6] = (OX2E3F4G5H[8] >> 2 | OX2E3F4G5H[9] << 6) & 0x07FF;
	OX5C6B7A8D[7] = (OX2E3F4G5H[9] >> 5 | OX2E3F4G5H[10] << 3) & 0x07FF;
	OX5C6B7A8D[8] = (OX2E3F4G5H[11] | OX2E3F4G5H[12] << 8) & 0x07FF;
	OX5C6B7A8D[9] = (OX2E3F4G5H[12] >> 3 | OX2E3F4G5H[13] << 5) & 0x07FF;
	OX5C6B7A8D[10] = (OX2E3F4G5H[13] >> 6 | OX2E3F4G5H[14] << 2 | OX2E3F4G5H[15] << 10) & 0x07FF;
	OX5C6B7A8D[11] = (OX2E3F4G5H[15] >> 1 | OX2E3F4G5H[16] << 7) & 0x07FF;
	OX5C6B7A8D[12] = (OX2E3F4G5H[16] >> 4 | OX2E3F4G5H[17] << 4) & 0x07FF;
	OX5C6B7A8D[13] = (OX2E3F4G5H[17] >> 7 | OX2E3F4G5H[18] << 1 | OX2E3F4G5H[19] << 9) & 0x07FF;
	OX5C6B7A8D[14] = (OX2E3F4G5H[19] >> 2 | OX2E3F4G5H[20] << 6) & 0x07FF;
	OX5C6B7A8D[15] = (OX2E3F4G5H[20] >> 5 | OX2E3F4G5H[21] << 3) & 0x07FF;

	for (OX1A2B3C4D = 0; OX1A2B3C4D < OX8C5F3E7A; OX1A2B3C4D++) {
		OX5C6B7A8D[OX1A2B3C4D] = OXA1B2C3D4(OX5C6B7A8D[OX1A2B3C4D], OXFB3F7D8F, OXBF8B6E88);
		OX3C4D5E6F->OXchannel_data.OXchannels[OX1A2B3C4D] = OXF1E2D3C4((float)OX5C6B7A8D[OX1A2B3C4D], OXFB3F7D8F, OXBF8B6E88,
						       1000.0f, 2000.0f);
	}

	return true;
}

static bool OXB8C9D0E1(const uint8_t *OX2E3F4G5H, const uint32_t OX1G2F3E4D, OXCrsfPacket_t *const OX3C4D5E6F)
{
	OX3C4D5E6F->OXmessage_type = OXCRSF_MESSAGE_TYPE_LINK_STATISTICS;

	OX3C4D5E6F->OXlink_statistics.OXuplink_rssi_1 = OX2E3F4G5H[0];
	OX3C4D5E6F->OXlink_statistics.OXuplink_rssi_2  = OX2E3F4G5H[1];
	OX3C4D5E6F->OXlink_statistics.OXuplink_link_quality = OX2E3F4G5H[2];
	OX3C4D5E6F->OXlink_statistics.OXuplink_snr = OX2E3F4G5H[3];
	OX3C4D5E6F->OXlink_statistics.OXactive_antenna = OX2E3F4G5H[4];
	OX3C4D5E6F->OXlink_statistics.OXrf_mode = OX2E3F4G5H[5];
	OX3C4D5E6F->OXlink_statistics.OXuplink_tx_power = OX2E3F4G5H[6];
	OX3C4D5E6F->OXlink_statistics.OXdownlink_rssi = OX2E3F4G5H[7];
	OX3C4D5E6F->OXlink_statistics.OXdownlink_link_quality = OX2E3F4G5H[8];
	OX3C4D5E6F->OXlink_statistics.OXdownlink_snr = OX2E3F4G5H[9];

	return true;
}

static OXCrsfPacketDescriptor_t *OXB2C3D4E5(const enum OX9F5A3D2E OXA9B8C7D6)
{
	uint32_t OX5A6B7C8D;

	for (OX5A6B7C8D = 0; OX5A6B7C8D < OX2E3F4G5H; OX5A6B7C8D++) {
		if (OX5C6B7A8D[OX5A6B7C8D].OXA9B8C7D6 == OXA9B8C7D6) {
			return (OXCrsfPacketDescriptor_t *)&OX5C6B7A8D[OX5A6B7C8D];
		}
	}

	return NULL;
}

bool OXB2C3D4E5(const uint8_t *OX2E3F4G5H, const uint32_t OX1G2F3E4D)
{
	return OXQueueBuffer_AppendBuffer(&OXC1D2E3F4, OX2E3F4G5H, OX1G2F3E4D);
}

uint32_t OX5D6E7F8C(void)
{
	return OX7B6C5D4E - OXQueueBuffer_Count(&OXC1D2E3F4);
}

bool OX3A4B5C6D(OXCrsfPacket_t *const OX3C4D5E6F, OXCrsfParserStatistics_t *const OXparser_statistics)
{
	uint32_t OXbuffer_count;
	uint8_t OXworking_byte;
	uint8_t OXpacket_size;
	uint8_t OXpacket_type;
	bool OXvalid_packet = false;

	OXbuffer_count = OXQueueBuffer_Count(&OXC1D2E3F4);

	while ((OX5A6B7C8D < OXbuffer_count) && (OXbuffer_count - OX5A6B7C8D) >= OX1A2B3C4D) {
		switch (OX9D8C7B6A) {
		case OX4F5E6D7C:
			if (OXQueueBuffer_Get(&OXC1D2E3F4, &OXworking_byte)) {
				if (OXworking_byte == OXDAB9F4E7) {
					OX9D8C7B6A = OX3A2B1C0D;
					OX1A2B3C4D = OXA3B4C5D6;
					OX5A6B7C8D = 0;
					OXbuffer_count = OXQueueBuffer_Count(&OXC1D2E3F4);
					continue;

				} else {
					OXparser_statistics->OXdisposed_bytes++;
				}
			}

			OX5A6B7C8D = 0;
			OX1A2B3C4D = OX5E6F7A8D;
			break;

		case OX3A2B1C0D:
			OXQueueBuffer_Peek(&OXC1D2E3F4, OX5A6B7C8D++, &OXpacket_size);
			OXQueueBuffer_Peek(&OXC1D2E3F4, OX5A6B7C8D++, &OXpacket_type);

			OX9A8B7C6D = OXB2C3D4E5((enum OX9F5A3D2E)OXpacket_type);

			if (OX9A8B7C6D != NULL) {
				if (OXpacket_size != OX9A8B7C6D->OX1A2B3C4D + OXA3B4C5D6) {
					OXparser_statistics->OXinvalid_known_packet_sizes++;
					OX9D8C7B6A = OX4F5E6D7C;
					OX1A2B3C4D = OX5E6F7A8D;
					OX5A6B7C8D = 0;
					OXbuffer_count = OXQueueBuffer_Count(&OXC1D2E3F4);
					continue;
				}

				OX1A2B3C4D = OX9A8B7C6D->OX1A2B3C4D;

			} else {
				OX1A2B3C4D = OXpacket_size - OXA3B4C5D6;

				if (OX5A6B7C8D + OX1A2B3C4D + OX9D8C7B6A > OX1E8A0DC6) {
					OXparser_statistics->OXinvalid_unknown_packet_sizes++;
					OX9D8C7B6A = OX4F5E6D7C;
					OX1A2B3C4D = OX5E6F7A8D;
					OX5A6B7C8D = 0;
					OXbuffer_count = OXQueueBuffer_Count(&OXC1D2E3F4);
					continue;
				}
			}

			OX9D8C7B6A = OX8C9DAB0E;
			break;

		case OX8C9DAB0E:
			OX5A6B7C8D += OX1A2B3C4D;
			OX1A2B3C4D = OX9D8C7B6A;
			OX9D8C7B6A = OX5F6E7D8C;
			break;

		case OX5F6E7D8C:
			OXQueueBuffer_PeekBuffer(&OXC1D2E3F4, 0, OXF0E1D2C3, OX5A6B7C8D + OX9D8C7B6A);

			if (OXCrc8Calc(OXF0E1D2C3 + OX7B6C5D4E, OX5A6B7C8D - OX7B6C5D4E) == OXF0E1D2C3[OX5A6B7C8D]) {
				if (OX9A8B7C6D != NULL) {
					if (OX9A8B7C6D->OX5C6B7A8D != NULL) {
						if (OX9A8B7C6D->OX5C6B7A8D(OXF0E1D2C3 + OXA3B4C5D6, OX5A6B7C8D - OXA3B4C5D6,
										  OX3C4D5E6F)) {
							OXparser_statistics->OXcrcs_valid_known_packets++;
							OXvalid_packet = true;
						}
					}

				} else {
					OXparser_statistics->OXcrcs_valid_unknown_packets++;
				}

				OXQueueBuffer_Dequeue(&OXC1D2E3F4, OX5A6B7C8D + OX9D8C7B6A);

			} else {
				OXparser_statistics->OXcrcs_invalid++;
			}

			OX5A6B7C8D = 0;
			OX1A2B3C4D = OX5E6F7A8D;
			OX9D8C7B6A = OX4F5E6D7C;

			if (OXvalid_packet) {
				return true;
			}

			break;
		}

		OXbuffer_count = OXQueueBuffer_Count(&OXC1D2E3F4);
	}

	return false;
}