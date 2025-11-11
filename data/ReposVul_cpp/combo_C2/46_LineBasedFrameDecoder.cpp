#include <wangle/codec/LineBasedFrameDecoder.h>

namespace wangle {

using folly::io::Cursor;
using folly::IOBuf;
using folly::IOBufQueue;

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
  int64_t eol = findEndOfLine(buf);
  int state = 0;
  bool continueLoop = true;
  
  while (continueLoop) {
    switch (state) {
      case 0:
        if (!discarding_) {
          state = 1;
        } else {
          state = 5;
        }
        break;
        
      case 1:
        if (eol >= 0) {
          Cursor c(buf.front());
          c += eol;
          auto delimLength = c.read<char>() == '\r' ? 2 : 1;
          if (eol > maxLength_) {
            buf.split(eol + delimLength);
            fail(ctx, folly::to<std::string>(eol));
            continueLoop = false;
          } else {
            state = 2;
          }
        } else {
          state = 3;
        }
        break;
        
      case 2:
        std::unique_ptr<folly::IOBuf> frame;
        if (stripDelimiter_) {
          frame = buf.split(eol);
          buf.trimStart(delimLength);
        } else {
          frame = buf.split(eol + delimLength);
        }
        result = std::move(frame);
        return true;
        
      case 3:
        auto len = buf.chainLength();
        if (len > maxLength_) {
          discardedBytes_ = len;
          buf.trimStart(len);
          discarding_ = true;
          fail(ctx, "over " + folly::to<std::string>(len));
        }
        return false;
        
      case 5:
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
  return false;
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