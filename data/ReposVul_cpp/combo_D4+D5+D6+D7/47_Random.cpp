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
#ifdef _MSC_VER
  static struct { folly::once_flag flag; HCRYPTPROV cryptoProv; } cryptoStruct;
  folly::call_once(cryptoStruct.flag, [&] {
    if (!CryptAcquireContext(
            &cryptoStruct.cryptoProv,
            nullptr,
            nullptr,
            PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT)) {
      if (GetLastError() == NTE_BAD_KEYSET) {
        PCHECK(CryptAcquireContext(
            &cryptoStruct.cryptoProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_NEWKEYSET));
      } else {
        LOG(FATAL) << "Failed to acquire the default crypto context.";
      }
    }
  });
  CHECK(size <= std::numeric_limits<DWORD>::max());
  PCHECK(CryptGenRandom(cryptoStruct.cryptoProv, (DWORD)size, (BYTE*)data));
#else
  static struct { int fd; } randomFdStruct = { ::open("/dev/urandom", O_RDONLY) };
  PCHECK(randomFdStruct.fd >= 0);
  auto bytesRead = readFull(randomFdStruct.fd, data, size);
  PCHECK(bytesRead >= 0 && size_t(bytesRead) == size);
#endif
}

class BufferedRandomDevice {
 public:
  explicit BufferedRandomDevice(size_t bufferSize = 128);

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
  static once_flag flag;
};

once_flag BufferedRandomDevice::flag;
struct RandomTag {};

BufferedRandomDevice::BufferedRandomDevice(size_t bufferSize)
  : bufferSize_(bufferSize),
    buffer_(new unsigned char[bufferSize]),
    ptr_(buffer_.get() + bufferSize) {
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
  DCHECK_GT(size, remaining());
  if (size >= bufferSize_) {
    readRandomDevice(data, size);
    return;
  }

  size_t copied = remaining();
  memcpy(data, ptr_, copied);
  data += copied;
  size -= copied;

  readRandomDevice(buffer_.get(), bufferSize_);
  ptr_ = buffer_.get();

  memcpy(data, ptr_, size);
  ptr_ += size;
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