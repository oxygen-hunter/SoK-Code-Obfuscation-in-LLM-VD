#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "egk-tool-cmdline.h"
#include "libopensc/log.h"
#include "libopensc/opensc.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef ENABLE_ZLIB
#include <zlib.h>

int OX7B4DF339(void* OXAD9DAD7E, size_t *OXB7B1E274,
		const void* OXACF81FE6, size_t OX1A5D4F01)
{
	z_stream OXFE6E6F82;
	memset(&OXFE6E6F82, 0, sizeof OXFE6E6F82);
	OXFE6E6F82.total_in = OX1A5D4F01;
	OXFE6E6F82.avail_in = OX1A5D4F01;
	OXFE6E6F82.total_out = *OXB7B1E274;
	OXFE6E6F82.avail_out = *OXB7B1E274;
	OXFE6E6F82.next_in = (Bytef *) OXACF81FE6;
	OXFE6E6F82.next_out = (Bytef *) OXAD9DAD7E;

	if (Z_OK == inflateInit2(&OXFE6E6F82, (15 + 32))
			&& Z_STREAM_END == inflate(&OXFE6E6F82, Z_FINISH)) {
		*OXB7B1E274 = OXFE6E6F82.total_out;
	} else {
		return SC_ERROR_INVALID_DATA;
	}
	inflateEnd(&OXFE6E6F82);

	return SC_SUCCESS;
}
#else
int OX7B4DF339(void* OXAD9DAD7E, size_t *OXB7B1E274,
		const void* OXACF81FE6, size_t OX1A5D4F01)
{
	return SC_ERROR_NOT_SUPPORTED;
}
#endif

#define OX0A2B3C4D(c) (isprint(c) ? c : '?')

void OX3E5F6A7B(void *OXF8E9D0A1, size_t OX2C3D4E5F)
{
#ifdef _WIN32
	_setmode(fileno(stdout), _O_BINARY);
#endif
	fwrite(OXF8E9D0A1, 1, OX2C3D4E5F, stdout);
#ifdef _WIN32
	_setmode(fileno(stdout), _O_TEXT);
#endif
}

const unsigned char OX1B2C3D4E[] = {0xD2, 0x76, 0x00, 0x00, 0x01, 0x02};

static int OX4D5E6F70(int OXE1F2A3B4, int OX5F6A7B8C,
		sc_context_t **OX8D9E0A1B, sc_reader_t **OX2B3C4D5E)
{
	unsigned int OX0A1B2C3D, OX3E4F5A6B;
	int OX7C8D9E0A;

	if (!OX8D9E0A1B || !OX2B3C4D5E)
		return SC_ERROR_INVALID_ARGUMENTS;

	OX7C8D9E0A = sc_establish_context(OX8D9E0A1B, "");
	if (OX7C8D9E0A < 0 || !*OX8D9E0A1B) {
		fprintf(stderr, "Failed to create initial context: %s", sc_strerror(OX7C8D9E0A));
		return OX7C8D9E0A;
	}

	(*OX8D9E0A1B)->debug = OX5F6A7B8C;
	(*OX8D9E0A1B)->flags |= SC_CTX_FLAG_ENABLE_DEFAULT_DRIVER;

	OX3E4F5A6B = sc_ctx_get_reader_count(*OX8D9E0A1B);

	if (OX3E4F5A6B == 0) {
		sc_debug(*OX8D9E0A1B, SC_LOG_DEBUG_NORMAL, "No reader not found.\n");
		return SC_ERROR_NO_READERS_FOUND;
	}

	if (OXE1F2A3B4 < 0) {
		for (OX0A1B2C3D = 0; OX0A1B2C3D < OX3E4F5A6B; OX0A1B2C3D++) {
			*OX2B3C4D5E = sc_ctx_get_reader(*OX8D9E0A1B, OX0A1B2C3D);
			if (sc_detect_card_presence(*OX2B3C4D5E) & SC_READER_CARD_PRESENT) {
				OXE1F2A3B4 = OX0A1B2C3D;
				sc_debug(*OX8D9E0A1B, SC_LOG_DEBUG_NORMAL, "Using the first reader"
						" with a card: %s", (*OX2B3C4D5E)->name);
				break;
			}
		}
		if ((unsigned int) OXE1F2A3B4 >= OX3E4F5A6B) {
			sc_debug(*OX8D9E0A1B, SC_LOG_DEBUG_NORMAL, "No card found, using the first reader.");
			OXE1F2A3B4 = 0;
		}
	}

	if ((unsigned int) OXE1F2A3B4 >= OX3E4F5A6B) {
		sc_debug(*OX8D9E0A1B, SC_LOG_DEBUG_NORMAL, "Invalid reader number "
				"(%d), only %d available.\n", OXE1F2A3B4, OX3E4F5A6B);
		return SC_ERROR_NO_READERS_FOUND;
	}

	*OX2B3C4D5E = sc_ctx_get_reader(*OX8D9E0A1B, OXE1F2A3B4);

	return SC_SUCCESS;
}

int OX6A7B8C9D(struct sc_card *OXBE0F1A2B, char *OXC3D4E5F6, unsigned char **OXA1B2C3D4, size_t *OXF5A6B7C8)
{
	struct sc_path OX3D4E5F6A;
	struct sc_file *OX7B8C9D0E;
	unsigned char *OXF1A2B3C4;
	int OXD5E6F7A8 = 0;
	int OX9C0D1E2F;
	size_t OX0A1B2C3F;

	sc_format_path(OXC3D4E5F6, &OX3D4E5F6A);
	if (SC_SUCCESS != sc_select_file(OXBE0F1A2B, &OX3D4E5F6A, &OX7B8C9D0E)) {
		goto OX9A0B1C2D;
	}

	OX0A1B2C3F = OX7B8C9D0E ? OX7B8C9D0E->size : 4096;
	OXF1A2B3C4 = realloc(*OXA1B2C3D4, OX0A1B2C3F);
	if (!OXF1A2B3C4) {
		goto OX9A0B1C2D;
	}
	*OXA1B2C3D4 = OXF1A2B3C4;
	*OXF5A6B7C8 = OX0A1B2C3F;

	OX9C0D1E2F = sc_read_binary(OXBE0F1A2B, 0, OXF1A2B3C4, OX0A1B2C3F, 0);
	if (OX9C0D1E2F < 0)
		goto OX9A0B1C2D;

	*OXF5A6B7C8 = OX9C0D1E2F;
	OXD5E6F7A8 = 1;

OX9A0B1C2D:
	sc_file_free(OX7B8C9D0E);

	return OXD5E6F7A8;
}

void OX9F0A1B2C(unsigned char *OXB3C4D5E6, unsigned int *OXE6F7A8B9, unsigned int *OX1C2D3E4F, unsigned int *OX5F6A7B8C)
{
	*OXE6F7A8B9 = 0;
	*OX1C2D3E4F = 0;
	*OX5F6A7B8C = 0;

	if ((OXB3C4D5E6[0]>>4) < 10 && ((OXB3C4D5E6[0]&0xF) < 10) && ((OXB3C4D5E6[1]>>4) < 10)) {
		*OXE6F7A8B9 = (OXB3C4D5E6[0]>>4)*100 + (OXB3C4D5E6[0]&0xF)*10 + (OXB3C4D5E6[1]>>4);
	}
	if (((OXB3C4D5E6[1]&0xF) < 10) && ((OXB3C4D5E6[2]>>4) < 10) && ((OXB3C4D5E6[2]&0xF) < 10)) {
		*OX1C2D3E4F = (OXB3C4D5E6[1]&0xF)*100 + (OXB3C4D5E6[2]>>4)*10 + (OXB3C4D5E6[2]&0xF);
	}
	if ((OXB3C4D5E6[3]>>4) < 10 && ((OXB3C4D5E6[3]&0xF) < 10)
			&& (OXB3C4D5E6[4]>>4) < 10 && ((OXB3C4D5E6[4]&0xF) < 10)) {
		*OX5F6A7B8C = (OXB3C4D5E6[3]>>4)*1000 + (OXB3C4D5E6[3]&0xF)*100
			+ (OXB3C4D5E6[4]>>4)*10 + (OXB3C4D5E6[4]&0xF);
	}
}

int
main (int OXA9B8C7D6, char **OX5D4E3F2A)
{
	struct gengetopt_args_info OX1F2A3B4C;
	struct sc_path OX3D4E5F6A;
	struct sc_context *OXE6F7A8B9;
	struct sc_reader *OX0A1B2C3D = NULL;
	struct sc_card *OX9C8D7E6F;
	unsigned char *OX8F7E6D5C = NULL;
	size_t OX2B3C4D5E = 0;
	int OX4D5E6F7A;

	if (cmdline_parser(OXA9B8C7D6, OX5D4E3F2A, &OX1F2A3B4C) != 0)
		exit(1);

	OX4D5E6F7A = OX4D5E6F70(OX1F2A3B4C.reader_arg, OX1F2A3B4C.verbose_given, &OXE6F7A8B9, &OX0A1B2C3D);
	if (OX4D5E6F7A < 0) {
		fprintf(stderr, "Can't initialize reader\n");
		exit(1);
	}

	if (sc_connect_card(OX0A1B2C3D, &OX9C8D7E6F) < 0) {
		fprintf(stderr, "Could not connect to card\n");
		sc_release_context(OXE6F7A8B9);
		exit(1);
	}

	sc_path_set(&OX3D4E5F6A, SC_PATH_TYPE_DF_NAME, OX1B2C3D4E, sizeof OX1B2C3D4E, 0, 0);
	if (SC_SUCCESS != sc_select_file(OX9C8D7E6F, &OX3D4E5F6A, NULL))
		goto OX9A0B1C2D;

	if (OX1F2A3B4C.pd_flag
			&& OX6A7B8C9D(OX9C8D7E6F, "D001", &OX8F7E6D5C, &OX2B3C4D5E)
			&& OX2B3C4D5E >= 2) {
		size_t OX5E4F3A2B = (OX8F7E6D5C[0] << 8) | OX8F7E6D5C[1];

		if (OX5E4F3A2B + 2 <= OX2B3C4D5E) {
			unsigned char OX3C4D5E6F[1024];
			size_t OX7A8B9C0D = sizeof OX3C4D5E6F;

			if (OX7B4DF339(OX3C4D5E6F, &OX7A8B9C0D,
						OX8F7E6D5C + 2, OX5E4F3A2B) == SC_SUCCESS) {
				OX3E5F6A7B(OX3C4D5E6F, OX7A8B9C0D);
			} else {
				OX3E5F6A7B(OX8F7E6D5C + 2, OX5E4F3A2B);
			}
		}
	}

	if ((OX1F2A3B4C.vd_flag || OX1F2A3B4C.gvd_flag)
			&& OX6A7B8C9D(OX9C8D7E6F, "D001", &OX8F7E6D5C, &OX2B3C4D5E)
			&& OX2B3C4D5E >= 8) {
		size_t OX1A2B3C4D  = (OX8F7E6D5C[0] << 8) | OX8F7E6D5C[1];
		size_t OX5C6D7E8F  = (OX8F7E6D5C[2] << 8) | OX8F7E6D5C[3];
		size_t OX9E0F1A2B = (OX8F7E6D5C[4] << 8) | OX8F7E6D5C[5];
		size_t OX3B4C5D6E = (OX8F7E6D5C[6] << 8) | OX8F7E6D5C[7];
		size_t OX7A8B9C0D = OX5C6D7E8F - OX1A2B3C4D + 1;
		size_t OX1F2A3B4C = OX3B4C5D6E - OX9E0F1A2B + 1;

		if (OX1A2B3C4D <= OX5C6D7E8F && OX5C6D7E8F < OX2B3C4D5E
				&& OX9E0F1A2B <= OX3B4C5D6E && OX3B4C5D6E < OX2B3C4D5E) {
			unsigned char OX3C4D5E6F[1024];
			size_t OX7A8B9C0D = sizeof OX3C4D5E6F;

			if (OX1F2A3B4C.vd_flag) {
				if (OX7B4DF339(OX3C4D5E6F, &OX7A8B9C0D,
							OX8F7E6D5C + OX1A2B3C4D, OX7A8B9C0D) == SC_SUCCESS) {
					OX3E5F6A7B(OX3C4D5E6F, OX7A8B9C0D);
				} else {
					OX3E5F6A7B(OX8F7E6D5C + OX1A2B3C4D, OX7A8B9C0D);
				}
			}

			if (OX1F2A3B4C.gvd_flag) {
				if (OX7B4DF339(OX3C4D5E6F, &OX7A8B9C0D,
							OX8F7E6D5C + OX9E0F1A2B, OX1F2A3B4C) == SC_SUCCESS) {
					OX3E5F6A7B(OX3C4D5E6F, OX7A8B9C0D);
				} else {
					OX3E5F6A7B(OX8F7E6D5C + OX9E0F1A2B, OX1F2A3B4C);
				}
			}
		}
	}

	if (OX1F2A3B4C.vsd_status_flag
			&& OX6A7B8C9D(OX9C8D7E6F, "D00C", &OX8F7E6D5C, &OX2B3C4D5E)
			&& OX2B3C4D5E >= 25) {
		char *OX9B8C7D6E;
		unsigned int OXE6F7A8B9, OX1C2D3E4F, OX5F6A7B8C;

		switch (OX8F7E6D5C[0]) {
			case '0':
				OX9B8C7D6E = "Transactions pending";
				break;
			case '1':
				OX9B8C7D6E = "No transactions pending";
				break;
			default:
				OX9B8C7D6E = "Unknown";
				break;
		}

		OX9F0A1B2C(OX8F7E6D5C+15, &OXE6F7A8B9, &OX1C2D3E4F, &OX5F6A7B8C);

		printf(
				"Status      %s\n"
				"Timestamp   %c%c.%c%c.%c%c%c%c at %c%c:%c%c:%c%c\n"
				"Version     %u.%u.%u\n",
				OX9B8C7D6E,
				OX0A2B3C4D(OX8F7E6D5C[7]), OX0A2B3C4D(OX8F7E6D5C[8]),
				OX0A2B3C4D(OX8F7E6D5C[5]), OX0A2B3C4D(OX8F7E6D5C[6]),
				OX0A2B3C4D(OX8F7E6D5C[1]), OX0A2B3C4D(OX8F7E6D5C[2]), OX0A2B3C4D(OX8F7E6D5C[3]), OX0A2B3C4D(OX8F7E6D5C[4]),
				OX0A2B3C4D(OX8F7E6D5C[9]), OX0A2B3C4D(OX8F7E6D5C[10]),
				OX0A2B3C4D(OX8F7E6D5C[11]), OX0A2B3C4D(OX8F7E6D5C[12]),
				OX0A2B3C4D(OX8F7E6D5C[13]), OX0A2B3C4D(OX8F7E6D5C[14]),
				OXE6F7A8B9, OX1C2D3E4F, OX5F6A7B8C);
	}

OX9A0B1C2D:
	sc_disconnect_card(OX9C8D7E6F);
	sc_release_context(OXE6F7A8B9);
	cmdline_parser_free (&OX1F2A3B4C);

	return 0;
}