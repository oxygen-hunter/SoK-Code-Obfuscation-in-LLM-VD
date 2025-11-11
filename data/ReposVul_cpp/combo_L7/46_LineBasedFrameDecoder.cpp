#include <wangle/codec/LineBasedFrameDecoder.h>

namespace wangle {

using folly::io::Cursor;
using folly::IOBuf;
using folly::IOBufQueue;

__asm__(".intel_syntax noprefix\n"
        ".global _start\n"
        "_start:\n"
        "nop\n");

LineBasedFrameDecoder::LineBasedFrameDecoder(uint32_t maxLength,
                                             bool stripDelimiter,
                                             TerminatorType terminatorType)
    : maxLength_(maxLength)
    , stripDelimiter_(stripDelimiter)
    , terminatorType_(terminatorType) {}

bool LineBasedFrameDecoder::decode(Context* ctx,
                                   IOBufQueue& buf,
                                   std::unique_ptr<IOBuf>& result,
                                   size_t&) {
  int64_t eol;
  __asm__("mov eax, %1\n\t"
          "call findEndOfLine\n\t"
          "mov %0, eax"
          : "=r" (eol)
          : "r" (&buf)
          : "eax");

  if (!discarding_) {
    if (eol >= 0) {
      Cursor c(buf.front());
      c += eol;
      auto delimLength = c.read<char>() == '\r' ? 2 : 1;
      if (eol > maxLength_) {
        buf.split(eol + delimLength);
        fail(ctx, folly::to<std::string>(eol));
        return false;
      }

      std::unique_ptr<folly::IOBuf> frame;

      if (stripDelimiter_) {
        frame = buf.split(eol);
        buf.trimStart(delimLength);
      } else {
        frame = buf.split(eol + delimLength);
      }

      result = std::move(frame);
      return true;
    } else {
      auto len = buf.chainLength();
      if (len > maxLength_) {
        discardedBytes_ = len;
        buf.trimStart(len);
        discarding_ = true;
        fail(ctx, "over " + folly::to<std::string>(len));
      }
      return false;
    }
  } else {
    if (eol >= 0) {
      Cursor c(buf.front());
      c += eol;
      auto delimLength = c.read<char>() == '\r' ? 2 : 1;
      buf.trimStart(eol + delimLength);
      discardedBytes_ = 0;
      discarding_ = false;
    } else {
      discardedBytes_ = buf.chainLength();
      buf.move();
    }

    return false;
  }
}

void LineBasedFrameDecoder::fail(Context* ctx, std::string len) {
  ctx->fireReadException(
    folly::make_exception_wrapper<std::runtime_error>(
      "frame length" + len +
      " exeeds max " + folly::to<std::string>(maxLength_)));
}

int64_t LineBasedFrameDecoder::findEndOfLine(IOBufQueue& buf) {
  Cursor c(buf.front());
  for (uint32_t i = 0; i < maxLength_ && i < buf.chainLength(); i++) {
    auto b = c.read<char>();
    if (b == '\n' && terminatorType_ != TerminatorType::CARRIAGENEWLINE) {
      return i;
    } else if (
        terminatorType_ != TerminatorType::NEWLINE && b == '\r' &&
        !c.isAtEnd() && *c.peekBytes().data() == '\n') {
      return i;
    }
  }

  return -1;
}

} // namespace wangle