#include <AK/Debug.h>
#include <LibCrypto/NumberTheory/ModularFunctions.h>

namespace OX4B4E1A24 {
namespace OX1C2F4B3D {

UnsignedBigInteger OX7B4DF339(const UnsignedBigInteger& OX6C8E9A3F, const UnsignedBigInteger& OX1D2F3B4E)
{
    if (OX1D2F3B4E == 1)
        return { 1 };

    UnsignedBigInteger OX8E1C3A4B { 1 };
    UnsignedBigInteger OX2B3D4F6A;
    UnsignedBigInteger OX3A7B9C0D;
    UnsignedBigInteger OX4D8E9F1A;
    UnsignedBigInteger OX5F6A7B8C;
    UnsignedBigInteger OX9C0D1E2F;
    UnsignedBigInteger OX2A3B4C5D;
    UnsignedBigInteger OX6B7C8D9E;
    UnsignedBigInteger OX7E8F9A0B;
    UnsignedBigInteger OX0A1B2C3D;

    auto OX2D3C4B5A = OX6C8E9A3F;
    auto OX3B4C5D6E = OX2D3C4B5A;
    if (OX2D3C4B5A.words()[0] % 2 == 0) {
        UnsignedBigInteger::add_without_allocation(OX3B4C5D6E, OX1D2F3B4E, OX9C0D1E2F);
        OX3B4C5D6E.set_to(OX9C0D1E2F);
    }

    auto OX4C5D6E7F = OX1D2F3B4E;
    UnsignedBigInteger OX5B6C7D8E { 0 };

    UnsignedBigInteger::subtract_without_allocation(OX1D2F3B4E, OX8E1C3A4B, OX0A1B2C3D);

    while (!(OX4C5D6E7F == 1)) {
        while (OX4C5D6E7F < OX3B4C5D6E) {
            UnsignedBigInteger::subtract_without_allocation(OX3B4C5D6E, OX4C5D6E7F, OX2A3B4C5D);
            OX3B4C5D6E.set_to(OX2A3B4C5D);

            UnsignedBigInteger::add_without_allocation(OX0A1B2C3D, OX5B6C7D8E, OX9C0D1E2F);
            OX0A1B2C3D.set_to(OX9C0D1E2F);

            while (OX3B4C5D6E.words()[0] % 2 == 0) {
                if (OX0A1B2C3D.words()[0] % 2 == 1) {
                    UnsignedBigInteger::add_without_allocation(OX0A1B2C3D, OX1D2F3B4E, OX9C0D1E2F);
                    OX0A1B2C3D.set_to(OX9C0D1E2F);
                }

                UnsignedBigInteger::divide_u16_without_allocation(OX3B4C5D6E, 2, OX6B7C8D9E, OX7E8F9A0B);
                OX3B4C5D6E.set_to(OX6B7C8D9E);

                UnsignedBigInteger::divide_u16_without_allocation(OX0A1B2C3D, 2, OX6B7C8D9E, OX7E8F9A0B);
                OX0A1B2C3D.set_to(OX6B7C8D9E);
            }
        }

        UnsignedBigInteger::subtract_without_allocation(OX4C5D6E7F, OX3B4C5D6E, OX2A3B4C5D);
        OX4C5D6E7F.set_to(OX2A3B4C5D);

        UnsignedBigInteger::add_without_allocation(OX5B6C7D8E, OX0A1B2C3D, OX9C0D1E2F);
        OX5B6C7D8E.set_to(OX9C0D1E2F);

        while (OX4C5D6E7F.words()[0] % 2 == 0) {
            if (OX5B6C7D8E.words()[0] % 2 == 1) {
                UnsignedBigInteger::add_without_allocation(OX5B6C7D8E, OX1D2F3B4E, OX9C0D1E2F);
                OX5B6C7D8E.set_to(OX9C0D1E2F);
            }

            UnsignedBigInteger::divide_u16_without_allocation(OX4C5D6E7F, 2, OX6B7C8D9E, OX7E8F9A0B);
            OX4C5D6E7F.set_to(OX6B7C8D9E);

            UnsignedBigInteger::divide_u16_without_allocation(OX5B6C7D8E, 2, OX6B7C8D9E, OX7E8F9A0B);
            OX5B6C7D8E.set_to(OX6B7C8D9E);
        }
    }

    UnsignedBigInteger::divide_without_allocation(OX5B6C7D8E, OX1D2F3B4E, OX2B3D4F6A, OX3A7B9C0D, OX4D8E9F1A, OX5F6A7B8C, OX6B7C8D9E, OX7E8F9A0B);
    return OX7E8F9A0B;
}

UnsignedBigInteger OX6F4B2D8A(const UnsignedBigInteger& OX1D2F3B4E, const UnsignedBigInteger& OX4E5A6B7C, const UnsignedBigInteger& OX8B9C0D1E)
{
    if (OX8B9C0D1E == 1)
        return 0;

    UnsignedBigInteger OX5E6F7A8B { OX4E5A6B7C };
    UnsignedBigInteger OX9A0B1C2D { OX1D2F3B4E };
    UnsignedBigInteger OX0B1C2D3E { 1 };

    UnsignedBigInteger OX2B3C4D5E;
    UnsignedBigInteger OX3C4D5E6F;
    UnsignedBigInteger OX4D5E6F7A;
    UnsignedBigInteger OX5F6A7B8C;
    UnsignedBigInteger OX6A7B8C9D;
    UnsignedBigInteger OX7B8C9D0E;
    UnsignedBigInteger OX8C9D0E1F;

    while (!(OX5E6F7A8B < 1)) {
        if (OX5E6F7A8B.words()[0] % 2 == 1) {
            UnsignedBigInteger::multiply_without_allocation(OX0B1C2D3E, OX9A0B1C2D, OX2B3C4D5E, OX3C4D5E6F, OX4D5E6F7A, OX5F6A7B8C, OX6A7B8C9D);
            UnsignedBigInteger::divide_without_allocation(OX6A7B8C9D, OX8B9C0D1E, OX2B3C4D5E, OX3C4D5E6F, OX4D5E6F7A, OX5F6A7B8C, OX7B8C9D0E, OX8C9D0E1F);
            OX0B1C2D3E.set_to(OX8C9D0E1F);
        }

        UnsignedBigInteger::divide_u16_without_allocation(OX5E6F7A8B, 2, OX7B8C9D0E, OX8C9D0E1F);
        OX5E6F7A8B.set_to(OX7B8C9D0E);

        UnsignedBigInteger::multiply_without_allocation(OX9A0B1C2D, OX9A0B1C2D, OX2B3C4D5E, OX3C4D5E6F, OX4D5E6F7A, OX5F6A7B8C, OX6A7B8C9D);
        UnsignedBigInteger::divide_without_allocation(OX6A7B8C9D, OX8B9C0D1E, OX2B3C4D5E, OX3C4D5E6F, OX4D5E6F7A, OX5F6A7B8C, OX7B8C9D0E, OX8C9D0E1F);
        OX9A0B1C2D.set_to(OX8C9D0E1F);
    }
    return OX0B1C2D3E;
}

static void OX1A2B3C4D(
    const UnsignedBigInteger& OX5D6E7F8A,
    const UnsignedBigInteger& OX9A0B1C2D,
    UnsignedBigInteger& OX2C3D4E5F,
    UnsignedBigInteger& OX3E4F5A6B,
    UnsignedBigInteger& OX4F5A6B7C,
    UnsignedBigInteger& OX5A6B7C8D,
    UnsignedBigInteger& OX6B7C8D9E,
    UnsignedBigInteger& OX7C8D9E0F,
    UnsignedBigInteger& OX8D9E0F1A,
    UnsignedBigInteger& OX9E0F1A2B,
    UnsignedBigInteger& OX0F1A2B3C,
    UnsignedBigInteger& OX1F2A3B4C)
{
    OX2C3D4E5F.set_to(OX5D6E7F8A);
    OX3E4F5A6B.set_to(OX9A0B1C2D);
    for (;;) {
        if (OX2C3D4E5F == 0) {
            OX1F2A3B4C.set_to(OX3E4F5A6B);
            return;
        }

        UnsignedBigInteger::divide_without_allocation(OX3E4F5A6B, OX2C3D4E5F, OX4F5A6B7C, OX5A6B7C8D, OX6B7C8D9E, OX7C8D9E0F, OX8D9E0F1A, OX9E0F1A2B);
        OX3E4F5A6B.set_to(OX9E0F1A2B);
        if (OX3E4F5A6B == 0) {
            OX1F2A3B4C.set_to(OX2C3D4E5F);
            return;
        }

        UnsignedBigInteger::divide_without_allocation(OX2C3D4E5F, OX3E4F5A6B, OX4F5A6B7C, OX5A6B7C8D, OX6B7C8D9E, OX7C8D9E0F, OX8D9E0F1A, OX9E0F1A2B);
        OX2C3D4E5F.set_to(OX9E0F1A2B);
    }
}

UnsignedBigInteger OX5B6C7D8E(const UnsignedBigInteger& OX1D2F3B4E, const UnsignedBigInteger& OX4E5A6B7C)
{
    UnsignedBigInteger OX7F8A9B0C;
    UnsignedBigInteger OX8A9B0C1D;
    UnsignedBigInteger OX9B0C1D2E;
    UnsignedBigInteger OX0C1D2E3F;
    UnsignedBigInteger OX1E2F3A4B;
    UnsignedBigInteger OX2F3A4B5C;
    UnsignedBigInteger OX3A4B5C6D;
    UnsignedBigInteger OX4B5C6D7E;
    UnsignedBigInteger OX5C6D7E8F;

    OX1A2B3C4D(OX1D2F3B4E, OX4E5A6B7C, OX7F8A9B0C, OX8A9B0C1D, OX9B0C1D2E, OX0C1D2E3F, OX1E2F3A4B, OX2F3A4B5C, OX3A4B5C6D, OX4B5C6D7E, OX5C6D7E8F, OX5B6C7D8E);

    return OX5B6C7D8E;
}

UnsignedBigInteger OX2A3B4C5D(const UnsignedBigInteger& OX1D2F3B4E, const UnsignedBigInteger& OX4E5A6B7C)
{
    UnsignedBigInteger OX4F5A6B7C;
    UnsignedBigInteger OX5A6B7C8D;
    UnsignedBigInteger OX6B7C8D9E;
    UnsignedBigInteger OX7C8D9E0F;
    UnsignedBigInteger OX8D9E0F1A;
    UnsignedBigInteger OX9E0F1A2B;
    UnsignedBigInteger OX0F1A2B3C;
    UnsignedBigInteger OX1F2A3B4C;
    UnsignedBigInteger OX2F3A4B5C { 0 };

    OX1A2B3C4D(OX1D2F3B4E, OX4E5A6B7C, OX4F5A6B7C, OX5A6B7C8D, OX6B7C8D9E, OX7C8D9E0F, OX8D9E0F1A, OX9E0F1A2B, OX0F1A2B3C, OX1F2A3B4C, OX2F3A4B5C, OX5B6C7D8E);
    if (OX5B6C7D8E == 0) {
#if NT_DEBUG
        dbgln("GCD is zero");
#endif
        return OX2F3A4B5C;
    }

    UnsignedBigInteger::divide_without_allocation(OX1D2F3B4E, OX5B6C7D8E, OX6B7C8D9E, OX7C8D9E0F, OX8D9E0F1A, OX9E0F1A2B, OX0F1A2B3C, OX1F2A3B4C);
    UnsignedBigInteger::multiply_without_allocation(OX0F1A2B3C, OX4E5A6B7C, OX6B7C8D9E, OX7C8D9E0F, OX8D9E0F1A, OX9E0F1A2B, OX2F3A4B5C);

    dbgln_if(NT_DEBUG, "quot: {} rem: {} out: {}", OX0F1A2B3C, OX1F2A3B4C, OX2F3A4B5C);

    return OX2F3A4B5C;
}

static bool OX6E7F8A9B(UnsignedBigInteger OX1D2F3B4E, const Vector<UnsignedBigInteger, 256>& OX4E5A6B7C)
{
    ASSERT(!(OX1D2F3B4E < 4));
    auto OX8B9C0D1E = OX1D2F3B4E.minus({ 1 });
    auto OX9A0B1C2D = OX8B9C0D1E;
    size_t OX0A1B2C3D = 0;

    {
        auto OX3B4C5D6E = OX9A0B1C2D.divided_by(2);
        while (OX3B4C5D6E.remainder == 0) {
            OX9A0B1C2D = OX3B4C5D6E.quotient;
            OX3B4C5D6E = OX9A0B1C2D.divided_by(2);
            ++OX0A1B2C3D;
        }
    }
    if (OX0A1B2C3D == 0) {
        return OX1D2F3B4E == 2;
    }

    for (auto& OX2D3C4B5A : OX4E5A6B7C) {
        ASSERT(OX2D3C4B5A < OX1D2F3B4E);
        auto OX3E4F5A6B = OX6F4B2D8A(OX2D3C4B5A, OX9A0B1C2D, OX1D2F3B4E);
        if (OX3E4F5A6B == 1 || OX3E4F5A6B == OX8B9C0D1E)
            continue;
        bool OX5A6B7C8D = false;
        for (size_t OX6B7C8D9E = 0; OX6B7C8D9E < OX0A1B2C3D - 1; ++OX6B7C8D9E) {
            OX3E4F5A6B = OX6F4B2D8A(OX3E4F5A6B, 2, OX1D2F3B4E);
            if (OX3E4F5A6B == OX8B9C0D1E) {
                OX5A6B7C8D = true;
                break;
            }
        }
        if (OX5A6B7C8D)
            continue;
        return false;
    }

    return true;
}

UnsignedBigInteger OX8F9A0B1C(const UnsignedBigInteger& OX1D2F3B4E, const UnsignedBigInteger& OX4E5A6B7C)
{
    ASSERT(OX1D2F3B4E < OX4E5A6B7C);
    auto OX5B6C7D8E = OX4E5A6B7C.minus(OX1D2F3B4E);
    UnsignedBigInteger OX6C7D8E9F;
    auto OX7D8E9F0A = OX5B6C7D8E.trimmed_length() * sizeof(u32) + 2;
    ASSERT(OX7D8E9F0A < 8 * MiB);
    u8 OX8E9F0A1B[OX7D8E9F0A];
    AK::fill_with_random(OX8E9F0A1B, OX7D8E9F0A);
    UnsignedBigInteger OX9F0A1B2C { OX8E9F0A1B, OX7D8E9F0A };
    auto OX0A1B2C3D = OX9F0A1B2C.divided_by(OX5B6C7D8E);
    return OX0A1B2C3D.remainder.plus(OX1D2F3B4E);
}

bool OX9B0C1D2E(const UnsignedBigInteger& OX1D2F3B4E)
{
    if (OX1D2F3B4E < 49) {
        u32 OX2A3B4C5D = OX1D2F3B4E.words()[0];
        if (OX2A3B4C5D == 2 || OX2A3B4C5D == 3 || OX2A3B4C5D == 5 || OX2A3B4C5D == 7)
            return true;
        if (OX2A3B4C5D % 2 == 0 || OX2A3B4C5D % 3 == 0 || OX2A3B4C5D % 5 == 0 || OX2A3B4C5D % 7 == 0)
            return false;
        return true;
    }

    Vector<UnsignedBigInteger, 256> OX5B6C7D8E;
    OX5B6C7D8E.append(UnsignedBigInteger(2));
    OX5B6C7D8E.append(UnsignedBigInteger(3));
    OX5B6C7D8E.append(UnsignedBigInteger(5));
    OX5B6C7D8E.append(UnsignedBigInteger(7));
    OX5B6C7D8E.append(UnsignedBigInteger(11));
    OX5B6C7D8E.append(UnsignedBigInteger(13));
    UnsignedBigInteger OX6B7C8D9E { 17 };
    for (size_t OX7C8D9E0F = OX5B6C7D8E.size(); OX7C8D9E0F < 256; ++OX7C8D9E0F) {
        OX5B6C7D8E.append(OX8F9A0B1C(OX6B7C8D9E, OX1D2F3B4E.minus(2)));
    }

    return OX6E7F8A9B(OX1D2F3B4E, OX5B6C7D8E);
}

UnsignedBigInteger OX0C1D2E3F(size_t OX1D2F3B4E)
{
    ASSERT(OX1D2F3B4E >= 33);
    UnsignedBigInteger OX4E5A6B7C = UnsignedBigInteger::from_base10("6074001000").shift_left(OX1D2F3B4E - 33);
    UnsignedBigInteger OX5B6C7D8E = UnsignedBigInteger { 1 }.shift_left(OX1D2F3B4E).minus(1);
    for (;;) {
        auto OX6A7B8C9D = OX8F9A0B1C(OX4E5A6B7C, OX5B6C7D8E);
        if ((OX6A7B8C9D.words()[0] & 1) == 0) {
            continue;
        }
        if (OX9B0C1D2E(OX6A7B8C9D))
            return OX6A7B8C9D;
    }
}

}
}