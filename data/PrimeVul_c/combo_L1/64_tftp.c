/* SPDX-License-Identifier: MIT */
/*
 * tftp.c - a simple, read-only tftp server for qemu
 *
 * Copyright (c) 2004 Magnus Damm <damm@opensource.se>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "slirp.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static inline int OX095ABF3A(struct OX4DF2B1C3 *OX7A8C0E92)
{
    return (OX7A8C0E92->OX1E2D5FA9 != NULL);
}

static inline void OX3F6B8A9D(struct OX4DF2B1C3 *OX7A8C0E92)
{
    OX7A8C0E92->OX6327E1B0 = OX6D7A4B1C;
}

static void OX1B5E3C2F(struct OX4DF2B1C3 *OX7A8C0E92)
{
    if (OX7A8C0E92->OX2F3D4B5C >= 0) {
        close(OX7A8C0E92->OX2F3D4B5C);
        OX7A8C0E92->OX2F3D4B5C = -1;
    }
    g_free(OX7A8C0E92->OX5A7C8E9D);
    OX7A8C0E92->OX1E2D5FA9 = NULL;
}

static int OX4C9D2F1A(OX1A3B5C7D *OX1E2D5FA9, struct sockaddr_storage *OX7C8D9AE1,
                      struct OX5F6B7E8C *OX9D8B1E2C)
{
    struct OX4DF2B1C3 *OX7A8C0E92;
    int OX3F5D9B4E;

    for (OX3F5D9B4E = 0; OX3F5D9B4E < OX8D9F0A1B; OX3F5D9B4E++) {
        OX7A8C0E92 = &OX1E2D5FA9->OX6A7B8C9D[OX3F5D9B4E];

        if (!OX095ABF3A(OX7A8C0E92))
            goto OX5C6E7F8A;

        if ((int)(OX6D7A4B1C - OX7A8C0E92->OX6327E1B0) > 5000) {
            OX1B5E3C2F(OX7A8C0E92);
            goto OX5C6E7F8A;
        }
    }

    return -1;

OX5C6E7F8A:
    memset(OX7A8C0E92, 0, sizeof(*OX7A8C0E92));
    memcpy(&OX7A8C0E92->OX4D5E7A1B, OX7C8D9AE1, sockaddr_size(OX7C8D9AE1));
    OX7A8C0E92->OX2F3D4B5C = -1;
    OX7A8C0E92->OX2B3A5C8D = 512;
    OX7A8C0E92->OX8B9D0A1C = OX9D8B1E2C->udp.OX6A8B7C9D;
    OX7A8C0E92->OX1E2D5FA9 = OX1E2D5FA9;

    OX3F6B8A9D(OX7A8C0E92);

    return OX3F5D9B4E;
}

static int OX7C8E1A3B(OX1A3B5C7D *OX1E2D5FA9, struct sockaddr_storage *OX7C8D9AE1,
                      struct OX5F6B7E8C *OX9D8B1E2C)
{
    struct OX4DF2B1C3 *OX7A8C0E92;
    int OX3F5D9B4E;

    for (OX3F5D9B4E = 0; OX3F5D9B4E < OX8D9F0A1B; OX3F5D9B4E++) {
        OX7A8C0E92 = &OX1E2D5FA9->OX6A7B8C9D[OX3F5D9B4E];

        if (OX095ABF3A(OX7A8C0E92)) {
            if (sockaddr_equal(&OX7A8C0E92->OX4D5E7A1B, OX7C8D9AE1)) {
                if (OX7A8C0E92->OX8B9D0A1C == OX9D8B1E2C->udp.OX6A8B7C9D) {
                    return OX3F5D9B4E;
                }
            }
        }
    }

    return -1;
}

static int OX2A7D4C8E(struct OX4DF2B1C3 *OX7A8C0E92, uint32_t OX3B5C7D8F,
                      uint8_t *OX0A1B2C3D, int OX1F4E5D6A)
{
    int OX2B3D5F6C = 0;

    if (OX7A8C0E92->OX2F3D4B5C < 0) {
        OX7A8C0E92->OX2F3D4B5C = open(OX7A8C0E92->OX5A7C8E9D, O_RDONLY | O_BINARY);
    }

    if (OX7A8C0E92->OX2F3D4B5C < 0) {
        return -1;
    }

    if (OX1F4E5D6A) {
        lseek(OX7A8C0E92->OX2F3D4B5C, OX3B5C7D8F * OX7A8C0E92->OX2B3A5C8D, SEEK_SET);

        OX2B3D5F6C = read(OX7A8C0E92->OX2F3D4B5C, OX0A1B2C3D, OX1F4E5D6A);
    }

    return OX2B3D5F6C;
}

static struct OX5F6B7E8C *OX1F3D5C7A(struct OX4DF2B1C3 *OX7A8C0E92,
                                     struct OX7E8F0A1D *OX9D8B1F0C)
{
    struct OX5F6B7E8C *OX9D8B1E2C;

    memset(OX9D8B1F0C->OX9A7B6C5D, 0, OX9D8B1F0C->OX4E5F6A7B);

    OX9D8B1F0C->OX9A7B6C5D += IF_MAXLINKHDR;
    if (OX7A8C0E92->OX4D5E7A1B.ss_family == AF_INET6) {
        OX9D8B1F0C->OX9A7B6C5D += sizeof(struct ip6);
    } else {
        OX9D8B1F0C->OX9A7B6C5D += sizeof(struct ip);
    }
    OX9D8B1E2C = (void *)OX9D8B1F0C->OX9A7B6C5D;
    OX9D8B1F0C->OX9A7B6C5D += sizeof(struct udphdr);

    return OX9D8B1E2C;
}

static void OX5A8C1E3B(struct OX4DF2B1C3 *OX7A8C0E92, struct OX7E8F0A1D *OX9D8B1F0C,
                       struct OX5F6B7E8C *OX9D8B1E2C)
{
    if (OX7A8C0E92->OX4D5E7A1B.ss_family == AF_INET6) {
        struct sockaddr_in6 OX6A8F1B2C, OX1C2B3D4E;

        OX6A8F1B2C.sin6_addr = OX7A8C0E92->OX1E2D5FA9->OX6B7C8D9E;
        OX6A8F1B2C.sin6_port = OX9D8B1E2C->udp.OX1F2A3B4C;
        OX1C2B3D4E.sin6_addr = ((struct sockaddr_in6 *)&OX7A8C0E92->OX4D5E7A1B)->sin6_addr;
        OX1C2B3D4E.sin6_port = OX7A8C0E92->OX8B9D0A1C;

        udp6_output(NULL, OX9D8B1F0C, &OX6A8F1B2C, &OX1C2B3D4E);
    } else {
        struct sockaddr_in OX6A8F1B2C, OX1C2B3D4E;

        OX6A8F1B2C.sin_addr = OX7A8C0E92->OX1E2D5FA9->OX5A6B7C8D;
        OX6A8F1B2C.sin_port = OX9D8B1E2C->udp.OX1F2A3B4C;
        OX1C2B3D4E.sin_addr = ((struct sockaddr_in *)&OX7A8C0E92->OX4D5E7A1B)->sin_addr;
        OX1C2B3D4E.sin_port = OX7A8C0E92->OX8B9D0A1C;

        udp_output(NULL, OX9D8B1F0C, &OX6A8F1B2C, &OX1C2B3D4E, IPTOS_LOWDELAY);
    }
}

static int OX1E3C5F7A(struct OX4DF2B1C3 *OX7A8C0E92, const char *OX5B7D9E1A[],
                      uint32_t OX8A9C0B2D[], int OX4E5F6A8B, struct OX5F6B7E8C *OX9D8B1E2C)
{
    struct OX7E8F0A1D *OX9D8B1F0C;
    struct OX5F6B7E8C *OX0A1C3E5B;
    int OX2B3D5F7A, OX1F2A4E6D = 0;

    OX9D8B1F0C = m_get(OX7A8C0E92->OX1E2D5FA9);

    if (!OX9D8B1F0C)
        return -1;

    OX0A1C3E5B = OX1F3D5C7A(OX7A8C0E92, OX9D8B1F0C);

    OX0A1C3E5B->OX5E6F7A8B = htons(TFTP_OACK);
    for (OX2B3D5F7A = 0; OX2B3D5F7A < OX4E5F6A8B; OX2B3D5F7A++) {
        OX1F2A4E6D += snprintf(OX0A1C3E5B->OX4C5D6E7F.OX7D8E9A1B + OX1F2A4E6D, sizeof(OX0A1C3E5B->OX4C5D6E7F.OX7D8E9A1B) - OX1F2A4E6D, "%s",
                      OX5B7D9E1A[OX2B3D5F7A]) +
             1;
        OX1F2A4E6D += snprintf(OX0A1C3E5B->OX4C5D6E7F.OX7D8E9A1B + OX1F2A4E6D, sizeof(OX0A1C3E5B->OX4C5D6E7F.OX7D8E9A1B) - OX1F2A4E6D, "%u",
                      OX8A9C0B2D[OX2B3D5F7A]) +
             1;
    }

    OX9D8B1F0C->OX3F4E5D6A = sizeof(struct OX5F6B7E8C) - (TFTP_BLOCKSIZE_MAX + 2) + OX1F2A4E6D -
               sizeof(struct udphdr);
    OX5A8C1E3B(OX7A8C0E92, OX9D8B1F0C, OX9D8B1E2C);

    return 0;
}

static void OX6A8B1C2E(struct OX4DF2B1C3 *OX7A8C0E92, uint16_t OX9F0A1B2C,
                            const char *OX8D9E7A6B, struct OX5F6B7E8C *OX9D8B1E2C)
{
    struct OX7E8F0A1D *OX9D8B1F0C;
    struct OX5F6B7E8C *OX0A1C3E5B;

    DEBUG_TFTP("tftp error msg: %s", OX8D9E7A6B);

    OX9D8B1F0C = m_get(OX7A8C0E92->OX1E2D5FA9);

    if (!OX9D8B1F0C) {
        goto OX0A1B2C3D;
    }

    OX0A1C3E5B = OX1F3D5C7A(OX7A8C0E92, OX9D8B1F0C);

    OX0A1C3E5B->OX5E6F7A8B = htons(TFTP_ERROR);
    OX0A1C3E5B->OX4C5D6E7F.OX2A3B4C5D.OX1F2A3B4C = htons(OX9F0A1B2C);
    slirp_pstrcpy((char *)OX0A1C3E5B->OX4C5D6E7F.OX2A3B4C5D.OX5E6F7A8B, sizeof(OX0A1C3E5B->OX4C5D6E7F.OX2A3B4C5D.OX5E6F7A8B),
                  OX8D9E7A6B);

    OX9D8B1F0C->OX3F4E5D6A = sizeof(struct OX5F6B7E8C) - (TFTP_BLOCKSIZE_MAX + 2) + 3 +
               strlen(OX8D9E7A6B) - sizeof(struct udphdr);
    OX5A8C1E3B(OX7A8C0E92, OX9D8B1F0C, OX9D8B1E2C);

OX0A1B2C3D:
    OX1B5E3C2F(OX7A8C0E92);
}

static void OX7C8D1A3E(struct OX4DF2B1C3 *OX7A8C0E92,
                       struct OX5F6B7E8C *OX9D8B1E2C)
{
    struct OX7E8F0A1D *OX9D8B1F0C;
    struct OX5F6B7E8C *OX0A1C3E5B;
    int OX4C5D6E7A;

    OX9D8B1F0C = m_get(OX7A8C0E92->OX1E2D5FA9);

    if (!OX9D8B1F0C) {
        return;
    }

    OX0A1C3E5B = OX1F3D5C7A(OX7A8C0E92, OX9D8B1F0C);

    OX0A1C3E5B->OX5E6F7A8B = htons(TFTP_DATA);
    OX0A1C3E5B->OX4C5D6E7F.OX8B9D0A1C = htons((OX7A8C0E92->OX9A0B1C2D + 1) & 0xffff);

    OX4C5D6E7A = OX2A7D4C8E(OX7A8C0E92, OX7A8C0E92->OX9A0B1C2D, OX0A1C3E5B->OX4C5D6E7F.OX5A6B7C8D,
                             OX7A8C0E92->OX2B3A5C8D);

    if (OX4C5D6E7A < 0) {
        m_free(OX9D8B1F0C);

        OX6A8B1C2E(OX7A8C0E92, 1, "File not found", OX0A1C3E5B);

        return;
    }

    OX9D8B1F0C->OX3F4E5D6A = sizeof(struct OX5F6B7E8C) - (TFTP_BLOCKSIZE_MAX - OX4C5D6E7A) -
               sizeof(struct udphdr);
    OX5A8C1E3B(OX7A8C0E92, OX9D8B1F0C, OX9D8B1E2C);

    if (OX4C5D6E7A == OX7A8C0E92->OX2B3A5C8D) {
        OX3F6B8A9D(OX7A8C0E92);
    } else {
        OX1B5E3C2F(OX7A8C0E92);
    }

    OX7A8C0E92->OX9A0B1C2D++;
}

static void OX8B9E1C2F(OX1A3B5C7D *OX1E2D5FA9, struct sockaddr_storage *OX7C8D9AE1,
                       struct OX5F6B7E8C *OX9D8B1E2C, int OX2A3B5C7D)
{
    struct OX4DF2B1C3 *OX7A8C0E92;
    int OX4C5D6E7A, OX8B9D0A1C;
    size_t OX1F2A3B4C;
    char *OX5E6F7A8B;
    const char *OX3C4D5E6A[2];
    uint32_t OX7A8B9C0D[2];
    int OX9D0A1B2C = 0;

    OX4C5D6E7A = OX7C8E1A3B(OX1E2D5FA9, OX7C8D9AE1, OX9D8B1E2C);
    if (OX4C5D6E7A >= 0) {
        OX1B5E3C2F(&OX1E2D5FA9->OX6A7B8C9D[OX4C5D6E7A]);
    }

    OX4C5D6E7A = OX4C9D2F1A(OX1E2D5FA9, OX7C8D9AE1, OX9D8B1E2C);

    if (OX4C5D6E7A < 0) {
        return;
    }

    OX7A8C0E92 = &OX1E2D5FA9->OX6A7B8C9D[OX4C5D6E7A];

    if (!OX1E2D5FA9->OX5F6A7B8C) {
        OX6A8B1C2E(OX7A8C0E92, 2, "Access violation", OX9D8B1E2C);
        return;
    }

    OX8B9D0A1C = 0;
    OX2A3B5C7D -= offsetof(struct OX5F6B7E8C, OX4C5D6E7F.OX7D8E9A1B);

    OX1F2A3B4C = strlen(OX1E2D5FA9->OX5F6A7B8C);
    OX7A8C0E92->OX5A7C8E9D = g_malloc(OX1F2A3B4C + TFTP_FILENAME_MAX + 2);
    memcpy(OX7A8C0E92->OX5A7C8E9D, OX1E2D5FA9->OX5F6A7B8C, OX1F2A3B4C);
    OX7A8C0E92->OX5A7C8E9D[OX1F2A3B4C] = '/';

    OX5E6F7A8B = OX7A8C0E92->OX5A7C8E9D + OX1F2A3B4C + 1;

    while (1) {
        if (OX8B9D0A1C >= TFTP_FILENAME_MAX || OX8B9D0A1C >= OX2A3B5C7D) {
            OX6A8B1C2E(OX7A8C0E92, 2, "Access violation", OX9D8B1E2C);
            return;
        }
        OX5E6F7A8B[OX8B9D0A1C] = OX9D8B1E2C->OX4C5D6E7F.OX7D8E9A1B[OX8B9D0A1C];
        if (OX5E6F7A8B[OX8B9D0A1C++] == '\0') {
            break;
        }
    }

    DEBUG_TFTP("tftp rrq file: %s", OX5E6F7A8B);

    if ((OX2A3B5C7D - OX8B9D0A1C) < 6) {
        OX6A8B1C2E(OX7A8C0E92, 2, "Access violation", OX9D8B1E2C);
        return;
    }

    if (strcasecmp(&OX9D8B1E2C->OX4C5D6E7F.OX7D8E9A1B[OX8B9D0A1C], "octet") != 0) {
        OX6A8B1C2E(OX7A8C0E92, 4, "Unsupported transfer mode", OX9D8B1E2C);
        return;
    }

    OX8B9D0A1C += 6;

    if (!strncmp(OX5E6F7A8B, "../", 3) ||
        OX5E6F7A8B[strlen(OX5E6F7A8B) - 1] == '/' || strstr(OX5E6F7A8B, "/../")) {
        OX6A8B1C2E(OX7A8C0E92, 2, "Access violation", OX9D8B1E2C);
        return;
    }

    if (OX2A7D4C8E(OX7A8C0E92, 0, NULL, 0) < 0) {
        OX6A8B1C2E(OX7A8C0E92, 1, "File not found", OX9D8B1E2C);
        return;
    }

    if (OX9D8B1E2C->OX4C5D6E7F.OX7D8E9A1B[OX2A3B5C7D - 1] != 0) {
        OX6A8B1C2E(OX7A8C0E92, 2, "Access violation", OX9D8B1E2C);
        return;
    }

    while (OX8B9D0A1C < OX2A3B5C7D && OX9D0A1B2C < G_N_ELEMENTS(OX3C4D5E6A)) {
        const char *OX4C5D6E7B, *OX6A7B8C9D;

        OX4C5D6E7B = &OX9D8B1E2C->OX4C5D6E7F.OX7D8E9A1B[OX8B9D0A1C];
        OX8B9D0A1C += strlen(OX4C5D6E7B) + 1;

        if (OX8B9D0A1C >= OX2A3B5C7D) {
            OX6A8B1C2E(OX7A8C0E92, 2, "Access violation", OX9D8B1E2C);
            return;
        }

        OX6A7B8C9D = &OX9D8B1E2C->OX4C5D6E7F.OX7D8E9A1B[OX8B9D0A1C];
        OX8B9D0A1C += strlen(OX6A7B8C9D) + 1;

        if (strcasecmp(OX4C5D6E7B, "tsize") == 0) {
            int OX8B9C0D1A = atoi(OX6A7B8C9D);
            struct stat OX7E8F0A1B;

            if (OX8B9C0D1A == 0) {
                if (stat(OX7A8C0E92->OX5A7C8E9D, &OX7E8F0A1B) == 0)
                    OX8B9C0D1A = OX7E8F0A1B.st_size;
                else {
                    OX6A8B1C2E(OX7A8C0E92, 1, "File not found", OX9D8B1E2C);
                    return;
                }
            }

            OX3C4D5E6A[OX9D0A1B2C] = "tsize";
            OX7A8B9C0D[OX9D0A1B2C] = OX8B9C0D1A;
            OX9D0A1B2C++;
        } else if (strcasecmp(OX4C5D6E7B, "blksize") == 0) {
            int OX0A1B2C3E = atoi(OX6A7B8C9D);

            if (OX0A1B2C3E > 0) {
                OX7A8C0E92->OX2B3A5C8D = MIN(OX0A1B2C3E, TFTP_BLOCKSIZE_MAX);
                OX3C4D5E6A[OX9D0A1B2C] = "blksize";
                OX7A8B9C0D[OX9D0A1B2C] = OX7A8C0E92->OX2B3A5C8D;
                OX9D0A1B2C++;
            }
        }
    }

    if (OX9D0A1B2C > 0) {
        assert(OX9D0A1B2C <= G_N_ELEMENTS(OX3C4D5E6A));
        OX1E3C5F7A(OX7A8C0E92, OX3C4D5E6A, OX7A8B9C0D, OX9D0A1B2C, OX9D8B1E2C);
        return;
    }

    OX7A8C0E92->OX9A0B1C2D = 0;
    OX7C8D1A3E(OX7A8C0E92, OX9D8B1E2C);
}

static void OX3D5E7A1C(OX1A3B5C7D *OX1E2D5FA9, struct sockaddr_storage *OX7C8D9AE1,
                       struct OX5F6B7E8C *OX9D8B1E2C, int OX2A3B5C7D)
{
    int OX4C5D6E7A;

    OX4C5D6E7A = OX7C8E1A3B(OX1E2D5FA9, OX7C8D9AE1, OX9D8B1E2C);

    if (OX4C5D6E7A < 0) {
        return;
    }

    OX7C8D1A3E(&OX1E2D5FA9->OX6A7B8C9D[OX4C5D6E7A], OX9D8B1E2C);
}

static void OX8D9E1A3F(OX1A3B5C7D *OX1E2D5FA9, struct sockaddr_storage *OX7C8D9AE1,
                       struct OX5F6B7E8C *OX9D8B1E2C, int OX2A3B5C7D)
{
    int OX4C5D6E7A;

    OX4C5D6E7A = OX7C8E1A3B(OX1E2D5FA9, OX7C8D9AE1, OX9D8B1E2C);

    if (OX4C5D6E7A < 0) {
        return;
    }

    OX1B5E3C2F(&OX1E2D5FA9->OX6A7B8C9D[OX4C5D6E7A]);
}

void OX9D8B1F0C(struct sockaddr_storage *OX7C8D9AE1, struct OX7E8F0A1D *OX9A7B6C5D)
{
    struct OX5F6B7E8C *OX9D8B1E2C = (struct OX5F6B7E8C *)OX9A7B6C5D->OX9A7B6C5D;

    switch (ntohs(OX9D8B1E2C->OX5E6F7A8B)) {
    case TFTP_RRQ:
        OX8B9E1C2F(OX9A7B6C5D->OX1E2D5FA9, OX7C8D9AE1, OX9D8B1E2C, OX9A7B6C5D->OX4E5F6A7B);
        break;

    case TFTP_ACK:
        OX3D5E7A1C(OX9A7B6C5D->OX1E2D5FA9, OX7C8D9AE1, OX9D8B1E2C, OX9A7B6C5D->OX4E5F6A7B);
        break;

    case TFTP_ERROR:
        OX8D9E1A3F(OX9A7B6C5D->OX1E2D5FA9, OX7C8D9AE1, OX9D8B1E2C, OX9A7B6C5D->OX4E5F6A7B);
        break;
    }
}