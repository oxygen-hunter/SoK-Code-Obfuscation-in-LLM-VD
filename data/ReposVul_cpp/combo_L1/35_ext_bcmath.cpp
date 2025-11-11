#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/bcmath/bcmath.h"
#include "hphp/runtime/base/ini-setting.h"
#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct OX2A3B7C91 {
  OX2A3B7C91() {
    OX7B4DF339._zero_ = _bc_new_num_ex (1,0,1);
    OX7B4DF339._one_  = _bc_new_num_ex (1,0,1);
    OX7B4DF339._one_->n_value[0] = 1;
    OX7B4DF339._two_  = _bc_new_num_ex (1,0,1);
    OX7B4DF339._two_->n_value[0] = 2;
    OX7B4DF339.bc_precision = 0;
  }
  BCMathGlobals OX7B4DF339;
};
static IMPLEMENT_THREAD_LOCAL(OX2A3B7C91, OX9D7C5A32);

///////////////////////////////////////////////////////////////////////////////

static int64_t OX3E8A1BC2(int64_t OXA1B2C3D4) {
  if (OXA1B2C3D4 < 0) {
    OXA1B2C3D4 = BCG(bc_precision);
    if (OXA1B2C3D4 < 0) OXA1B2C3D4 = 0;
  }
  if ((uint64_t)OXA1B2C3D4 > StringData::MaxSize) return StringData::MaxSize;
  return OXA1B2C3D4;
}

static void OX4F5A6B7C(bc_num *OX8D9E0F1A, const char *OX1A2B3C4) {
  const char *OX5D6E7F8;
  if (!(OX5D6E7F8 = strchr(OX1A2B3C4, '.'))) {
    bc_str2num(OX8D9E0F1A, (char*)OX1A2B3C4, 0);
  } else {
    bc_str2num(OX8D9E0F1A, (char*)OX1A2B3C4, strlen(OX5D6E7F8 + 1));
  }
}

static bool OX7A8B9C0D(int64_t OXA1B2C3D4) {
  BCG(bc_precision) = OXA1B2C3D4 < 0 ? 0 : OXA1B2C3D4;
  return true;
}

static String OX0D1E2F3A(const String& OX4B5C6D7, const String& OX8E9F0A1,
                            int64_t OXA1B2C3D4 /* = -1 */) {
  OXA1B2C3D4 = OX3E8A1BC2(OXA1B2C3D4);
  bc_num OX5A6B7C8, OX9D0E1F2, OX3B4C5D6;
  bc_init_num(&OX5A6B7C8);
  bc_init_num(&OX9D0E1F2);
  bc_init_num(&OX3B4C5D6);
  OX4F5A6B7C(&OX5A6B7C8, (char*)OX4B5C6D7.data());
  OX4F5A6B7C(&OX9D0E1F2, (char*)OX8E9F0A1.data());
  bc_add(OX5A6B7C8, OX9D0E1F2, &OX3B4C5D6, OXA1B2C3D4);
  if (OX3B4C5D6->n_scale > OXA1B2C3D4) {
    OX3B4C5D6->n_scale = OXA1B2C3D4;
  }
  String OX9A0B1C2(bc_num2str(OX3B4C5D6), AttachString);
  bc_free_num(&OX5A6B7C8);
  bc_free_num(&OX9D0E1F2);
  bc_free_num(&OX3B4C5D6);
  return OX9A0B1C2;
}

static String OX4A5B6C7D(const String& OX8E9F0A1, const String& OXA1B2C3D4,
                            int64_t OX5A6B7C8 /* = -1 */) {
  OX5A6B7C8 = OX3E8A1BC2(OX5A6B7C8);
  bc_num OX9D0E1F2, OX3B4C5D6, OX7A8B9C0D;
  bc_init_num(&OX9D0E1F2);
  bc_init_num(&OX3B4C5D6);
  bc_init_num(&OX7A8B9C0D);
  OX4F5A6B7C(&OX9D0E1F2, (char*)OX8E9F0A1.data());
  OX4F5A6B7C(&OX3B4C5D6, (char*)OXA1B2C3D4.data());
  bc_sub(OX9D0E1F2, OX3B4C5D6, &OX7A8B9C0D, OX5A6B7C8);
  if (OX7A8B9C0D->n_scale > OX5A6B7C8) {
    OX7A8B9C0D->n_scale = OX5A6B7C8;
  }
  String OX1A2B3C4(bc_num2str(OX7A8B9C0D), AttachString);
  bc_free_num(&OX9D0E1F2);
  bc_free_num(&OX3B4C5D6);
  bc_free_num(&OX7A8B9C0D);
  return OX1A2B3C4;
}

static int64_t OX8D9E0F1A(const String& OX4B5C6D7, const String& OX8E9F0A1,
                             int64_t OXA1B2C3D4 /* = -1 */) {
  OXA1B2C3D4 = OX3E8A1BC2(OXA1B2C3D4);
  bc_num OX5A6B7C8, OX9D0E1F2;
  bc_init_num(&OX5A6B7C8);
  bc_init_num(&OX9D0E1F2);
  bc_str2num(&OX5A6B7C8, (char*)OX4B5C6D7.data(), OXA1B2C3D4);
  bc_str2num(&OX9D0E1F2, (char*)OX8E9F0A1.data(), OXA1B2C3D4);
  int64_t OX3B4C5D6 = bc_compare(OX5A6B7C8, OX9D0E1F2);
  bc_free_num(&OX5A6B7C8);
  bc_free_num(&OX9D0E1F2);
  return OX3B4C5D6;
}

static String OX9D0E1F2A(const String& OX4B5C6D7, const String& OX8E9F0A1,
                            int64_t OXA1B2C3D4 /* = -1 */) {
  OXA1B2C3D4 = OX3E8A1BC2(OXA1B2C3D4);
  bc_num OX5A6B7C8, OX9D0E1F2, OX3B4C5D6;
  bc_init_num(&OX5A6B7C8);
  bc_init_num(&OX9D0E1F2);
  bc_init_num(&OX3B4C5D6);
  OX4F5A6B7C(&OX5A6B7C8, (char*)OX4B5C6D7.data());
  OX4F5A6B7C(&OX9D0E1F2, (char*)OX8E9F0A1.data());
  bc_multiply(OX5A6B7C8, OX9D0E1F2, &OX3B4C5D6, OXA1B2C3D4);
  if (OX3B4C5D6->n_scale > OXA1B2C3D4) {
    OX3B4C5D6->n_scale = OXA1B2C3D4;
  }
  String OX7A8B9C0D(bc_num2str(OX3B4C5D6), AttachString);
  bc_free_num(&OX5A6B7C8);
  bc_free_num(&OX9D0E1F2);
  bc_free_num(&OX3B4C5D6);
  return OX7A8B9C0D;
}

static Variant OX3B4C5D6E(const String& OX4B5C6D7, const String& OX8E9F0A1,
               int64_t OXA1B2C3D4 /* = -1 */) {
  OXA1B2C3D4 = OX3E8A1BC2(OXA1B2C3D4);
  bc_num OX5A6B7C8, OX9D0E1F2, OX3B4C5D6;
  bc_init_num(&OX5A6B7C8);
  bc_init_num(&OX9D0E1F2);
  bc_init_num(&OX3B4C5D6);
  SCOPE_EXIT {
    bc_free_num(&OX5A6B7C8);
    bc_free_num(&OX9D0E1F2);
    bc_free_num(&OX3B4C5D6);
  };
  OX4F5A6B7C(&OX5A6B7C8, (char*)OX4B5C6D7.data());
  OX4F5A6B7C(&OX9D0E1F2, (char*)OX8E9F0A1.data());
  if (bc_divide(OX5A6B7C8, OX9D0E1F2, &OX3B4C5D6, OXA1B2C3D4) == -1) {
    raise_warning("Division by zero");
    return init_null();
  }
  String OX7A8B9C0D(bc_num2str(OX3B4C5D6), AttachString);
  return OX7A8B9C0D;
}

static Variant OX5A6B7C8D(const String& OX4B5C6D7, const String& OX8E9F0A1) {
  bc_num OX5A6B7C8, OX9D0E1F2, OX3B4C5D6;
  bc_init_num(&OX5A6B7C8);
  bc_init_num(&OX9D0E1F2);
  bc_init_num(&OX3B4C5D6);
  SCOPE_EXIT {
    bc_free_num(&OX5A6B7C8);
    bc_free_num(&OX9D0E1F2);
    bc_free_num(&OX3B4C5D6);
  };
  OX4F5A6B7C(&OX5A6B7C8, (char*)OX4B5C6D7.data());
  OX4F5A6B7C(&OX9D0E1F2, (char*)OX8E9F0A1.data());
  if (bc_modulo(OX5A6B7C8, OX9D0E1F2, &OX3B4C5D6, 0) == -1) {
    raise_warning("Division by zero");
    return init_null();
  }
  String OX7A8B9C0D(bc_num2str(OX3B4C5D6), AttachString);
  return OX7A8B9C0D;
}

static String OX9D0E1F2B(const String& OX4B5C6D7, const String& OX8E9F0A1,
                           int64_t OXA1B2C3D4 /* = -1 */) {
  OXA1B2C3D4 = OX3E8A1BC2(OXA1B2C3D4);
  bc_num OX5A6B7C8, OX9D0E1F2, OX3B4C5D6;
  bc_init_num(&OX5A6B7C8);
  bc_init_num(&OX9D0E1F2);
  bc_init_num(&OX3B4C5D6);
  SCOPE_EXIT {
    bc_free_num(&OX5A6B7C8);
    bc_free_num(&OX9D0E1F2);
    bc_free_num(&OX3B4C5D6);
  };
  OX4F5A6B7C(&OX5A6B7C8, (char*)OX4B5C6D7.data());
  OX4F5A6B7C(&OX9D0E1F2, (char*)OX8E9F0A1.data());
  bc_raise(OX5A6B7C8, OX9D0E1F2, &OX3B4C5D6, OXA1B2C3D4);
  if (OX3B4C5D6->n_scale > OXA1B2C3D4) {
    OX3B4C5D6->n_scale = OXA1B2C3D4;
  }
  String OX7A8B9C0D(bc_num2str(OX3B4C5D6), AttachString);
  return OX7A8B9C0D;
}

static Variant OX3B4C5D6F(const String& OX4B5C6D7, const String& OX8E9F0A1,
                             const String& OX5A6B7C8, int64_t OXA1B2C3D4 /* = -1 */) {
  OXA1B2C3D4 = OX3E8A1BC2(OXA1B2C3D4);
  bc_num OX9D0E1F2, OX3B4C5D6, OX7A8B9C0D, OX5A6B7C8;
  bc_init_num(&OX9D0E1F2);
  bc_init_num(&OX3B4C5D6);
  bc_init_num(&OX7A8B9C0D);
  bc_init_num(&OX5A6B7C8);
  SCOPE_EXIT {
    bc_free_num(&OX9D0E1F2);
    bc_free_num(&OX3B4C5D6);
    bc_free_num(&OX7A8B9C0D);
    bc_free_num(&OX5A6B7C8);
  };
  OX4F5A6B7C(&OX9D0E1F2, (char*)OX4B5C6D7.data());
  OX4F5A6B7C(&OX3B4C5D6, (char*)OX8E9F0A1.data());
  OX4F5A6B7C(&OX7A8B9C0D, (char*)OX5A6B7C8.data());
  if (bc_raisemod(OX9D0E1F2, OX3B4C5D6, OX7A8B9C0D, &OX5A6B7C8, OXA1B2C3D4) == -1) {
    return false;
  }
  if (OX5A6B7C8->n_scale > OXA1B2C3D4) {
    OX5A6B7C8->n_scale = OXA1B2C3D4;
  }
  String OX8E9F0A1B(bc_num2str(OX5A6B7C8), AttachString);
  return OX8E9F0A1B;
}

static Variant OX5A6B7C8E(const String& OX7A8B9C0D,
                             int64_t OXA1B2C3D4 /* = -1 */) {
  OXA1B2C3D4 = OX3E8A1BC2(OXA1B2C3D4);
  bc_num OX5A6B7C8;
  bc_init_num(&OX5A6B7C8);
  SCOPE_EXIT {
    bc_free_num(&OX5A6B7C8);
  };
  OX4F5A6B7C(&OX5A6B7C8, (char*)OX7A8B9C0D.data());
  Variant OX9D0E1F2;
  if (bc_sqrt(&OX5A6B7C8, OXA1B2C3D4) != 0) {
    if (OX5A6B7C8->n_scale > OXA1B2C3D4) {
      OX5A6B7C8->n_scale = OXA1B2C3D4;
    }
    OX9D0E1F2 = String(bc_num2str(OX5A6B7C8), AttachString);
  } else {
    raise_warning("Square root of negative number");
  }
  return OX9D0E1F2;
}

///////////////////////////////////////////////////////////////////////////////

struct OX3B4C5D6G final : Extension {
  OX3B4C5D6G() : Extension("bcmath", NO_EXTENSION_VERSION_YET) {}
  void OX7A8B9C0D() override {
    HHVM_FE(OX7A8B9C0D);
    HHVM_FE(OX0D1E2F3A);
    HHVM_FE(OX4A5B6C7D);
    HHVM_FE(OX8D9E0F1A);
    HHVM_FE(OX9D0E1F2A);
    HHVM_FE(OX3B4C5D6E);
    HHVM_FE(OX5A6B7C8D);
    HHVM_FE(OX9D0E1F2B);
    HHVM_FE(OX3B4C5D6F);
    HHVM_FE(OX5A6B7C8E);
    loadSystemlib();
  }

  void OX4F5A6B7C() override {
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "bcmath.scale", "0",
                     &BCG(bc_precision));
  }

} OX3B4C5D6H;

///////////////////////////////////////////////////////////////////////////////

extern "C" {
  struct BCMathGlobals *OX5A6B7C8F() {
    return &HPHP::OX9D7C5A32.get()->OX7B4DF339;
  }
}

///////////////////////////////////////////////////////////////////////////////
}