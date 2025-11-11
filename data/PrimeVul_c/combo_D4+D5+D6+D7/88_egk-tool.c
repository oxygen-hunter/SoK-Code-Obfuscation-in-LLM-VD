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

int uncompress_gzip(void* uncompressed, size_t *uncompressed_len,
		const void* compressed, size_t compressed_len)
{
	z_stream s;
	memset(&s, 0, sizeof s);
	s.total_in = compressed_len;
	s.avail_in = compressed_len;
	s.total_out = *uncompressed_len;
	s.avail_out = *uncompressed_len;
	s.next_in = (Bytef *) compressed;
	s.next_out = (Bytef *) uncompressed;

	if (Z_OK == inflateInit2(&s, (15 + 32))
			&& Z_STREAM_END == inflate(&s, Z_FINISH)) {
		*uncompressed_len = s.total_out;
	} else {
		return SC_ERROR_INVALID_DATA;
	}
	inflateEnd(&s);

	return SC_SUCCESS;
}
#else
int uncompress_gzip(void* uncompressed, size_t *uncompressed_len,
		const void* compressed, size_t compressed_len)
{
	return SC_ERROR_NOT_SUPPORTED;
}
#endif

#define PRINT(c) (isprint(c) ? c : '?')

void dump_binary(void *buf, size_t buf_len)
{
#ifdef _WIN32
	_setmode(fileno(stdout), _O_BINARY);
#endif
	fwrite(buf, 1, buf_len, stdout);
#ifdef _WIN32
	_setmode(fileno(stdout), _O_TEXT);
#endif
}

const unsigned char aid_hca[] = {0xD2, 0x76, 0x00, 0x00, 0x01, 0x02};

static int initialize(int reader_id, int verbose,
		sc_context_t **ctx, sc_reader_t **reader)
{
	struct { int x; unsigned int y; } vars = {0, 0};
	unsigned int i;
	if (!ctx || !reader)
		return SC_ERROR_INVALID_ARGUMENTS;

	vars.x = sc_establish_context(ctx, "");
	if (vars.x < 0 || !*ctx) {
		fprintf(stderr, "Failed to create initial context: %s", sc_strerror(vars.x));
		return vars.x;
	}

	(*ctx)->debug = verbose;
	(*ctx)->flags |= SC_CTX_FLAG_ENABLE_DEFAULT_DRIVER;

	vars.y = sc_ctx_get_reader_count(*ctx);

	if (vars.y == 0) {
		sc_debug(*ctx, SC_LOG_DEBUG_NORMAL, "No reader not found.\n");
		return SC_ERROR_NO_READERS_FOUND;
	}

	if (reader_id < 0) {
		for (i = 0; i < vars.y; i++) {
			*reader = sc_ctx_get_reader(*ctx, i);
			if (sc_detect_card_presence(*reader) & SC_READER_CARD_PRESENT) {
				reader_id = i;
				sc_debug(*ctx, SC_LOG_DEBUG_NORMAL, "Using the first reader"
						" with a card: %s", (*reader)->name);
				break;
			}
		}
		if ((unsigned int) reader_id >= vars.y) {
			sc_debug(*ctx, SC_LOG_DEBUG_NORMAL, "No card found, using the first reader.");
			reader_id = 0;
		}
	}

	if ((unsigned int) reader_id >= vars.y) {
		sc_debug(*ctx, SC_LOG_DEBUG_NORMAL, "Invalid reader number "
				"(%d), only %d available.\n", reader_id, vars.y);
		return SC_ERROR_NO_READERS_FOUND;
	}

	*reader = sc_ctx_get_reader(*ctx, reader_id);

	return SC_SUCCESS;
}

int read_file(struct sc_card *card, char *str_path, unsigned char **data, size_t *data_len)
{
	struct sc_path path;
	struct sc_file *file;
	unsigned char *p;
	struct { int ok; int r; size_t len; } vars = {0, 0, 0};

	sc_format_path(str_path, &path);
	if (SC_SUCCESS != sc_select_file(card, &path, &file)) {
		goto err;
	}

	vars.len = file ? file->size : 4096;
	p = realloc(*data, vars.len);
	if (!p) {
		goto err;
	}
	*data = p;
	*data_len = vars.len;

	vars.r = sc_read_binary(card, 0, p, vars.len, 0);
	if (vars.r < 0)
		goto err;

	*data_len = vars.r;
	vars.ok = 1;

err:
	sc_file_free(file);

	return vars.ok;
}

void decode_version(unsigned char *bcd, unsigned int *major, unsigned int *minor, unsigned int *fix)
{
	*major = 0;
	*minor = 0;
	*fix = 0;

	if ((bcd[0]>>4) < 10 && ((bcd[0]&0xF) < 10) && ((bcd[1]>>4) < 10)) {
		*major = (bcd[0]>>4)*100 + (bcd[0]&0xF)*10 + (bcd[1]>>4);
	}
	if (((bcd[1]&0xF) < 10) && ((bcd[2]>>4) < 10) && ((bcd[2]&0xF) < 10)) {
		*minor = (bcd[1]&0xF)*100 + (bcd[2]>>4)*10 + (bcd[2]&0xF);
	}
	if ((bcd[3]>>4) < 10 && ((bcd[3]&0xF) < 10)
			&& (bcd[4]>>4) < 10 && ((bcd[4]&0xF) < 10)) {
		*fix = (bcd[3]>>4)*1000 + (bcd[3]&0xF)*100
			+ (bcd[4]>>4)*10 + (bcd[4]&0xF);
	}
}

int
main (int argc, char **argv)
{
	struct gengetopt_args_info cmdline;
	struct sc_context *ctx;
	struct sc_reader *reader = NULL;
	struct sc_card *card;
	struct { struct sc_path path; unsigned char *data; size_t data_len; int r; } vars = {{0}, NULL, 0, 0};

	if (cmdline_parser(argc, argv, &cmdline) != 0)
		exit(1);

	vars.r = initialize(cmdline.reader_arg, cmdline.verbose_given, &ctx, &reader);
	if (vars.r < 0) {
		fprintf(stderr, "Can't initialize reader\n");
		exit(1);
	}

	if (sc_connect_card(reader, &card) < 0) {
		fprintf(stderr, "Could not connect to card\n");
		sc_release_context(ctx);
		exit(1);
	}

	sc_path_set(&vars.path, SC_PATH_TYPE_DF_NAME, aid_hca, sizeof aid_hca, 0, 0);
	if (SC_SUCCESS != sc_select_file(card, &vars.path, NULL))
		goto err;

	if (cmdline.pd_flag
			&& read_file(card, "D001", &vars.data, &vars.data_len)
			&& vars.data_len >= 2) {
		size_t len_pd = (vars.data[0] << 8) | vars.data[1];

		if (len_pd + 2 <= vars.data_len) {
			unsigned char uncompressed[1024];
			size_t uncompressed_len = sizeof uncompressed;

			if (uncompress_gzip(uncompressed, &uncompressed_len,
						vars.data + 2, len_pd) == SC_SUCCESS) {
				dump_binary(uncompressed, uncompressed_len);
			} else {
				dump_binary(vars.data + 2, len_pd);
			}
		}
	}

	if ((cmdline.vd_flag || cmdline.gvd_flag)
			&& read_file(card, "D001", &vars.data, &vars.data_len)
			&& vars.data_len >= 8) {
		struct { size_t off_vd; size_t end_vd; size_t off_gvd; size_t end_gvd; size_t len_vd; size_t len_gvd; } offsets = {
			(vars.data[0] << 8) | vars.data[1],
			(vars.data[2] << 8) | vars.data[3],
			(vars.data[4] << 8) | vars.data[5],
			(vars.data[6] << 8) | vars.data[7],
			0, 0
		};
		offsets.len_vd = offsets.end_vd - offsets.off_vd + 1;
		offsets.len_gvd = offsets.end_gvd - offsets.off_gvd + 1;

		if (offsets.off_vd <= offsets.end_vd && offsets.end_vd < vars.data_len
				&& offsets.off_gvd <= offsets.end_gvd && offsets.end_gvd < vars.data_len) {
			unsigned char uncompressed[1024];
			size_t uncompressed_len = sizeof uncompressed;

			if (cmdline.vd_flag) {
				if (uncompress_gzip(uncompressed, &uncompressed_len,
							vars.data + offsets.off_vd, offsets.len_vd) == SC_SUCCESS) {
					dump_binary(uncompressed, uncompressed_len);
				} else {
					dump_binary(vars.data + offsets.off_vd, offsets.len_vd);
				}
			}

			if (cmdline.gvd_flag) {
				if (uncompress_gzip(uncompressed, &uncompressed_len,
							vars.data + offsets.off_gvd, offsets.len_gvd) == SC_SUCCESS) {
					dump_binary(uncompressed, uncompressed_len);
				} else {
					dump_binary(vars.data + offsets.off_gvd, offsets.len_gvd);
				}
			}
		}
	}

	if (cmdline.vsd_status_flag
			&& read_file(card, "D00C", &vars.data, &vars.data_len)
			&& vars.data_len >= 25) {
		char *status;
		unsigned int major, minor, fix;

		switch (vars.data[0]) {
			case '0':
				status = "Transactions pending";
				break;
			case '1':
				status = "No transactions pending";
				break;
			default:
				status = "Unknown";
				break;
		}

		decode_version(vars.data+15, &major, &minor, &fix);

		printf(
				"Status      %s\n"
				"Timestamp   %c%c.%c%c.%c%c%c%c at %c%c:%c%c:%c%c\n"
				"Version     %u.%u.%u\n",
				status,
				PRINT(vars.data[7]), PRINT(vars.data[8]),
				PRINT(vars.data[5]), PRINT(vars.data[6]),
				PRINT(vars.data[1]), PRINT(vars.data[2]), PRINT(vars.data[3]), PRINT(vars.data[4]),
				PRINT(vars.data[9]), PRINT(vars.data[10]),
				PRINT(vars.data[11]), PRINT(vars.data[12]),
				PRINT(vars.data[13]), PRINT(vars.data[14]),
				major, minor, fix);
	}

err:
	sc_disconnect_card(card);
	sc_release_context(ctx);
	cmdline_parser_free(&cmdline);

	return 0;
}