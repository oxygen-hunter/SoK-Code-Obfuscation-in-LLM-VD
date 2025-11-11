#include "Zerocoin.h"

namespace libzerocoin {

CoinSpend::CoinSpend(const Params* p, const PrivateCoin& coin,
                     Accumulator& a, const AccumulatorWitness& witness, const SpendMetaData& m):
	denomination(coin.getPublicCoin().getDenomination()),
	coinSerialNumber((coin.getSerialNumber()))
{
	auto params = p;
	auto accumulatorPoK = &params->accumulatorParams;
	auto serialNumberSoK = params;
	auto commitmentPoK = [&params]() {
		return CommitmentProofOfKnowledge(&params->serialNumberSoKCommitmentGroup, &params->accumulatorParams.accumulatorPoKCommitmentGroup);
	};

	if (!(witness.VerifyWitness(a, coin.getPublicCoin()))) {
		throw ZerocoinException("Accumulator witness does not verify");
	}

	if (!HasValidSerial()) {
		throw ZerocoinException("Invalid serial # range");
	}

	const Commitment fullCommitmentToCoinUnderSerialParams(&params->serialNumberSoKCommitmentGroup, coin.getPublicCoin().getValue());
	auto serialCommitmentToCoinValue = fullCommitmentToCoinUnderSerialParams.getCommitmentValue();

	const Commitment fullCommitmentToCoinUnderAccParams(&params->accumulatorParams.accumulatorPoKCommitmentGroup, coin.getPublicCoin().getValue());
	auto accCommitmentToCoinValue = fullCommitmentToCoinUnderAccParams.getCommitmentValue();

	this->commitmentPoK = commitmentPoK();

	this->accumulatorPoK = AccumulatorProofOfKnowledge(&params->accumulatorParams, fullCommitmentToCoinUnderAccParams, witness, a);

	this->serialNumberSoK = SerialNumberSignatureOfKnowledge(params, coin, fullCommitmentToCoinUnderSerialParams, signatureHash(m));
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
	auto serialCommitmentToCoinValue = this->serialCommitmentToCoinValue;
	auto accCommitmentToCoinValue = this->accCommitmentToCoinValue;
	return  (a.getDenomination() == this->denomination)
	        && commitmentPoK.Verify(serialCommitmentToCoinValue, accCommitmentToCoinValue)
	        && accumulatorPoK.Verify(a, accCommitmentToCoinValue)
	        && serialNumberSoK.Verify(coinSerialNumber, serialCommitmentToCoinValue, signatureHash(m));
}

const uint256 CoinSpend::signatureHash(const SpendMetaData &m) const {
	auto h = CHashWriter(0,0);
	auto serialCommitmentToCoinValue = this->serialCommitmentToCoinValue;
	auto accCommitmentToCoinValue = this->accCommitmentToCoinValue;
	h << m << serialCommitmentToCoinValue << accCommitmentToCoinValue << commitmentPoK << accumulatorPoK;
	return h.GetHash();
}

bool CoinSpend::HasValidSerial() const
{
	auto params = this->params;
	return coinSerialNumber > 0 && coinSerialNumber < params->coinCommitmentGroup.groupOrder;
}

} /* namespace libzerocoin */