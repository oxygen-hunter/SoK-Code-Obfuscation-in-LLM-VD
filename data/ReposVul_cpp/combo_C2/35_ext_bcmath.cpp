#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/bcmath/bcmath.h"
#include "hphp/runtime/base/ini-setting.h"
#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct bcmath_data {
  bcmath_data() {
    data._zero_ = _bc_new_num_ex (1,0,1);
    data._one_  = _bc_new_num_ex (1,0,1);
    data._one_->n_value[0] = 1;
    data._two_  = _bc_new_num_ex (1,0,1);
    data._two_->n_value[0] = 2;
    data.bc_precision = 0;
  }
  BCMathGlobals data;
};
static IMPLEMENT_THREAD_LOCAL(bcmath_data, s_globals);

///////////////////////////////////////////////////////////////////////////////

static int64_t adjust_scale(int64_t scale) {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (scale < 0) {
          dispatcher = 1;
          break;
        }
        dispatcher = 3;
        break;
      case 1:
        scale = BCG(bc_precision);
        dispatcher = 2;
        break;
      case 2:
        if (scale < 0) scale = 0;
        dispatcher = 3;
        break;
      case 3:
        if ((uint64_t)scale > StringData::MaxSize) return StringData::MaxSize;
        return scale;
    }
  }
}

static void php_str2num(bc_num *num, const char *str) {
  const char *p;
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!(p = strchr(str, '.'))) {
          dispatcher = 1;
          break;
        }
        dispatcher = 2;
        break;
      case 1:
        bc_str2num(num, (char*)str, 0);
        return;
      case 2:
        bc_str2num(num, (char*)str, strlen(p + 1));
        return;
    }
  }
}

static bool HHVM_FUNCTION(bcscale, int64_t scale) {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        BCG(bc_precision) = scale < 0 ? 0 : scale;
        return true;
    }
  }
}

static String HHVM_FUNCTION(bcadd, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  int dispatcher = 0;
  bc_num first, second, result;
  while (true) {
    switch (dispatcher) {
      case 0:
        scale = adjust_scale(scale);
        bc_init_num(&first);
        bc_init_num(&second);
        bc_init_num(&result);
        php_str2num(&first, (char*)left.data());
        php_str2num(&second, (char*)right.data());
        bc_add(first, second, &result, scale);
        dispatcher = 1;
        break;
      case 1:
        if (result->n_scale > scale) {
          result->n_scale = scale;
        }
        String ret(bc_num2str(result), AttachString);
        bc_free_num(&first);
        bc_free_num(&second);
        bc_free_num(&result);
        return ret;
    }
  }
}

static String HHVM_FUNCTION(bcsub, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  int dispatcher = 0;
  bc_num first, second, result;
  while (true) {
    switch (dispatcher) {
      case 0:
        scale = adjust_scale(scale);
        bc_init_num(&first);
        bc_init_num(&second);
        bc_init_num(&result);
        php_str2num(&first, (char*)left.data());
        php_str2num(&second, (char*)right.data());
        bc_sub(first, second, &result, scale);
        dispatcher = 1;
        break;
      case 1:
        if (result->n_scale > scale) {
          result->n_scale = scale;
        }
        String ret(bc_num2str(result), AttachString);
        bc_free_num(&first);
        bc_free_num(&second);
        bc_free_num(&result);
        return ret;
    }
  }
}

static int64_t HHVM_FUNCTION(bccomp, const String& left, const String& right,
                             int64_t scale /* = -1 */) {
  int dispatcher = 0;
  bc_num first, second;
  while (true) {
    switch (dispatcher) {
      case 0:
        scale = adjust_scale(scale);
        bc_init_num(&first);
        bc_init_num(&second);
        bc_str2num(&first, (char*)left.data(), scale);
        bc_str2num(&second, (char*)right.data(), scale);
        int64_t ret = bc_compare(first, second);
        bc_free_num(&first);
        bc_free_num(&second);
        return ret;
    }
  }
}

static String HHVM_FUNCTION(bcmul, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  int dispatcher = 0;
  bc_num first, second, result;
  while (true) {
    switch (dispatcher) {
      case 0:
        scale = adjust_scale(scale);
        bc_init_num(&first);
        bc_init_num(&second);
        bc_init_num(&result);
        php_str2num(&first, (char*)left.data());
        php_str2num(&second, (char*)right.data());
        bc_multiply(first, second, &result, scale);
        dispatcher = 1;
        break;
      case 1:
        if (result->n_scale > scale) {
          result->n_scale = scale;
        }
        String ret(bc_num2str(result), AttachString);
        bc_free_num(&first);
        bc_free_num(&second);
        bc_free_num(&result);
        return ret;
    }
  }
}

static Variant HHVM_FUNCTION(bcdiv, const String& left, const String& right,
               int64_t scale /* = -1 */) {
  int dispatcher = 0;
  bc_num first, second, result;
  SCOPE_EXIT {
    bc_free_num(&first);
    bc_free_num(&second);
    bc_free_num(&result);
  };
  while (true) {
    switch (dispatcher) {
      case 0:
        scale = adjust_scale(scale);
        bc_init_num(&first);
        bc_init_num(&second);
        bc_init_num(&result);
        php_str2num(&first, (char*)left.data());
        php_str2num(&second, (char*)right.data());
        dispatcher = 1;
        break;
      case 1:
        if (bc_divide(first, second, &result, scale) == -1) {
          raise_warning("Division by zero");
          return init_null();
        }
        dispatcher = 2;
        break;
      case 2:
        String ret(bc_num2str(result), AttachString);
        return ret;
    }
  }
}

static Variant HHVM_FUNCTION(bcmod, const String& left, const String& right) {
  int dispatcher = 0;
  bc_num first, second, result;
  SCOPE_EXIT {
    bc_free_num(&first);
    bc_free_num(&second);
    bc_free_num(&result);
  };
  while (true) {
    switch (dispatcher) {
      case 0:
        bc_init_num(&first);
        bc_init_num(&second);
        bc_init_num(&result);
        php_str2num(&first, (char*)left.data());
        php_str2num(&second, (char*)right.data());
        dispatcher = 1;
        break;
      case 1:
        if (bc_modulo(first, second, &result, 0) == -1) {
          raise_warning("Division by zero");
          return init_null();
        }
        dispatcher = 2;
        break;
      case 2:
        String ret(bc_num2str(result), AttachString);
        return ret;
    }
  }
}

static String HHVM_FUNCTION(bcpow, const String& left, const String& right,
                           int64_t scale /* = -1 */) {
  int dispatcher = 0;
  bc_num first, second, result;
  SCOPE_EXIT {
    bc_free_num(&first);
    bc_free_num(&second);
    bc_free_num(&result);
  };
  while (true) {
    switch (dispatcher) {
      case 0:
        scale = adjust_scale(scale);
        bc_init_num(&first);
        bc_init_num(&second);
        bc_init_num(&result);
        php_str2num(&first, (char*)left.data());
        php_str2num(&second, (char*)right.data());
        bc_raise(first, second, &result, scale);
        dispatcher = 1;
        break;
      case 1:
        if (result->n_scale > scale) {
          result->n_scale = scale;
        }
        String ret(bc_num2str(result), AttachString);
        return ret;
    }
  }
}

static Variant HHVM_FUNCTION(bcpowmod, const String& left, const String& right,
                             const String& modulus, int64_t scale /* = -1 */) {
  int dispatcher = 0;
  bc_num first, second, mod, result;
  SCOPE_EXIT {
    bc_free_num(&first);
    bc_free_num(&second);
    bc_free_num(&mod);
    bc_free_num(&result);
  };
  while (true) {
    switch (dispatcher) {
      case 0:
        scale = adjust_scale(scale);
        bc_init_num(&first);
        bc_init_num(&second);
        bc_init_num(&mod);
        bc_init_num(&result);
        php_str2num(&first, (char*)left.data());
        php_str2num(&second, (char*)right.data());
        php_str2num(&mod, (char*)modulus.data());
        dispatcher = 1;
        break;
      case 1:
        if (bc_raisemod(first, second, mod, &result, scale) == -1) {
          return false;
        }
        dispatcher = 2;
        break;
      case 2:
        if (result->n_scale > scale) {
          result->n_scale = scale;
        }
        String ret(bc_num2str(result), AttachString);
        return ret;
    }
  }
}

static Variant HHVM_FUNCTION(bcsqrt, const String& operand,
                             int64_t scale /* = -1 */) {
  int dispatcher = 0;
  bc_num result;
  SCOPE_EXIT {
    bc_free_num(&result);
  };
  while (true) {
    switch (dispatcher) {
      case 0:
        scale = adjust_scale(scale);
        bc_init_num(&result);
        php_str2num(&result, (char*)operand.data());
        dispatcher = 1;
        break;
      case 1:
        Variant ret;
        if (bc_sqrt(&result, scale) != 0) {
          if (result->n_scale > scale) {
            result->n_scale = scale;
          }
          ret = String(bc_num2str(result), AttachString);
        } else {
          raise_warning("Square root of negative number");
        }
        return ret;
    }
  }
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