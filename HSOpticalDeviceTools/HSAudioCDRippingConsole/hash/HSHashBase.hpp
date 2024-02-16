#pragma once

#include "HSHashValue.hpp"

BEGIN_HSHASH_NAMESPACE

namespace Base {


	template <typename tnHashValueType>  class CHashBase {
	protected:

		EComputeState State;

	public:

		using HashValueType = tnHashValueType;

		virtual bool IsUpdatable (void) const {
			return State == EComputeState::Updatable;
		}

		virtual bool IsFilaziled (void) const {
			return State == EComputeState::Finalized;
		}

		virtual void Reset (void) = 0;


		virtual bool Update (const void *pData, uint64_t dataSize) {
			return false;
		}

		virtual bool Update (const char *pString) {
			if (pString == nullptr) return false;
			return this->Update (pString, strlen (pString) * sizeof (char));
		}

		virtual bool Update (const wchar_t *pString) {
			if (pString == nullptr) return false;
			return this->Update (pString, wcslen (pString) * sizeof (wchar_t));
		}

		virtual bool Finalize (void) = 0;

		virtual bool GetHash (HashValueType *pHash) const = 0;

		virtual bool GetIntermediateHash (HashValueType *pHash) = 0;

		virtual bool Compute (const void *pData, uint64_t dataSize) {
			if (pData == nullptr) return false;
			if (dataSize > 0) {
				if (this->Update (pData, dataSize) == false) {
					return false;
				}
			}
			return this->Finalize ();
		}

		virtual bool Compute (const char *pString) {
			if (pString == nullptr) return false;
			return this->Compute (pString, strlen (pString) * sizeof (char));
		}

		virtual bool Compute (const wchar_t *pString) {
			if (pString == nullptr) return false;
			return this->Compute (pString, wcslen (pString) * sizeof (wchar_t));
		}

		template <typename ElementType> bool Put (ElementType value) {
			return this->Update (&value, sizeof (ElementType));
		}

		template <typename ElementType, size_t NumberOfElements> bool ArrayPut (ElementType (&values)[NumberOfElements]) {
			return this->Update (&values, sizeof (ElementType) *NumberOfElements);
		}
		template <typename ElementType> bool ArrayPut (ElementType *pElement, size_t NumberOfElements) {
			return this->Update (pElement, sizeof (ElementType) *NumberOfElements);
		}
	};

	template <size_t MessageBlockSize, typename MessageSizeType, typename HashValueType>
	class CHashBaseWithMessageBlock : public CHashBase< HashValueType> {
	
	protected:
		uint8_t m_MessageBlock[MessageBlockSize];
		MessageSizeType  m_AllMessageSize;
		size_t m_MessageAddPosition;

		virtual void BlockProcess (void) = 0;
		virtual void MessageBufferProcess (void) {
			BlockProcess ();
			m_AllMessageSize += MessageBlockSize;
		}

	public:

		static const size_t m_MessageBlockSize = MessageBlockSize;

		CHashBaseWithMessageBlock () {
			Reset ();
		}

		virtual void Reset (void) {
			m_MessageAddPosition = 0;
			m_AllMessageSize = 0;
		}


		virtual MessageSizeType GetCurrentMessageSize (void) const {
			return this->m_AllMessageSize;
		}

		virtual bool GetIntermediateHash (HashValueType *pHash) {
			if (pHash == nullptr) return false;
			if (this->IsFilaziled ()) return this->GetHash (pHash);
			
			bool bRet = false;

			//現在の状態をバックアップする
			uint8_t backup_MessageBlock[MessageBlockSize];
			MessageSizeType  backup_AllMessageSize;
			size_t backup_MessageAddPosition;
			memcpy (backup_MessageBlock, this->m_MessageBlock, MessageBlockSize);
			backup_AllMessageSize = this->m_AllMessageSize;
			backup_MessageAddPosition = this->m_MessageAddPosition;

			//ファイナライズをしてハッシュを取得する
			if (this->Finalize ()) {
				bRet = this->GetHash (pHash);
			}


			//バックアップから状態を復元する
			memcpy (this->m_MessageBlock,backup_MessageBlock,  MessageBlockSize);
			this->m_AllMessageSize = backup_AllMessageSize;
			this->m_MessageAddPosition = backup_MessageAddPosition;
			this->State = EComputeState::Updatable;

			return bRet;
		}


	};
}

END_HSHASH_NAMESPACE