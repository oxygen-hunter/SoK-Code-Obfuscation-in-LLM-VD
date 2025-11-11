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

namespace folly {

namespace {

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL
};

class VirtualMachine {
public:
  VirtualMachine() : pc(0) {}

  void execute(std::vector<int> program) {
    while (pc < program.size()) {
      switch (program[pc]) {
        case PUSH:
          stack.push_back(program[++pc]);
          break;
        case POP:
          stack.pop_back();
          break;
        case ADD: {
          int b = stack.back(); stack.pop_back();
          int a = stack.back(); stack.pop_back();
          stack.push_back(a + b);
          break;
        }
        case SUB: {
          int b = stack.back(); stack.pop_back();
          int a = stack.back(); stack.pop_back();
          stack.push_back(a - b);
          break;
        }
        case JMP:
          pc = program[++pc] - 1;
          break;
        case JZ:
          if (stack.back() == 0) {
            pc = program[++pc] - 1;
          } else {
            pc++;
          }
          break;
        case LOAD:
          stack.push_back(memory[program[++pc]]);
          break;
        case STORE:
          memory[program[++pc]] = stack.back();
          stack.pop_back();
          break;
        case CALL:
          executeFunction(program[++pc]);
          break;
        default:
          break;
      }
      pc++;
    }
  }

private:
  std::vector<int> stack;
  std::vector<int> memory = std::vector<int>(256, 0);
  int pc;

  void executeFunction(int func) {
    switch (func) {
      case 1:
        executeSecureRandom();
        break;
      case 2:
        executeThreadLocalPRNG();
        break;
      default:
        break;
    }
  }

  void executeSecureRandom() {
    // Obfuscated logic for secureRandom
    void* data = (void*)stack.back(); stack.pop_back();
    size_t size = stack.back(); stack.pop_back();
    Random::secureRandom(data, size);
  }

  void executeThreadLocalPRNG() {
    // Obfuscated logic for ThreadLocalPRNG
    stack.push_back(ThreadLocalPRNG()());
  }
};

void readRandomDevice(void* data, size_t size) {
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
      if (GetLastError() == NTE_BAD_KEYSET) {
        PCHECK(CryptAcquireContext(
            &cryptoProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_NEWKEYSET));
      } else {
        LOG(FATAL) << "Failed to acquire the default crypto context.";
      }
    }
  });
  CHECK(size <= std::numeric_limits<DWORD>::max());
  PCHECK(CryptGenRandom(cryptoProv, (DWORD)size, (BYTE*)data));
#else
  static int randomFd = ::open("/dev/urandom", O_RDONLY);
  PCHECK(randomFd >= 0);
  auto bytesRead = readFull(randomFd, data, size);
  PCHECK(bytesRead >= 0 && size_t(bytesRead) == size);
#endif
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
    ptr_(buffer_.get() + bufferSize) {
  call_once(flag, [this]() {
    detail::AtFork::registerHandler(
        this,
        []() { return true; },
        []() {},
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