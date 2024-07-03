#pragma once
#include "HSHashMessageBlockBufferingEngine.hpp"
#include <memory>

BEGIN_HSHASH_NAMESPACE

using CMD5Value = Base::CHashValueBase<uint32_t, 4, EHashValueEndian::Big>;

class CMD5 : public Base::CHashMessageBlockBufferingEngine<64, uint64_t, CMD5Value> {

private:
	using Parent = Base::CHashMessageBlockBufferingEngine<64, uint64_t, CMD5Value>;

	static const uint32_t m_DefaultHash[4];
	uint32_t m_HashBlockData[4];

	enum struct LogicType {
		F = 0,
		G,
		H,
		I
	};

	static const uint32_t ConstantTable[64];

	static uint32_t Logic (uint32_t x, uint32_t y, uint32_t z, LogicType  type);

	static  uint32_t RotateU32 (uint32_t x, uint32_t s);

	static uint32_t Operation (uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x_k, uint32_t s, uint32_t i, LogicType type);


	virtual void BlockProcess (void);

public:
	CMD5 ();
	virtual void Reset (void);
	virtual bool Finalize (void);

	virtual bool GetHash (HashValueType *pHash) const;

	virtual bool GetIntermediateHash (HashValueType *pHash);

};






END_HSHASH_NAMESPACE