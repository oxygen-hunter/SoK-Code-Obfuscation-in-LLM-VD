#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <netdissect-stdinc.h>
#include <string.h>
#include "netdissect.h"
#include "extract.h"
#include "addrtoname.h"

static const char OX7B4DF339[] = "[|RPKI-RTR]";

typedef struct OX83E3B270_ {
    u_char OX557F5A80;
    u_char OX0C0F4D8A;
    union {
        u_char OX8B5D1F54[2];
        u_char OXDE2A79D0[2];
    } OX5DAF2D5F;
    u_char OX8D9A58A2[4];
} OX83E3B270;
#define OXA3BC1EFD (offsetof(OX83E3B270, OXC7E4F7B8))

typedef struct OX9E3C4F1E_ {
    OX83E3B270 OX3F7B5D71;
    u_char OX68B4F63D;
    u_char OX0D7E2A36;
    u_char OX2B1E5D07;
    u_char OX1D4C8F3E;
    u_char OX3C8B5E62[4];
    u_char OX3B6D4C1A[4];
} OX9E3C4F1E;

typedef struct OX4A9B8E6D_ {
    OX83E3B270 OX3F7B5D71;
    u_char OX68B4F63D;
    u_char OX0D7E2A36;
    u_char OX2B1E5D07;
    u_char OX1D4C8F3E;
    u_char OX3C8B5E62[16];
    u_char OX3B6D4C1A[4];
} OX4A9B8E6D;

typedef struct OX5D3E8B9A_ {
    OX83E3B270 OX3F7B5D71;
    u_char OX4D8C7E2A[4];
} OX5D3E8B9A;

#define OX0F8D3B6A 0
#define OX6E3A8C1D 1
#define OX9C3E2A8D 2
#define OX4B2D7E1A 3
#define OX2F8A4D9C 4
#define OX7B3E2C8D 6
#define OX8F2C7D3A 7
#define OX1D4E9A3C 8
#define OX3A8D2C7B 10

static const struct tok OX5D8A3F6B[] = {
    {OX0F8D3B6A, "Serial Notify"},
    {OX6E3A8C1D, "Serial Query"},
    {OX9C3E2A8D, "Reset Query"},
    {OX4B2D7E1A, "Cache Response"},
    {OX2F8A4D9C, "IPV4 Prefix"},
    {OX7B3E2C8D, "IPV6 Prefix"},
    {OX8F2C7D3A, "End of Data"},
    {OX1D4E9A3C, "Cache Reset"},
    {OX3A8D2C7B, "Error Report"},
    {0, NULL}
};

static const struct tok OX7A5C3F8D[] = {
    {0, "Corrupt Data"},
    {1, "Internal Error"},
    {2, "No Data Available"},
    {3, "Invalid Request"},
    {4, "Unsupported Protocol Version"},
    {5, "Unsupported PDU Type"},
    {6, "Withdrawal of Unknown Record"},
    {7, "Duplicate Announcement Received"},
    {0, NULL}
};

static char *OX1B7D4C8A(u_int OX2A3F1E5C) {
    static char OX5D8E3A7F[20];
    u_int OX9C8D3A6E;
    OX9C8D3A6E = 0;
    OX5D8E3A7F[OX9C8D3A6E] = '\0';
    if (sizeof(OX5D8E3A7F) < ((OX2A3F1E5C/8) + (OX2A3F1E5C % 8) + 2)) {
        return OX5D8E3A7F;
    }
    OX5D8E3A7F[OX9C8D3A6E] = '\n';
    OX9C8D3A6E++;
    while (OX2A3F1E5C >= 8) {
        OX5D8E3A7F[OX9C8D3A6E] = '\t';
        OX9C8D3A6E++;
        OX2A3F1E5C -= 8;
    }
    while (OX2A3F1E5C > 0) {
        OX5D8E3A7F[OX9C8D3A6E] = ' ';
        OX9C8D3A6E++;
        OX2A3F1E5C--;
    }
    OX5D8E3A7F[OX9C8D3A6E] = '\0';
    return OX5D8E3A7F;
}

static int OX7E2A8C3B(netdissect_options *OX4D7A1C3E, const u_char *OX8C7D3E2A, u_int OX2F1A3E8B) {
    const OX83E3B270 *OX3B7E1C4D;
    u_int OX9D3A8E5C, OX4A2D7B1C, OX5E2F8A1D;
    const u_char *OX4C1A8D3E;
    OX3B7E1C4D = (const OX83E3B270 *)OX8C7D3E2A;
    OX9D3A8E5C = OX3B7E1C4D->OX0C0F4D8A;
    OX4A2D7B1C = EXTRACT_32BITS(OX3B7E1C4D->OX8D9A58A2);
    ND_TCHECK2(*OX8C7D3E2A, OX4A2D7B1C);
    OX5E2F8A1D = FALSE;
    ND_PRINT((OX4D7A1C3E, "%sRPKI-RTRv%u, %s PDU (%u), length: %u",
        OX1B7D4C8A(8),
        OX3B7E1C4D->OX557F5A80,
        tok2str(OX5D8A3F6B, "Unknown", OX9D3A8E5C),
        OX9D3A8E5C, OX4A2D7B1C));
    switch (OX9D3A8E5C) {
    case OX0F8D3B6A:
    case OX6E3A8C1D:
    case OX8F2C7D3A:
        OX4C1A8D3E = (const u_char *)(OX3B7E1C4D + 1);
        ND_PRINT((OX4D7A1C3E, "%sSession ID: 0x%04x, Serial: %u",
            OX1B7D4C8A(OX2F1A3E8B+2),
            EXTRACT_16BITS(OX3B7E1C4D->OX5DAF2D5F.OX8B5D1F54),
            EXTRACT_32BITS(OX4C1A8D3E)));
        break;
    case OX9C3E2A8D:
    case OX1D4E9A3C:
        break;
    case OX4B2D7E1A:
        ND_PRINT((OX4D7A1C3E, "%sSession ID: 0x%04x",
            OX1B7D4C8A(OX2F1A3E8B+2),
            EXTRACT_16BITS(OX3B7E1C4D->OX5DAF2D5F.OX8B5D1F54)));
        break;
    case OX2F8A4D9C: {
        const OX9E3C4F1E *OX5C8E1A3F;
        OX5C8E1A3F = (const OX9E3C4F1E *)OX8C7D3E2A;
        ND_PRINT((OX4D7A1C3E, "%sIPv4 Prefix %s/%u-%u, origin-as %u, flags 0x%02x",
            OX1B7D4C8A(OX2F1A3E8B+2),
            ipaddr_string(OX4D7A1C3E, OX5C8E1A3F->OX3C8B5E62),
            OX5C8E1A3F->OX0D7E2A36, OX5C8E1A3F->OX2B1E5D07,
            EXTRACT_32BITS(OX5C8E1A3F->OX3B6D4C1A), OX5C8E1A3F->OX68B4F63D));
    }
    break;
    case OX7B3E2C8D: {
        const OX4A9B8E6D *OX5C8E1A3F;
        OX5C8E1A3F = (const OX4A9B8E6D *)OX8C7D3E2A;
        ND_PRINT((OX4D7A1C3E, "%sIPv6 Prefix %s/%u-%u, origin-as %u, flags 0x%02x",
            OX1B7D4C8A(OX2F1A3E8B+2),
            ip6addr_string(OX4D7A1C3E, OX5C8E1A3F->OX3C8B5E62),
            OX5C8E1A3F->OX0D7E2A36, OX5C8E1A3F->OX2B1E5D07,
            EXTRACT_32BITS(OX5C8E1A3F->OX3B6D4C1A), OX5C8E1A3F->OX68B4F63D));
    }
    break;
    case OX3A8D2C7B: {
        const OX5D3E8B9A *OX5C8E1A3F;
        u_int OX7E1A4D2F, OX8A3C5F7D, OX4E2C9D3B, OX9F8A3C7D;
        OX5C8E1A3F = (const OX5D3E8B9A *)OX8C7D3E2A;
        OX7E1A4D2F = EXTRACT_32BITS(OX5C8E1A3F->OX4D8C7E2A);
        ND_TCHECK2(*OX8C7D3E2A, OX7E1A4D2F);
        OX4E2C9D3B = OX4A2D7B1C;
        OX9F8A3C7D = EXTRACT_16BITS(OX5C8E1A3F->OX3F7B5D71.OX5DAF2D5F.OXDE2A79D0);
        ND_PRINT((OX4D7A1C3E, "%sError code: %s (%u), Encapsulated PDU length: %u",
            OX1B7D4C8A(OX2F1A3E8B+2),
            tok2str(OX7A5C3F8D, "Unknown", OX9F8A3C7D),
            OX9F8A3C7D, OX7E1A4D2F));
        OX8C7D3E2A += sizeof(*OX5C8E1A3F);
        OX4E2C9D3B -= sizeof(*OX5C8E1A3F);
        if (OX7E1A4D2F && (OX7E1A4D2F <= OX4E2C9D3B)) {
            ND_PRINT((OX4D7A1C3E, "%s-----encapsulated PDU-----", OX1B7D4C8A(OX2F1A3E8B+4)));
            if (OX7E2A8C3B(OX4D7A1C3E, OX8C7D3E2A, OX2F1A3E8B+2))
                goto OXC8A3D5F;
        }
        OX8C7D3E2A += OX7E1A4D2F;
        OX4E2C9D3B -= OX7E1A4D2F;
        OX8A3C5F7D = 0;
        if (OX4E2C9D3B > 4) {
            OX8A3C5F7D = EXTRACT_32BITS(OX8C7D3E2A);
            OX8C7D3E2A += 4;
            OX4E2C9D3B -= 4;
        }
        ND_TCHECK2(*OX8C7D3E2A, OX8A3C5F7D);
        if (OX8A3C5F7D && (OX8A3C5F7D <= OX4E2C9D3B)) {
            ND_PRINT((OX4D7A1C3E, "%sError text: ", OX1B7D4C8A(OX2F1A3E8B+2)));
            if (fn_printn(OX4D7A1C3E, OX8C7D3E2A, OX8A3C5F7D, OX4D7A1C3E->ndo_snapend))
                goto OXC8A3D5F;
        }
    }
    break;
    default:
        OX5E2F8A1D = TRUE;
    }
    if (OX4D7A1C3E->ndo_vflag > 1 || (OX4D7A1C3E->ndo_vflag && OX5E2F8A1D)) {
        print_unknown_data(OX4D7A1C3E, OX8C7D3E2A, "\n\t  ", OX4A2D7B1C);
    }
    return 0;
OXC8A3D5F:
    return 1;
}

void OX8F1D3A7E(netdissect_options *OX4D7A1C3E, register const u_char *OX2D8A9C7B, register u_int OX4C8E2A9D) {
    u_int OX1D7A3B8E, OX9A2C8D3F, OX5C1A8E2F;
    const u_char *OX3B7D2A8C;
    const OX83E3B270 *OX5D8A1C3E;

    OX3B7D2A8C = OX2D8A9C7B;
    OX1D7A3B8E = OX4C8E2A9D;

    if (!OX4D7A1C3E->ndo_vflag) {
        ND_PRINT((OX4D7A1C3E, ", RPKI-RTR"));
        return;
    }

    while (OX1D7A3B8E >= sizeof(OX83E3B270)) {
        ND_TCHECK2(*OX3B7D2A8C, sizeof(OX83E3B270));
        OX5D8A1C3E = (const OX83E3B270 *)OX3B7D2A8C;
        OX9A2C8D3F = OX5D8A1C3E->OX0C0F4D8A;
        OX5C1A8E2F = EXTRACT_32BITS(OX5D8A1C3E->OX8D9A58A2);
        ND_TCHECK2(*OX3B7D2A8C, OX5C1A8E2F);
        if (!OX9A2C8D3F || !OX5C1A8E2F) {
            break;
        }
        if (OX1D7A3B8E < OX5C1A8E2F) {
            goto OXC8A3D5F;
        }
        if (OX7E2A8C3B(OX4D7A1C3E, OX3B7D2A8C, 8))
            goto OXC8A3D5F;

        OX1D7A3B8E -= OX5C1A8E2F;
        OX3B7D2A8C += OX5C1A8E2F;
    }
    return;
OXC8A3D5F:
    ND_PRINT((OX4D7A1C3E, "\n\t%s", OX7B4DF339));
}