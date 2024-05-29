#pragma once

#include <cstdint>

#include "HSHashDefine.hpp"

BEGIN_HSHASH_NAMESPACE

namespace	Exception {

	template <typename T> class COutOfRangeException {
	private:

		T m_invalidvalue;
		T m_validMinValue;
		T m_validMaxValue;


	public:

		COutOfRangeException (T invalidValue, T validMinValue, T validMaxValue) {
			m_invalidvalue = invalidValue;
			m_validMinValue = validMinValue;
			m_validMaxValue = validMaxValue;
		}

		~COutOfRangeException () = default;


		T GetInvalidValue (void) const {
			return m_invalidvalue;
		}

		T GetValidMinValue (void) const {
			return m_validMinValue;
		}

		T GetValidMaxValue (void) const {
			return m_validMaxValue;
		}
	};


	using COutOfRangeExceptionInt32 = COutOfRangeException<int32_t>;
	using COutOfRangeExceptionSizeT = COutOfRangeException<size_t>;

	template <typename T> class CInvalidValueException {
	private:

		T m_invalidvalue;
		T m_validValue;


	public:

		CInvalidValueException (T invalidValue, T validValue) {
			m_invalidvalue = invalidValue;
			m_validValue = validValue;
		}

		~CInvalidValueException () = default;

		T GetInvalidValue (void) const {
			return m_invalidvalue;
		}

		T GetValidValue (void) const {
			return m_validValue;
		}
	};

	using  CInvalidValueExceptionInt32 = CInvalidValueException<int32_t>;
	using  CInvalidValueExceptionSizeT = CInvalidValueException<size_t>;
}

END_HSHASH_NAMESPACE