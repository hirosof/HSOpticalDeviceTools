#pragma once

#include "HSSHABase.hpp"

BEGIN_HSHASH_NAMESPACE

using CSHA1Value = Base::CHashValueBase<uint32_t, 5>;
class CSHA1 : public Base::CSHABase32BitUnit<CSHA1Value> {
private:
	static const uint32_t m_DefaultHash[5];
	uint32_t m_HashBlockData[5];
	
	uint32_t Rotate (uint32_t x, int32_t bit);
	uint32_t GetConstData (int32_t constant);
	uint32_t LogicalExp (int32_t constant, uint32_t x, uint32_t y, uint32_t z);

	virtual void BlockProcess (void);
public:
	CSHA1 ();
	virtual void Reset (void);
	virtual bool GetHash (HashValueType *pHash) const;
	virtual bool GetIntermediateHash (HashValueType *pHash);
};

END_HSHASH_NAMESPACE