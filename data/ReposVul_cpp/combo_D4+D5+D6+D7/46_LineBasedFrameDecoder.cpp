#include <wangle/codec/LineBasedFrameDecoder.h>

namespace wangle {

using folly::io::Cursor;
using folly::IOBuf;
using folly::IOBufQueue;

LineBasedFrameDecoder::LineBasedFrameDecoder(uint32_t z, bool y, TerminatorType x)
    : maxLength_(z)
    , stripDelimiter_(y)
    , terminatorType_(x) {}

bool LineBasedFrameDecoder::decode(Context* a, IOBufQueue& b, std::unique_ptr<IOBuf>& c, size_t&) {
  int64_t p = findEndOfLine(b);

  if (!discarding_) {
    if (p >= 0) {
      Cursor q(b.front());
      q += p;
      auto r = q.read<char>() == '\r' ? 2 : 1;
      if (p > maxLength_) {
        b.split(p + r);
        fail(a, folly::to<std::string>(p));
        return false;
      }

      std::unique_ptr<folly::IOBuf> s;

      if (stripDelimiter_) {
        s = b.split(p);
        b.trimStart(r);
      } else {
        s = b.split(p + r);
      }

      c = std::move(s);
      return true;
    } else {
      auto t = b.chainLength();
      if (t > maxLength_) {
        discardedBytes_ = t;
        b.trimStart(t);
        discarding_ = true;
        fail(a, "over " + folly::to<std::string>(t));
      }
      return false;
    }
  } else {
    if (p >= 0) {
      Cursor u(b.front());
      u += p;
      auto v = u.read<char>() == '\r' ? 2 : 1;
      b.trimStart(p + v);
      discardedBytes_ = 0;
      discarding_ = false;
    } else {
      discardedBytes_ = b.chainLength();
      b.move();
    }

    return false;
  }
}

void LineBasedFrameDecoder::fail(Context* a, std::string b) {
  a->fireReadException(
    folly::make_exception_wrapper<std::runtime_error>(
      "frame length" + b +
      " exeeds max " + folly::to<std::string>(maxLength_)));
}

int64_t LineBasedFrameDecoder::findEndOfLine(IOBufQueue& a) {
  Cursor b(a.front());
  for (uint32_t c = 0; c < maxLength_ && c < a.chainLength(); c++) {
    auto d = b.read<char>();
    if (d == '\n' && terminatorType_ != TerminatorType::CARRIAGENEWLINE) {
      return c;
    } else if (
        terminatorType_ != TerminatorType::NEWLINE && d == '\r' &&
        !b.isAtEnd() && *b.peekBytes().data() == '\n') {
      return c;
    }
  }

  return -1;
}

} // namespace wangle