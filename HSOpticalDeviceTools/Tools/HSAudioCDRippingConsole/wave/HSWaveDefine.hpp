#pragma once
#include <stdint.h>
#include <Windows.h>
#include <mmreg.h>
#include "CHSWaveBuffer.hpp"

/*
通常型定義
*/
using HSWAVE_NORMALIZED = double;

struct HSWAVE_Unknown {
	uint8_t value;
};

using HSWAVE_PCM8 = uint8_t;

using HSWAVE_PCM16 = int16_t;

struct HSWAVE_PCM24 {
	uint8_t bytes [ 3 ];

	int operator=( int nValue ) {
		bytes [ 0 ] = nValue & 0xFF;
		bytes [ 1 ] = ( nValue & 0xFF00 ) >> 8;
		bytes [ 2 ] = ( nValue & 0xFF0000 ) >> 16;
		return nValue;
	}

	operator int ( ) {
		int buf = bytes [ 0 ] | ( bytes [ 1 ] << 8 ) | ( bytes [ 2 ] << 16 );
		if ( bytes [ 2 ] & 0x80 ) buf |= 0xFF << 24;
		return buf;
	}

};

using HSWAVE_PCM32 = int32_t;
using HSWAVE_IEEE_FLOAT32 = float;
using HSWAVE_IEEE_FLOAT64 = double;


struct HSWAVE_PCMU {
	uint8_t val;
};

struct HSWAVE_PCMA {
	uint8_t val;
};


/*
バッファ型定義
*/



using CHSWaveBufferNormalized = CHSWaveBuffer<HSWAVE_NORMALIZED>;
using CHSWaveBufferUnknown = CHSWaveBuffer<HSWAVE_Unknown>;


using CHSWaveBufferPCM8 = CHSWaveBuffer<HSWAVE_PCM8>;
using CHSWaveBufferPCM16 = CHSWaveBuffer<HSWAVE_PCM16>;
using CHSWaveBufferPCM24 = CHSWaveBuffer<HSWAVE_PCM24>;
using CHSWaveBufferPCM32 = CHSWaveBuffer<HSWAVE_PCM32>;
using CHSWaveBufferIEEEFloat32 = CHSWaveBuffer<HSWAVE_IEEE_FLOAT32>;
using CHSWaveBufferIEEEFloat64 = CHSWaveBuffer<HSWAVE_IEEE_FLOAT64>;

using CHSWaveBufferPCMU = CHSWaveBuffer<HSWAVE_PCMU>;
using CHSWaveBufferPCMA = CHSWaveBuffer<HSWAVE_PCMA>;



/*
定数定義
*/

//PCM - 8bit
#define	HSWAVE_PCM8_MIN 0
#define HSWAVE_PCM8_MAX 255

//PCM - 8bit(作業上符号付きにした場合)
#define	HSWAVE_PCM8E_MIN -128
#define HSWAVE_PCM8E_MAX 127

//PCM - 16bit
#define	HSWAVE_PCM16_MIN -32768
#define HSWAVE_PCM16_MAX 32767

//PCM - 32bit
#define	HSWAVE_PCM32_MIN (-2147483647-1)
#define HSWAVE_PCM32_MAX 2147483647

//IEEE FLOAT (PCM FLOAT) - 32bit(float-data)
#define	HSWAVE_IEEE_FLOAT32_MIN -1.0f
#define HSWAVE_IEEE_FLOAT32_MAX 1.0f

//IEEE FLOAT (PCM FLOAT) - 64bit(float-data)
#define	HSWAVE_IEEE_FLOAT64_MIN -1.0
#define HSWAVE_IEEE_FLOAT64_MAX 1.0


//PCM - 24bit
#define	HSWAVE_PCM24_MIN -8388608
#define HSWAVE_PCM24_MAX 8388607



const uint16_t HSPCMU_LEVELTABLES [ 8 ] = {
	0x00FF ,0x01FF, 0x03FF ,0x07FF ,
	0x0FFF ,0x1FFF ,0x3FFF ,0x7FFF

};

const uint16_t HSPCMA_LEVELTABLES [ 8 ] = {
	0x00FF ,0x01FF, 0x03FF ,0x07FF ,
	0x0FFF ,0x1FFF ,0x3FFF ,0x7FFF
};