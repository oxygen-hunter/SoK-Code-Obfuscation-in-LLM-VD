/*
 * Copyright 2011-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#include <wincrypt.h> // @manual
#endif

namespace folly {

namespace {

void readRandomDevice(void* data, size_t size) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0: {
#ifdef _MSC_VER
        static folly::once_flag flag;
        static HCRYPTPROV cryptoProv;
        folly::call_once(flag, [&] {
          if (!CryptAcquireContext(
                  &cryptoProv,
                  nullptr,
                  nullptr,
                  PROV_RSA_FULL,
                  CRYPT_VERIFYCONTEXT)) {
            state = 1;
          } else {
            state = 3;
          }
        });
#else
        static int randomFd = ::open("/dev/urandom", O_RDONLY);
        PCHECK(randomFd >= 0);
        auto bytesRead = readFull(randomFd, data, size);
        PCHECK(bytesRead >= 0 && size_t(bytesRead) == size);
        return;
#endif
        break;
      }
      case 1: {
#ifdef _MSC_VER
        if (GetLastError() == NTE_BAD_KEYSET) {
          state = 2;
        } else {
          state = 4;
        }
#endif
        break;
      }
      case 2: {
#ifdef _MSC_VER
        PCHECK(CryptAcquireContext(
            &cryptoProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_NEWKEYSET));
        state = 3;
#endif
        break;
      }
      case 3: {
#ifdef _MSC_VER
        CHECK(size <= std::numeric_limits<DWORD>::max());
        PCHECK(CryptGenRandom(cryptoProv, (DWORD)size, (BYTE*)data));
        return;
#endif
        break;
      }
      case 4: {
#ifdef _MSC_VER
        LOG(FATAL) << "Failed to acquire the default crypto context.";
#endif
        return;
      }
    }
  }
}

class BufferedRandomDevice {
 public:
  static once_flag flag;
  static constexpr size_t kDefaultBufferSize = 128;

  explicit BufferedRandomDevice(size_t bufferSize = kDefaultBufferSize);

  void get(void* data, size_t size) {
    if (LIKELY(size <= remaining())) {
      memcpy(data, ptr_, size);
      ptr_ += size;
    } else {
      getSlow(static_cast<unsigned char*>(data), size);
    }
  }

 private:
  void getSlow(unsigned char* data, size_t size);

  inline size_t remaining() const {
    return size_t(buffer_.get() + bufferSize_ - ptr_);
  }

  const size_t bufferSize_;
  std::unique_ptr<unsigned char[]> buffer_;
  unsigned char* ptr_;
};

once_flag BufferedRandomDevice::flag;
struct RandomTag {};

BufferedRandomDevice::BufferedRandomDevice(size_t bufferSize)
  : bufferSize_(bufferSize),
    buffer_(new unsigned char[bufferSize]),
    ptr_(buffer_.get() + bufferSize) {  // refill on first use
  call_once(flag, [this]() {
    detail::AtFork::registerHandler(
        this,
        /*prepare*/ []() { return true; },
        /*parent*/ []() {},
        /*child*/
        []() {
          using Single = SingletonThreadLocal<BufferedRandomDevice, RandomTag>;
          auto& t = Single::get();
          t.ptr_ = t.buffer_.get() + t.bufferSize_;
        });
  });
}

void BufferedRandomDevice::getSlow(unsigned char* data, size_t size) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0: {
        DCHECK_GT(size, remaining());
        if (size >= bufferSize_) {
          state = 1;
        } else {
          state = 2;
        }
        break;
      }
      case 1: {
        readRandomDevice(data, size);
        return;
      }
      case 2: {
        size_t copied = remaining();
        memcpy(data, ptr_, copied);
        data += copied;
        size -= copied;
        state = 3;
        break;
      }
      case 3: {
        readRandomDevice(buffer_.get(), bufferSize_);
        ptr_ = buffer_.get();
        memcpy(data, ptr_, size);
        ptr_ += size;
        return;
      }
    }
  }
}

} // namespace

void Random::secureRandom(void* data, size_t size) {
  using Single = SingletonThreadLocal<BufferedRandomDevice, RandomTag>;
  Single::get().get(data, size);
}

ThreadLocalPRNG::result_type ThreadLocalPRNG::operator()() {
  struct Wrapper {
    Random::DefaultGenerator object{Random::create()};
  };
  using Single = SingletonThreadLocal<Wrapper, RandomTag>;
  return Single::get().object();
}
} // namespace folly