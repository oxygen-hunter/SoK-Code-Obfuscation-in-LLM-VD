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
	unsigned char atr[SC_MAX_ATR_SIZE];
	size_t atr_len = sizeof(atr);

	if (sc_hex_to_bin(atr_str, atr, &atr_len))
		return 0;
	if (atr_len < 2)
		return 0;
	if (atr[0] != 0x3B && atr[0] != 0x3F)
		return 0;
	return 1;
}

int
util_connect_card_ex(sc_context_t *ctx, sc_card_t **cardp,
		 const char *reader_id, int do_wait, int do_lock, int verbose)
{
	struct sc_reader *reader = NULL, *found = NULL;
	struct sc_card *card = NULL;
	int r;

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
			unsigned int i = 0;
			reader = NULL;
			void try_next_reader() {
				if (i < sc_ctx_get_reader_count(ctx)) {
					reader = sc_ctx_get_reader(ctx, i);
					i++;
					if (sc_detect_card_presence(reader) & SC_READER_CARD_PRESENT) {
						fprintf(stderr, "Using reader with a card: %s\n", reader->name);
					} else {
						try_next_reader();
					}
				} else {
					reader = sc_ctx_get_reader(ctx, 0);
				}
			}
			try_next_reader();
		}
		else {
			if (is_string_valid_atr(reader_id))   {
				unsigned char atr_buf[SC_MAX_ATR_SIZE];
				size_t atr_buf_len = sizeof(atr_buf);
				unsigned int i = 0;
				
				sc_hex_to_bin(reader_id, atr_buf, &atr_buf_len);
				void find_reader_with_atr() {
					if (i < sc_ctx_get_reader_count(ctx)) {
						struct sc_reader *rdr = sc_ctx_get_reader(ctx, i);
						i++;
						if (!(sc_detect_card_presence(rdr) & SC_READER_CARD_PRESENT) ||
						    rdr->atr.len != atr_buf_len ||
						    memcmp(rdr->atr.value, atr_buf, rdr->atr.len)) {
							find_reader_with_atr();
						} else {
							fprintf(stderr, "Matched ATR in reader: %s\n", rdr->name);
							reader = rdr;
						}
					}
				}
				find_reader_with_atr();
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
	void print_binary_rec(int i) {
		if (i < count) {
			unsigned char c = buf[i];
			const char *format;
			if (!isprint(c))
				format = "\\x%02X";
			else
				format = "%c";
			fprintf(f, format, c);
			print_binary_rec(i + 1);
		}
	}
	print_binary_rec(0);
	(void) fflush(f);
}

void util_hex_dump(FILE *f, const u8 *in, int len, const char *sep)
{
	void hex_dump_rec(int i) {
		if (i < len) {
			if (sep != NULL && i)
				fprintf(f, "%s", sep);
			fprintf(f, "%02X", in[i]);
			hex_dump_rec(i + 1);
		}
	}
	hex_dump_rec(0);
}

void util_hex_dump_asc(FILE *f, const u8 *in, size_t count, int addr)
{
	void hex_dump_asc_rec(size_t count, int lines) {
		if (count) {
			char ascbuf[17];
			size_t i;

			if (addr >= 0) {
				fprintf(f, "%08X: ", addr);
				addr += 16;
			}
			for (i = 0; i < count && i < 16; i++) {
				fprintf(f, "%02X ", *in);
				if (isprint(*in))
					ascbuf[i] = *in;
				else
					ascbuf[i] = '.';
				in++;
			}
			count -= i;
			ascbuf[i] = 0;
			for (; i < 16 && lines; i++)
				fprintf(f, "   ");
			fprintf(f, "%s\n", ascbuf);
			hex_dump_asc_rec(count, lines + 1);
		}
	}
	hex_dump_asc_rec(count, 0);
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

		if (acl == SC_AC_UNKNOWN) return "N/A";
		if (acl == SC_AC_NEVER) return "NEVR";
		if (acl == SC_AC_NONE) return "NONE";
		if (acl == SC_AC_CHV) {
			strcpy(buf, "CHV");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 3, "%d", e->key_ref);
		} else if (acl == SC_AC_TERM) {
			strcpy(buf, "TERM");
		} else if (acl == SC_AC_PRO) {
			strcpy(buf, "PROT");
		} else if (acl == SC_AC_AUT) {
			strcpy(buf, "AUTH");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 4, "%d", e->key_ref);
		} else if (acl == SC_AC_SEN) {
			strcpy(buf, "Sec.Env. ");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 3, "#%d", e->key_ref);
		} else if (acl == SC_AC_SCB) {
			strcpy(buf, "Sec.ControlByte ");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 3, "Ox%X", e->key_ref);
		} else if (acl == SC_AC_IDA) {
			strcpy(buf, "PKCS#15 AuthID ");
			if (e->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(buf + 3, "#%d", e->key_ref);
		} else {
			strcpy(buf, "????");
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
	size_t i;
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

	void getpass_rec(size_t i) {
		if (i < MAX_PASS_SIZE - 1) {
#ifndef _WIN32
			ch = getchar();
#else
			ch = _getch();
#endif
			if (ch == 0 || ch == 3 || ch == '\n' || ch == '\r') {
				buf[i] = 0;
				return;
			}
			buf[i] = (char) ch;
			getpass_rec(i + 1);
		}
	}
	getpass_rec(0);

#ifndef _WIN32
	tcsetattr (fileno (stdout), TCSAFLUSH, &old);
	fputs("\n", stdout);
#endif
	if (ch == 0 || ch == 3) {
		free(buf);
		return -1;
	}

	if (*lineptr && (!len || *len < i+1)) {
		free(*lineptr);
		*lineptr = NULL;
	}

	if (*lineptr) {
		memcpy(*lineptr,buf,i+1);
		memset(buf, 0, MAX_PASS_SIZE);
		free(buf);
	} else {
		*lineptr = buf;
		if (len)
			*len = MAX_PASS_SIZE;
	}
	return i;
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