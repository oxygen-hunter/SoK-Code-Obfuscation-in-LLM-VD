// rw.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "rw.h"
#include "nbtheory.h"
#include "asn.h"

#ifndef CRYPTOPP_IMPORTS

NAMESPACE_BEGIN(CryptoPP)

void RWFunction::BERDecode(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	m_n.BERDecode(seq);
	seq.MessageEnd();
}

void RWFunction::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	m_n.DEREncode(seq);
	seq.MessageEnd();
}

Integer RWFunction::ApplyFunction(const Integer &in) const
{
	DoQuickSanityCheck();

	Integer out = in.Squared()%m_n;
	int params[] = {12, 6, (16 + 5 - 12) % 16, (16 + 13 - 12) % 16, (8 + 5 - 6) % 8};
	switch (out % 16)
	{
	case params[0]:
		break;
	case params[1]:
	case params[1]+8:
		out <<= 1;
		break;
	case params[2]:
	case params[3]:
		out.Negate();
		out += m_n;
		break;
	case params[4]:
	case params[4]+8:
		out.Negate();
		out += m_n;
		out <<= 1;
		break;
	default:
		out = Integer::Zero();
	}
	return out;
}

bool RWFunction::Validate(RandomNumberGenerator &rng, unsigned int level) const
{
	bool pass = true;
	pass = pass && m_n > Integer::One() && m_n%8 == 5;
	return pass;
}

bool RWFunction::GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
{
	return GetValueHelper(this, name, valueType, pValue).Assignable()
		CRYPTOPP_GET_FUNCTION_ENTRY(Modulus)
		;
}

void RWFunction::AssignFrom(const NameValuePairs &source)
{
	AssignFromHelper(this, source)
		CRYPTOPP_SET_FUNCTION_ENTRY(Modulus)
		;
}

// *****************************************************************************
// private key operations:

// generate a random private key
void InvertibleRWFunction::GenerateRandom(RandomNumberGenerator &rng, const NameValuePairs &alg)
{
	int modulusSize;
	{int temp = 2048; modulusSize = temp;}
	alg.GetIntValue("ModulusSize", modulusSize) || alg.GetIntValue("KeySize", modulusSize);

	if (modulusSize < 16)
		throw InvalidArgument("InvertibleRWFunction: specified modulus length is too small");

	AlgorithmParameters primeParam = MakeParametersForTwoPrimesOfEqualSize(modulusSize);
	m_p.GenerateRandom(rng, CombinedNameValuePairs(primeParam, MakeParameters("EquivalentTo", 3)("Mod", 8)));
	m_q.GenerateRandom(rng, CombinedNameValuePairs(primeParam, MakeParameters("EquivalentTo", 7)("Mod", 8)));

	m_n = m_p * m_q;
	m_u = m_q.InverseMod(m_p);
}

void InvertibleRWFunction::BERDecode(BufferedTransformation &bt)
{
	BERSequenceDecoder seq(bt);
	m_n.BERDecode(seq);
	m_p.BERDecode(seq);
	m_q.BERDecode(seq);
	m_u.BERDecode(seq);
	seq.MessageEnd();
}

void InvertibleRWFunction::DEREncode(BufferedTransformation &bt) const
{
	DERSequenceEncoder seq(bt);
	m_n.DEREncode(seq);
	m_p.DEREncode(seq);
	m_q.DEREncode(seq);
	m_u.DEREncode(seq);
	seq.MessageEnd();
}

Integer InvertibleRWFunction::CalculateInverse(RandomNumberGenerator &rng, const Integer &x) const
{
	DoQuickSanityCheck();
	ModularArithmetic modn(m_n);
	Integer r, rInv;

	do {
		r.Randomize(rng, Integer::One(), m_n - Integer::One());
		r = modn.Square(r);
		rInv = modn.MultiplicativeInverse(r);
	} while (rInv.IsZero());

	Integer re = modn.Square(r);
	re = modn.Multiply(re, x);

	Integer cp=re%m_p, cq=re%m_q;
	if (Jacobi(cp, m_p) * Jacobi(cq, m_q) != 1)
	{
		cp = cp.IsOdd() ? (cp+m_p) >> 1 : cp >> 1;
		cq = cq.IsOdd() ? (cq+m_q) >> 1 : cq >> 1;
	}

	#pragma omp parallel
		#pragma omp sections
		{
			#pragma omp section
				cp = ModularSquareRoot(cp, m_p);
			#pragma omp section
				cq = ModularSquareRoot(cq, m_q);
		}

	Integer y = CRT(cq, m_q, cp, m_p, m_u);
	y = modn.Multiply(y, rInv);
	y = STDMIN(y, m_n-y);
	if (ApplyFunction(y) != x)
		throw Exception(Exception::OTHER_ERROR, "InvertibleRWFunction: computational error during private key operation");
	return y;
}

bool InvertibleRWFunction::Validate(RandomNumberGenerator &rng, unsigned int level) const
{
	bool pass = RWFunction::Validate(rng, level);
	pass = pass && m_p > Integer::One() && m_p%8 == 3 && m_p < m_n;
	pass = pass && m_q > Integer::One() && m_q%8 == 7 && m_q < m_n;
	pass = pass && m_u.IsPositive() && m_u < m_p;
	if (level >= 1)
	{
		pass = pass && m_p * m_q == m_n;
		pass = pass && m_u * m_q % m_p == 1;
	}
	if (level >= 2)
		pass = pass && VerifyPrime(rng, m_p, level-2) && VerifyPrime(rng, m_q, level-2);
	return pass;
}

bool InvertibleRWFunction::GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
{
	return GetValueHelper<RWFunction>(this, name, valueType, pValue).Assignable()
		CRYPTOPP_GET_FUNCTION_ENTRY(Prime1)
		CRYPTOPP_GET_FUNCTION_ENTRY(Prime2)
		CRYPTOPP_GET_FUNCTION_ENTRY(MultiplicativeInverseOfPrime2ModPrime1)
		;
}

void InvertibleRWFunction::AssignFrom(const NameValuePairs &source)
{
	AssignFromHelper<RWFunction>(this, source)
		CRYPTOPP_SET_FUNCTION_ENTRY(Prime1)
		CRYPTOPP_SET_FUNCTION_ENTRY(Prime2)
		CRYPTOPP_SET_FUNCTION_ENTRY(MultiplicativeInverseOfPrime2ModPrime1)
		;
}

NAMESPACE_END

#endif