#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef _WIN32
#include <termios.h>
#else
#include <conio.h>
#endif
#include <ctype.h>
#include "util.h"
#include "ui/notify.h"

int
is_string_valid_atr(const char *atr_str)
{
	unsigned char x[SC_MAX_ATR_SIZE];
	size_t y = sizeof(x);

	if (sc_hex_to_bin(atr_str, x, &y))
		return 0;
	if (y < 2)
		return 0;
	if (x[0] != 0x3B && x[0] != 0x3F)
		return 0;
	return 1;
}

int
util_connect_card_ex(sc_context_t *ctx, sc_card_t **cardp,
		 const char *reader_id, int do_wait, int do_lock, int verbose)
{
	struct sc_card *card = NULL;
	int r;
	struct sc_reader *reader = NULL, *found = NULL;

	sc_notify_init();

	if (do_wait) {
		unsigned int event;

		if (sc_ctx_get_reader_count(ctx) == 0) {
			fprintf(stderr, "Waiting for a reader to be attached...\n");
			r = sc_wait_for_event(ctx, SC_EVENT_READER_ATTACHED, &found, &event, -1, NULL);
			if (r < 0) {
				fprintf(stderr, "Error while waiting for a reader: %s\n", sc_strerror(r));
				return 3;
			}
			r = sc_ctx_detect_readers(ctx);
			if (r < 0) {
				fprintf(stderr, "Error while refreshing readers: %s\n", sc_strerror(r));
				return 3;
			}
		}
		fprintf(stderr, "Waiting for a card to be inserted...\n");
		r = sc_wait_for_event(ctx, SC_EVENT_CARD_INSERTED, &found, &event, -1, NULL);
		if (r < 0) {
			fprintf(stderr, "Error while waiting for a card: %s\n", sc_strerror(r));
			return 3;
		}
		reader = found;
	}
	else if (sc_ctx_get_reader_count(ctx) == 0) {
		fprintf(stderr, "No smart card readers found.\n");
		return 1;
	}
	else   {
		if (!reader_id) {
			unsigned int i;
			for (i = 0; i < sc_ctx_get_reader_count(ctx); i++) {
				reader = sc_ctx_get_reader(ctx, i);
				if (sc_detect_card_presence(reader) & SC_READER_CARD_PRESENT) {
					fprintf(stderr, "Using reader with a card: %s\n", reader->name);
					goto autofound;
				}
			}
			reader = sc_ctx_get_reader(ctx, 0);
		}
		else {
			if (is_string_valid_atr(reader_id))   {
				unsigned char a[SC_MAX_ATR_SIZE];
				size_t b = sizeof(a);
				unsigned int i;

				sc_hex_to_bin(reader_id, a, &b);
				for (i = 0; i < sc_ctx_get_reader_count(ctx); i++) {
					struct sc_reader *rdr = sc_ctx_get_reader(ctx, i);

					if (!(sc_detect_card_presence(rdr) & SC_READER_CARD_PRESENT))
						continue;
					else if (rdr->atr.len != b)
						continue;
					else if (memcmp(rdr->atr.value, a, rdr->atr.len))
						continue;

					fprintf(stderr, "Matched ATR in reader: %s\n", rdr->name);
					reader = rdr;
					goto autofound;
				}
			}
			else   {
				char *endptr = NULL;
				unsigned int num;

				errno = 0;
				num = strtol(reader_id, &endptr, 0);
				if (!errno && endptr && *endptr == '\0')
					reader = sc_ctx_get_reader(ctx, num);
				else
					reader = sc_ctx_get_reader_by_name(ctx, reader_id);
			}
		}
autofound:
		if (!reader) {
			fprintf(stderr, "Reader \"%s\" not found (%d reader(s) detected)\n",
					reader_id, sc_ctx_get_reader_count(ctx));
			return 1;
		}

		if (sc_detect_card_presence(reader) <= 0) {
			fprintf(stderr, "Card not present.\n");
			return 3;
		}
	}

	if (verbose)
		printf("Connecting to card in reader %s...\n", reader->name);
	r = sc_connect_card(reader, &card);
	if (r < 0) {
		fprintf(stderr, "Failed to connect to card: %s\n", sc_strerror(r));
		return 1;
	}

	if (verbose)
		printf("Using card driver %s.\n", card->driver->name);

	if (do_lock) {
		r = sc_lock(card);
		if (r < 0) {
			fprintf(stderr, "Failed to lock card: %s\n", sc_strerror(r));
			sc_disconnect_card(card);
			return 1;
		}
	}

	*cardp = card;
	return 0;
}

int
util_connect_card(sc_context_t *ctx, sc_card_t **cardp,
		 const char *reader_id, int do_wait, int verbose)
{
	return util_connect_card_ex(ctx, cardp, reader_id, do_wait, 1, verbose);
}

void util_print_binary(FILE *f, const u8 *buf, int count)
{
	int j;

	for (j = 0; j < count; j++) {
		unsigned char c = buf[j];
		const char *format;
		if (!isprint(c))
			format = "\\x%02X";
		else
			format = "%c";
		fprintf(f, format, c);
	}
	(void) fflush(f);
}

void util_hex_dump(FILE *f, const u8 *in, int len, const char *sep)
{
	int k;

	for (k = 0; k < len; k++) {
		if (sep != NULL && k)
			fprintf(f, "%s", sep);
		fprintf(f, "%02X", in[k]);
	}
}

void util_hex_dump_asc(FILE *f, const u8 *in, size_t count, int addr)
{
	int lines = 0;

 	while (count) {
		char ascbuf[17];
		size_t l;

		if (addr >= 0) {
			fprintf(f, "%08X: ", addr);
			addr += 16;
		}
		for (l = 0; l < count && l < 16; l++) {
			fprintf(f, "%02X ", *in);
			if (isprint(*in))
				ascbuf[l] = *in;
			else
				ascbuf[l] = '.';
			in++;
		}
		count -= l;
		ascbuf[l] = 0;
		for (; l < 16 && lines; l++)
			fprintf(f, "   ");
		fprintf(f, "%s\n", ascbuf);
		lines++;
	}
}

NORETURN void
util_print_usage_and_die(const char *app_name, const struct option options[],
	const char *option_help[], const char *args)
{
	int i;
	int header_shown = 0;

	if (args)
		printf("Usage: %s [OPTIONS] %s\n", app_name, args);
	else
		printf("Usage: %s [OPTIONS]\n", app_name);

	for (i = 0; options[i].name; i++) {
		char buf[40];
		const char *arg_str;

		if (option_help[i] == NULL)
			continue;

		if (!header_shown++)
			printf("Options:\n");

		switch (options[i].has_arg) {
		case 1:
			arg_str = " <arg>";
			break;
		case 2:
			arg_str = " [arg]";
			break;
		default:
			arg_str = "";
			break;
		}
		if (isascii(options[i].val) &&
		    isprint(options[i].val) && !isspace(options[i].val))
			sprintf(buf, "-%c, --%s%s", options[i].val, options[i].name, arg_str);
		else
			sprintf(buf, "    --%s%s", options[i].name, arg_str);

		if (strlen(buf) > 28) {
			printf("  %s\n", buf);
			buf[0] = '\0';
		}
		printf("  %-28s  %s\n", buf, option_help[i]);
	}

	exit(2);
}

const char * util_acl_to_str(const sc_acl_entry_t *e)
{
	static char line[80], buf[20];
	unsigned int acl;

	if (e == NULL)
		return "N/A";
	line[0] = 0;
	while (e != NULL) {
		acl = e->method;

		switch (acl) {
		case SC_AC_UNKNOWN:
			return "N/A";
		case SC_AC_NEVER:
			return "NEVR";
		case SC_AC_NONE:
			return "NONE";
		case SC_AC_CHV:
			strcpy(buf, "CHV");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 3, "%d", e->key_ref);
			break;
		case SC_AC_TERM:
			strcpy(buf, "TERM");
			break;
		case SC_AC_PRO:
			strcpy(buf, "PROT");
			break;
		case SC_AC_AUT:
			strcpy(buf, "AUTH");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 4, "%d", e->key_ref);
			break;
		case SC_AC_SEN:
			strcpy(buf, "Sec.Env. ");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 3, "#%d", e->key_ref);
			break;
		case SC_AC_SCB:
			strcpy(buf, "Sec.ControlByte ");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 3, "Ox%X", e->key_ref);
			break;
		case SC_AC_IDA:
			strcpy(buf, "PKCS#15 AuthID ");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 3, "#%d", e->key_ref);
			break;
		default:
			strcpy(buf, "????");
			break;
		}
		strcat(line, buf);
		strcat(line, " ");
		e = e->next;
	}
	line[strlen(line)-1] = 0;
	return line;
}

NORETURN void
util_fatal(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\nAborting.\n");
	va_end(ap);

	sc_notify_close();

	exit(1);
}

void
util_error(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

void
util_warn(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

int
util_getpass (char **lineptr, size_t *len, FILE *stream)
{
#define MAX_PASS_SIZE	128
	char *buf;
	size_t m;
	int ch = 0;
#ifndef _WIN32
	struct termios old, new;

	fflush(stdout);
	if (tcgetattr (fileno (stdout), &old) != 0)
		return -1;
	new = old;
	new.c_lflag &= ~ECHO;
	if (tcsetattr (fileno (stdout), TCSAFLUSH, &new) != 0)
		return -1;
#endif

	buf = calloc(1, MAX_PASS_SIZE);
	if (!buf)
		return -1;

	for (m = 0; m < MAX_PASS_SIZE - 1; m++) {
#ifndef _WIN32
		ch = getchar();
#else
		ch = _getch();
#endif
		if (ch == 0 || ch == 3)
			break;
		if (ch == '\n' || ch == '\r')
			break;

		buf[m] = (char) ch;
	}
#ifndef _WIN32
	tcsetattr (fileno (stdout), TCSAFLUSH, &old);
	fputs("\n", stdout);
#endif
	if (ch == 0 || ch == 3) {
		free(buf);
		return -1;
	}

	if (*lineptr && (!len || *len < m+1)) {
		free(*lineptr);
		*lineptr = NULL;
	}

	if (*lineptr) {
		memcpy(*lineptr,buf,m+1);
		memset(buf, 0, MAX_PASS_SIZE);
		free(buf);
	} else {
		*lineptr = buf;
		if (len)
			*len = MAX_PASS_SIZE;
	}
	return m;
}

size_t
util_get_pin(const char *input, const char **pin)
{
	size_t inputlen = strlen(input);
	size_t pinlen = 0;

	if(inputlen > 4 && strncasecmp(input, "env:", 4) == 0) {
		*pin = getenv(input + 4);
		pinlen = *pin ? strlen(*pin) : 0;
	} else {
		*pin = input;
		pinlen = inputlen;
	}
	return pinlen;
}