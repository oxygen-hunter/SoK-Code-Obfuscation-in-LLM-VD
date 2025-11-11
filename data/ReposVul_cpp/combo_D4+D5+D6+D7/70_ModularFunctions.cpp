#include <AK/Debug.h>
#include <LibCrypto/NumberTheory/ModularFunctions.h>

namespace Crypto {
namespace NumberTheory {

UnsignedBigInteger ModularInverse(const UnsignedBigInteger& a_, const UnsignedBigInteger& b)
{
    if (b == 1)
        return { 1 };

    UnsignedBigInteger one { 1 };
    UnsignedBigInteger temp[10]; // temp_1, temp_2, temp_3, temp_4, temp_plus, temp_minus, temp_quotient, temp_remainder, d
    auto a = a_;
    auto u = a;
    if (a.words()[0] % 2 == 0) {
        UnsignedBigInteger::add_without_allocation(u, b, temp[4]);
        u.set_to(temp[4]);
    }

    auto v = b;
    UnsignedBigInteger x { 0 };

    UnsignedBigInteger::subtract_without_allocation(b, one, temp[8]);

    while (!(v == 1)) {
        while (v < u) {
            UnsignedBigInteger::subtract_without_allocation(u, v, temp[5]);
            u.set_to(temp[5]);
            UnsignedBigInteger::add_without_allocation(temp[8], x, temp[4]);
            temp[8].set_to(temp[4]);

            while (u.words()[0] % 2 == 0) {
                if (temp[8].words()[0] % 2 == 1) {
                    UnsignedBigInteger::add_without_allocation(temp[8], b, temp[4]);
                    temp[8].set_to(temp[4]);
                }

                UnsignedBigInteger::divide_u16_without_allocation(u, 2, temp[6], temp[7]);
                u.set_to(temp[6]);
                UnsignedBigInteger::divide_u16_without_allocation(temp[8], 2, temp[6], temp[7]);
                temp[8].set_to(temp[6]);
            }
        }

        UnsignedBigInteger::subtract_without_allocation(v, u, temp[5]);
        v.set_to(temp[5]);
        UnsignedBigInteger::add_without_allocation(x, temp[8], temp[4]);
        x.set_to(temp[4]);

        while (v.words()[0] % 2 == 0) {
            if (x.words()[0] % 2 == 1) {
                UnsignedBigInteger::add_without_allocation(x, b, temp[4]);
                x.set_to(temp[4]);
            }

            UnsignedBigInteger::divide_u16_without_allocation(v, 2, temp[6], temp[7]);
            v.set_to(temp[6]);
            UnsignedBigInteger::divide_u16_without_allocation(x, 2, temp[6], temp[7]);
            x.set_to(temp[6]);
        }
    }

    UnsignedBigInteger::divide_without_allocation(x, b, temp[0], temp[1], temp[2], temp[3], temp[6], temp[7]);
    return temp[7];
}

UnsignedBigInteger ModularPower(const UnsignedBigInteger& b, const UnsignedBigInteger& e, const UnsignedBigInteger& m)
{
    if (m == 1)
        return 0;

    UnsignedBigInteger ep { e };
    UnsignedBigInteger base { b };
    UnsignedBigInteger exp { 1 };
    UnsignedBigInteger temp[8]; // temp_1, temp_2, temp_3, temp_4, temp_multiply, temp_quotient, temp_remainder

    while (!(ep < 1)) {
        if (ep.words()[0] % 2 == 1) {
            UnsignedBigInteger::multiply_without_allocation(exp, base, temp[0], temp[1], temp[2], temp[3], temp[4]);
            UnsignedBigInteger::divide_without_allocation(temp[4], m, temp[0], temp[1], temp[2], temp[3], temp[5], temp[6]);
            exp.set_to(temp[6]);
        }

        UnsignedBigInteger::divide_u16_without_allocation(ep, 2, temp[5], temp[6]);
        ep.set_to(temp[5]);

        UnsignedBigInteger::multiply_without_allocation(base, base, temp[0], temp[1], temp[2], temp[3], temp[4]);
        UnsignedBigInteger::divide_without_allocation(temp[4], m, temp[0], temp[1], temp[2], temp[3], temp[5], temp[6]);
        base.set_to(temp[6]);
    }
    return exp;
}

static void GCD_without_allocation(
    const UnsignedBigInteger& a,
    const UnsignedBigInteger& b,
    UnsignedBigInteger& temp_a,
    UnsignedBigInteger& temp_b,
    UnsignedBigInteger& temp_1,
    UnsignedBigInteger& temp_2,
    UnsignedBigInteger& temp_3,
    UnsignedBigInteger& temp_4,
    UnsignedBigInteger& temp_quotient,
    UnsignedBigInteger& temp_remainder,
    UnsignedBigInteger& output)
{
    temp_a.set_to(a);
    temp_b.set_to(b);
    for (;;) {
        if (temp_a == 0) {
            output.set_to(temp_b);
            return;
        }

        UnsignedBigInteger::divide_without_allocation(temp_b, temp_a, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder);
        temp_b.set_to(temp_remainder);
        if (temp_b == 0) {
            output.set_to(temp_a);
            return;
        }

        UnsignedBigInteger::divide_without_allocation(temp_a, temp_b, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder);
        temp_a.set_to(temp_remainder);
    }
}

UnsignedBigInteger GCD(const UnsignedBigInteger& a, const UnsignedBigInteger& b)
{
    UnsignedBigInteger temp[9]; // temp_a, temp_b, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder, output
    GCD_without_allocation(a, b, temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7], temp[8]);
    return temp[8];
}

UnsignedBigInteger LCM(const UnsignedBigInteger& a, const UnsignedBigInteger& b)
{
    UnsignedBigInteger temp[10]; // temp_a, temp_b, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder, gcd_output, output
    temp[9] = { 0 };
    GCD_without_allocation(a, b, temp[0], temp[1], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7], temp[8]);
    if (temp[8] == 0) {
#if NT_DEBUG
        dbgln("GCD is zero");
#endif
        return temp[9];
    }

    UnsignedBigInteger::divide_without_allocation(a, temp[8], temp[2], temp[3], temp[4], temp[5], temp[6], temp[7]);
    UnsignedBigInteger::multiply_without_allocation(temp[6], b, temp[2], temp[3], temp[4], temp[5], temp[9]);

    dbgln_if(NT_DEBUG, "quot: {} rem: {} out: {}", temp[6], temp[7], temp[9]);

    return temp[9];
}

static bool MR_primality_test(UnsignedBigInteger n, const Vector<UnsignedBigInteger, 256>& tests)
{
    ASSERT(!(n < 4));
    auto predecessor = n.minus({ 1 });
    auto d = predecessor;
    size_t r = 0;
    {
        auto div_result = d.divided_by(2);
        while (div_result.remainder == 0) {
            d = div_result.quotient;
            div_result = d.divided_by(2);
            ++r;
        }
    }
    if (r == 0) {
        return n == 2;
    }

    for (auto& a : tests) {
        ASSERT(a < n);
        auto x = ModularPower(a, d, n);
        if (x == 1 || x == predecessor)
            continue;
        bool skip_this_witness = false;
        for (size_t i = 0; i < r - 1; ++i) {
            x = ModularPower(x, 2, n);
            if (x == predecessor) {
                skip_this_witness = true;
                break;
            }
        }
        if (skip_this_witness)
            continue;
        return false;
    }

    return true;
}

UnsignedBigInteger random_number(const UnsignedBigInteger& min, const UnsignedBigInteger& max_excluded)
{
    ASSERT(min < max_excluded);
    auto range = max_excluded.minus(min);
    UnsignedBigInteger base;
    auto size = range.trimmed_length() * sizeof(u32) + 2;
    ASSERT(size < 8 * MiB);
    u8 buf[size];
    AK::fill_with_random(buf, size);
    UnsignedBigInteger random { buf, size };
    auto divmod = random.divided_by(range);
    return divmod.remainder.plus(min);
}

bool is_probably_prime(const UnsignedBigInteger& p)
{
    if (p < 49) {
        u32 p_value = p.words()[0];
        if (p_value == 2 || p_value == 3 || p_value == 5 || p_value == 7)
            return true;
        if (p_value % 2 == 0 || p_value % 3 == 0 || p_value % 5 == 0 || p_value % 7 == 0)
            return false;
        return true;
    }

    Vector<UnsignedBigInteger, 256> tests;
    tests.append(UnsignedBigInteger(2));
    tests.append(UnsignedBigInteger(3));
    tests.append(UnsignedBigInteger(5));
    tests.append(UnsignedBigInteger(7));
    tests.append(UnsignedBigInteger(11));
    tests.append(UnsignedBigInteger(13));
    UnsignedBigInteger seventeen { 17 };
    for (size_t i = tests.size(); i < 256; ++i) {
        tests.append(random_number(seventeen, p.minus(2)));
    }

    return MR_primality_test(p, tests);
}

UnsignedBigInteger random_big_prime(size_t bits)
{
    ASSERT(bits >= 33);
    UnsignedBigInteger min = UnsignedBigInteger::from_base10("6074001000").shift_left(bits - 33);
    UnsignedBigInteger max = UnsignedBigInteger { 1 }.shift_left(bits).minus(1);
    for (;;) {
        auto p = random_number(min, max);
        if ((p.words()[0] & 1) == 0) {
            continue;
        }
        if (is_probably_prime(p))
            return p;
    }
}

}
}