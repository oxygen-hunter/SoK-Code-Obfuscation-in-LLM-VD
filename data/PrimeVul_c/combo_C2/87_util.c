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

int is_string_valid_atr(const char *atr_str) {
    unsigned char atr[SC_MAX_ATR_SIZE];
    size_t atr_len = sizeof(atr);
    int _state = 0;
    while (1) {
        switch (_state) {
            case 0:
                if (sc_hex_to_bin(atr_str, atr, &atr_len)) {
                    _state = 1;
                } else {
                    _state = 2;
                }
                break;
            case 1:
                return 0;
            case 2:
                if (atr_len < 2) {
                    _state = 1;
                } else {
                    _state = 3;
                }
                break;
            case 3:
                if (atr[0] != 0x3B && atr[0] != 0x3F) {
                    _state = 1;
                } else {
                    _state = 4;
                }
                break;
            case 4:
                return 1;
        }
    }
}

int util_connect_card_ex(sc_context_t *ctx, sc_card_t **cardp, const char *reader_id, int do_wait, int do_lock, int verbose) {
    struct sc_reader *reader = NULL, *found = NULL;
    struct sc_card *card = NULL;
    int r, _state = 0;

    sc_notify_init();

    while (1) {
        switch (_state) {
            case 0:
                if (do_wait) {
                    unsigned int event;
                    if (sc_ctx_get_reader_count(ctx) == 0) {
                        fprintf(stderr, "Waiting for a reader to be attached...\n");
                        r = sc_wait_for_event(ctx, SC_EVENT_READER_ATTACHED, &found, &event, -1, NULL);
                        if (r < 0) {
                            fprintf(stderr, "Error while waiting for a reader: %s\n", sc_strerror(r));
                            _state = 12;
                            break;
                        }
                        r = sc_ctx_detect_readers(ctx);
                        if (r < 0) {
                            fprintf(stderr, "Error while refreshing readers: %s\n", sc_strerror(r));
                            _state = 12;
                            break;
                        }
                    }
                    fprintf(stderr, "Waiting for a card to be inserted...\n");
                    r = sc_wait_for_event(ctx, SC_EVENT_CARD_INSERTED, &found, &event, -1, NULL);
                    if (r < 0) {
                        fprintf(stderr, "Error while waiting for a card: %s\n", sc_strerror(r));
                        _state = 12;
                        break;
                    }
                    reader = found;
                    _state = 1;
                } else if (sc_ctx_get_reader_count(ctx) == 0) {
                    fprintf(stderr, "No smart card readers found.\n");
                    _state = 11;
                } else {
                    _state = 2;
                }
                break;
            case 1:
                _state = 9;
                break;
            case 2:
                if (!reader_id) {
                    unsigned int i;
                    for (i = 0; i < sc_ctx_get_reader_count(ctx); i++) {
                        reader = sc_ctx_get_reader(ctx, i);
                        if (sc_detect_card_presence(reader) & SC_READER_CARD_PRESENT) {
                            fprintf(stderr, "Using reader with a card: %s\n", reader->name);
                            _state = 8;
                            break;
                        }
                    }
                    if (_state != 8) {
                        reader = sc_ctx_get_reader(ctx, 0);
                        _state = 3;
                    }
                } else {
                    _state = 4;
                }
                break;
            case 3:
                _state = 6;
                break;
            case 4:
                if (is_string_valid_atr(reader_id)) {
                    unsigned char atr_buf[SC_MAX_ATR_SIZE];
                    size_t atr_buf_len = sizeof(atr_buf);
                    unsigned int i;
                    sc_hex_to_bin(reader_id, atr_buf, &atr_buf_len);
                    for (i = 0; i < sc_ctx_get_reader_count(ctx); i++) {
                        struct sc_reader *rdr = sc_ctx_get_reader(ctx, i);
                        if (!(sc_detect_card_presence(rdr) & SC_READER_CARD_PRESENT)) continue;
                        else if (rdr->atr.len != atr_buf_len) continue;
                        else if (memcmp(rdr->atr.value, atr_buf, rdr->atr.len)) continue;
                        fprintf(stderr, "Matched ATR in reader: %s\n", rdr->name);
                        reader = rdr;
                        _state = 8;
                        break;
                    }
                    if (_state != 8) {
                        _state = 5;
                    }
                } else {
                    _state = 5;
                }
                break;
            case 5: {
                char *endptr = NULL;
                unsigned int num;
                errno = 0;
                num = strtol(reader_id, &endptr, 0);
                if (!errno && endptr && *endptr == '\0') {
                    reader = sc_ctx_get_reader(ctx, num);
                } else {
                    reader = sc_ctx_get_reader_by_name(ctx, reader_id);
                }
                _state = 6;
                break;
            }
            case 6:
                if (!reader) {
                    fprintf(stderr, "Reader \"%s\" not found (%d reader(s) detected)\n", reader_id, sc_ctx_get_reader_count(ctx));
                    _state = 11;
                } else {
                    _state = 7;
                }
                break;
            case 7:
                if (sc_detect_card_presence(reader) <= 0) {
                    fprintf(stderr, "Card not present.\n");
                    _state = 12;
                } else {
                    _state = 9;
                }
                break;
            case 8:
                _state = 9;
                break;
            case 9:
                if (verbose) {
                    printf("Connecting to card in reader %s...\n", reader->name);
                }
                r = sc_connect_card(reader, &card);
                if (r < 0) {
                    fprintf(stderr, "Failed to connect to card: %s\n", sc_strerror(r));
                    _state = 11;
                } else {
                    _state = 10;
                }
                break;
            case 10:
                if (verbose) {
                    printf("Using card driver %s.\n", card->driver->name);
                }
                if (do_lock) {
                    r = sc_lock(card);
                    if (r < 0) {
                        fprintf(stderr, "Failed to lock card: %s\n", sc_strerror(r));
                        sc_disconnect_card(card);
                        _state = 11;
                        break;
                    }
                }
                *cardp = card;
                return 0;
            case 11:
                return 1;
            case 12:
                return 3;
        }
    }
}

int util_connect_card(sc_context_t *ctx, sc_card_t **cardp, const char *reader_id, int do_wait, int verbose) {
    return util_connect_card_ex(ctx, cardp, reader_id, do_wait, 1, verbose);
}

void util_print_binary(FILE *f, const u8 *buf, int count) {
    int i, _state = 0;
    while (1) {
        switch (_state) {
            case 0:
                i = 0;
                _state = 1;
                break;
            case 1:
                if (i < count) {
                    unsigned char c = buf[i];
                    const char *format;
                    if (!isprint(c))
                        format = "\\x%02X";
                    else
                        format = "%c";
                    fprintf(f, format, c);
                    i++;
                } else {
                    _state = 2;
                }
                break;
            case 2:
                fflush(f);
                return;
        }
    }
}

void util_hex_dump(FILE *f, const u8 *in, int len, const char *sep) {
    int i, _state = 0;
    while (1) {
        switch (_state) {
            case 0:
                i = 0;
                _state = 1;
                break;
            case 1:
                if (i < len) {
                    if (sep != NULL && i)
                        fprintf(f, "%s", sep);
                    fprintf(f, "%02X", in[i]);
                    i++;
                } else {
                    _state = 2;
                }
                break;
            case 2:
                return;
        }
    }
}

void util_hex_dump_asc(FILE *f, const u8 *in, size_t count, int addr) {
    int lines = 0, _state = 0;
    while (1) {
        switch (_state) {
            case 0:
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
                    lines++;
                } else {
                    _state = 1;
                }
                break;
            case 1:
                return;
        }
    }
}

NORETURN void util_print_usage_and_die(const char *app_name, const struct option options[], const char *option_help[], const char *args) {
    int i, header_shown = 0, _state = 0;
    while (1) {
        switch (_state) {
            case 0:
                if (args)
                    printf("Usage: %s [OPTIONS] %s\n", app_name, args);
                else
                    printf("Usage: %s [OPTIONS]\n", app_name);
                i = 0;
                _state = 1;
                break;
            case 1:
                if (options[i].name) {
                    char buf[40];
                    const char *arg_str;
                    if (option_help[i] == NULL) {
                        i++;
                        continue;
                    }
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
                    if (isascii(options[i].val) && isprint(options[i].val) && !isspace(options[i].val))
                        sprintf(buf, "-%c, --%s%s", options[i].val, options[i].name, arg_str);
                    else
                        sprintf(buf, "    --%s%s", options[i].name, arg_str);
                    if (strlen(buf) > 28) {
                        printf("  %s\n", buf);
                        buf[0] = '\0';
                    }
                    printf("  %-28s  %s\n", buf, option_help[i]);
                    i++;
                } else {
                    _state = 2;
                }
                break;
            case 2:
                exit(2);
        }
    }
}

const char *util_acl_to_str(const sc_acl_entry_t *e) {
    static char line[80], buf[20];
    unsigned int acl;
    int _state = 0;

    while (1) {
        switch (_state) {
            case 0:
                if (e == NULL) {
                    return "N/A";
                }
                line[0] = 0;
                _state = 1;
                break;
            case 1:
                if (e != NULL) {
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
                } else {
                    _state = 2;
                }
                break;
            case 2:
                line[strlen(line) - 1] = 0;
                return line;
        }
    }
}

NORETURN void util_fatal(const char *fmt, ...) {
    va_list ap;
    int _state = 0;

    while (1) {
        switch (_state) {
            case 0:
                va_start(ap, fmt);
                fprintf(stderr, "error: ");
                vfprintf(stderr, fmt, ap);
                fprintf(stderr, "\nAborting.\n");
                va_end(ap);
                sc_notify_close();
                exit(1);
        }
    }
}

void util_error(const char *fmt, ...) {
    va_list ap;
    int _state = 0;

    while (1) {
        switch (_state) {
            case 0:
                va_start(ap, fmt);
                fprintf(stderr, "error: ");
                vfprintf(stderr, fmt, ap);
                fprintf(stderr, "\n");
                va_end(ap);
                return;
        }
    }
}

void util_warn(const char *fmt, ...) {
    va_list ap;
    int _state = 0;

    while (1) {
        switch (_state) {
            case 0:
                va_start(ap, fmt);
                fprintf(stderr, "warning: ");
                vfprintf(stderr, fmt, ap);
                fprintf(stderr, "\n");
                va_end(ap);
                return;
        }
    }
}

int util_getpass(char **lineptr, size_t *len, FILE *stream) {
#define MAX_PASS_SIZE 128
    char *buf;
    size_t i;
    int ch = 0, _state = 0;
#ifndef _WIN32
    struct termios old, new;
#endif

    while (1) {
        switch (_state) {
            case 0:
#ifndef _WIN32
                fflush(stdout);
                if (tcgetattr(fileno(stdout), &old) != 0) {
                    return -1;
                }
                new = old;
                new.c_lflag &= ~ECHO;
                if (tcsetattr(fileno(stdout), TCSAFLUSH, &new) != 0) {
                    return -1;
                }
#endif
                buf = calloc(1, MAX_PASS_SIZE);
                if (!buf) {
                    return -1;
                }
                i = 0;
                _state = 1;
                break;
            case 1:
                if (i < MAX_PASS_SIZE - 1) {
#ifndef _WIN32
                    ch = getchar();
#else
                    ch = _getch();
#endif
                    if (ch == 0 || ch == 3) {
                        _state = 3;
                    } else if (ch == '\n' || ch == '\r') {
                        _state = 2;
                    } else {
                        buf[i] = (char)ch;
                        i++;
                    }
                } else {
                    _state = 2;
                }
                break;
            case 2:
#ifndef _WIN32
                tcsetattr(fileno(stdout), TCSAFLUSH, &old);
                fputs("\n", stdout);
#endif
                if (*lineptr && (!len || *len < i + 1)) {
                    free(*lineptr);
                    *lineptr = NULL;
                }
                if (*lineptr) {
                    memcpy(*lineptr, buf, i + 1);
                    memset(buf, 0, MAX_PASS_SIZE);
                    free(buf);
                } else {
                    *lineptr = buf;
                    if (len) {
                        *len = MAX_PASS_SIZE;
                    }
                }
                return i;
            case 3:
                free(buf);
                return -1;
        }
    }
}

size_t util_get_pin(const char *input, const char **pin) {
    size_t inputlen = strlen(input);
    size_t pinlen = 0, _state = 0;

    while (1) {
        switch (_state) {
            case 0:
                if (inputlen > 4 && strncasecmp(input, "env:", 4) == 0) {
                    *pin = getenv(input + 4);
                    pinlen = *pin ? strlen(*pin) : 0;
                } else {
                    *pin = input;
                    pinlen = inputlen;
                }
                return pinlen;
        }
    }
}