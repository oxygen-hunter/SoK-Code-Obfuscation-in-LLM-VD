#include <fizz/record/EncryptedRecordLayer.h>
#include <fizz/crypto/aead/IOBufUtil.h>

namespace fizz {

using OX1A2B3C4D = typename std::underlying_type<ContentType>::type;
using OX2B3C4D5E = typename std::underlying_type<ProtocolVersion>::type;

static constexpr uint16_t OX3B4C5D6E = 0x4000 + 256;
static constexpr size_t OX4C5D6E7F = sizeof(ContentType) + sizeof(ProtocolVersion) + sizeof(uint16_t);

OX5D6E7F8G::OX5D6E7F8G(OX6E7F8G9H OX7F8G9H0I) : OX8G9H0I1J(OX7F8G9H0I) {}

folly::Optional<Buf> OX5D6E7F8G::OX9H0I1J2K(folly::IOBufQueue& OX0I1J2K3L) {
  while (true) {
    auto OX1J2K3L4M = OX0I1J2K3L.front();
    folly::io::Cursor OX2K3L4M5N(OX1J2K3L4M);

    if (OX0I1J2K3L.empty() || !OX2K3L4M5N.canAdvance(OX4C5D6E7F)) {
      return folly::none;
    }

    std::array<uint8_t, OX4C5D6E7F> OX3L4M5N6O;
    folly::io::Cursor OX4M5N6O7P(OX2K3L4M5N);
    OX4M5N6O7P.pull(OX3L4M5N6O.data(), OX3L4M5N6O.size());
    folly::IOBuf OX5N6O7P8Q{folly::IOBuf::wrapBufferAsValue(folly::range(OX3L4M5N6O))};

    auto OX6O7P8Q9R = static_cast<ContentType>(OX2K3L4M5N.readBE<OX1A2B3C4D>());
    OX2K3L4M5N.skip(sizeof(ProtocolVersion));

    auto OX7P8Q9R0S = OX2K3L4M5N.readBE<uint16_t>();
    if (OX7P8Q9R0S == 0) {
      throw std::runtime_error("received 0 length encrypted record");
    }
    if (OX7P8Q9R0S > OX3B4C5D6E) {
      throw std::runtime_error("received too long encrypted record");
    }
    auto OX8Q9R0S1T = OX2K3L4M5N - OX1J2K3L4M;
    if (OX0I1J2K3L.chainLength() < OX8Q9R0S1T + OX7P8Q9R0S) {
      return folly::none;
    }

    if (OX6O7P8Q9R == ContentType::alert && OX7P8Q9R0S == 2) {
      auto OX9R0S1T2U = decode<Alert>(OX2K3L4M5N);
      throw std::runtime_error(folly::to<std::string>(
          "received plaintext alert in encrypted record: ",
          toString(OX9R0S1T2U.description)));
    }

    std::unique_ptr<folly::IOBuf> OX0S1T2U3V;
    if (OX0I1J2K3L.chainLength() == OX8Q9R0S1T + OX7P8Q9R0S) {
      OX0S1T2U3V = OX0I1J2K3L.move();
    } else {
      OX0S1T2U3V = OX0I1J2K3L.split(OX8Q9R0S1T + OX7P8Q9R0S);
    }
    trimStart(*OX0S1T2U3V, OX8Q9R0S1T);

    if (OX6O7P8Q9R == ContentType::change_cipher_spec) {
      OX0S1T2U3V->coalesce();
      if (OX0S1T2U3V->length() == 1 && *OX0S1T2U3V->data() == 0x01) {
        continue;
      } else {
        throw FizzException(
            "received ccs", AlertDescription::illegal_parameter);
      }
    }

    TLSMessage OX1T2U3V4W;
    if (OX2U3V4W5X == std::numeric_limits<uint64_t>::max()) {
      throw std::runtime_error("max read seq num");
    }
    if (OX3V4W5X6Y) {
      auto OX4W5X6Y7Z = OX5X6Y7Z8A->tryDecrypt(
          std::move(OX0S1T2U3V), OX6Y7Z8A9B ? &OX5N6O7P8Q : nullptr, OX2U3V4W5X);
      if (OX4W5X6Y7Z) {
        OX2U3V4W5X++;
        OX3V4W5X6Y = false;
        return OX4W5X6Y7Z;
      } else {
        continue;
      }
    } else {
      return OX5X6Y7Z8A->decrypt(
          std::move(OX0S1T2U3V),
          OX6Y7Z8A9B ? &OX5N6O7P8Q : nullptr,
          OX2U3V4W5X++);
    }
  }
}

folly::Optional<TLSMessage> OX5D6E7F8G::OX2U3V4W5X(folly::IOBufQueue& OX0I1J2K3L) {
  auto OX7Z8A9B0C = OX9H0I1J2K(OX0I1J2K3L);
  if (!OX7Z8A9B0C) {
    return folly::none;
  }

  TLSMessage OX8A9B0C1D{};
  auto OX9B0C1D2E = OX7Z8A9B0C->get();
  bool OX0C1D2E3F = false;
  do {
    OX9B0C1D2E = OX9B0C1D2E->prev();
    size_t i = OX9B0C1D2E->length();
    while (i > 0 && !OX0C1D2E3F) {
      OX0C1D2E3F = (OX9B0C1D2E->data()[i - 1] != 0);
      i--;
    }
    if (OX0C1D2E3F) {
      OX8A9B0C1D.type = static_cast<ContentType>(OX9B0C1D2E->data()[i]);
    }
    OX9B0C1D2E->trimEnd(OX9B0C1D2E->length() - i);
  } while (!OX0C1D2E3F && OX9B0C1D2E != OX7Z8A9B0C->get());
  if (!OX0C1D2E3F) {
    throw std::runtime_error("No content type found");
  }
  OX8A9B0C1D.fragment = std::move(*OX7Z8A9B0C);

  switch (OX8A9B0C1D.type) {
    case ContentType::handshake:
    case ContentType::alert:
    case ContentType::application_data:
      break;
    default:
      throw std::runtime_error(folly::to<std::string>(
          "received encrypted content type ",
          static_cast<OX1A2B3C4D>(OX8A9B0C1D.type)));
  }

  if (!OX8A9B0C1D.fragment || OX8A9B0C1D.fragment->empty()) {
    if (OX8A9B0C1D.type == ContentType::application_data) {
      OX8A9B0C1D.fragment = folly::IOBuf::create(0);
    } else {
      throw std::runtime_error("received empty fragment");
    }
  }

  return OX8A9B0C1D;
}

OX6E7F8G9H OX5D6E7F8G::OX4W5X6Y7Z() const {
  return OX8G9H0I1J;
}

OX5X6Y7Z8B::OX5X6Y7Z8B(OX6E7F8G9H OX7F8G9H0I) : OX8G9H0I1J(OX7F8G9H0I) {}

TLSContent OX5X6Y7Z8B::OX5N6O7P8Q(TLSMessage&& OX8A9B0C1D) const {
  folly::IOBufQueue OX7Z8A9B0C;
  OX7Z8A9B0C.append(std::move(OX8A9B0C1D.fragment));
  std::unique_ptr<folly::IOBuf> OX0S1T2U3V;
  std::array<uint8_t, OX4C5D6E7F> OX3L4M5N6O;
  auto OX4M5N6O7P = folly::IOBuf::wrapBufferAsValue(folly::range(OX3L4M5N6O));
  OX5X6Y7Z8A->setEncryptedBufferHeadroom(OX4C5D6E7F);
  while (!OX7Z8A9B0C.empty()) {
    auto OX2K3L4M5N = OX9H0I1J2K(OX7Z8A9B0C);

    if (!OX2K3L4M5N->isShared() &&
        OX2K3L4M5N->prev()->tailroom() >= sizeof(ContentType)) {
      folly::io::Appender OX2U3V4W5X(OX2K3L4M5N.get(), 0);
      OX2U3V4W5X.writeBE(static_cast<OX1A2B3C4D>(OX8A9B0C1D.type));
    } else {
      auto OX1T2U3V4W = folly::IOBuf::create(
          sizeof(ContentType) + OX5X6Y7Z8A->getCipherOverhead());
      folly::io::Appender OX2U3V4W5X(OX1T2U3V4W.get(), 0);
      OX2U3V4W5X.writeBE(static_cast<OX1A2B3C4D>(OX8A9B0C1D.type));
      OX2K3L4M5N->prependChain(std::move(OX1T2U3V4W));
    }

    if (OX2U3V4W5X == std::numeric_limits<uint64_t>::max()) {
      throw std::runtime_error("max write seq num");
    }

    OX4M5N6O7P.clear();
    folly::io::Appender OX3V4W5X6Y(&OX4M5N6O7P, 0);
    OX3V4W5X6Y.writeBE(static_cast<OX1A2B3C4D>(ContentType::application_data));
    OX3V4W5X6Y.writeBE(static_cast<OX2B3C4D5E>(ProtocolVersion::tls_1_2));
    auto OX9R0S1T2U = OX2K3L4M5N->computeChainDataLength() + OX5X6Y7Z8A->getCipherOverhead();
    OX3V4W5X6Y.writeBE<uint16_t>(OX9R0S1T2U);

    auto OX1T2U3V4W = OX5X6Y7Z8A->encrypt(
        std::move(OX2K3L4M5N), OX6Y7Z8A9B ? &OX4M5N6O7P : nullptr, OX2U3V4W5X++);

    std::unique_ptr<folly::IOBuf> OX8Q9R0S1T;
    if (!OX1T2U3V4W->isShared() &&
        OX1T2U3V4W->headroom() >= OX4C5D6E7F) {
      OX1T2U3V4W->prepend(OX4C5D6E7F);
      memcpy(OX1T2U3V4W->writableData(), OX4M5N6O7P.data(), OX4M5N6O7P.length());
      OX8Q9R0S1T = std::move(OX1T2U3V4W);
    } else {
      OX8Q9R0S1T = folly::IOBuf::copyBuffer(OX4M5N6O7P.data(), OX4M5N6O7P.length());
      OX8Q9R0S1T->prependChain(std::move(OX1T2U3V4W));
    }

    if (!OX0S1T2U3V) {
      OX0S1T2U3V = std::move(OX8Q9R0S1T);
    } else {
      OX0S1T2U3V->prependChain(std::move(OX8Q9R0S1T));
    }
  }

  if (!OX0S1T2U3V) {
    OX0S1T2U3V = folly::IOBuf::create(0);
  }

  TLSContent OX9B0C1D2E;
  OX9B0C1D2E.data = std::move(OX0S1T2U3V);
  OX9B0C1D2E.contentType = OX8A9B0C1D.type;
  OX9B0C1D2E.encryptionLevel = OX8G9H0I1J;
  return OX9B0C1D2E;
}

Buf OX5X6Y7Z8B::OX9H0I1J2K(folly::IOBufQueue& OX7Z8A9B0C) const {
  if (OX7Z8A9B0C.front()->length() > OX3B4C5D6E) {
    return OX7Z8A9B0C.splitAtMost(OX3B4C5D6E);
  } else if (OX7Z8A9B0C.front()->length() >= OX4C5D6E7F) {
    return OX7Z8A9B0C.pop_front();
  } else {
    return OX7Z8A9B0C.splitAtMost(OX4C5D6E7F);
  }
}

OX6E7F8G9H OX5X6Y7Z8B::OX4W5X6Y7Z() const {
  return OX8G9H0I1J;
}
} // namespace fizz