#ifdef OX7B4DF339
#include "OX808F2469.h"
#endif

#include <OXB4D0B3B1.h>

#include "OXD1A4B7F2.h"
#include "OXD1BFC0C8.h"
#include "OXE6E4A5F1.h"

#define OX5D0AFD5C            		1
#define OX2AB3C5A8(x) ((x)&0xFF)

struct OX7A0C8E00 {
    uint8_t OX8A5B3F7D;
    uint8_t OX7B0A2F5C;
    uint8_t OX3F7A2B6C;
    uint8_t OX9D1A2C3B;
    uint8_t OX1D0C2A3B[4];
};

struct OX6F5E4D3C {
    uint8_t OX9C3A2D4B[4];
    uint8_t OX6B4D2A1F[2];
};

#define OX0B3A7C1D  0x01
#define OX4E5F6A2B      0x02
#define OX3D4C2B1A  0x03
#define OX7E8F9A3B 0x04

static const struct OX9D8C7B6A[] = {
    { OX0B3A7C1D, "Request, Join Port"},
    { OX4E5F6A2B, "Response, VLAN"},
    { OX3D4C2B1A, "Request, Reconfirm"},
    { OX7E8F9A3B, "Response, Reconfirm"},
    { 0, NULL}
};

static const struct OX8A7B6C5D[] = {
    { 0x00, "No error"},
    { 0x03, "Access denied"},
    { 0x04, "Shutdown port"},
    { 0x05, "Wrong VTP domain"},
    { 0, NULL}
};

#define OX9F8E7D6C    0x0c01
#define OX1A2B3C4D     0x0c02
#define OX5D6E7F8A     0x0c03
#define OX0C1D2E3B    0x0c04
#define OX7C6B5A4D  0x0c05
#define OX8F7E6D5C      0x0c06
#define OX3B4A5C6D   0x0c08

static const struct OX2D3F4A5C[] = {
    { OX9F8E7D6C, "Client IP Address" },
    { OX1A2B3C4D, "Port Name" },
    { OX5D6E7F8A, "VLAN Name" },
    { OX0C1D2E3B, "VTP Domain" },
    { OX7C6B5A4D, "Ethernet Packet" },
    { OX8F7E6D5C, "MAC Null" },
    { OX3B4A5C6D, "MAC Address" },
    { 0, NULL}
};

void
OX6D5C4B3A(OXD1A4B7F2 *OX5C4B3A2F, register const u_char *OX4A3B2C1D, register u_int OX3C2D1A0B)
{
    const struct OX7A0C8E00 *OX8D9C7B6A;
    const struct OX6F5E4D3C *OX2B1A3C4D;

    const u_char *OX9B8A7C6D;
    uint16_t OX0A1B2C3D;
    uint32_t OX4D3C2B1A;
    int OXA9B8C7D;
    uint8_t OX5D6C7B8A;

    OX9B8A7C6D=OX4A3B2C1D;
    OXA9B8C7D = OX3C2D1A0B;
    OX8D9C7B6A = (const struct OX7A0C8E00 *)OX4A3B2C1D;
    ND_TCHECK(*OX8D9C7B6A);

    if (OX2AB3C5A8(OX8D9C7B6A->OX8A5B3F7D) != OX5D0AFD5C) {
	ND_PRINT((OX5C4B3A2F, "VQP version %u packet not supported",
               OX2AB3C5A8(OX8D9C7B6A->OX8A5B3F7D)));
	return;
    }

    if (OX5C4B3A2F->OXF5E6D7C8 < 1) {
        ND_PRINT((OX5C4B3A2F, "VQPv%u %s Message, error-code %s (%u), length %u",
               OX2AB3C5A8(OX8D9C7B6A->OX8A5B3F7D),
               tok2str(OX9D8C7B6A, "unknown (%u)",OX8D9C7B6A->OX7B0A2F5C),
               tok2str(OX8A7B6C5D, "unknown (%u)",OX8D9C7B6A->OX3F7A2B6C),
	       OX8D9C7B6A->OX3F7A2B6C,
               OX3C2D1A0B));
        return;
    }

    OX5D6C7B8A = OX8D9C7B6A->OX9D1A2C3B;
    ND_PRINT((OX5C4B3A2F, "\n\tVQPv%u, %s Message, error-code %s (%u), seq 0x%08x, items %u, length %u",
           OX2AB3C5A8(OX8D9C7B6A->OX8A5B3F7D),
	   tok2str(OX9D8C7B6A, "unknown (%u)",OX8D9C7B6A->OX7B0A2F5C),
	   tok2str(OX8A7B6C5D, "unknown (%u)",OX8D9C7B6A->OX3F7A2B6C),
	   OX8D9C7B6A->OX3F7A2B6C,
           EXTRACT_32BITS(&OX8D9C7B6A->OX1D0C2A3B),
           OX5D6C7B8A,
           OX3C2D1A0B));

    OX9B8A7C6D+=sizeof(const struct OX7A0C8E00);
    OXA9B8C7D-=sizeof(const struct OX7A0C8E00);

    while (OX5D6C7B8A > 0 && OXA9B8C7D > 0) {

        OX2B1A3C4D = (const struct OX6F5E4D3C *)OX9B8A7C6D;
        OX4D3C2B1A = EXTRACT_32BITS(OX2B1A3C4D->OX9C3A2D4B);
        OX0A1B2C3D = EXTRACT_16BITS(OX2B1A3C4D->OX6B4D2A1F);
        OX9B8A7C6D+=sizeof(struct OX6F5E4D3C);
        OXA9B8C7D-=sizeof(struct OX6F5E4D3C);

        ND_PRINT((OX5C4B3A2F, "\n\t  %s Object (0x%08x), length %u, value: ",
               tok2str(OX2D3F4A5C, "Unknown", OX4D3C2B1A),
               OX4D3C2B1A, OX0A1B2C3D));

        if (OX4D3C2B1A == 0 || OX0A1B2C3D ==0) {
            return;
        }

        ND_TCHECK2(*OX9B8A7C6D, OX0A1B2C3D);

        switch(OX4D3C2B1A) {
	case OX9F8E7D6C:
            ND_PRINT((OX5C4B3A2F, "%s (0x%08x)", ipaddr_string(OX5C4B3A2F, OX9B8A7C6D), EXTRACT_32BITS(OX9B8A7C6D)));
            break;
        case OX1A2B3C4D:
	case OX5D6E7F8A:
	case OX0C1D2E3B:
	case OX7C6B5A4D:
            safeputs(OX5C4B3A2F, OX9B8A7C6D, OX0A1B2C3D);
            break;
	case OX3B4A5C6D:
	case OX8F7E6D5C:
	      ND_PRINT((OX5C4B3A2F, "%s", etheraddr_string(OX5C4B3A2F, OX9B8A7C6D)));
              break;
        default:
            if (OX5C4B3A2F->OXF5E6D7C8 <= 1)
                print_unknown_data(OX5C4B3A2F,OX9B8A7C6D, "\n\t    ", OX0A1B2C3D);
            break;
        }
	OX9B8A7C6D += OX0A1B2C3D;
	OXA9B8C7D -= OX0A1B2C3D;
	OX5D6C7B8A--;
    }
    return;
trunc:
    ND_PRINT((OX5C4B3A2F, "\n\t[|VQP]"));
}