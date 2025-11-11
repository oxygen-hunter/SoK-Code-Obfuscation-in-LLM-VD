#include <Python.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

extern "C" {
#include "egk-tool-cmdline.h"
#include "libopensc/log.h"
#include "libopensc/opensc.h"
#include <ctype.h>
}

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef ENABLE_ZLIB
#include <zlib.h>

int uncompress_gzip(void* uncompressed, size_t *uncompressed_len,
		const void* compressed, size_t compressed_len)
{
	z_stream stream;
	memset(&stream, 0, sizeof stream);
	stream.total_in = compressed_len;
	stream.avail_in = compressed_len;
	stream.total_out = *uncompressed_len;
	stream.avail_out = *uncompressed_len;
	stream.next_in = (Bytef *) compressed;
	stream.next_out = (Bytef *) uncompressed;

	if (Z_OK == inflateInit2(&stream, (15 + 32))
			&& Z_STREAM_END == inflate(&stream, Z_FINISH)) {
		*uncompressed_len = stream.total_out;
	} else {
		return SC_ERROR_INVALID_DATA;
	}
	inflateEnd(&stream);

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

using namespace std;

const unsigned char aid_hca[] = {0xD2, 0x76, 0x00, 0x00, 0x01, 0x02};

static int initialize(int reader_id, int verbose,
		sc_context_t **ctx, sc_reader_t **reader)
{
	unsigned int i, reader_count;
	int r;

	if (!ctx || !reader)
		return SC_ERROR_INVALID_ARGUMENTS;

	r = sc_establish_context(ctx, "");
	if (r < 0 || !*ctx) {
		fprintf(stderr, "Failed to create initial context: %s", sc_strerror(r));
		return r;
	}

	(*ctx)->debug = verbose;
	(*ctx)->flags |= SC_CTX_FLAG_ENABLE_DEFAULT_DRIVER;

	reader_count = sc_ctx_get_reader_count(*ctx);

	if (reader_count == 0) {
		sc_debug(*ctx, SC_LOG_DEBUG_NORMAL, "No reader not found.\n");
		return SC_ERROR_NO_READERS_FOUND;
	}

	if (reader_id < 0) {
		for (i = 0; i < reader_count; i++) {
			*reader = sc_ctx_get_reader(*ctx, i);
			if (sc_detect_card_presence(*reader) & SC_READER_CARD_PRESENT) {
				reader_id = i;
				sc_debug(*ctx, SC_LOG_DEBUG_NORMAL, "Using the first reader"
						" with a card: %s", (*reader)->name);
				break;
			}
		}
		if ((unsigned int) reader_id >= reader_count) {
			sc_debug(*ctx, SC_LOG_DEBUG_NORMAL, "No card found, using the first reader.");
			reader_id = 0;
		}
	}

	if ((unsigned int) reader_id >= reader_count) {
		sc_debug(*ctx, SC_LOG_DEBUG_NORMAL, "Invalid reader number "
				"(%d), only %d available.\n", reader_id, reader_count);
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
	int ok = 0;
	int r;
	size_t len;

	sc_format_path(str_path, &path);
	if (SC_SUCCESS != sc_select_file(card, &path, &file)) {
		goto err;
	}

	len = file ? file->size : 4096;
	p = (unsigned char *)realloc(*data, len);
	if (!p) {
		goto err;
	}
	*data = p;
	*data_len = len;

	r = sc_read_binary(card, 0, p, len, 0);
	if (r < 0)
		goto err;

	*data_len = r;
	ok = 1;

err:
	sc_file_free(file);

	return ok;
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

int main(int argc, char **argv)
{
	Py_Initialize();
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	pName = PyUnicode_DecodeFSDefault("cmdline_parser");
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		pFunc = PyObject_GetAttrString(pModule, "cmdline_parser");
		if (PyCallable_Check(pFunc)) {
			pArgs = PyTuple_Pack(2, PyLong_FromLong(argc), PyList_FromString(argv));
			pValue = PyObject_CallObject(pFunc, pArgs);
			Py_DECREF(pArgs);
			if (pValue != NULL) {
				std::cout << "Result of call: " << PyLong_AsLong(pValue) << std::endl;
				Py_DECREF(pValue);
			}
			else {
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr, "Call failed\n");
				return 1;
			}
		}
		else {
			if (PyErr_Occurred())
				PyErr_Print();
			fprintf(stderr, "Cannot find function \"cmdline_parser\"\n");
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"cmdline_parser\"\n");
		return 1;
	}
	Py_Finalize();
	return 0;
}