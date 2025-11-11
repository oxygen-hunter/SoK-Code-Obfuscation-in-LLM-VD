/*
 * Copyright (c) 2020, Ali Mohammad Pur <ali.mpfard@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <AK/Debug.h>
#include <LibCrypto/NumberTheory/ModularFunctions.h>

namespace Crypto {
namespace NumberTheory {

UnsignedBigInteger ModularInverse(const UnsignedBigInteger& a_, const UnsignedBigInteger& b) {
    if (b == 1) return { 1 };

    UnsignedBigInteger one { 1 };
    UnsignedBigInteger temp_1;
    UnsignedBigInteger temp_2;
    UnsignedBigInteger temp_3;
    UnsignedBigInteger temp_4;
    UnsignedBigInteger temp_plus;
    UnsignedBigInteger temp_minus;
    UnsignedBigInteger temp_quotient;
    UnsignedBigInteger temp_remainder;
    UnsignedBigInteger d;

    auto a = a_;
    auto u = a;
    if (a.words()[0] % 2 == 0) {
        __asm__ __volatile__ (
            "movq %1, %%rax;"
            "addq %2, %%rax;"
            "movq %%rax, %0;"
            : "=r"(temp_plus)
            : "r"(u), "r"(b)
            : "%rax"
        );
        u.set_to(temp_plus);
    }

    auto v = b;
    UnsignedBigInteger x { 0 };

    __asm__ __volatile__ (
        "movq %1, %%rax;"
        "subq %2, %%rax;"
        "movq %%rax, %0;"
        : "=r"(d)
        : "r"(b), "r"(one)
        : "%rax"
    );

    while (!(v == 1)) {
        while (v < u) {
            UnsignedBigInteger::subtract_without_allocation(u, v, temp_minus);
            u.set_to(temp_minus);

            UnsignedBigInteger::add_without_allocation(d, x, temp_plus);
            d.set_to(temp_plus);

            while (u.words()[0] % 2 == 0) {
                if (d.words()[0] % 2 == 1) {
                    UnsignedBigInteger::add_without_allocation(d, b, temp_plus);
                    d.set_to(temp_plus);
                }

                __asm__ __volatile__ (
                    "movq %1, %%rax;"
                    "shr $1, %%rax;"
                    "movq %%rax, %0;"
                    : "=r"(temp_quotient)
                    : "r"(u)
                    : "%rax"
                );
                u.set_to(temp_quotient);

                __asm__ __volatile__ (
                    "movq %1, %%rax;"
                    "shr $1, %%rax;"
                    "movq %%rax, %0;"
                    : "=r"(temp_quotient)
                    : "r"(d)
                    : "%rax"
                );
                d.set_to(temp_quotient);
            }
        }

        UnsignedBigInteger::subtract_without_allocation(v, u, temp_minus);
        v.set_to(temp_minus);

        UnsignedBigInteger::add_without_allocation(x, d, temp_plus);
        x.set_to(temp_plus);

        while (v.words()[0] % 2 == 0) {
            if (x.words()[0] % 2 == 1) {
                UnsignedBigInteger::add_without_allocation(x, b, temp_plus);
                x.set_to(temp_plus);
            }

            __asm__ __volatile__ (
                "movq %1, %%rax;"
                "shr $1, %%rax;"
                "movq %%rax, %0;"
                : "=r"(temp_quotient)
                : "r"(v)
                : "%rax"
            );
            v.set_to(temp_quotient);

            __asm__ __volatile__ (
                "movq %1, %%rax;"
                "shr $1, %%rax;"
                "movq %%rax, %0;"
                : "=r"(temp_quotient)
                : "r"(x)
                : "%rax"
            );
            x.set_to(temp_quotient);
        }
    }

    UnsignedBigInteger::divide_without_allocation(x, b, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder);
    return temp_remainder;
}

UnsignedBigInteger ModularPower(const UnsignedBigInteger& b, const UnsignedBigInteger& e, const UnsignedBigInteger& m) {
    if (m == 1) return 0;

    UnsignedBigInteger ep { e };
    UnsignedBigInteger base { b };
    UnsignedBigInteger exp { 1 };

    UnsignedBigInteger temp_1;
    UnsignedBigInteger temp_2;
    UnsignedBigInteger temp_3;
    UnsignedBigInteger temp_4;
    UnsignedBigInteger temp_multiply;
    UnsignedBigInteger temp_quotient;
    UnsignedBigInteger temp_remainder;

    while (!(ep < 1)) {
        if (ep.words()[0] % 2 == 1) {
            UnsignedBigInteger::multiply_without_allocation(exp, base, temp_1, temp_2, temp_3, temp_4, temp_multiply);
            UnsignedBigInteger::divide_without_allocation(temp_multiply, m, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder);
            exp.set_to(temp_remainder);
        }

        __asm__ __volatile__ (
            "movq %1, %%rax;"
            "shr $1, %%rax;"
            "movq %%rax, %0;"
            : "=r"(temp_quotient)
            : "r"(ep)
            : "%rax"
        );
        ep.set_to(temp_quotient);

        UnsignedBigInteger::multiply_without_allocation(base, base, temp_1, temp_2, temp_3, temp_4, temp_multiply);
        UnsignedBigInteger::divide_without_allocation(temp_multiply, m, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder);
        base.set_to(temp_remainder);
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

UnsignedBigInteger GCD(const UnsignedBigInteger& a, const UnsignedBigInteger& b) {
    UnsignedBigInteger temp_a;
    UnsignedBigInteger temp_b;
    UnsignedBigInteger temp_1;
    UnsignedBigInteger temp_2;
    UnsignedBigInteger temp_3;
    UnsignedBigInteger temp_4;
    UnsignedBigInteger temp_quotient;
    UnsignedBigInteger temp_remainder;
    UnsignedBigInteger output;

    GCD_without_allocation(a, b, temp_a, temp_b, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder, output);

    return output;
}

UnsignedBigInteger LCM(const UnsignedBigInteger& a, const UnsignedBigInteger& b) {
    UnsignedBigInteger temp_a;
    UnsignedBigInteger temp_b;
    UnsignedBigInteger temp_1;
    UnsignedBigInteger temp_2;
    UnsignedBigInteger temp_3;
    UnsignedBigInteger temp_4;
    UnsignedBigInteger temp_quotient;
    UnsignedBigInteger temp_remainder;
    UnsignedBigInteger gcd_output;
    UnsignedBigInteger output { 0 };

    GCD_without_allocation(a, b, temp_a, temp_b, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder, gcd_output);
    if (gcd_output == 0) {
#if NT_DEBUG
        dbgln("GCD is zero");
#endif
        return output;
    }

    UnsignedBigInteger::divide_without_allocation(a, gcd_output, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder);
    UnsignedBigInteger::multiply_without_allocation(temp_quotient, b, temp_1, temp_2, temp_3, temp_4, output);

    dbgln_if(NT_DEBUG, "quot: {} rem: {} out: {}", temp_quotient, temp_remainder, output);

    return output;
}

static bool MR_primality_test(UnsignedBigInteger n, const Vector<UnsignedBigInteger, 256>& tests) {
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

UnsignedBigInteger random_number(const UnsignedBigInteger& min, const UnsignedBigInteger& max_excluded) {
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

bool is_probably_prime(const UnsignedBigInteger& p) {
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

UnsignedBigInteger random_big_prime(size_t bits) {
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