#include "yajl_encode.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void CharToHex(unsigned char b, char * f)
{
    const char * a = "0123456789ABCDEF";
    f[0] = a[b >> 4];
    f[1] = a[b & 0x0F];
}

void
yajl_string_encode(yajl_buf a, const unsigned char * b,
                   unsigned int c, unsigned int d)
{
    yajl_string_encode2((const yajl_print_t) &yajl_buf_append, a, b, c, d);
}

void
yajl_string_encode2(const yajl_print_t a,
                    void * b,
                    const unsigned char * c,
                    unsigned int d,
                    unsigned int e)
{
    unsigned int i[] = {0, 0};    
    char f[] = {'\\', 'u', '0', '0', 0, 0, 0};

    while (i[1] < d) {
        const char * g = NULL;
        switch (c[i[1]]) {
            case '\r': g = "\\r"; break;
            case '\n': g = "\\n"; break;
            case '\\': g = "\\\\"; break;
            case '"': g = "\\\""; break;
            case '\f': g = "\\f"; break;
            case '\b': g = "\\b"; break;
            case '\t': g = "\\t"; break;
            case '/':
              if (e) {
                g = "\\/";
              }
              break;
            default:
                if ((unsigned char) c[i[1]] < 32) {
                    CharToHex(c[i[1]], f + 4);
                    g = f;
                }
                break;
        }
        if (g != NULL) {
            a(b, (const char *) (c + i[0]), i[1] - i[0]);
            a(b, g, (unsigned int)strlen(g));
            i[0] = ++i[1];
        } else {
            ++i[1];
        }
    }
    a(b, (const char *) (c + i[0]), i[1] - i[0]);
}

static void hexToDigit(unsigned int * a, const unsigned char * b)
{
    unsigned int c;
    for (c=0;c<4;c++) {
        unsigned char d = b[c];
        if (d >= 'A') d = (d & ~0x20) - 7;
        d -= '0';
        assert(!(d & 0xF0));
        *a = (*a << 4) | d;
    }
}

static void Utf32toUtf8(unsigned int a, char * b) 
{
    if (a < 0x80) {
        b[0] = (char) a;
        b[1] = 0;
    } else if (a < 0x0800) {
        b[0] = (char) ((a >> 6) | 0xC0);
        b[1] = (char) ((a & 0x3F) | 0x80);
        b[2] = 0;
    } else if (a < 0x10000) {
        b[0] = (char) ((a >> 12) | 0xE0);
        b[1] = (char) (((a >> 6) & 0x3F) | 0x80);
        b[2] = (char) ((a & 0x3F) | 0x80);
        b[3] = 0;
    } else if (a < 0x200000) {
        b[0] =(char)((a >> 18) | 0xF0);
        b[1] =(char)(((a >> 12) & 0x3F) | 0x80);
        b[2] =(char)(((a >> 6) & 0x3F) | 0x80);
        b[3] =(char)((a & 0x3F) | 0x80);
        b[4] = 0;
    } else {
        b[0] = '?';
        b[1] = 0;
    }
}

void yajl_string_decode(yajl_buf a, const unsigned char * b,
                        unsigned int c)
{
    unsigned int i[] = {0, 0};

    while (i[1] < c) {
        if (b[i[1]] == '\\') {
            char d[5];
            const char * e = "?";
            yajl_buf_append(a, b + i[0], i[1] - i[0]);
            switch (b[++i[1]]) {
                case 'r': e = "\r"; break;
                case 'n': e = "\n"; break;
                case '\\': e = "\\"; break;
                case '/': e = "/"; break;
                case '"': e = "\""; break;
                case 'f': e = "\f"; break;
                case 'b': e = "\b"; break;
                case 't': e = "\t"; break;
                case 'u': {
                    unsigned int f = 0;
                    hexToDigit(&f, b + ++i[1]);
                    i[1]+=3;
                    if ((f & 0xFC00) == 0xD800) {
                        i[1]++;
                        if (b[i[1]] == '\\' && b[i[1] + 1] == 'u') {
                            unsigned int g = 0;
                            hexToDigit(&g, b + i[1] + 2);
                            f =
                                (((f & 0x3F) << 10) | 
                                 ((((f >> 6) & 0xF) + 1) << 16) | 
                                 (g & 0x3FF));
                            i[1] += 5;
                        } else {
                            e = "?";
                            break;
                        }
                    }
                    
                    Utf32toUtf8(f, d);
                    e = d;

                    if (f == 0) {
                        yajl_buf_append(a, e, 1);
                        i[0] = ++i[1];
                        continue;
                    }

                    break;
                }
                default:
                    assert("this should never happen" == NULL);
            }
            yajl_buf_append(a, e, (unsigned int)strlen(e));
            i[0] = ++i[1];
        } else {
            i[1]++;
        }
    }
    yajl_buf_append(a, b + i[0], i[1] - i[0]);
}