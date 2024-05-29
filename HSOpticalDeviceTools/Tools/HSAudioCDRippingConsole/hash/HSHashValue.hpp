#pragma once

#include "HSHashException.hpp"
#include "HSHashCommonFunctions.hpp"
#include <cstdio>
#include <string>

BEGIN_HSHASH_NAMESPACE

namespace Base {


	template<typename T> class CHashValueBaseAbstruct {
	public:

		virtual size_t GetSize (void) const = 0;

		virtual size_t GetWordSize (void) const = 0;

		virtual size_t CountWordElements (void) const = 0;

		virtual size_t Count (void) const = 0;

		virtual T GetWordValue (size_t index) const = 0;

		virtual uint8_t GetValue (size_t index) const = 0;
		virtual ::std::string ToString (void) const = 0;
		virtual ::std::wstring ToWString (void) const = 0;

	};


	template <typename T, size_t TElementSize , EHashValueEndian  Endian = EHashValueEndian::Little, size_t lastLimitSize = sizeof(T)>
	class CHashValueBase : public CHashValueBaseAbstruct<T> {

	private:
		T m_hashValue[TElementSize];


	public:

		using ElementType = T;
		static const size_t m_ElementSize = TElementSize;

		CHashValueBase (void) {
	
			for ( size_t i = 0; i < TElementSize; i++ ) {
				m_hashValue[i] = 0;
			}

		}

		CHashValueBase (const T (&hashValue)[TElementSize]) {
			for (size_t i = 0; i < TElementSize; i++){
				m_hashValue[i] = hashValue[i];
				if (Endian == EHashValueEndian::Big) {
					m_hashValue[i] = Functions::InverseEndian (m_hashValue[i]);
				}
			}
		}


		CHashValueBase& operator=(const CHashValueBase& value) {
			for (size_t i = 0; i < TElementSize; i++){
				m_hashValue[i] = value.m_hashValue[i];
			}
			return *this;
		}

		virtual size_t GetSize (void) const{
			return sizeof (m_hashValue) -(sizeof (T) - lastLimitSize);
		}
		
		virtual size_t GetWordSize (void) const{
			return sizeof (T);
		}
		
		virtual size_t CountWordElements (void) const{
			return TElementSize;
		}

		virtual size_t Count (void) const{
			return GetSize ();
		}


		virtual T GetWordValue (size_t index) const {
			if (index >= TElementSize) throw Exception::COutOfRangeExceptionSizeT (index, 0, TElementSize - 1);
			T value = m_hashValue[index];
			if (index == TElementSize - 1) {
				if (GetWordSize () != lastLimitSize) {
					size_t invalidBytesSize = GetWordSize () - lastLimitSize;
					T mask = 0xFF;
					for (size_t i = 1; i < invalidBytesSize; i++)
					{
						mask <<= 8;
						mask |= 0xFF;
					}
					value &= ~mask;
				}
			}
			return value;
		}

		virtual uint8_t GetValue (size_t index) const{
			const size_t wordSize = GetWordSize ();
			if (index >= GetSize()) throw Exception::COutOfRangeExceptionSizeT (index, 0, GetSize () - 1);
			size_t valueIndex = index / wordSize;
			size_t byteIndex = wordSize - 1 - (index % wordSize);
			T wordValue = GetWordValue (valueIndex);
			return (wordValue >> (byteIndex * 8)) & 0xFF;
		}

		const uint8_t operator[](size_t index) const {
			return GetValue (index);
		}

		
		virtual ::std::string ToString (void) const {
			char text[3];
			::std::string s;
			for (size_t i = 0; i < this->Count(); i++){
				sprintf_s (text, "%02x", this->GetValue (i));
				s += text;
			}
			return s;
		}

		virtual ::std::wstring ToWString (void) const {
			wchar_t text[3];
			::std::wstring s;
			for (size_t i = 0; i < this->Count(); i++){
				swprintf_s (text, L"%02x", this->GetValue (i));
				s += text;
			}
			return s;
		}

		virtual  bool IsEqual (const CHashValueBase &rhs) const {
			const size_t cmp_targetsize = sizeof (T) * (TElementSize - 1);
			if (memcmp (this->m_hashValue, rhs.m_hashValue, cmp_targetsize) != 0) {
				return false;
			}
			T last = this->GetWordValue (TElementSize - 1);
			T rhs_last = rhs.GetWordValue (TElementSize - 1);
			return last == rhs_last;
		}

		virtual  bool IsNotEqual (const CHashValueBase &rhs) const {
			return this->IsEqual (rhs) == false;
		}


		bool operator==(const CHashValueBase &rhs) const {
			return this->IsEqual (rhs);
		}
		bool operator!=(const CHashValueBase &rhs) const {
			return this->IsNotEqual (rhs);
		}

	};

}

END_HSHASH_NAMESPACE