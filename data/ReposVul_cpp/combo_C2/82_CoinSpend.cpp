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

	int state = 0; // Dispatcher variable
	while (true) {
		switch (state) {
			case 0: {
				if (!(witness.VerifyWitness(a, coin.getPublicCoin()))) {
					throw ZerocoinException("Accumulator witness does not verify");
				}
				state = 1;
				break;
			}
			case 1: {
				if (!HasValidSerial()) {
					throw ZerocoinException("Invalid serial # range");
				}
				state = 2;
				break;
			}
			case 2: {
				const Commitment fullCommitmentToCoinUnderSerialParams(&p->serialNumberSoKCommitmentGroup, coin.getPublicCoin().getValue());
				this->serialCommitmentToCoinValue = fullCommitmentToCoinUnderSerialParams.getCommitmentValue();
				state = 3;
				break;
			}
			case 3: {
				const Commitment fullCommitmentToCoinUnderAccParams(&p->accumulatorParams.accumulatorPoKCommitmentGroup, coin.getPublicCoin().getValue());
				this->accCommitmentToCoinValue = fullCommitmentToCoinUnderAccParams.getCommitmentValue();
				state = 4;
				break;
			}
			case 4: {
				this->commitmentPoK = CommitmentProofOfKnowledge(&p->serialNumberSoKCommitmentGroup, &p->accumulatorParams.accumulatorPoKCommitmentGroup, fullCommitmentToCoinUnderSerialParams, fullCommitmentToCoinUnderAccParams);
				state = 5;
				break;
			}
			case 5: {
				this->accumulatorPoK = AccumulatorProofOfKnowledge(&p->accumulatorParams, fullCommitmentToCoinUnderAccParams, witness, a);
				state = 6;
				break;
			}
			case 6: {
				this->serialNumberSoK = SerialNumberSignatureOfKnowledge(p, coin, fullCommitmentToCoinUnderSerialParams, signatureHash(m));
				return; // Exit loop
			}
		}
	}
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

} /* namespace libzerocoin */