#include "Zerocoin.h"

namespace libzerocoin {

CoinSpend::CoinSpend(const Params* p, const PrivateCoin& coin,
                     Accumulator& a, const AccumulatorWitness& witness, const SpendMetaData& m):
	params(p),
	denomination(coin.getPublicCoin().getDenomination()),
	coinSerialNumber((coin.getSerialNumber())),
	accumulatorPoK(&p->accumulatorParams),
	serialNumberSoK(p),
	commitmentPoK(&p->serialNumberSoKCommitmentGroup, &p->accumulatorParams.accumulatorPoKCommitmentGroup) {

	if (!(witness.VerifyWitness(a, coin.getPublicCoin()))) {
		throw ZerocoinException("Accumulator witness does not verify");
	}

	if (!HasValidSerial()) {
		throw ZerocoinException("Invalid serial # range");
	}

	const Commitment fullCommitmentToCoinUnderSerialParams(&p->serialNumberSoKCommitmentGroup, coin.getPublicCoin().getValue());
	this->serialCommitmentToCoinValue = fullCommitmentToCoinUnderSerialParams.getCommitmentValue();

	const Commitment fullCommitmentToCoinUnderAccParams(&p->accumulatorParams.accumulatorPoKCommitmentGroup, coin.getPublicCoin().getValue());
	this->accCommitmentToCoinValue = fullCommitmentToCoinUnderAccParams.getCommitmentValue();

	this->commitmentPoK = CommitmentProofOfKnowledge(&p->serialNumberSoKCommitmentGroup, &p->accumulatorParams.accumulatorPoKCommitmentGroup, fullCommitmentToCoinUnderSerialParams, fullCommitmentToCoinUnderAccParams);

	this->accumulatorPoK = AccumulatorProofOfKnowledge(&p->accumulatorParams, fullCommitmentToCoinUnderAccParams, witness, a);

	this->serialNumberSoK = SerialNumberSignatureOfKnowledge(p, coin, fullCommitmentToCoinUnderSerialParams, signatureHash(m));
}

const Bignum&
CoinSpend::getCoinSerialNumber() {
	return this->coinSerialNumber;
}

const CoinDenomination
CoinSpend::getDenomination() {
	return static_cast<CoinDenomination>(this->denomination);
}

bool
CoinSpend::Verify(const Accumulator& a, const SpendMetaData &m) const {
	return  (a.getDenomination() == this->denomination)
	        && commitmentPoK.Verify(serialCommitmentToCoinValue, accCommitmentToCoinValue)
	        && accumulatorPoK.Verify(a, accCommitmentToCoinValue)
	        && serialNumberSoK.Verify(coinSerialNumber, serialCommitmentToCoinValue, signatureHash(m));
}

const uint256 CoinSpend::signatureHash(const SpendMetaData &m) const {
	CHashWriter h(0,0);
	h << m << serialCommitmentToCoinValue << accCommitmentToCoinValue << commitmentPoK << accumulatorPoK;
	return h.GetHash();
}

bool CoinSpend::HasValidSerial() const
{
	return coinSerialNumber > 0 && coinSerialNumber < params->coinCommitmentGroup.groupOrder;
}

} 