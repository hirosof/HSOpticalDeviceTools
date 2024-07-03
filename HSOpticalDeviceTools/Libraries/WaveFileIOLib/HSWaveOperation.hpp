//2016/12/15
#pragma once


#include <stdint.h>
#include "CHSWaveBuffer.hpp"
#include "HSWaveDefine.hpp"


/*
	ä÷êî
*/
template <typename T>  bool HSWaveBufferPreparation ( CHSWaveBuffer<T> *pBuffer , uint32_t NumberOfSamples , uint8_t NumberOfChannels ) {
	if ( pBuffer == nullptr ) return false;
	if ( pBuffer->IsReady ( ) ) {
		if ( pBuffer->IsAllocatedReady ( ) ) {
			return  pBuffer->ReAlloc ( NumberOfSamples , NumberOfChannels );
		} else {
			if ( pBuffer->CountSamples ( ) < NumberOfSamples ) return false;
			if ( pBuffer->CountChannels ( ) < NumberOfChannels ) return false;
			return true;
		}
	} else {
		return pBuffer->Alloc ( NumberOfSamples , NumberOfChannels );
	}
}

template <typename T , typename U>  bool HSWaveBufferPreparation ( CHSWaveBuffer<T> *pBaseBuffer , CHSWaveBuffer<U> *pTargetBuffer ) {
	if ( pBaseBuffer == nullptr ) return false;

	if ( pBaseBuffer->IsReady ( ) ) {
		return HSWaveBufferPreparation ( pTargetBuffer ,
			pBaseBuffer->CountSamples ( ) ,
			pBaseBuffer->CountChannels ( ) );
	}

	return false;
}

bool HSWave_Normalize ( CHSWaveBufferPCM8 *pInBuffer , CHSWaveBufferNormalized *pOutBuffer );
bool HSWave_Normalize ( CHSWaveBufferPCM16 *pInBuffer , CHSWaveBufferNormalized *pOutBuffer );
bool HSWave_Normalize ( CHSWaveBufferPCM24 *pInBuffer , CHSWaveBufferNormalized *pOutBuffer );
bool HSWave_Normalize ( CHSWaveBufferPCM32 *pInBuffer , CHSWaveBufferNormalized *pOutBuffer );
bool HSWave_Normalize ( CHSWaveBufferIEEEFloat32 *pInBuffer , CHSWaveBufferNormalized *pOutBuffer );
bool HSWave_Normalize ( CHSWaveBufferIEEEFloat64 *pInBuffer , CHSWaveBufferNormalized *pOutBuffer );
bool HSWave_Normalize ( CHSWaveBufferPCMU *pInBuffer , CHSWaveBufferNormalized *pOutBuffer );
bool HSWave_Normalize ( CHSWaveBufferPCMA *pInBuffer , CHSWaveBufferNormalized *pOutBuffer );

bool HSWave_Normalize ( void *pInBuffer ,uint32_t NumberOfSamples ,  WAVEFORMATEX format , CHSWaveBufferNormalized *pOutBuffer );
template <typename T>  bool HSWave_Normalize ( CHSWaveBuffer<T> *pInBuffer , WAVEFORMATEX format , CHSWaveBufferNormalized *pOutBuffer ) {
	if ( pInBuffer == nullptr ) return false;
	if ( pInBuffer->IsReady ( ) == false ) return false;
	
	bool bRet = false;
	void *pBuffer = pInBuffer->GetBufferPointer ( 0 , 0 );

	uint32_t samples = pInBuffer->GetBufferSizeBytes ( ) / format.nBlockAlign;
	uint8_t channels = static_cast<uint8_t>( format.nChannels);

	if ( format.wFormatTag == WAVE_FORMAT_PCM ) {

		switch ( format.wBitsPerSample ) {
			case  8:
				{
					CHSWaveBufferPCM8 pcm8;
					pcm8.Attach ( ( HSWAVE_PCM8* ) pBuffer ,
						samples ,
						channels
					);

					bRet = HSWave_Normalize ( &pcm8 , pOutBuffer );
				}
				break;
			case 16:
				{
					CHSWaveBufferPCM16 pcm16;
					pcm16.Attach ( ( HSWAVE_PCM16* ) pBuffer ,
						samples ,
						channels
					);

					bRet = HSWave_Normalize ( &pcm16 , pOutBuffer );
				}
				break;
			case 24:
				{
					CHSWaveBufferPCM24 pcm24;
					pcm24.Attach ( ( HSWAVE_PCM24* ) pBuffer ,
						samples ,
						channels
					);

					bRet = HSWave_Normalize ( &pcm24 , pOutBuffer );
				}
				break;
			case 32:
				{
					CHSWaveBufferPCM32 pcm32;
					pcm32.Attach ( ( HSWAVE_PCM32* ) pBuffer ,
						samples ,
						channels
					);

					bRet = HSWave_Normalize ( &pcm32 , pOutBuffer );
				}
				break;
		}


	} else if ( format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT ) {
		if ( format.wBitsPerSample == 32 ) {
			CHSWaveBufferIEEEFloat32 ieee_float32;

			ieee_float32.Attach ( ( HSWAVE_IEEE_FLOAT32* ) pBuffer ,
				samples ,
				channels
			);

			bRet = HSWave_Normalize ( &ieee_float32 , pOutBuffer );
		} else if ( format.wBitsPerSample == 64 ) {
			CHSWaveBufferIEEEFloat64 ieee_float64;

			ieee_float64.Attach ( ( HSWAVE_IEEE_FLOAT64* ) pBuffer ,
				samples ,
				channels
			);

			bRet = HSWave_Normalize ( &ieee_float64 , pOutBuffer );

		}
	} else if ( format.wFormatTag == WAVE_FORMAT_MULAW ) {
		if ( format.wBitsPerSample == 8 ) {
			CHSWaveBufferPCMU pcmu;

			pcmu.Attach ( ( HSWAVE_PCMU* ) pBuffer ,
				samples ,
				channels
			);

			bRet = HSWave_Normalize ( &pcmu , pOutBuffer );
		}
	} else if ( format.wFormatTag == WAVE_FORMAT_ALAW ) {
		if ( format.wBitsPerSample == 8 ) {
			CHSWaveBufferPCMA pcma;

			pcma.Attach ( ( HSWAVE_PCMA* ) pBuffer ,
				samples ,
				channels
			);

			bRet = HSWave_Normalize ( &pcma , pOutBuffer );
		}
	}

	return bRet;
}



bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferPCM8 *pOutBuffer );
bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferPCM16 *pOutBuffer );
bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferPCM24 *pOutBuffer );
bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferPCM32 *pOutBuffer );
bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferIEEEFloat32 *pOutBuffer );
bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferIEEEFloat64 *pOutBuffer );
bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferPCMU *pOutBuffer );
bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferPCMA *pOutBuffer );

bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferUnknown *pOutBuffer, WAVEFORMATEX OutFormat );
bool HSWave_UnNormalize ( CHSWaveBufferNormalized *pInBuffer , CHSWaveBufferUnknown *pOutBuffer , uint16_t uFormatTag , uint16_t uBitsPerSample);


