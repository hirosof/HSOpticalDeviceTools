#pragma once
#include "HSHashMessageBlockBufferingEngine.hpp"
#include <memory>

BEGIN_HSHASH_NAMESPACE

namespace Base {

	template <size_t MessageBlockSize , typename MessageSizeType ,typename HashValueType> 
	using CSHABase = CHashMessageBlockBufferingEngine<MessageBlockSize, MessageSizeType, HashValueType>;
	
	template <typename HashValueType> class CSHABase32BitUnit : public  CSHABase<64, uint64_t, HashValueType> {
	public:

		virtual bool Finalize (void) {
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
				this->m_MessageBlock[63 - i] = (finalDataBitsSize >> (8 * i)) & 0xFF;
			}

			this->BlockProcess ();

			this->State = EComputeState::Finalized;

			return true;
		}
	};

	template <typename HashValueType> class CSHABase64BitUnit : public  CSHABase<128, uint64_t, HashValueType> {
	public:

		virtual bool Finalize (void) {
			if (this->State != EComputeState::Updatable) return false;

			uint64_t finalDataSize = this->m_AllMessageSize + this->m_MessageAddPosition;
			uint64_t finalDataBitsSize = finalDataSize * 8;

			this->m_MessageBlock[this->m_MessageAddPosition] = 0x80;

			memset (&this->m_MessageBlock[this->m_MessageAddPosition + 1], 0, 128 - (this->m_MessageAddPosition + 1));

			//0x80をセットした位置が896ビット目(112バイト目) 
			//以上であればハッシュブロックを実行する
			if (this->m_MessageAddPosition >= 112) {
				size_t targetSize = this->m_MessageAddPosition;
				this->BlockProcess ();
				memset (this->m_MessageBlock, 0, targetSize);
			}

			for (size_t i = 0; i < 8; i++) {
				this->m_MessageBlock[127 - i] = (finalDataBitsSize >> (8 * i)) & 0xFF;
			}

			this->BlockProcess ();

			this->State = EComputeState::Finalized;
			return true;
		}
	};
}


END_HSHASH_NAMESPACE