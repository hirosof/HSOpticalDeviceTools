#pragma once
#include "HSSHABase.hpp"

BEGIN_HSHASH_NAMESPACE

namespace Base {

	template <typename HashValueType> class CSHA2_256Base : public CSHABase32BitUnit<HashValueType> {
	protected:
		uint32_t m_HashBlockData[8];
	private:

		const uint32_t (&m_DefaultHash)[8];

		static const  uint32_t ConstantData[64];
		
		static uint32_t Rotate (uint32_t x, int bit) {
			return Functions::RightRotate<uint32_t> (x, bit);
		}


		static uint32_t ShiftCalc (int mode, uint32_t x) {
			switch (mode) {
			case 0:
				return CSHA2_256Base::Rotate (x, 7) ^ CSHA2_256Base::Rotate (x, 18) ^ (x >> 3);
			case 1:
				return CSHA2_256Base::Rotate (x, 17) ^ CSHA2_256Base::Rotate (x, 19) ^ (x >> 10);

			case 2:
				return CSHA2_256Base::Rotate (x, 2) ^ CSHA2_256Base::Rotate (x, 13) ^ CSHA2_256Base::Rotate (x, 22);

			case 3:
				return CSHA2_256Base::Rotate (x, 6) ^ CSHA2_256Base::Rotate (x, 11) ^ CSHA2_256Base::Rotate (x, 25);
			}
			return 0;
		}

		static uint32_t LogicalExp (int mode, uint32_t x, uint32_t y, uint32_t z) {
			if (mode) return (x&y) ^ (x&z) ^ (y&z);
			else 	return (x & y) ^ (~x & z);
		}


		virtual void BlockProcess (void) {

			unsigned __int32 md[8];		//メッセージダイジェストに加算する値
			unsigned __int32 Data[64];	//ハッシュブロックデータ

			//入力データを16ブロックに分割
			for (int i = 0, j = 0; i < 16; i++, j += 4) {
				Data[i] = this->m_MessageBlock[j] << 24;
				Data[i] |= this->m_MessageBlock[j + 1] << 16;
				Data[i] |= this->m_MessageBlock[j + 2] << 8;
				Data[i] |= this->m_MessageBlock[j + 3];
			}

			//Data[16]～Data[63]まで準備
			for (int i = 16; i <= 63; i++) {
				Data[i] = this->ShiftCalc (1, Data[i - 2]) + this->ShiftCalc (0, Data[i - 15]) + Data[i - 7] + Data[i - 16];
			}

			//メッセージダイジェストをコピー
			memcpy (md, this->m_HashBlockData, sizeof (md));

			//Data[0] ～ Data[63]まで使用してメッセージダイジェストに加算する値を算出
			for (int i = 0; i <= 63; i++) {
				unsigned __int32 temp[2];
				temp[0] = this->ShiftCalc (3, md[4]) + this->LogicalExp (0, md[4], md[5], md[6]);
				temp[0] += md[7] + this->ConstantData[i] + Data[i];
				temp[1] = this->ShiftCalc (2, md[0]) + this->LogicalExp (1, md[0], md[1], md[2]);

				md[7] = md[6];
				md[6] = md[5];
				md[5] = md[4];
				md[4] = md[3] + temp[0];
				md[3] = md[2];
				md[2] = md[1];
				md[1] = md[0];
				md[0] = temp[0] + temp[1];
			}

			//メッセージダイジェストに加算
			for (int i = 0; i < 8; i++)
				this->m_HashBlockData[i] += md[i];
		}

	public:
		CSHA2_256Base (const uint32_t (&DefaultHash)[8]) : m_DefaultHash(DefaultHash){
			this->Reset ();
		}


		virtual void Reset (void)
		{
			Base::CSHABase32BitUnit<HashValueType>::Reset ();
			memcpy (this->m_HashBlockData, this->m_DefaultHash, sizeof (m_DefaultHash));
			this->State = EComputeState::Updatable;
		}

		virtual bool GetIntermediateHash (HashValueType *pHash) {
			if (pHash == nullptr) return false;
			if (this->IsFilaziled ()) return this->GetHash (pHash);

			//親クラスの型
			using Parent = Base::CSHABase32BitUnit<HashValueType>;

			//ハッシュブロックデータのバックアップ
			uint32_t backup_HashBlockData[8];
			memcpy (backup_HashBlockData, this->m_HashBlockData, sizeof (this->m_HashBlockData));

			//中間ハッシュの取得
			bool bRet = Parent::GetIntermediateHash (pHash);

			//ハッシュブロックデータの復元
			memcpy (this->m_HashBlockData, backup_HashBlockData, sizeof (this->m_HashBlockData));

			return bRet;
		}
	};

	template <typename HashValueType> class CSHA2_512Base : public CSHABase64BitUnit<HashValueType> {
	protected:
		uint64_t m_HashBlockData[8];
	private:

		const uint64_t (&m_DefaultHash)[8];

		static const uint64_t ConstantData[80];

		static uint64_t Rotate (uint64_t x, uint32_t y) {
			return Functions::RightRotate<uint64_t> (x, y);
		}

		static uint64_t Ch (uint64_t x, uint64_t y, uint64_t z) {
			return (x&y) ^ ((~x) & z);
		}

		static uint64_t Maj (uint64_t x, uint64_t y, uint64_t z) {
			return (x &y) ^ (x&z) ^ (y&z);
		}

		static uint64_t LargeSigma0 (uint64_t x) {
			return CSHA2_512Base::Rotate (x, 28) ^ CSHA2_512Base::Rotate (x, 34) ^ CSHA2_512Base::Rotate (x, 39);
		}

		static uint64_t LargeSigma1 (uint64_t x) {
			return CSHA2_512Base::Rotate (x, 14) ^ CSHA2_512Base::Rotate (x, 18) ^ CSHA2_512Base::Rotate (x, 41);
		}

		static uint64_t SmallSigma0 (uint64_t x) {
			return CSHA2_512Base::Rotate (x, 1) ^ CSHA2_512Base::Rotate (x, 8) ^ (x >> 7);
		}

		static uint64_t SmallSigma1 (uint64_t x) {
			return CSHA2_512Base::Rotate (x, 19) ^ CSHA2_512Base::Rotate (x, 61) ^ (x >> 6);
		}

		static uint64_t ByteReverse (uint64_t x) {
			uint64_t reverseVal = 0;

			uint8_t *pInput = reinterpret_cast<uint8_t*>(&x);
			uint8_t *pOutput = reinterpret_cast<uint8_t*>(&reverseVal);

			for (int i = 0; i < 8; i++) {
				*(pOutput + i) = *(pInput + (7 - i));
			}

			return reverseVal;
		}

		virtual void BlockProcess (void) {

			uint64_t  AddHash[8];
			memcpy (AddHash, this->m_HashBlockData, 64);

			uint64_t W[80];

			memcpy (W, this->m_MessageBlock, 128); // uint64_tのサイズ(8バイト) * 16要素 = 128バイト

			for (uint32_t i = 0; i < 16; i++) {
				W[i] = this->ByteReverse (W[i]);
			}

			for (uint32_t t = 16; t <= 79; t++) {
				W[t] = this->SmallSigma1 (W[t - 2]) + W[t - 7] + SmallSigma0 (W[t - 15]) + W[t - 16];
			}

			uint64_t LargeT[2];
			for (uint32_t t = 0; t <= 79; t++) {

				LargeT[0] = AddHash[7] + this->LargeSigma1 (AddHash[4]);
				LargeT[0] += this->Ch (AddHash[4], AddHash[5], AddHash[6]) + this->ConstantData[t] + W[t];

				LargeT[1] = this->LargeSigma0 (AddHash[0]) + this->Maj (AddHash[0], AddHash[1], AddHash[2]);

				AddHash[7] = AddHash[6];
				AddHash[6] = AddHash[5];
				AddHash[5] = AddHash[4];

				AddHash[4] = AddHash[3] + LargeT[0];

				AddHash[3] = AddHash[2];
				AddHash[2] = AddHash[1];
				AddHash[1] = AddHash[0];

				AddHash[0] = LargeT[0] + LargeT[1];
			}

			for (uint32_t i = 0; i < 8; i++) {
				this->m_HashBlockData[i] += AddHash[i];
			}
		}

	public:

		CSHA2_512Base (const uint64_t (&DefaultHash)[8]) : m_DefaultHash (DefaultHash) {
			this->Reset ();
		}

		virtual void Reset (void)
		{
			Base::CSHABase64BitUnit<HashValueType>::Reset ();
			memcpy (this->m_HashBlockData, this->m_DefaultHash, sizeof (m_DefaultHash));
			this->State = EComputeState::Updatable;
		}

		virtual bool GetIntermediateHash (HashValueType *pHash) {
			if (pHash == nullptr) return false;
			if (this->IsFilaziled ()) return this->GetHash (pHash);

			//親クラスの型
			using Parent = Base::CSHABase64BitUnit<HashValueType>;

			//ハッシュブロックデータのバックアップ
			uint64_t backup_HashBlockData[8];
			memcpy (backup_HashBlockData, this->m_HashBlockData, sizeof (this->m_HashBlockData));

			//中間ハッシュの取得
			bool bRet = Parent::GetIntermediateHash (pHash);

			//ハッシュブロックデータの復元
			memcpy (this->m_HashBlockData, backup_HashBlockData, sizeof (this->m_HashBlockData));

			return bRet;
		}

	};

}

using CSHA224Value = Base::CHashValueBase<uint32_t, 7>;
class CSHA224 : public Base::CSHA2_256Base<CSHA224Value> {
private:
	static const uint32_t DefaultHash[8];
public:
	CSHA224 ();
	virtual bool GetHash (HashValueType *pHash) const;
};

using CSHA256Value = Base::CHashValueBase<uint32_t, 8>;
class CSHA256 : public Base::CSHA2_256Base<CSHA256Value> {
private:
	static const uint32_t DefaultHash[8];
public:
	CSHA256 ();
	virtual bool GetHash (HashValueType *pHash) const;
};

using CSHA384Value = Base::CHashValueBase<uint64_t, 6>;
class CSHA384 : public  Base::CSHA2_512Base<CSHA384Value> {
private:
	static const uint64_t DefaultHash[8];
public:
	CSHA384 ();
	virtual bool GetHash (HashValueType *pHash) const;
};

using CSHA512Value = Base::CHashValueBase<uint64_t, 8>;
class CSHA512 : public  Base::CSHA2_512Base<CSHA512Value> {
private:
	static const uint64_t DefaultHash[8];
public:
	CSHA512 ();
	virtual bool GetHash (HashValueType *pHash) const;
};

using CSHA512Per224Value = Base::CHashValueBase<uint64_t, 4, EHashValueEndian::Little ,sizeof (uint32_t)>;
class CSHA512Per224 : public Base::CSHA2_512Base< CSHA512Per224Value> {
private:
	static const uint64_t DefaultHash[8];
public:
	CSHA512Per224 ();
	virtual bool GetHash (HashValueType *pHash) const;
};

using CSHA512Per256Value = Base::CHashValueBase<uint64_t, 4>;
class CSHA512Per256 : public Base::CSHA2_512Base< CSHA512Per256Value> {
private:
	static const uint64_t DefaultHash[8];
public:
	CSHA512Per256 ();
	virtual bool GetHash (HashValueType *pHash) const;
};

END_HSHASH_NAMESPACE