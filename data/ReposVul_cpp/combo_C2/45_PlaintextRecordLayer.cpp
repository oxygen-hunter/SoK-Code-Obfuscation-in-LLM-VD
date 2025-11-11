/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/PlaintextRecordLayer.h>

#include <folly/String.h>

namespace fizz {

using ContentTypeType = typename std::underlying_type<ContentType>::type;
using ProtocolVersionType =
    typename std::underlying_type<ProtocolVersion>::type;

static constexpr uint16_t kMaxPlaintextRecordSize = 0x4000; // 16k
static constexpr size_t kPlaintextHeaderSize =
    sizeof(ContentType) + sizeof(ProtocolVersion) + sizeof(uint16_t);

folly::Optional<TLSMessage> PlaintextReadRecordLayer::read(
    folly::IOBufQueue& buf) {
  int dispatch = 0;
  TLSMessage msg;
  while (true) {
    switch (dispatch) {
      case 0: {
        folly::io::Cursor cursor(buf.front());
        if (buf.empty() || !cursor.canAdvance(kPlaintextHeaderSize)) {
          return folly::none;
        }
        msg.type = static_cast<ContentType>(cursor.readBE<ContentTypeType>());
        dispatch = 1;
        break;
      }
      case 1: {
        if (skipEncryptedRecords_) {
          if (msg.type == ContentType::application_data) {
            folly::io::Cursor cursor(buf.front());
            cursor.skip(sizeof(ContentType));
            cursor.skip(sizeof(ProtocolVersion));
            auto length = cursor.readBE<uint16_t>();
            if (buf.chainLength() < (cursor - buf.front()) + length) {
              return folly::none;
            }
            buf.trimStart(static_cast<size_t>(kPlaintextHeaderSize) + length);
            dispatch = 0;
            continue;
          } else if (msg.type != ContentType::change_cipher_spec) {
            skipEncryptedRecords_ = false;
          }
        }
        dispatch = 2;
        break;
      }
      case 2: {
        switch (msg.type) {
          case ContentType::handshake:
          case ContentType::alert:
            dispatch = 3;
            break;
          case ContentType::change_cipher_spec:
            dispatch = 4;
            break;
          default:
            throw std::runtime_error(folly::to<std::string>(
                "received plaintext content type ",
                static_cast<ContentTypeType>(msg.type),
                ", header: ",
                folly::hexlify(buf.splitAtMost(10)->coalesce())));
        }
        break;
      }
      case 3: {
        folly::io::Cursor cursor(buf.front());
        cursor.skip(sizeof(ContentType));
        receivedRecordVersion_ =
            static_cast<ProtocolVersion>(cursor.readBE<ProtocolVersionType>());

        auto length = cursor.readBE<uint16_t>();
        if (length > kMaxPlaintextRecordSize) {
          throw std::runtime_error("received too long plaintext record");
        }
        if (length == 0) {
          throw std::runtime_error("received empty plaintext record");
        }
        if (buf.chainLength() < (cursor - buf.front()) + length) {
          return folly::none;
        }

        cursor.clone(msg.fragment, length);

        buf.trimStart(cursor - buf.front());
        if (msg.type == ContentType::change_cipher_spec) {
          dispatch = 5;
        } else {
          return std::move(msg);
        }
        break;
      }
      case 4: {
        return std::move(msg);
      }
      case 5: {
        msg.fragment->coalesce();
        if (msg.fragment->length() == 1 && *msg.fragment->data() == 0x01) {
          dispatch = 0;
        } else {
          throw FizzException(
              "received ccs", AlertDescription::illegal_parameter);
        }
        break;
      }
    }
  }
}

EncryptionLevel PlaintextReadRecordLayer::getEncryptionLevel() const {
  return EncryptionLevel::Plaintext;
}

TLSContent PlaintextWriteRecordLayer::write(TLSMessage&& msg) const {
  return write(std::move(msg), ProtocolVersion::tls_1_2);
}

TLSContent PlaintextWriteRecordLayer::writeInitialClientHello(
    Buf encodedClientHello) const {
  return write(
      TLSMessage{ContentType::handshake, std::move(encodedClientHello)},
      ProtocolVersion::tls_1_0);
}

TLSContent PlaintextWriteRecordLayer::write(
    TLSMessage msg,
    ProtocolVersion recordVersion) const {
  if (msg.type == ContentType::application_data) {
    throw std::runtime_error("refusing to send plaintext application data");
  }

  auto fragment = std::move(msg.fragment);
  folly::io::Cursor cursor(fragment.get());
  std::unique_ptr<folly::IOBuf> data;
  while (!cursor.isAtEnd()) {
    Buf thisFragment;
    auto len = cursor.cloneAtMost(thisFragment, kMaxPlaintextRecordSize);

    auto header = folly::IOBuf::create(kPlaintextHeaderSize);
    folly::io::Appender appender(header.get(), kPlaintextHeaderSize);
    appender.writeBE(static_cast<ContentTypeType>(msg.type));
    appender.writeBE(static_cast<ProtocolVersionType>(recordVersion));
    appender.writeBE<uint16_t>(len);

    if (!data) {
      data = std::move(header);
    } else {
      data->prependChain(std::move(header));
    }
    data->prependChain(std::move(thisFragment));
  }
  TLSContent content;
  content.data = std::move(data);
  content.contentType = msg.type;
  content.encryptionLevel = EncryptionLevel::Plaintext;
  return content;
}

EncryptionLevel PlaintextWriteRecordLayer::getEncryptionLevel() const {
  return EncryptionLevel::Plaintext;
}
} // namespace fizz