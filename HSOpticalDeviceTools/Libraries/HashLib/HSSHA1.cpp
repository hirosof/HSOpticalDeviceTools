#include "HSSHA1.hpp"

const uint32_t hirosof::Hash::CSHA1::m_DefaultHash[5] = {
	0x67452301 , 0xefcdab89 , 0x98badcfe , 0x10325476 , 0xc3d2e1f0
};

uint32_t hirosof::Hash::CSHA1::Rotate (uint32_t x, int32_t bit)
{
	return Functions::LeftRotate<uint32_t> (x, bit);
}

uint32_t hirosof::Hash::CSHA1::GetConstData (int32_t constant)
{
	if (constant <= 19)return 0x5a827999;
	else if (constant <= 39)return 0x6ed9eba1;
	else if (constant <= 59)return 0x8f1bbcdc;
	else if (constant <= 79)return 0xca62c1d6;
	return 0;
}

uint32_t hirosof::Hash::CSHA1::LogicalExp (int32_t constant, uint32_t x, uint32_t y, uint32_t z)
{
	if (constant <= 19)return (x&y) | (~x&z);
	else if (constant <= 39)return x ^ y^z;
	else if (constant <= 59)return (x&y) | (x&z) | (y&z);
	else if (constant <= 79)return x ^ y^z;
	return 0;
}

void hirosof::Hash::CSHA1::BlockProcess (void)
{
	unsigned __int32 md[5];		//メッセージダイジェストに加算する値
	unsigned __int32 Data[80];	//ハッシュブロックデータ

	//入力データを16ブロックに分割
	for (int i = 0, j = 0; i < 16; i++, j += 4) {
		Data[i] = this->m_MessageBlock[j] << 24;
		Data[i] |= this->m_MessageBlock[j + 1] << 16;
		Data[i] |= this->m_MessageBlock[j + 2] << 8;
		Data[i] |= this->m_MessageBlock[j + 3];
	}

	//Data[16]～Data[79]までローテート関数を使ってデータ準備
	for (int i = 16; i <= 79; i++) {
		unsigned __int32  rotbase;
		rotbase = Data[i - 3] ^ Data[i - 8] ^ Data[i - 14] ^ Data[i - 16];
		Data[i] = this->Rotate (rotbase, 1);
	}

	memcpy (md, this->m_HashBlockData, sizeof (md));

	//Data[0] ～ Data[79]まで使用してメッセージダイジェストに加算する値を算出
	for (int i = 0; i <= 79; i++) {
		unsigned __int32 temp;
		temp = this->Rotate (md[0], 5) + this->LogicalExp (i, md[1], md[2], md[3]) + md[4];
		temp += Data[i] + this->GetConstData (i);
		md[4] = md[3];
		md[3] = md[2];
		md[2] = this->Rotate (md[1], 30);
		md[1] = md[0];
		md[0] = temp;
	}

	//メッセージダイジェストに加算
	for (int i = 0; i < 5; i++)
		this->m_HashBlockData[i] += md[i];
}

hirosof::Hash::CSHA1::CSHA1 ()
{
	this->Reset ();
}

void hirosof::Hash::CSHA1::Reset (void)
{
	Base::CSHABase32BitUnit<CSHA1Value>::Reset ();
	memcpy (this->m_HashBlockData, this->m_DefaultHash, sizeof (m_DefaultHash));
	this->State = EComputeState::Updatable;
}

bool hirosof::Hash::CSHA1::GetHash (HashValueType * pHash) const
{
	if (this->State != EComputeState::Finalized) return false;

	if (pHash != nullptr) {
		*pHash = HashValueType (this->m_HashBlockData);
		return true;
	}

	return false;
}

bool hirosof::Hash::CSHA1::GetIntermediateHash (HashValueType * pHash) {
	if (pHash == nullptr) return false;
	if (this->IsFilaziled ()) return this->GetHash (pHash);

	//親クラスの型
	using Parent = Base::CSHABase32BitUnit<CSHA1Value>;

	//ハッシュブロックデータのバックアップ
	uint32_t backup_HashBlockData[5];
	memcpy (backup_HashBlockData, this->m_HashBlockData, sizeof (this->m_HashBlockData));

	//中間ハッシュの取得
	bool bRet = Parent::GetIntermediateHash (pHash);

	//ハッシュブロックデータの復元
	memcpy (this->m_HashBlockData, backup_HashBlockData, sizeof (this->m_HashBlockData));

	return bRet;
}

