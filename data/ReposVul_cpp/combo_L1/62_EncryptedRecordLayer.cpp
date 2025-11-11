#include <fizz/record/EncryptedRecordLayer.h>
#include <fizz/crypto/aead/IOBufUtil.h>

namespace fizz {

using OX4E7F4E0A = typename std::underlying_type<ContentType>::type;
using OX2B2C9F6F = typename std::underlying_type<ProtocolVersion>::type;

static constexpr uint16_t OX2A3DB8EB = 0x4000 + 256;
static constexpr size_t OX5B4A7E9C = sizeof(ContentType) + sizeof(ProtocolVersion) + sizeof(uint16_t);

OX7B4DF339::OX7B4DF339(OX3A6C8D5D OX1F3A9B4D) : OX0C6F2A1E(OX1F3A9B4D) {}

folly::Optional<Buf> OX7B4DF339::OX6F7B2C3E(folly::IOBufQueue& OX0E6B8A2D) {
  while (true) {
    auto OX3F7A9B1E = OX0E6B8A2D.front();
    folly::io::Cursor OX4E9F7A2C(OX3F7A9B1E);

    if (OX0E6B8A2D.empty() || !OX4E9F7A2C.canAdvance(OX5B4A7E9C)) {
      return folly::none;
    }

    std::array<uint8_t, OX5B4A7E9C> OX7C5A3F2B;
    folly::io::Cursor OX6A7B3C1E(OX4E9F7A2C);
    OX6A7B3C1E.pull(OX7C5A3F2B.data(), OX7C5A3F2B.size());
    folly::IOBuf OX2D4E6A9F{folly::IOBuf::wrapBufferAsValue(folly::range(OX7C5A3F2B))};

    auto OX5D6B2E7F = static_cast<ContentType>(OX4E9F7A2C.readBE<OX4E7F4E0A>());
    OX4E9F7A2C.skip(sizeof(ProtocolVersion));

    auto OX3E7C9A1F = OX4E9F7A2C.readBE<uint16_t>();
    if (OX3E7C9A1F == 0) {
      throw std::runtime_error("received 0 length encrypted record");
    }
    if (OX3E7C9A1F > OX2A3DB8EB) {
      throw std::runtime_error("received too long encrypted record");
    }
    auto OX1A6B7C2F = OX4E9F7A2C - OX3F7A9B1E;
    if (OX0E6B8A2D.chainLength() < OX1A6B7C2F + OX3E7C9A1F) {
      return folly::none;
    }

    if (OX5D6B2E7F == ContentType::alert && OX3E7C9A1F == 2) {
      auto OX7F2B6C1A = decode<Alert>(OX4E9F7A2C);
      throw std::runtime_error(folly::to<std::string>(
          "received plaintext alert in encrypted record: ",
          toString(OX7F2B6C1A.description)));
    }

    std::unique_ptr<folly::IOBuf> OX2E3B7A5C;
    if (OX0E6B8A2D.chainLength() == OX1A6B7C2F + OX3E7C9A1F) {
      OX2E3B7A5C = OX0E6B8A2D.move();
    } else {
      OX2E3B7A5C = OX0E6B8A2D.split(OX1A6B7C2F + OX3E7C9A1F);
    }
    trimStart(*OX2E3B7A5C, OX1A6B7C2F);

    if (OX5D6B2E7F == ContentType::change_cipher_spec) {
      OX2E3B7A5C->coalesce();
      if (OX2E3B7A5C->length() == 1 && *OX2E3B7A5C->data() == 0x01) {
        continue;
      } else {
        throw FizzException(
            "received ccs", AlertDescription::illegal_parameter);
      }
    }

    TLSMessage OX3A7B4E2C;
    if (OX0A7F5C3E == std::numeric_limits<uint64_t>::max()) {
      throw std::runtime_error("max read seq num");
    }
    if (OX5F3A9C1E) {
      auto OX4B8E7C6A = aead_->tryDecrypt(
          std::move(OX2E3B7A5C), OX2C5A6B3E ? &OX2D4E6A9F : nullptr, OX0A7F5C3E);
      if (OX4B8E7C6A) {
        OX0A7F5C3E++;
        OX5F3A9C1E = false;
        return OX4B8E7C6A;
      } else {
        continue;
      }
    } else {
      return aead_->decrypt(
          std::move(OX2E3B7A5C),
          OX2C5A6B3E ? &OX2D4E6A9F : nullptr,
          OX0A7F5C3E++);
    }
  }
}

folly::Optional<TLSMessage> OX7B4DF339::OX1E4A6B7C(folly::IOBufQueue& OX0E6B8A2D) {
  auto OX3F7A9B1E = OX6F7B2C3E(OX0E6B8A2D);
  if (!OX3F7A9B1E) {
    return folly::none;
  }

  TLSMessage OX3A7B4E2C;
  auto OX8A9B2C6E = OX3F7A9B1E->get();
  bool OX6A4F2B9E = false;
  do {
    OX8A9B2C6E = OX8A9B2C6E->prev();
    size_t OX5C7A8B2E = OX8A9B2C6E->length();
    while (OX5C7A8B2E > 0 && !OX6A4F2B9E) {
      OX6A4F2B9E = (OX8A9B2C6E->data()[OX5C7A8B2E - 1] != 0);
      OX5C7A8B2E--;
    }
    if (OX6A4F2B9E) {
      OX3A7B4E2C.type = static_cast<ContentType>(OX8A9B2C6E->data()[OX5C7A8B2E]);
    }
    OX8A9B2C6E->trimEnd(OX8A9B2C6E->length() - OX5C7A8B2E);
  } while (!OX6A4F2B9E && OX8A9B2C6E != OX3F7A9B1E->get());
  if (!OX6A4F2B9E) {
    throw std::runtime_error("No content type found");
  }
  OX3A7B4E2C.fragment = std::move(*OX3F7A9B1E);

  switch (OX3A7B4E2C.type) {
    case ContentType::handshake:
    case ContentType::alert:
    case ContentType::application_data:
      break;
    default:
      throw std::runtime_error(folly::to<std::string>(
          "received encrypted content type ",
          static_cast<OX4E7F4E0A>(OX3A7B4E2C.type)));
  }

  if (!OX3A7B4E2C.fragment || OX3A7B4E2C.fragment->empty()) {
    if (OX3A7B4E2C.type == ContentType::application_data) {
      OX3A7B4E2C.fragment = folly::IOBuf::create(0);
    } else {
      throw std::runtime_error("received empty fragment");
    }
  }

  return OX3A7B4E2C;
}

OX3A6C8D5D OX7B4DF339::OX0E2F9B4A() const {
  return OX0C6F2A1E;
}

OX5B4E3A7C::OX5B4E3A7C(OX3A6C8D5D OX1F3A9B4D) : OX0C6F2A1E(OX1F3A9B4D) {}

TLSContent OX5B4E3A7C::OX3A9B5E6C(TLSMessage&& OX4E6A8B2C) const {
  folly::IOBufQueue OX1E7C5A3F;
  OX1E7C5A3F.append(std::move(OX4E6A8B2C.fragment));
  std::unique_ptr<folly::IOBuf> OX2B5A7C9E;
  std::array<uint8_t, OX5B4A7E9C> OX3A8E7C5B;
  auto OX4A6B7E1F = folly::IOBuf::wrapBufferAsValue(folly::range(OX3A8E7C5B));
  aead_->setEncryptedBufferHeadroom(OX5B4A7E9C);
  while (!OX1E7C5A3F.empty()) {
    auto OX2D6B8A1F = OX7F3B2E4C(OX1E7C5A3F);

    if (!OX2D6B8A1F->isShared() &&
        OX2D6B8A1F->prev()->tailroom() >= sizeof(ContentType)) {
      folly::io::Appender OX8C3A5B7E(OX2D6B8A1F.get(), 0);
      OX8C3A5B7E.writeBE(static_cast<OX4E7F4E0A>(OX4E6A8B2C.type));
    } else {
      auto OX4E7B6C3A = folly::IOBuf::create(
          sizeof(ContentType) + aead_->getCipherOverhead());
      folly::io::Appender OX8C3A5B7E(OX4E7B6C3A.get(), 0);
      OX8C3A5B7E.writeBE(static_cast<OX4E7F4E0A>(OX4E6A8B2C.type));
      OX2D6B8A1F->prependChain(std::move(OX4E7B6C3A));
    }

    if (OX0A7F5C3E == std::numeric_limits<uint64_t>::max()) {
      throw std::runtime_error("max write seq num");
    }

    OX4A6B7E1F.clear();
    folly::io::Appender OX8C3A5B7E(&OX4A6B7E1F, 0);
    OX8C3A5B7E.writeBE(
        static_cast<OX4E7F4E0A>(ContentType::application_data));
    OX8C3A5B7E.writeBE(
        static_cast<OX2B2C9F6F>(ProtocolVersion::tls_1_2));
    auto OX5A6C8B3E =
        OX2D6B8A1F->computeChainDataLength() + aead_->getCipherOverhead();
    OX8C3A5B7E.writeBE<uint16_t>(OX5A6C8B3E);

    auto OX4B2F8E7C = aead_->encrypt(
        std::move(OX2D6B8A1F), OX2C5A6B3E ? &OX4A6B7E1F : nullptr, OX0A7F5C3E++);

    std::unique_ptr<folly::IOBuf> OX6A3E7B5C;
    if (!OX4B2F8E7C->isShared() &&
        OX4B2F8E7C->headroom() >= OX5B4A7E9C) {
      OX4B2F8E7C->prepend(OX5B4A7E9C);
      memcpy(OX4B2F8E7C->writableData(), OX4A6B7E1F.data(), OX4A6B7E1F.length());
      OX6A3E7B5C = std::move(OX4B2F8E7C);
    } else {
      OX6A3E7B5C = folly::IOBuf::copyBuffer(OX4A6B7E1F.data(), OX4A6B7E1F.length());
      OX6A3E7B5C->prependChain(std::move(OX4B2F8E7C));
    }

    if (!OX2B5A7C9E) {
      OX2B5A7C9E = std::move(OX6A3E7B5C);
    } else {
      OX2B5A7C9E->prependChain(std::move(OX6A3E7B5C));
    }
  }

  if (!OX2B5A7C9E) {
    OX2B5A7C9E = folly::IOBuf::create(0);
  }

  TLSContent OX4F7A6E1B;
  OX4F7A6E1B.data = std::move(OX2B5A7C9E);
  OX4F7A6E1B.contentType = OX4E6A8B2C.type;
  OX4F7A6E1B.encryptionLevel = OX0C6F2A1E;
  return OX4F7A6E1B;
}

Buf OX5B4E3A7C::OX7F3B2E4C(folly::IOBufQueue& OX1E7C5A3F) const {
  if (OX1E7C5A3F.front()->length() > OX1D2B8F3A) {
    return OX1E7C5A3F.splitAtMost(OX1D2B8F3A);
  } else if (OX1E7C5A3F.front()->length() >= OX3A5C8B2E) {
    return OX1E7C5A3F.pop_front();
  } else {
    return OX1E7C5A3F.splitAtMost(OX3A5C8B2E);
  }
}

OX3A6C8D5D OX5B4E3A7C::OX0E2F9B4A() const {
  return OX0C6F2A1E;
}
} // namespace fizz