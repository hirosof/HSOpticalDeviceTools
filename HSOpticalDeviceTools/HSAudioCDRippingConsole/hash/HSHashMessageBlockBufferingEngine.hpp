#pragma once
#include "HSHashBase.hpp"
#include <memory>

BEGIN_HSHASH_NAMESPACE

namespace Base {


	template <size_t MessageBlockSize, typename MessageSizeType, typename HashValueType>
	class CHashMessageBlockBufferingEngine : public CHashBaseWithMessageBlock<MessageBlockSize, MessageSizeType, HashValueType> {
	private:
		using BaseType = CHashBaseWithMessageBlock<MessageBlockSize, MessageSizeType, HashValueType>;
	public:


		virtual bool Update (const void *pData, uint64_t dataSize) {
			if (this->State != EComputeState::Updatable) return false;
			if (pData == nullptr) return false;
			if (dataSize == 0)return false;

			const uint8_t *lpBytesData = static_cast<const uint8_t*>(pData);

			if (this->m_MessageAddPosition + dataSize >= MessageBlockSize) {

				size_t addfirstSize = MessageBlockSize - this->m_MessageAddPosition;

				memcpy (&this->m_MessageBlock[this->m_MessageAddPosition], lpBytesData, addfirstSize);

				this->MessageBufferProcess ();
				uint64_t NumBlock = (dataSize - addfirstSize) / MessageBlockSize;
				size_t NumRest = (dataSize - addfirstSize) % MessageBlockSize;
				const uint8_t *lpRestBytesData = lpBytesData + addfirstSize;

				for (uint64_t i = 0; i < NumBlock; i++) {
					memcpy (&this->m_MessageBlock[0], lpRestBytesData + MessageBlockSize * i, MessageBlockSize);
					this->MessageBufferProcess ();
				}
				memcpy (&this->m_MessageBlock[0], lpRestBytesData + MessageBlockSize * NumBlock, NumRest);
			} else {
				memcpy (&this->m_MessageBlock[this->m_MessageAddPosition], pData, static_cast<size_t>(dataSize));
			}
			this->m_MessageAddPosition = (this->m_MessageAddPosition + dataSize) % MessageBlockSize;

			return true;
		}

		virtual bool Update (const char *pString) {
			return BaseType::Update (pString);
		}
		virtual bool Update (const wchar_t *pString) {
			return BaseType::Update (pString);
		}

	};



}


END_HSHASH_NAMESPACE