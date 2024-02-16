#include "HSHashMD5.hpp"

const uint32_t hirosof::Hash::CMD5::m_DefaultHash[4] = {
	0x67452301,	0xefcdab89,	0x98badcfe,	0x10325476
};


const uint32_t hirosof::Hash::CMD5::ConstantTable[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};



uint32_t hirosof::Hash::CMD5::Logic (uint32_t x, uint32_t y, uint32_t z, LogicType type) {
	switch (type) {
		case CMD5::LogicType::F:
			return  (x & y) | ((~x) & z);
		case CMD5::LogicType::G:
			return (x & z) | (y &(~z));
		case CMD5::LogicType::H:
			return x ^ y ^ z;
		case CMD5::LogicType::I:
			return y ^ (x | (~z));
	}
	return 0;
}

uint32_t hirosof::Hash::CMD5::RotateU32 (uint32_t x, uint32_t s) {
	return Functions::LeftRotate (x, s);
}

uint32_t hirosof::Hash::CMD5::Operation (uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x_k, uint32_t s, uint32_t i, LogicType type) {
	uint32_t calc = a + x_k + CMD5::ConstantTable[i] + CMD5::Logic (b, c, d, type);
	return CMD5::RotateU32 (calc, s) + b;
}

void hirosof::Hash::CMD5::BlockProcess (void) {
	uint32_t baseDigest[4];

	memcpy (baseDigest, this->m_HashBlockData, sizeof (baseDigest));

	uint32_t input[16];

	memcpy (input, this->m_MessageBlock, 64);

	const uint32_t s_table[4][4] = {
		{7,12,17,22},
		{5,9,14,20},
		{4,11,16,23},
		{6,10,15,21}
	};

	const uint32_t k_first_table[4] = { 0,1,5,0 };
	const uint32_t k_add_table[4] = { 1,5,3,7 };
	const LogicType logicTypes[4] = { LogicType::F , LogicType::G , LogicType::H , LogicType::I };

	for (uint32_t rid = 0; rid < 4; rid++) {
		for (uint32_t oid = 0; oid < 16; oid++) {
			uint32_t rotationId = oid % 4;
			uint32_t digestIndex = (4 - rotationId) % 4;

			uint32_t k = (k_first_table[rid] + k_add_table[rid] * oid) % 16;
			uint32_t s = s_table[rid][rotationId];
			uint32_t i = oid + 16 * rid;

			this->m_HashBlockData[digestIndex] = this->Operation (
				this->m_HashBlockData[digestIndex],
				this->m_HashBlockData[(digestIndex + 1) % 4],
				this->m_HashBlockData[(digestIndex + 2) % 4],
				this->m_HashBlockData[(digestIndex + 3) % 4],
				input[k], s, i, logicTypes[rid]
			);
		}
	}

	for (uint32_t i = 0; i < 4; i++) {
		this->m_HashBlockData[i] += baseDigest[i];
	}

}

hirosof::Hash::CMD5::CMD5 () {
	Reset ();
}

void hirosof::Hash::CMD5::Reset (void) {
	Parent::Reset ();
	memcpy (this->m_HashBlockData, this->m_DefaultHash, sizeof (m_DefaultHash));
	this->State = EComputeState::Updatable;
}

bool hirosof::Hash::CMD5::Finalize (void) {
	if (this->State != EComputeState::Updatable) return false;

	uint64_t finalDataSize = this->m_AllMessageSize + this->m_MessageAddPosition;
	uint64_t finalDataBitsSize = finalDataSize * 8;

	this->m_MessageBlock[this->m_MessageAddPosition] = 0x80;

	memset (&this->m_MessageBlock[this->m_MessageAddPosition + 1], 0, 64 - (this->m_MessageAddPosition + 1));

	//0x80をセットした位置が448ビット目(56バイト目) 
	//以上であればハッシュブロックを実行する
	if (this->m_MessageAddPosition >= 56) {
		this->BlockProcess ();
		memset (this->m_MessageBlock, 0, 56);
	}

	for (size_t i = 0; i < 8; i++) {
		this->m_MessageBlock[56 + i] = (finalDataBitsSize >> (8 * i)) & 0xFF;
	}

	this->BlockProcess ();

	this->State = EComputeState::Finalized;

	return true;
}

bool hirosof::Hash::CMD5::GetHash (HashValueType * pHash) const {
	if (this->State != EComputeState::Finalized) return false;

	if (pHash != nullptr) {
		*pHash = HashValueType (this->m_HashBlockData);
		return true;
	}

	return false;
}

bool hirosof::Hash::CMD5::GetIntermediateHash (HashValueType * pHash) {
	if (pHash == nullptr) return false;
	if (this->IsFilaziled ()) return this->GetHash (pHash);

	//ハッシュブロックデータのバックアップ
	uint32_t backup_HashBlockData[4];
	memcpy (backup_HashBlockData, this->m_HashBlockData, sizeof (this->m_HashBlockData));

	//中間ハッシュの取得
	bool bRet = Parent::GetIntermediateHash (pHash);

	//ハッシュブロックデータの復元
	memcpy (this->m_HashBlockData, backup_HashBlockData, sizeof (this->m_HashBlockData));

	return bRet;
}
