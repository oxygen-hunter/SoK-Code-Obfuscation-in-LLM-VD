#include <AK/Debug.h>
#include <LibCrypto/NumberTheory/ModularFunctions.h>
#include <Python.h>

namespace Crypto {
namespace NumberTheory {

extern "C" PyObject* ModularInverse(PyObject* self, PyObject* args) {
    PyObject *pya, *pyb;
    if (!PyArg_ParseTuple(args, "OO", &pya, &pyb))
        return nullptr;

    UnsignedBigInteger a = reinterpret_cast<UnsignedBigInteger*>(PyCapsule_GetPointer(pya, nullptr));
    UnsignedBigInteger b = reinterpret_cast<UnsignedBigInteger*>(PyCapsule_GetPointer(pyb, nullptr));

    if (b == 1)
        return PyCapsule_New(new UnsignedBigInteger { 1 }, nullptr, nullptr);

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

    auto a_ = a;
    auto u = a_;
    if (a_.words()[0] % 2 == 0) {
        UnsignedBigInteger::add_without_allocation(u, b, temp_plus);
        u.set_to(temp_plus);
    }

    auto v = b;
    UnsignedBigInteger x { 0 };

    UnsignedBigInteger::subtract_without_allocation(b, one, d);

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

                UnsignedBigInteger::divide_u16_without_allocation(u, 2, temp_quotient, temp_remainder);
                u.set_to(temp_quotient);

                UnsignedBigInteger::divide_u16_without_allocation(d, 2, temp_quotient, temp_remainder);
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

            UnsignedBigInteger::divide_u16_without_allocation(v, 2, temp_quotient, temp_remainder);
            v.set_to(temp_quotient);

            UnsignedBigInteger::divide_u16_without_allocation(x, 2, temp_quotient, temp_remainder);
            x.set_to(temp_quotient);
        }
    }

    UnsignedBigInteger::divide_without_allocation(x, b, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder);
    return PyCapsule_New(new UnsignedBigInteger(temp_remainder), nullptr, nullptr);
}

extern "C" PyObject* ModularPower(PyObject* self, PyObject* args) {
    PyObject *pyb, *pye, *pym;
    if (!PyArg_ParseTuple(args, "OOO", &pyb, &pye, &pym))
        return nullptr;

    UnsignedBigInteger b = reinterpret_cast<UnsignedBigInteger*>(PyCapsule_GetPointer(pyb, nullptr));
    UnsignedBigInteger e = reinterpret_cast<UnsignedBigInteger*>(PyCapsule_GetPointer(pye, nullptr));
    UnsignedBigInteger m = reinterpret_cast<UnsignedBigInteger*>(PyCapsule_GetPointer(pym, nullptr));

    if (m == 1)
        return PyCapsule_New(new UnsignedBigInteger(0), nullptr, nullptr);

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

        UnsignedBigInteger::divide_u16_without_allocation(ep, 2, temp_quotient, temp_remainder);
        ep.set_to(temp_quotient);

        UnsignedBigInteger::multiply_without_allocation(base, base, temp_1, temp_2, temp_3, temp_4, temp_multiply);
        UnsignedBigInteger::divide_without_allocation(temp_multiply, m, temp_1, temp_2, temp_3, temp_4, temp_quotient, temp_remainder);
        base.set_to(temp_remainder);
    }

    return PyCapsule_New(new UnsignedBigInteger(exp), nullptr, nullptr);
}

} // namespace NumberTheory
} // namespace Crypto

static PyMethodDef CryptoMethods[] = {
    {"ModularInverse", Crypto::NumberTheory::ModularInverse, METH_VARARGS, "Compute modular inverse."},
    {"ModularPower", Crypto::NumberTheory::ModularPower, METH_VARARGS, "Compute modular power."},
    {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef crypto_module = {
    PyModuleDef_HEAD_INIT,
    "crypto",
    nullptr,
    -1,
    CryptoMethods
};

PyMODINIT_FUNC PyInit_crypto(void) {
    return PyModule_Create(&crypto_module);
}