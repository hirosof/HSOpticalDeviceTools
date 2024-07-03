#pragma once

#include "HSWaveOperation.hpp"
#include "HSWaveMath.hpp"
#include <process.h>


#define HSWAVE_CONV_THREADNUMS 8

struct THSWAVECONV_RESAMPLINGMTDATA {
	CHSWaveBufferNormalized *pinBuf;
	CHSWaveBufferNormalized *poutBuf;
	double FrequencyRate;
	uint32_t ProcessStartIndex;
	uint32_t ProcessEndIndex;

	CHSWaveBufferNormalized *pinPrevBuf;
	CHSWaveBufferNormalized *pinNextBuf;
};


bool HSWaveConv_ReSampling (
	CHSWaveBufferNormalized *pinBuf , 
	uint32_t uInSamplingFrequency , 
	CHSWaveBufferNormalized *poutBuf , 
	uint32_t uOutSamplingFrequency
);

bool HSWaveConv_ReSampling (
	CHSWaveBufferNormalized *pinBuf , 
	CHSWaveBufferNormalized *poutBuf , 
	double FrequencyRate
);

uint32_t HSWaveConvCalc_ReSamplingOutSamples ( 
	uint32_t NumberOfInputAllSamples , 
	uint32_t NumberOfProcessBlockSamples , 
	double rate
);


bool HSWaveConv_ReSampling_MT (
	CHSWaveBufferNormalized *pinBuf , 
	CHSWaveBufferNormalized *poutBuf , 
	double FrequencyRate,
	CHSWaveBufferNormalized *pinPrevBuf = nullptr ,
	CHSWaveBufferNormalized *pinNextBuf = nullptr

);

unsigned __stdcall __HSWaveConv_ReSampling_ThreadProc ( void *pData );

