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
  return decodeRecursive(ctx, buf, result, 0);
}

bool LineBasedFrameDecoder::decodeRecursive(Context* ctx,
                                            IOBufQueue& buf,
                                            std::unique_ptr<IOBuf>& result,
                                            int64_t eol) {
  eol = findEndOfLine(buf);

  switch (!discarding_) {
    case true: {
      switch (eol >= 0) {
        case true: {
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
        }
        default: {
          auto len = buf.chainLength();
          if (len > maxLength_) {
            discardedBytes_ = len;
            buf.trimStart(len);
            discarding_ = true;
            fail(ctx, "over " + folly::to<std::string>(len));
          }
          return false;
        }
      }
    }
    default: {
      switch (eol >= 0) {
        case true: {
          Cursor c(buf.front());
          c += eol;
          auto delimLength = c.read<char>() == '\r' ? 2 : 1;
          buf.trimStart(eol + delimLength);
          discardedBytes_ = 0;
          discarding_ = false;
        }
        default: {
          discardedBytes_ = buf.chainLength();
          buf.move();
        }
      }
      return false;
    }
  }
}

void LineBasedFrameDecoder::fail(Context* ctx, std::string len) {
  ctx->fireReadException(
    folly::make_exception_wrapper<std::runtime_error>(
      "frame length" + len +
      " exeeds max " + folly::to<std::string>(maxLength_)));
}

int64_t LineBasedFrameDecoder::findEndOfLine(IOBufQueue& buf) {
  return findEndOfLineRecursive(buf, 0);
}

int64_t LineBasedFrameDecoder::findEndOfLineRecursive(IOBufQueue& buf, uint32_t i) {
  if (i >= maxLength_ || i >= buf.chainLength()) {
    return -1;
  }
  Cursor c(buf.front());
  auto b = c.read<char>();
  if (b == '\n' && terminatorType_ != TerminatorType::CARRIAGENEWLINE) {
    return i;
  } else if (
      terminatorType_ != TerminatorType::NEWLINE && b == '\r' &&
      !c.isAtEnd() && *c.peekBytes().data() == '\n') {
    return i;
  }
  return findEndOfLineRecursive(buf, i + 1);
}

} // namespace wangle