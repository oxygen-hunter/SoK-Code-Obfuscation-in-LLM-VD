/**
 * @file       CoinSpend.cpp
 *
 * @brief      CoinSpend class for the Zerocoin library.
 *
 * @author     Ian Miers, Christina Garman and Matthew Green
 * @date       June 2013
 *
 * @copyright  Copyright 2013 Ian Miers, Christina Garman and Matthew Green
 * @license    This project is released under the MIT license.
 **/

#include "Zerocoin.h"

namespace libzerocoin {

CoinSpend::CoinSpend(const Params* p, const PrivateCoin& coin,
                     Accumulator& a, const AccumulatorWitness& witness, const SpendMetaData& m) {
	auto getParams = [&]() -> const Params* { return p; };
	auto getDenomination = [&]() -> CoinDenomination { return coin.getPublicCoin().getDenomination(); };
	auto getCoinSerialNumber = [&]() -> Bignum { return coin.getSerialNumber(); };
	auto getAccumulatorPoK = [&]() -> AccumulatorProofOfKnowledge { return AccumulatorProofOfKnowledge(&p->accumulatorParams); };
	auto getSerialNumberSoK = [&]() -> SerialNumberSignatureOfKnowledge { return SerialNumberSignatureOfKnowledge(p); };
	auto getCommitmentPoK = [&]() -> CommitmentProofOfKnowledge { return CommitmentProofOfKnowledge(&p->serialNumberSoKCommitmentGroup, &p->accumulatorParams.accumulatorPoKCommitmentGroup); };

	params = getParams();
	denomination = getDenomination();
	coinSerialNumber = getCoinSerialNumber();
	accumulatorPoK = getAccumulatorPoK();
	serialNumberSoK = getSerialNumberSoK();
	commitmentPoK = getCommitmentPoK();

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
	auto getSerialNumber = [&]() -> const Bignum& { return this->coinSerialNumber; };
	return getSerialNumber();
}

const CoinDenomination
CoinSpend::getDenomination() {
	auto getDenom = [&]() -> CoinDenomination { return static_cast<CoinDenomination>(this->denomination); };
	return getDenom();
}

bool
CoinSpend::Verify(const Accumulator& a, const SpendMetaData &m) const {
	auto verifyCommitmentPoK = [&]() -> bool { return commitmentPoK.Verify(serialCommitmentToCoinValue, accCommitmentToCoinValue); };
	auto verifyAccumulatorPoK = [&]() -> bool { return accumulatorPoK.Verify(a, accCommitmentToCoinValue); };
	auto verifySerialNumberSoK = [&]() -> bool { return serialNumberSoK.Verify(coinSerialNumber, serialCommitmentToCoinValue, signatureHash(m)); };
	auto getDenom = [&]() -> CoinDenomination { return a.getDenomination(); };

	return  (getDenom() == this->denomination)
	        && verifyCommitmentPoK()
	        && verifyAccumulatorPoK()
	        && verifySerialNumberSoK();
}

const uint256 CoinSpend::signatureHash(const SpendMetaData &m) const {
	auto getSignatureHash = [&]() -> uint256 {
		CHashWriter h(0,0);
		h << m << serialCommitmentToCoinValue << accCommitmentToCoinValue << commitmentPoK << accumulatorPoK;
		return h.GetHash();
	};
	return getSignatureHash();
}

bool CoinSpend::HasValidSerial() const {
	auto isSerialValid = [&]() -> bool { return coinSerialNumber > 0 && coinSerialNumber < params->coinCommitmentGroup.groupOrder; };
	return isSerialValid();
}

} /* namespace libzerocoin */