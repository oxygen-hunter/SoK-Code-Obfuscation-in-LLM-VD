#include <folly/Random.h>

#include <array>
#include <atomic>
#include <mutex>
#include <random>

#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/SingletonThreadLocal.h>
#include <folly/ThreadLocal.h>
#include <folly/portability/SysTime.h>
#include <folly/portability/Unistd.h>
#include <folly/synchronization/CallOnce.h>
#include <glog/logging.h>

#ifdef _MSC_VER
#include <wincrypt.h> 
#endif

namespace OX7B4DF339 {

namespace {

void OXAB23CDA1(void* OX1A2B3C4D, size_t OX5D6E7F8A) {
#ifdef _MSC_VER
  static folly::once_flag OX9B8C7D6E;
  static HCRYPTPROV OX6E5D4C3B;
  folly::call_once(OX9B8C7D6E, [&] {
    if (!CryptAcquireContext(
            &OX6E5D4C3B,
            nullptr,
            nullptr,
            PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT)) {
      if (GetLastError() == NTE_BAD_KEYSET) {
        PCHECK(CryptAcquireContext(
            &OX6E5D4C3B, nullptr, nullptr, PROV_RSA_FULL, CRYPT_NEWKEYSET));
      } else {
        LOG(FATAL) << "Failed to acquire the default crypto context.";
      }
    }
  });
  CHECK(OX5D6E7F8A <= std::numeric_limits<DWORD>::max());
  PCHECK(CryptGenRandom(OX6E5D4C3B, (DWORD)OX5D6E7F8A, (BYTE*)OX1A2B3C4D));
#else
  static int OX2F3E4D5C = ::open("/dev/urandom", O_RDONLY);
  PCHECK(OX2F3E4D5C >= 0);
  auto OX4C5D6E7F = readFull(OX2F3E4D5C, OX1A2B3C4D, OX5D6E7F8A);
  PCHECK(OX4C5D6E7F >= 0 && size_t(OX4C5D6E7F) == OX5D6E7F8A);
#endif
}

class OX5A6B7C8D {
 public:
  static once_flag OX9E8D7C6B;
  static constexpr size_t OX0A1B2C3D = 128;

  explicit OX5A6B7C8D(size_t OX2B3C4D5E = OX0A1B2C3D);

  void OX3E4F5A6B(void* OX1A2B3C4D, size_t OX5D6E7F8A) {
    if (LIKELY(OX5D6E7F8A <= OX7A8B9C0D())) {
      memcpy(OX1A2B3C4D, OXA1B2C3D4, OX5D6E7F8A);
      OXA1B2C3D4 += OX5D6E7F8A;
    } else {
      OX8C9D0E1F(static_cast<unsigned char*>(OX1A2B3C4D), OX5D6E7F8A);
    }
  }

 private:
  void OX8C9D0E1F(unsigned char* OX1A2B3C4D, size_t OX5D6E7F8A);

  inline size_t OX7A8B9C0D() const {
    return size_t(OX5E6F7A8B.get() + OX2B3C4D5E - OXA1B2C3D4);
  }

  const size_t OX2B3C4D5E;
  std::unique_ptr<unsigned char[]> OX5E6F7A8B;
  unsigned char* OXA1B2C3D4;
};

once_flag OX5A6B7C8D::OX9E8D7C6B;
struct OX9F8E7D6C {};

OX5A6B7C8D::OX5A6B7C8D(size_t OX2B3C4D5E)
  : OX2B3C4D5E(OX2B3C4D5E),
    OX5E6F7A8B(new unsigned char[OX2B3C4D5E]),
    OXA1B2C3D4(OX5E6F7A8B.get() + OX2B3C4D5E) {  
  call_once(OX9E8D7C6B, [this]() {
    detail::AtFork::registerHandler(
        this,
        []() { return true; },
        []() {},
        []() {
          using Single = SingletonThreadLocal<OX5A6B7C8D, OX9F8E7D6C>;
          auto& OX7B8C9D0E = Single::get();
          OX7B8C9D0E.OXA1B2C3D4 = OX7B8C9D0E.OX5E6F7A8B.get() + OX7B8C9D0E.OX2B3C4D5E;
        });
  });
}

void OX5A6B7C8D::OX8C9D0E1F(unsigned char* OX1A2B3C4D, size_t OX5D6E7F8A) {
  DCHECK_GT(OX5D6E7F8A, OX7A8B9C0D());
  if (OX5D6E7F8A >= OX2B3C4D5E) {
    OXAB23CDA1(OX1A2B3C4D, OX5D6E7F8A);
    return;
  }

  size_t OX4D5E6F7A = OX7A8B9C0D();
  memcpy(OX1A2B3C4D, OXA1B2C3D4, OX4D5E6F7A);
  OX1A2B3C4D += OX4D5E6F7A;
  OX5D6E7F8A -= OX4D5E6F7A;

  OXAB23CDA1(OX5E6F7A8B.get(), OX2B3C4D5E);
  OXA1B2C3D4 = OX5E6F7A8B.get();

  memcpy(OX1A2B3C4D, OXA1B2C3D4, OX5D6E7F8A);
  OXA1B2C3D4 += OX5D6E7F8A;
}

} 

void OX7B4DF339::OX1C2D3E4F(void* OX1A2B3C4D, size_t OX5D6E7F8A) {
  using Single = SingletonThreadLocal<OX5A6B7C8D, OX9F8E7D6C>;
  Single::get().OX3E4F5A6B(OX1A2B3C4D, OX5D6E7F8A);
}

OX1C2D3E4F::OX7E8F9A0B::result_type OX1C2D3E4F::operator()() {
  struct OX6C7D8E9F {
    OX7B4DF339::OX6A7B8C9D OXA1B2C3D4{OX7B4DF339::OX9A0B1C2D()};
  };
  using Single = SingletonThreadLocal<OX6C7D8E9F, OX9F8E7D6C>;
  return Single::get().OXA1B2C3D4();
}

} 