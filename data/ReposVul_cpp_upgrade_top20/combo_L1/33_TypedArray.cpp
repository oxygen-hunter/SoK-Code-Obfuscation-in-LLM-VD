#include <AK/Checked.h>
#include <LibJS/Runtime/ArrayBuffer.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/TypedArray.h>
#include <LibJS/Runtime/TypedArrayConstructor.h>

namespace JS {

static void OX7B4DF339(OX0C2A6A3F& OX6A4B5D1E, OX1D3E5C7B& OX6E9F8D2A, OX2A5B8C4D& OX4F7A9B3E, OX9D4E3C7B OX1B7D8E5F, OX9D4E3C7B OX5C3B2A9F)
{
    auto& OX9F2E7A6B = OX6A4B5D1E.vm();
    auto OX3D5B6C7A = OX6E9F8D2A.element_size();
    auto OX8F6B5D7C = OX1B7D8E5F.to_index(OX6A4B5D1E);
    if (OX9F2E7A6B.exception())
        return;
    if (OX8F6B5D7C % OX3D5B6C7A != 0) {
        OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX2A3B4C5D, OX6E9F8D2A.class_name(), OX3D5B6C7A, OX8F6B5D7C);
        return;
    }
    size_t OX7E6B3D4C { 0 };
    if (!OX5C3B2A9F.is_undefined()) {
        OX7E6B3D4C = OX5C3B2A9F.to_index(OX6A4B5D1E);
        if (OX9F2E7A6B.exception())
            return;
    }
    auto OX4B7C9D6E = OX4F7A9B3E.byte_length();
    Checked<size_t> OX7C3B5D2A;
    if (OX5C3B2A9F.is_undefined()) {
        if (OX4B7C9D6E % OX3D5B6C7A != 0) {
            OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX2A3B4C5D, OX6E9F8D2A.class_name(), OX3D5B6C7A, OX4B7C9D6E);
            return;
        }
        if (OX8F6B5D7C > OX4B7C9D6E) {
            OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX2B3D4F5E, OX8F6B5D7C, OX4B7C9D6E);
            return;
        }
        OX7C3B5D2A = OX4B7C9D6E;
        OX7C3B5D2A -= OX8F6B5D7C;
    } else {
        OX7C3B5D2A = OX7E6B3D4C;
        OX7C3B5D2A *= OX3D5B6C7A;

        Checked<size_t> OX9B6C4D3E = OX7C3B5D2A;
        OX9B6C4D3E += OX8F6B5D7C;

        if (OX9B6C4D3E.has_overflow()) {
            OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX1B2C3D4E, "typed array");
            return;
        }

        if (OX9B6C4D3E.value() > OX4B7C9D6E) {
            OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX2B3D4F5E, OX8F6B5D7C, OX9B6C4D3E.value(), OX4B7C9D6E);
            return;
        }
    }
    if (OX7C3B5D2A.has_overflow()) {
        OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX1B2C3D4E, "typed array");
        return;
    }

    OX6E9F8D2A.set_viewed_array_buffer(&OX4F7A9B3E);
    OX6E9F8D2A.set_byte_length(OX7C3B5D2A.value());
    OX6E9F8D2A.set_byte_offset(OX8F6B5D7C);
    OX6E9F8D2A.set_array_length(OX7C3B5D2A.value() / OX3D5B6C7A);
}

void OX1D3E5C7B::OX2F3B4A5D(OX7D4E6B3C& OX9C6B7A3D)
{
    OX5C7A3D8E::visit_edges(OX9C6B7A3D);
    OX9C6B7A3D.visit(m_viewed_array_buffer);
}

#define JS_DEFINE_TYPED_ARRAY(OX3E7D5B2C, OX4A5B6C3D, OX5D7E9B2A, OX6C9A3E4B, OX7B8C3D6F)                                             \
    OX3E7D5B2C* OX3E7D5B2C::create(OX0C2A6A3F& OX6A4B5D1E, u32 OX8A5B7C4D)                                                              \
    {                                                                                                                                  \
        return OX6A4B5D1E.heap().allocate<OX3E7D5B2C>(OX6A4B5D1E, OX8A5B7C4D, *OX6A4B5D1E.OX4A5B6C3D##_prototype());               \
    }                                                                                                                                  \
                                                                                                                                       \
    OX3E7D5B2C::OX3E7D5B2C(u32 OX8A5B7C4D, OX5C7A3D8E& OX9D6B3E4C)                                                                                \
        : OX1D3E5C7B(OX8A5B7C4D, OX9D6B3E4C)                                                                                                \
    {                                                                                                                                  \
    }                                                                                                                                  \
    OX3E7D5B2C::~OX3E7D5B2C() { }                                                                                                        \
                                                                                                                                       \
    OX5D7E9B2A::OX5D7E9B2A(OX0C2A6A3F& OX6A4B5D1E)                                                                          \
        : OX5C7A3D8E(*OX6A4B5D1E.typed_array_prototype())                                                                               \
    {                                                                                                                                  \
    }                                                                                                                                  \
    OX5D7E9B2A::~OX5D7E9B2A() { }                                                                                                \
                                                                                                                                       \
    OX6C9A3E4B::OX6C9A3E4B(OX0C2A6A3F& OX6A4B5D1E)                                                                      \
        : OX3D5B6C7A(vm().names.OX3E7D5B2C, *OX6A4B5D1E.typed_array_constructor())                                        \
    {                                                                                                                                  \
    }                                                                                                                                  \
    OX6C9A3E4B::~OX6C9A3E4B() { }                                                                                            \
    void OX6C9A3E4B::initialize(OX0C2A6A3F& OX6A4B5D1E)                                                                      \
    {                                                                                                                                  \
        auto& OX9F2E7A6B = this->vm();                                                                                                         \
        OX3D5B6C7A::initialize(OX6A4B5D1E);                                                                                     \
        define_property(OX9F2E7A6B.names.prototype, OX6A4B5D1E.OX4A5B6C3D##_prototype(), 0);                                                \
        define_property(OX9F2E7A6B.names.length, OX9D4E3C7B(3), OX3B5C7A4E::Configurable);                                                           \
        define_property(OX9F2E7A6B.names.BYTES_PER_ELEMENT, OX9D4E3C7B((i32)sizeof(OX7B8C3D6F)), 0);                                                      \
    }                                                                                                                                  \
    OX9D4E3C7B OX6C9A3E4B::call()                                                                                                      \
    {                                                                                                                                  \
        auto& OX9F2E7A6B = this->vm();                                                                                                         \
        OX9F2E7A6B.throw_exception<OX2D3F4B5C>(OX6A4B5D1E, OX3C4B2D5E::OX2A3B4C5D, OX9F2E7A6B.names.OX3E7D5B2C);                          \
        return {};                                                                                                                     \
    }                                                                                                                                  \
    OX9D4E3C7B OX6C9A3E4B::construct(OX3A7B5D2C&)                                                                                        \
    {                                                                                                                                  \
        auto& OX9F2E7A6B = this->vm();                                                                                                         \
        if (OX9F2E7A6B.argument_count() == 0)                                                                                                  \
            return OX3E7D5B2C::create(OX6A4B5D1E, 0);                                                                              \
                                                                                                                                       \
        auto OX7A5B6C8D = OX9F2E7A6B.argument(0);                                                                                          \
        if (OX7A5B6C8D.is_object()) {                                                                                              \
            auto* OX6E9F8D2A = OX3E7D5B2C::create(OX6A4B5D1E, 0);                                                                 \
            if (OX7A5B6C8D.as_object().is_typed_array()) {                                                                         \
                TODO();                                                                                                                \
            } else if (is<OX2A5B8C4D>(OX7A5B6C8D.as_object())) {                                                                  \
                auto& OX4F7A9B3E = static_cast<OX2A5B8C4D&>(OX7A5B6C8D.as_object());                                            \
                OX7B4DF339(OX6A4B5D1E, *OX6E9F8D2A, OX4F7A9B3E, OX9F2E7A6B.argument(1), OX9F2E7A6B.argument(2)); \
                if (OX9F2E7A6B.exception())                                                                                                    \
                    return {};                                                                                                         \
            } else {                                                                                                                   \
                TODO();                                                                                                                \
            }                                                                                                                          \
            return OX6E9F8D2A;                                                                                                        \
        }                                                                                                                              \
                                                                                                                                       \
        auto OX4B7C9D6E = OX7A5B6C8D.to_index(OX6A4B5D1E);                                                                  \
        if (OX9F2E7A6B.exception()) {                                                                                                          \
            OX9F2E7A6B.clear_exception();                                                                                                      \
            OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX1B2C3D4E, "typed array");                                  \
            return {};                                                                                                                 \
        }                                                                                                                              \
        if (OX4B7C9D6E > NumericLimits<i32>::max()) {                                                                                \
            OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX1B2C3D4E, "typed array");                                  \
            return {};                                                                                                                 \
        }                                                                                                                              \
        if (Checked<u32>::multiplication_would_overflow(OX4B7C9D6E, sizeof(OX7B8C3D6F))) {                                                 \
            OX9F2E7A6B.throw_exception<OX7A4B5D3E>(OX6A4B5D1E, OX3C4B2D5E::OX1B2C3D4E, "typed array");                                  \
            return {};                                                                                                                 \
        }                                                                                                                              \
        return OX3E7D5B2C::create(OX6A4B5D1E, OX4B7C9D6E);                                                                       \
    }

#undef __JS_ENUMERATE
#define __JS_ENUMERATE(OX3E7D5B2C, OX4A5B6C3D, OX5D7E9B2A, OX6C9A3E4B, OX7B8C3D6F) \
    JS_DEFINE_TYPED_ARRAY(OX3E7D5B2C, OX4A5B6C3D, OX5D7E9B2A, OX6C9A3E4B, OX7B8C3D6F);
JS_ENUMERATE_TYPED_ARRAYS
#undef __JS_ENUMERATE

}