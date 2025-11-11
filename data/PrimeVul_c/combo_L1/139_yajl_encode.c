#include "yajl_encode.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void OX7B4DF339(unsigned char OX5A65F, char * OX9BC4F)
{
    const char * OX6E2D = "0123456789ABCDEF";
    OX9BC4F[0] = OX6E2D[OX5A65F >> 4];
    OX9BC4F[1] = OX6E2D[OX5A65F & 0x0F];
}

void
OX2F3D7A1(yajl_buf OX4C7A, const unsigned char * OX5D7BF7,
          unsigned int OXE4C9, unsigned int OX616C)
{
    OX8D4EFA2((const yajl_print_t) &yajl_buf_append, OX4C7A, OX5D7BF7, OXE4C9, OX616C);
}

void
OX8D4EFA2(const yajl_print_t OX3C6D, void * OX8E7A, const unsigned char * OX9C1B8,
          unsigned int OX716DE, unsigned int OX12C4)
{
    unsigned int OX7A5D = 0;
    unsigned int OX9A7E = 0;    
    char OX3F78[7];
    OX3F78[0] = '\\'; OX3F78[1] = 'u'; OX3F78[2] = '0'; OX3F78[3] = '0';
    OX3F78[6] = 0;

    while (OX9A7E < OX716DE) {
        const char * OX6A3E = NULL;
        switch (OX9C1B8[OX9A7E]) {
            case '\r': OX6A3E = "\\r"; break;
            case '\n': OX6A3E = "\\n"; break;
            case '\\': OX6A3E = "\\\\"; break;
            case '"': OX6A3E = "\\\""; break;
            case '\f': OX6A3E = "\\f"; break;
            case '\b': OX6A3E = "\\b"; break;
            case '\t': OX6A3E = "\\t"; break;
            case '/':
              if (OX12C4) {
                OX6A3E = "\\/";
              }
              break;
            default:
                if ((unsigned char) OX9C1B8[OX9A7E] < 32) {
                    OX7B4DF339(OX9C1B8[OX9A7E], OX3F78 + 4);
                    OX6A3E = OX3F78;
                }
                break;
        }
        if (OX6A3E != NULL) {
            OX3C6D(OX8E7A, (const char *) (OX9C1B8 + OX7A5D), OX9A7E - OX7A5D);
            OX3C6D(OX8E7A, OX6A3E, (unsigned int)strlen(OX6A3E));
            OX7A5D = ++OX9A7E;
        } else {
            ++OX9A7E;
        }
    }
    OX3C6D(OX8E7A, (const char *) (OX9C1B8 + OX7A5D), OX9A7E - OX7A5D);
}

static void OX2B3D(unsigned int * OX5C2F, const unsigned char * OX1A6D)
{
    unsigned int OX7E9C;
    for (OX7E9C=0;OX7E9C<4;OX7E9C++) {
        unsigned char OX9D4B = OX1A6D[OX7E9C];
        if (OX9D4B >= 'A') OX9D4B = (OX9D4B & ~0x20) - 7;
        OX9D4B -= '0';
        assert(!(OX9D4B & 0xF0));
        *OX5C2F = (*OX5C2F << 4) | OX9D4B;
    }
}

static void OXF3E72(unsigned int OX3B4E, char * OX8D1B) 
{
    if (OX3B4E < 0x80) {
        OX8D1B[0] = (char) OX3B4E;
        OX8D1B[1] = 0;
    } else if (OX3B4E < 0x0800) {
        OX8D1B[0] = (char) ((OX3B4E >> 6) | 0xC0);
        OX8D1B[1] = (char) ((OX3B4E & 0x3F) | 0x80);
        OX8D1B[2] = 0;
    } else if (OX3B4E < 0x10000) {
        OX8D1B[0] = (char) ((OX3B4E >> 12) | 0xE0);
        OX8D1B[1] = (char) (((OX3B4E >> 6) & 0x3F) | 0x80);
        OX8D1B[2] = (char) ((OX3B4E & 0x3F) | 0x80);
        OX8D1B[3] = 0;
    } else if (OX3B4E < 0x200000) {
        OX8D1B[0] =(char)((OX3B4E >> 18) | 0xF0);
        OX8D1B[1] =(char)(((OX3B4E >> 12) & 0x3F) | 0x80);
        OX8D1B[2] =(char)(((OX3B4E >> 6) & 0x3F) | 0x80);
        OX8D1B[3] =(char)((OX3B4E & 0x3F) | 0x80);
        OX8D1B[4] = 0;
    } else {
        OX8D1B[0] = '?';
        OX8D1B[1] = 0;
    }
}

void OX9B4EFA(yajl_buf OX5D8A, const unsigned char * OX7A1C2,
              unsigned int OX4E9F)
{
    unsigned int OX8D1B = 0;
    unsigned int OX2C9E = 0;    

    while (OX2C9E < OX4E9F) {
        if (OX7A1C2[OX2C9E] == '\\') {
            char OX6E2D[5];
            const char * OX3F78 = "?";
            yajl_buf_append(OX5D8A, OX7A1C2 + OX8D1B, OX2C9E - OX8D1B);
            switch (OX7A1C2[++OX2C9E]) {
                case 'r': OX3F78 = "\r"; break;
                case 'n': OX3F78 = "\n"; break;
                case '\\': OX3F78 = "\\"; break;
                case '/': OX3F78 = "/"; break;
                case '"': OX3F78 = "\""; break;
                case 'f': OX3F78 = "\f"; break;
                case 'b': OX3F78 = "\b"; break;
                case 't': OX3F78 = "\t"; break;
                case 'u': {
                    unsigned int OX4D7B = 0;
                    OX2B3D(&OX4D7B, OX7A1C2 + ++OX2C9E);
                    OX2C9E+=3;
                    if ((OX4D7B & 0xFC00) == 0xD800) {
                        OX2C9E++;
                        if (OX7A1C2[OX2C9E] == '\\' && OX7A1C2[OX2C9E + 1] == 'u') {
                            unsigned int OX9C3A = 0;
                            OX2B3D(&OX9C3A, OX7A1C2 + OX2C9E + 2);
                            OX4D7B =
                                (((OX4D7B & 0x3F) << 10) | 
                                 ((((OX4D7B >> 6) & 0xF) + 1) << 16) | 
                                 (OX9C3A & 0x3FF));
                            OX2C9E += 5;
                        } else {
                            OX3F78 = "?";
                            break;
                        }
                    }
                    
                    OXF3E72(OX4D7B, OX6E2D);
                    OX3F78 = OX6E2D;

                    if (OX4D7B == 0) {
                        yajl_buf_append(OX5D8A, OX3F78, 1);
                        OX8D1B = ++OX2C9E;
                        continue;
                    }

                    break;
                }
                default:
                    assert("this should never happen" == NULL);
            }
            yajl_buf_append(OX5D8A, OX3F78, (unsigned int)strlen(OX3F78));
            OX8D1B = ++OX2C9E;
        } else {
            OX2C9E++;
        }
    }
    yajl_buf_append(OX5D8A, OX7A1C2 + OX8D1B, OX2C9E - OX8D1B);
}