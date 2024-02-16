#pragma once

#include "HSHashDefine.hpp"

BEGIN_HSHASH_NAMESPACE

namespace Functions {
	template <typename T> T LeftRotate (T   value, uint32_t numberOfRotateBits) {
		uint32_t  typeBits = sizeof (T) * 8;
		uint32_t realRotateBits = numberOfRotateBits % typeBits;
		return (value << realRotateBits) | (value >> (typeBits - realRotateBits));
	}

	template <typename T> T RightRotate (T   value, uint32_t numberOfRotateBits) {
		uint32_t  typeBits = sizeof (T) * 8;
		uint32_t realRotateBits = numberOfRotateBits % typeBits;
		return (value >> realRotateBits) | (value << (typeBits - realRotateBits));
	}

	template <typename T> T InverseEndian (T value) {
		T retval = 0;
		const size_t  t_size = sizeof (T);
		uint8_t *pRet = reinterpret_cast<uint8_t*>(&retval);
		for (size_t i = 0; i < t_size; i++) {
			*(pRet + t_size - 1 - i) = (value >> (8 * i)) & 0xFF;
		}
		return retval;
	}
}



END_HSHASH_NAMESPACE