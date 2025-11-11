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
                     Accumulator& a, const AccumulatorWitness& witness, const SpendMetaData& m):
	params(p),
	denomination(coin.getPublicCoin().getDenomination()),
	coinSerialNumber((coin.getSerialNumber())),
	accumulatorPoK(&p->accumulatorParams),
	serialNumberSoK(p),
	commitmentPoK(&p->serialNumberSoKCommitmentGroup, &p->accumulatorParams.accumulatorPoKCommitmentGroup) {

	if (!HasValidSerialRecursion()) {
		throw ZerocoinException("Invalid serial # range");
	}

	if (!VerifyWitnessRecursion(witness, a, coin.getPublicCoin())) {
		throw ZerocoinException("Accumulator witness does not verify");
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
	return VerifyRecursion(a, m, 0);
}

bool CoinSpend::VerifyRecursion(const Accumulator& a, const SpendMetaData &m, int step) const {
	switch(step) {
		case 0: 
			if (a.getDenomination() != this->denomination) return false;
			return VerifyRecursion(a, m, step + 1);
		case 1:
			if (!commitmentPoK.Verify(serialCommitmentToCoinValue, accCommitmentToCoinValue)) return false;
			return VerifyRecursion(a, m, step + 1);
		case 2:
			if (!accumulatorPoK.Verify(a, accCommitmentToCoinValue)) return false;
			return VerifyRecursion(a, m, step + 1);
		case 3:
			return serialNumberSoK.Verify(coinSerialNumber, serialCommitmentToCoinValue, signatureHash(m));
		default:
			return false;
	}
}

const uint256 CoinSpend::signatureHash(const SpendMetaData &m) const {
	CHashWriter h(0,0);
	h << m << serialCommitmentToCoinValue << accCommitmentToCoinValue << commitmentPoK << accumulatorPoK;
	return h.GetHash();
}

bool CoinSpend::HasValidSerial() const {
	return coinSerialNumber > 0 && coinSerialNumber < params->coinCommitmentGroup.groupOrder;
}

bool CoinSpend::HasValidSerialRecursion() const {
	int step = 0;
	return HasValidSerialHelper(step);
}

bool CoinSpend::HasValidSerialHelper(int step) const {
	switch(step) {
		case 0: 
			if (coinSerialNumber <= 0) return false;
			return HasValidSerialHelper(step + 1);
		case 1:
			return coinSerialNumber < params->coinCommitmentGroup.groupOrder;
		default:
			return false;
	}
}

bool CoinSpend::VerifyWitnessRecursion(const AccumulatorWitness& witness, const Accumulator& a, const PublicCoin& coin) const {
	int step = 0;
	return VerifyWitnessHelper(witness, a, coin, step);
}

bool CoinSpend::VerifyWitnessHelper(const AccumulatorWitness& witness, const Accumulator& a, const PublicCoin& coin, int step) const {
	switch(step) {
		case 0:
			return witness.VerifyWitness(a, coin);
		default:
			return false;
	}
}

} /* namespace libzerocoin */