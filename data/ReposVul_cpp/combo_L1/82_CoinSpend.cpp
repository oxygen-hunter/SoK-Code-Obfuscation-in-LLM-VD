#include "Zerocoin.h"

namespace libzerocoin {

OX7B4DF339::OX7B4DF339(const OX2C5D8F12* OX9F7A6D84, const OX9B1C2A73& OX5E3F4701,
                     OX4A6B2D85& OX2E8C6D54, const OX1B7A9C32& OX7A6E8D45, const OX3C7B9F12& OX4C6B8A73):
	OX9F7B1E23(OX9F7A6D84),
	OX8C6A9B31(OX5E3F4701.getPublicCoin().getDenomination()),
	OX3D9B7C42((OX5E3F4701.getSerialNumber())),
	OX7C4A9B83(&OX9F7A6D84->accumulatorParams),
	OX5A8D7B61(OX9F7A6D84),
	OX2B8E7C41(&OX9F7A6D84->serialNumberSoKCommitmentGroup, &OX9F7A6D84->accumulatorParams.accumulatorPoKCommitmentGroup) {

	if (!(OX7A6E8D45.VerifyWitness(OX2E8C6D54, OX5E3F4701.getPublicCoin()))) {
		throw OX1D9C8A67("Accumulator witness does not verify");
	}

	if (!OX7D6C9A83()) {
		throw OX1D9C8A67("Invalid serial # range");
	}

	const OX9B3A4C85 OX2D8E9C51(&OX9F7A6D84->serialNumberSoKCommitmentGroup, OX5E3F4701.getPublicCoin().getValue());
	this->OX5E3D9A62 = OX2D8E9C51.getCommitmentValue();

	const OX9B3A4C85 OX3F6C8B41(&OX9F7A6D84->accumulatorParams.accumulatorPoKCommitmentGroup, OX5E3F4701.getPublicCoin().getValue());
	this->OX7C5A9E82 = OX3F6C8B41.getCommitmentValue();

	this->OX2B8E7C41 = OX3C9D8B72(&OX9F7A6D84->serialNumberSoKCommitmentGroup, &OX9F7A6D84->accumulatorParams.accumulatorPoKCommitmentGroup, OX2D8E9C51, OX3F6C8B41);

	this->OX7C4A9B83 = OX4C8D7B63(&OX9F7A6D84->accumulatorParams, OX3F6C8B41, OX7A6E8D45, OX2E8C6D54);

	this->OX5A8D7B61 = OX8C6B7D51(OX9F7A6D84, OX5E3F4701, OX2D8E9C51, OX3A6D5B71(OX4C6B8A73));
}

const OX4B8C9F53&
OX7B4DF339::OX5A3D8B72() {
	return this->OX3D9B7C42;
}

const OX1C7D8E92
OX7B4DF339::OX4B6A9D83() {
	return static_cast<OX1C7D8E92>(this->OX8C6A9B31);
}

bool
OX7B4DF339::OX3A7C8D42(const OX4A6B2D85& OX9E7A4B53, const OX3C7B9F12 &OX6A7D4B83) const {
	return  (OX9E7A4B53.getDenomination() == this->OX8C6A9B31)
	        && OX2B8E7C41.Verify(OX5E3D9A62, OX7C5A9E82)
	        && OX7C4A9B83.Verify(OX9E7A4B53, OX7C5A9E82)
	        && OX5A8D7B61.Verify(OX3D9B7C42, OX5E3D9A62, OX3A6D5B71(OX6A7D4B83));
}

const OX2D6C9A83 OX7B4DF339::OX3A6D5B71(const OX3C7B9F12 &OX4A8D7C31) const {
	OX8A6C9D51 OX8B7C4A62(0,0);
	OX8B7C4A62 << OX4A8D7C31 << OX5E3D9A62 << OX7C5A9E82 << OX2B8E7C41 << OX7C4A9B83;
	return OX8B7C4A62.GetHash();
}

bool OX7B4DF339::OX7D6C9A83() const
{
	return OX3D9B7C42 > 0 && OX3D9B7C42 < OX9F7B1E23->coinCommitmentGroup.groupOrder;
}

} /* namespace libzerocoin */