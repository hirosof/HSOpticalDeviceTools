#pragma once

#include "HSSHA1.hpp"
#include "HSSHA2.hpp"
#include "HSHashMD5.hpp"
BEGIN_HSHASH_NAMESPACE

namespace HMAC {

	template <typename tnHashAlgorithm> class CHMACKey {
	private:
		uint8_t m_key[tnHashAlgorithm::m_MessageBlockSize];
	public:

		using HashAlgorithm = tnHashAlgorithm;
		static const size_t m_KeySize = HashAlgorithm::m_MessageBlockSize;

		CHMACKey () {
			for (size_t i = 0; i < m_KeySize; i++)
			{
				m_key[i] = 0;
			}
		}

		CHMACKey (const uint8_t (&key)[m_KeySize]){
			for (size_t i = 0; i < m_KeySize; i++)
			{
				m_key[i] = key[i];
			}
		}

		CHMACKey& operator=(const CHMACKey& key) {
			for (size_t i = 0; i < m_KeySize; i++) {
				m_key[i] = key.m_key[i];
			}
			return *this;
		}

		static size_t Count (void) {
			return m_KeySize;
		}

		uint8_t GetValue (size_t index) const {
			if (index >= m_KeySize) throw Exception::COutOfRangeExceptionSizeT (index, 0, m_KeySize - 1);
			return this->m_key[index];
		}

		const uint8_t operator[](size_t index) const {
			return GetValue (index);
		}

	};

	template <typename tnHashAlgorithm> class CHMACKeyBuilder {
	public:
		using HashAlgorithm = tnHashAlgorithm;
		using KeyType = CHMACKey <HashAlgorithm>;
	private:
		uint8_t m_keyBuffer[tnHashAlgorithm::m_MessageBlockSize];
		uint64_t m_KeySaltSize;
		HashAlgorithm m_hash;
		EComputeState State;
	public:


		CHMACKeyBuilder () {
			this->Reset ();
		}

		void Reset (void) {
			m_KeySaltSize = 0;
			m_hash.Reset ();
			State = EComputeState::Updatable;
		}

		bool IsUpdatable (void) const {
			return State == EComputeState::Updatable;
		}

		bool IsFilaziled (void) const {
			return State == EComputeState::Finalized;
		}

		bool Update (const void *pData, uint64_t dataSize) {
			if (State != EComputeState::Updatable) return false;
			if (pData == nullptr)return false;
			if (dataSize == 0)return false;

			const uint8_t *lpBytesData = static_cast<const uint8_t*>(pData);

			if (m_KeySaltSize > HashAlgorithm::m_MessageBlockSize) {
				m_hash.Update (pData, dataSize);
			} else  if(m_KeySaltSize + dataSize > HashAlgorithm::m_MessageBlockSize){
				m_hash.Update (m_keyBuffer, m_KeySaltSize);
				m_hash.Update (lpBytesData, dataSize);
			} else {
				memcpy (&this->m_keyBuffer[this->m_KeySaltSize], lpBytesData, static_cast<size_t>(dataSize));
			}

			this->m_KeySaltSize += dataSize;

			return true;
		}

		bool Update (const char *pString) {
			if (pString == nullptr) return false;
			return this->Update (pString, strlen (pString) * sizeof (char));
		}

		bool Update (const wchar_t *pString) {
			if (pString == nullptr) return false;
			return this->Update (pString, wcslen (pString) * sizeof (wchar_t));
		}

		bool Compute (const void *pData, uint64_t dataSize) {
			if (pData == nullptr) return false;
			if (dataSize > 0) {
				if (this->Update (pData, dataSize) == false) {
					return false;
				}
			}
			return this->Finalize ();
		}

		bool Compute (const char *pString) {
			if (pString == nullptr) return false;
			return this->Compute (pString, strlen (pString) * sizeof (char));
		}

		bool Compute (const wchar_t *pString) {
			if (pString == nullptr) return false;
			return this->Compute (pString, wcslen (pString) * sizeof (wchar_t));
		}

		bool Finalize (void) {
			if (State != EComputeState::Updatable) return false;

			if (m_KeySaltSize <= HashAlgorithm::m_MessageBlockSize) {
				memset (&this->m_keyBuffer[this->m_KeySaltSize], 0, 
					static_cast<size_t>(HashAlgorithm::m_MessageBlockSize - m_KeySaltSize));
			} else {
				if (m_hash.Finalize () == false) return false;

				typename HashAlgorithm::HashValueType value;

				if (m_hash.GetHash (&value) == false) return false;

				memset (&this->m_keyBuffer[0], 0, HashAlgorithm::m_MessageBlockSize);

				for (size_t i = 0; i < value.Count(); i++)
				{
					this->m_keyBuffer[i] = value[i];
				}
			}

			State = EComputeState::Finalized;

			return true;
		}

		bool GetKey (KeyType *pKey) const {
			if (this->State != EComputeState::Finalized) return false;
			if (pKey != nullptr) {
				*pKey = KeyType (this->m_keyBuffer);
				return true;
			}
			return false;
		};

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
	
	template <typename tnHashAlgorithm> class CHMAC : public Base::CHashBase<typename tnHashAlgorithm::HashValueType> {
	public:
		using HashAlgorithm = tnHashAlgorithm;
		using KeyType = CHMACKey <HashAlgorithm>;
		using KeyBuilder = CHMACKeyBuilder<HashAlgorithm>;
	private:
		KeyType m_key;
		HashAlgorithm  m_ihash;
		HashAlgorithm  m_ohash;
	public:

		CHMAC () {
			Reset ();
		}

		CHMAC (const KeyBuilder &keybuilder) {
			this->ResetWithChangeKey (keybuilder);
		}

		CHMAC (const KeyType &key) {
			this->ResetWithChangeKey (key);
		}

		CHMAC (const void *pKeyData, uint64_t keyDataSize) {
			if (this->ResetWithChangeKey (pKeyData, keyDataSize) == false) {
				Reset ();
			}
		}

		CHMAC (const char *pKeyString) {
			if (this->ResetWithChangeKey (pKeyString) == false) {
				Reset ();
			}
		}

		CHMAC (const wchar_t *pKeyString) {
			if (this->ResetWithChangeKey (pKeyString) == false) {
				Reset ();
			}
		}

		void Reset (void) {
			m_ihash.Reset ();
			for (size_t i = 0; i < HashAlgorithm::m_MessageBlockSize; i++){
				uint8_t u = m_key[i] ^ 0x36;
				m_ihash.Update (&u , sizeof (uint8_t));
			}
			this->State = EComputeState::Updatable;
		}

		bool ResetWithChangeKey (const void *pKeyData, uint64_t keyDataSize) {
			KeyBuilder kbuilder;
			if (kbuilder.Compute (pKeyData, keyDataSize)) {
				return this->ResetWithChangeKey (kbuilder);
			}
			return false;
		}

		bool ResetWithChangeKey (const char *pKeyString) {
			KeyBuilder kbuilder;
			if (kbuilder.Compute (pKeyString)) {
				return this->ResetWithChangeKey (kbuilder);
			}
			return false;

		}

		bool ResetWithChangeKey (const wchar_t *pKeyString) {
			KeyBuilder kbuilder;
			if (kbuilder.Compute (pKeyString)) {
				return this->ResetWithChangeKey (kbuilder);
			}
			return false;

		}

		bool ResetWithChangeKey (const KeyBuilder &keybuilder) {

			KeyBuilder kbuilder (keybuilder);

			if (kbuilder.IsFilaziled () == false) {
				if (kbuilder.Finalize () == false) {
					return false;
				}
			}

			KeyType key;

			if (kbuilder.GetKey (&key)) {
				return this->ResetWithChangeKey (key);
			}

			return false;
		}

		bool ResetWithChangeKey (const KeyType &key) {
			this->m_key = key;
			this->Reset ();
			return true;
		}

		bool Update (const void *pData, uint64_t dataSize) {
			if (this->State != EComputeState::Updatable) return false;
			return this->m_ihash.Update (pData, dataSize);
		}

		bool Update (const char *pString) {
			return Base::CHashBase<typename HashAlgorithm::HashValueType>::Update (pString);
		}

		bool Update (const wchar_t *pString) {
			return Base::CHashBase<typename HashAlgorithm::HashValueType>::Update (pString);
		}

		bool Finalize (void) {
			if (this->State != EComputeState::Updatable) return false;
			
			this->m_ihash.Finalize ();

			m_ohash.Reset ();

			for (size_t i = 0; i < HashAlgorithm::m_MessageBlockSize; i++){
				uint8_t u = m_key[i] ^ 0x5c;
				m_ohash.Update (&u , sizeof (uint8_t));
			}

			typename HashAlgorithm::HashValueType ivalue;

			m_ihash.GetHash (&ivalue);

			for (size_t i = 0; i < ivalue.Count(); i++){
				uint8_t u = ivalue.GetValue (i);
				m_ohash.Update (&u, sizeof (uint8_t));
			}

			m_ohash.Finalize ();

			this->State = EComputeState::Finalized;
			return true;
		}

		bool GetHash (typename HashAlgorithm::HashValueType  *pHash) const {
			if (this->State != EComputeState::Finalized) return false;
			return this->m_ohash.GetHash (pHash);
		}

		bool GetIntermediateHash (typename HashAlgorithm::HashValueType *pHash) {
			
			if (pHash == nullptr) return false;
			if (this->IsFilaziled ()) return this->GetHash (pHash);
	
			CHMAC clone_instance (*this);

			if (clone_instance.Finalize ()) {
				return clone_instance.GetHash (pHash);
			}

			return false;
		}


	};

	using CHMAC_MD5 = CHMAC<CMD5>;
	using CHMAC_SHA1 = CHMAC<CSHA1>;
	using CHMAC_SHA224 = CHMAC<CSHA224>;
	using CHMAC_SHA256 = CHMAC<CSHA256>;
	using CHMAC_SHA384 = CHMAC<CSHA384>;
	using CHMAC_SHA512 = CHMAC<CSHA512>;
	using CHMAC_SHA512Per224 = CHMAC<CSHA512Per224>;
	using CHMAC_SHA512Per256 = CHMAC<CSHA512Per256>;
}

END_HSHASH_NAMESPACE