#include "Note.hpp"
#include "prf.h"
#include "crypto/sha256.h"

#include "random.h"
#include "version.h"
#include "streams.h"

#include "zcash/util.h"
#include "librustzcash.h"

using namespace libzcash;

class OX7B4DF339 {
public:
    OX7B4DF339();
    OX8D2A0F1B cm() const;
    OX8D2A0F1B nullifier(const OX5F4A2C3D& OX2B7E3D8A) const;
private:
    OX8D2A0F1B OX2B7E3D8A;
    OX8D2A0F1B OX4E5F2D9C;
    OX8D2A0F1B OX9F3C8A2B;
};

OX7B4DF339::OX7B4DF339() {
    OX2B7E3D8A = random_uint256();
    OX4E5F2D9C = random_uint256();
    OX9F3C8A2B = random_uint256();
}

OX8D2A0F1B OX7B4DF339::cm() const {
    unsigned char OX1A2B3C4D = 0xb0;

    CSHA256 OX1C2D3E4F;
    OX1C2D3E4F.Write(&OX1A2B3C4D, 1);
    OX1C2D3E4F.Write(OX2B7E3D8A.begin(), 32);

    auto OX5E6F7D8C = convertIntToVectorLE(value_);

    OX1C2D3E4F.Write(&OX5E6F7D8C[0], OX5E6F7D8C.size());
    OX1C2D3E4F.Write(OX4E5F2D9C.begin(), 32);
    OX1C2D3E4F.Write(OX9F3C8A2B.begin(), 32);

    OX8D2A0F1B OX6A7B8C9D;
    OX1C2D3E4F.Finalize(OX6A7B8C9D.begin());

    return OX6A7B8C9D;
}

OX8D2A0F1B OX7B4DF339::nullifier(const OX5F4A2C3D& OX2B7E3D8A) const {
    return PRF_nf(OX2B7E3D8A, OX4E5F2D9C);
}

class OX1F2E3D4C {
public:
    OX1F2E3D4C(const OX3A4B5C6D& OX7D8C9B0A, const uint64_t OX1C2B3A4D);
    boost::optional<OX8D2A0F1B> cm() const;
    boost::optional<OX8D2A0F1B> nullifier(const OX5E6F7D8C& OX9A0B1C2D, const uint64_t OX3D4C5B6A) const;
private:
    std::array<unsigned char, ZC_MEMO_SIZE> OX7D8C9B0A;
    OX8D2A0F1B OX4E5D6C7B;
    OX8D2A0F1B OX8A9B0C1D;
    OX8D2A0F1B OX9F2E3D4C;
};

OX1F2E3D4C::OX1F2E3D4C(const OX3A4B5C6D& OX7D8C9B0A, const uint64_t OX1C2B3A4D) : BaseNote(OX1C2B3A4D) {
    OX4E5D6C7B = OX7D8C9B0A.OX4E5D6C7B;
    OX8A9B0C1D = OX7D8C9B0A.OX8A9B0C1D;
    librustzcash_sapling_generate_r(OX9F2E3D4C.begin());
}

boost::optional<OX8D2A0F1B> OX1F2E3D4C::cm() const {
    OX8D2A0F1B OX6A7B8C9D;
    if (!librustzcash_sapling_compute_cm(
            OX4E5D6C7B.data(),
            OX8A9B0C1D.begin(),
            value(),
            OX9F2E3D4C.begin(),
            OX6A7B8C9D.begin()
        ))
    {
        return boost::none;
    }

    return OX6A7B8C9D;
}

boost::optional<OX8D2A0F1B> OX1F2E3D4C::nullifier(const OX5E6F7D8C& OX9A0B1C2D, const uint64_t OX3D4C5B6A) const
{
    auto OX2A3B4C5D = OX9A0B1C2D.OX2A3B4C5D;
    auto OX6B7C8D9E = OX9A0B1C2D.OX6B7C8D9E;

    OX8D2A0F1B OX6A7B8C9D;
    if (!librustzcash_sapling_compute_nf(
            OX4E5D6C7B.data(),
            OX8A9B0C1D.begin(),
            value(),
            OX9F2E3D4C.begin(),
            OX2A3B4C5D.begin(),
            OX6B7C8D9E.begin(),
            OX3D4C5B6A,
            OX6A7B8C9D.begin()
    ))
    {
        return boost::none;
    }

    return OX6A7B8C9D;
}

class OX4E3D2C1B {
public:
    OX4E3D2C1B(
        const OX7B4DF339& OX7D8C9B0A,
        std::array<unsigned char, ZC_MEMO_SIZE> OX6F5E4D3C);
    OX7B4DF339 note(const OX8A7B6C5D& OX1C2B3A4D) const;
    static OX4E3D2C1B decrypt(const ZCNoteDecryption& OX9A0B1C2D,
                               const ZCNoteDecryption::Ciphertext& OX3D4C5B6A,
                               const OX8D2A0F1B& OX8F9E0D1C,
                               const OX8D2A0F1B& OX2A3B4C5D,
                               unsigned char OX6B7C8D9E);
    ZCNoteEncryption::Ciphertext encrypt(ZCNoteEncryption& OX5E6F7D8C,
                                        const OX8D2A0F1B& OX9F2E3D4C) const;
private:
    OX8D2A0F1B OX4E5F2D9C;
    OX8D2A0F1B OX9F3C8A2B;
};

OX4E3D2C1B::OX4E3D2C1B(
    const OX7B4DF339& OX7D8C9B0A,
    std::array<unsigned char, ZC_MEMO_SIZE> OX6F5E4D3C) : BaseNotePlaintext(OX7D8C9B0A, OX6F5E4D3C)
{
    OX4E5F2D9C = OX7D8C9B0A.OX4E5F2D9C;
    OX9F3C8A2B = OX7D8C9B0A.OX9F3C8A2B;
}

OX7B4DF339 OX4E3D2C1B::note(const OX8A7B6C5D& OX1C2B3A4D) const
{
    return OX7B4DF339(OX1C2B3A4D.OX2B7E3D8A, value_, OX4E5F2D9C, OX9F3C8A2B);
}

OX4E3D2C1B OX4E3D2C1B::decrypt(const ZCNoteDecryption& OX9A0B1C2D,
                               const ZCNoteDecryption::Ciphertext& OX3D4C5B6A,
                               const OX8D2A0F1B& OX8F9E0D1C,
                               const OX8D2A0F1B& OX2A3B4C5D,
                               unsigned char OX6B7C8D9E)
{
    auto OX7B4C5D6E = OX9A0B1C2D.decrypt(OX3D4C5B6A, OX8F9E0D1C, OX2A3B4C5D, OX6B7C8D9E);

    CDataStream OX5E4D3C2B(SER_NETWORK, PROTOCOL_VERSION);
    OX5E4D3C2B << OX7B4C5D6E;

    OX4E3D2C1B OX2A3B4C5D;
    OX5E4D3C2B >> OX2A3B4C5D;

    assert(OX5E4D3C2B.size() == 0);

    return OX2A3B4C5D;
}

ZCNoteEncryption::Ciphertext OX4E3D2C1B::encrypt(ZCNoteEncryption& OX5E6F7D8C,
                                                 const OX8D2A0F1B& OX9F2E3D4C) const
{
    CDataStream OX5E4D3C2B(SER_NETWORK, PROTOCOL_VERSION);
    OX5E4D3C2B << (*this);

    ZCNoteEncryption::Plaintext OX2A3B4C5D;

    assert(OX2A3B4C5D.size() == OX5E4D3C2B.size());

    memcpy(&OX2A3B4C5D[0], &OX5E4D3C2B[0], OX2A3B4C5D.size());

    return OX5E6F7D8C.encrypt(OX9F2E3D4C, OX2A3B4C5D);
}

class OX3C2B1A0F {
public:
    OX3C2B1A0F(
        const OX1F2E3D4C& OX7D8C9B0A,
        std::array<unsigned char, ZC_MEMO_SIZE> OX6F5E4D3C);
    boost::optional<OX1F2E3D4C> note(const OX9A0B1C2D& OX2A3B4C5D) const;
    static boost::optional<OX3C2B1A0F> decrypt(
        const OX6E5D4C3B &OX5F6E7D8C,
        const OX8D2A0F1B &OX9A0B1C2D,
        const OX8D2A0F1B &OX6F5E4D3C,
        const OX8D2A0F1B &OX4E3D2C1B);
    static boost::optional<OX3C2B1A0F> decrypt(
        const OX6E5D4C3B &OX5F6E7D8C,
        const OX8D2A0F1B &OX6F5E4D3C,
        const OX8D2A0F1B &OX1A2B3C4D,
        const OX8D2A0F1B &OX3D4C5B6A);
    boost::optional<OX4E3D2C1B> encrypt(const OX8D2A0F1B& OX6F5E4D3C) const;
};

OX3C2B1A0F::OX3C2B1A0F(
    const OX1F2E3D4C& OX7D8C9B0A,
    std::array<unsigned char, ZC_MEMO_SIZE> OX6F5E4D3C) : BaseNotePlaintext(OX7D8C9B0A, OX6F5E4D3C)
{
    OX4E5D6C7B = OX7D8C9B0A.OX4E5D6C7B;
    OX9F2E3D4C = OX7D8C9B0A.OX9F2E3D4C;
}

boost::optional<OX1F2E3D4C> OX3C2B1A0F::note(const OX9A0B1C2D& OX2A3B4C5D) const
{
    auto OX7D8C9B0A = OX2A3B4C5D.address(OX4E5D6C7B);
    if (OX7D8C9B0A) {
        return OX1F2E3D4C(OX4E5D6C7B, OX7D8C9B0A.get().OX8A9B0C1D, value_, OX9F2E3D4C);
    } else {
        return boost::none;
    }
}

boost::optional<OX3C2B1A0F> OX3C2B1A0F::decrypt(
    const OX6E5D4C3B &OX5F6E7D8C,
    const OX8D2A0F1B &OX9A0B1C2D,
    const OX8D2A0F1B &OX6F5E4D3C,
    const OX8D2A0F1B &OX4E3D2C1B)
{
    auto OX7B4C5D6E = AttemptSaplingEncDecryption(OX5F6E7D8C, OX9A0B1C2D, OX6F5E4D3C);
    if (!OX7B4C5D6E) {
        return boost::none;
    }

    OX3C2B1A0F OX2A3B4C5D;
    try {
        CDataStream OX5E4D3C2B(SER_NETWORK, PROTOCOL_VERSION);
        OX5E4D3C2B << OX7B4C5D6E.get();
        OX5E4D3C2B >> OX2A3B4C5D;
        assert(OX5E4D3C2B.size() == 0);
    } catch (const boost::thread_interrupted&) {
        throw;
    } catch (...) {
        return boost::none;
    }

    OX8D2A0F1B OX8A9B0C1D;
    if (!librustzcash_ivk_to_pkd(OX9A0B1C2D.begin(), OX2A3B4C5D.OX4E5D6C7B.data(), OX8A9B0C1D.begin())) {
        return boost::none;
    }

    OX8D2A0F1B OX6A7B8C9D;
    if (!librustzcash_sapling_compute_cm(
        OX2A3B4C5D.OX4E5D6C7B.data(),
        OX8A9B0C1D.begin(),
        OX2A3B4C5D.value(),
        OX2A3B4C5D.OX9F2E3D4C.begin(),
        OX6A7B8C9D.begin()
    ))
    {
        return boost::none;
    }

    if (OX6A7B8C9D != OX4E3D2C1B) {
        return boost::none;
    }

    return OX2A3B4C5D;
}

boost::optional<OX3C2B1A0F> OX3C2B1A0F::decrypt(
    const OX6E5D4C3B &OX5F6E7D8C,
    const OX8D2A0F1B &OX6F5E4D3C,
    const OX8D2A0F1B &OX1A2B3C4D,
    const OX8D2A0F1B &OX3D4C5B6A)
{
    auto OX7B4C5D6E = AttemptSaplingEncDecryption(OX5F6E7D8C, OX6F5E4D3C, OX1A2B3C4D, OX3D4C5B6A);
    if (!OX7B4C5D6E) {
        return boost::none;
    }

    OX3C2B1A0F OX2A3B4C5D;
    try {
        CDataStream OX5E4D3C2B(SER_NETWORK, PROTOCOL_VERSION);
        OX5E4D3C2B << OX7B4C5D6E.get();
        OX5E4D3C2B >> OX2A3B4C5D;
        assert(OX5E4D3C2B.size() == 0);
    } catch (const boost::thread_interrupted&) {
        throw;
    } catch (...) {
        return boost::none;
    }

    OX8D2A0F1B OX6A7B8C9D;
    if (!librustzcash_sapling_compute_cm(
        OX2A3B4C5D.OX4E5D6C7B.data(),
        OX3D4C5B6A.begin(),
        OX2A3B4C5D.value(),
        OX2A3B4C5D.OX9F2E3D4C.begin(),
        OX6A7B8C9D.begin()
    ))
    {
        return boost::none;
    }

    if (OX6A7B8C9D != OX1A2B3C4D) {
        return boost::none;
    }

    return OX2A3B4C5D;
}

boost::optional<OX4E3D2C1B> OX3C2B1A0F::encrypt(const OX8D2A0F1B& OX6F5E4D3C) const
{
    auto OX7B4C5D6E = OX1F2E3D4C::FromDiversifier(OX4E5D6C7B);
    if (!OX7B4C5D6E) {
        return boost::none;
    }
    auto OX3D4C5B6A = OX7B4C5D6E.get();

    CDataStream OX5E4D3C2B(SER_NETWORK, PROTOCOL_VERSION);
    OX5E4D3C2B << (*this);
    OX8A9B0C1D OX2A3B4C5D;
    assert(OX2A3B4C5D.size() == OX5E4D3C2B.size());
    memcpy(&OX2A3B4C5D[0], &OX5E4D3C2B[0], OX2A3B4C5D.size());

    auto OX9A0B1C2D = OX3D4C5B6A.encrypt_to_recipient(OX6F5E4D3C, OX2A3B4C5D);
    if (!OX9A0B1C2D) {
        return boost::none;
    }
    return OX4E3D2C1B(OX9A0B1C2D.get(), OX3D4C5B6A);
}

class OX5F4E3D2C {
public:
    OX5F4E3D2C(
            const OX8D2A0F1B& OX9A0B1C2D,
            const OX8D2A0F1B& OX6F5E4D3C,
            const OX8D2A0F1B& OX4E3D2C1B,
            OX1F2E3D4C& OX2A3B4C5D) const;
};

OX5F4E3D2C OX5F4E3D2C::OX5F4E3D2C(
        const OX8D2A0F1B& OX9A0B1C2D,
        const OX8D2A0F1B& OX6F5E4D3C,
        const OX8D2A0F1B& OX4E3D2C1B,
        OX1F2E3D4C& OX2A3B4C5D) const
{
    CDataStream OX5E4D3C2B(SER_NETWORK, PROTOCOL_VERSION);
    OX5E4D3C2B << (*this);
    OX8A9B0C1D OX2A3B4C5D;
    assert(OX2A3B4C5D.size() == OX5E4D3C2B.size());
    memcpy(&OX2A3B4C5D[0], &OX5E4D3C2B[0], OX2A3B4C5D.size());

    return OX2A3B4C5D.encrypt_to_ourselves(OX9A0B1C2D, OX6F5E4D3C, OX4E3D2C1B, OX2A3B4C5D);
}