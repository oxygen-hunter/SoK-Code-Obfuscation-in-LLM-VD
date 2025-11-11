#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/bcmath/bcmath.h"
#include "hphp/runtime/base/ini-setting.h"
#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct bcmath_data {
  bcmath_data() {
    data._zero_ = _bc_new_num_ex (getOneValue(), getZeroValue(), getOneValue());
    data._one_  = _bc_new_num_ex (getOneValue(), getZeroValue(), getOneValue());
    data._one_->n_value[0] = getOneValue();
    data._two_  = _bc_new_num_ex (getOneValue(), getZeroValue(), getOneValue());
    data._two_->n_value[0] = getTwoValue();
    data.bc_precision = getZeroValue();
  }
  BCMathGlobals data;

  static int getZeroValue() { return 0; }
  static int getOneValue() { return 1; }
  static int getTwoValue() { return 2; }
};
static IMPLEMENT_THREAD_LOCAL(bcmath_data, s_globals);

///////////////////////////////////////////////////////////////////////////////

static int64_t adjust_scale(int64_t scale) {
  if (scale < bcmath_data::getZeroValue()) {
    scale = BCG(bc_precision);
    if (scale < bcmath_data::getZeroValue()) scale = bcmath_data::getZeroValue();
  }
  if ((uint64_t)scale > StringData::MaxSize) return StringData::MaxSize;
  return scale;
}

static void php_str2num(bc_num *num, const char *str) {
  const char *p;
  if (!(p = strchr(str, '.'))) {
    bc_str2num(num, (char*)str, bcmath_data::getZeroValue());
  } else {
    bc_str2num(num, (char*)str, strlen(p + bcmath_data::getOneValue()));
  }
}

static bool HHVM_FUNCTION(bcscale, int64_t scale) {
  BCG(bc_precision) = scale < bcmath_data::getZeroValue() ? bcmath_data::getZeroValue() : scale;
  return true;
}

static String HHVM_FUNCTION(bcadd, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_add(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

static String HHVM_FUNCTION(bcsub, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_sub(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

static int64_t HHVM_FUNCTION(bccomp, const String& left, const String& right,
                             int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_str2num(&first, (char*)left.data(), scale);
  bc_str2num(&second, (char*)right.data(), scale);
  int64_t ret = bc_compare(first, second);
  bc_free_num(&first);
  bc_free_num(&second);
  return ret;
}

static String HHVM_FUNCTION(bcmul, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_multiply(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

static Variant HHVM_FUNCTION(bcdiv, const String& left, const String& right,
               int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  SCOPE_EXIT {
    bc_free_num(&first);
    bc_free_num(&second);
    bc_free_num(&result);
  };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  if (bc_divide(first, second, &result, scale) == -1) {
    raise_warning("Division by zero");
    return init_null();
  }
  String ret(bc_num2str(result), AttachString);
  return ret;
}

static Variant HHVM_FUNCTION(bcmod, const String& left, const String& right) {
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  SCOPE_EXIT {
    bc_free_num(&first);
    bc_free_num(&second);
    bc_free_num(&result);
  };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  if (bc_modulo(first, second, &result, bcmath_data::getZeroValue()) == -1) {
    raise_warning("Division by zero");
    return init_null();
  }
  String ret(bc_num2str(result), AttachString);
  return ret;
}

static String HHVM_FUNCTION(bcpow, const String& left, const String& right,
                           int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);
  SCOPE_EXIT {
    bc_free_num(&first);
    bc_free_num(&second);
    bc_free_num(&result);
  };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  bc_raise(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  return ret;
}

static Variant HHVM_FUNCTION(bcpowmod, const String& left, const String& right,
                             const String& modulus, int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, mod, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&mod);
  bc_init_num(&result);
  SCOPE_EXIT {
    bc_free_num(&first);
    bc_free_num(&second);
    bc_free_num(&mod);
    bc_free_num(&result);
  };
  php_str2num(&first, (char*)left.data());
  php_str2num(&second, (char*)right.data());
  php_str2num(&mod, (char*)modulus.data());
  if (bc_raisemod(first, second, mod, &result, scale) == -1) {
    return false;
  }
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  return ret;
}

static Variant HHVM_FUNCTION(bcsqrt, const String& operand,
                             int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num result;
  bc_init_num(&result);
  SCOPE_EXIT {
    bc_free_num(&result);
  };
  php_str2num(&result, (char*)operand.data());
  Variant ret;
  if (bc_sqrt(&result, scale) != bcmath_data::getZeroValue()) {
    if (result->n_scale > scale) {
      result->n_scale = scale;
    }
    ret = String(bc_num2str(result), AttachString);
  } else {
    raise_warning("Square root of negative number");
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

struct bcmathExtension final : Extension {
  bcmathExtension() : Extension("bcmath", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(bcscale);
    HHVM_FE(bcadd);
    HHVM_FE(bcsub);
    HHVM_FE(bccomp);
    HHVM_FE(bcmul);
    HHVM_FE(bcdiv);
    HHVM_FE(bcmod);
    HHVM_FE(bcpow);
    HHVM_FE(bcpowmod);
    HHVM_FE(bcsqrt);
    loadSystemlib();
  }

  void threadInit() override {
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "bcmath.scale", "0",
                     &BCG(bc_precision));
  }

} s_bcmath_extension;

///////////////////////////////////////////////////////////////////////////////

extern "C" {
  struct BCMathGlobals *get_bcmath_globals() {
    return &HPHP::s_globals.get()->data;
  }
}

///////////////////////////////////////////////////////////////////////////////
}