//2016/12/15
#include "HSWaveOperation.hpp"

bool HSWave_Normalize ( CHSWaveBufferPCM8 * pInBuffer , CHSWaveBufferNormalized * pOutBuffer )
{

	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp -= 128;

			if ( tmp >= 0 ) {
				pOutBuffer->SetBufferValue (
					tmp / HSWAVE_PCM8E_MAX ,
					cid , sid
				);
			} else {
				pOutBuffer->SetBufferValue (
					(-tmp) / HSWAVE_PCM8E_MIN ,
					cid , sid
				);
			}
		}
	}
	return true;
}

bool HSWave_Normalize ( CHSWaveBufferPCM16 * pInBuffer , CHSWaveBufferNormalized * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;
	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			if ( tmp >= 0 ) {
				pOutBuffer->SetBufferValue (
					tmp / HSWAVE_PCM16_MAX ,
					cid , sid
				);
			} else {
				pOutBuffer->SetBufferValue (
					( -tmp ) / HSWAVE_PCM16_MIN ,
					cid , sid
				);
			}
		}
	}
	return true;
}

bool HSWave_Normalize ( CHSWaveBufferPCM24 * pInBuffer , CHSWaveBufferNormalized * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;
	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = (int32_t) pInBuffer->GetBufferReference ( cid , sid );

			if ( tmp >= 0 ) {
				pOutBuffer->SetBufferValue (
					tmp / HSWAVE_PCM24_MAX ,
					cid , sid
				);
			} else {
				pOutBuffer->SetBufferValue (
					( -tmp ) / HSWAVE_PCM24_MIN ,
					cid , sid
				);
			}
		}
	}
	return true;
}

bool HSWave_Normalize ( CHSWaveBufferPCM32 * pInBuffer , CHSWaveBufferNormalized * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;
	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );
	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			if ( tmp >= 0 ) {
				pOutBuffer->SetBufferValue (
					tmp / HSWAVE_PCM32_MAX ,
					cid , sid
				);
			} else {
				pOutBuffer->SetBufferValue (
					( -tmp ) / HSWAVE_PCM32_MIN ,
					cid , sid
				);
			}
		}
	}
	return true;
}

bool HSWave_Normalize ( CHSWaveBufferIEEEFloat32 * pInBuffer , CHSWaveBufferNormalized * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;
	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			pOutBuffer->SetBufferValue (
				min ( 1.0 , max ( -1.0 , tmp ) ) ,
				cid , sid
			);

		}
	}
	return true;
}

bool HSWave_Normalize ( CHSWaveBufferIEEEFloat64 * pInBuffer , CHSWaveBufferNormalized * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;
	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			pOutBuffer->SetBufferValue (
				min ( 1.0 , max ( -1.0 , tmp ) ) ,
				cid , sid
			);

		}
	}
	return true;
}

bool HSWave_Normalize ( CHSWaveBufferPCMU * pInBuffer , CHSWaveBufferNormalized * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;
	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;
	HSWAVE_PCMU pcmu;
	uint8_t input , level , lowbit;

	int32_t  buf;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			pcmu = pInBuffer->GetBufferReference ( cid , sid );

			input = ~pcmu.val;

			level = ( input >> 4 ) & 0x07;
			lowbit = input & 0x0F;

			buf = ( static_cast< int32_t >( lowbit ) << 3 ) + 0x84;
			buf = ( buf << level ) - 0x84;

			if ( input & 0x80 ) {
				buf *= -1;
				tmp = max ( HSWAVE_PCM16_MIN , buf );
			} else {
				tmp = min ( HSWAVE_PCM16_MAX , buf );
			}

			if ( tmp >= 0 ) {
				pOutBuffer->SetBufferValue (
					tmp / HSWAVE_PCM16_MAX ,
					cid , sid
				);
			} else {
				pOutBuffer->SetBufferValue (
					( -tmp ) / HSWAVE_PCM16_MIN ,
					cid , sid
				);
			}
		}

	}

	return true;
}

bool HSWave_Normalize ( CHSWaveBufferPCMA * pInBuffer , CHSWaveBufferNormalized * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;
	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;
	HSWAVE_PCMA pcma;
	uint8_t input , level , lowbit;

	int32_t  buf;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			pcma = pInBuffer->GetBufferReference ( cid , sid );

			input = pcma.val ^ 0xD5;

			level = ( input >> 4 ) & 0x07;
			lowbit = input & 0x0F;


			buf = ( static_cast< int32_t >( lowbit ) << 4 );
			if ( level == 0 ) {
				buf += 0x0008;
			} else {
				buf += 0x0108;
				buf <<= ( level - 1 );
			}

			if ( input & 0x80 ) {
				buf *= -1;
				tmp = max ( HSWAVE_PCM16_MIN , buf );
			} else {
				tmp = min ( HSWAVE_PCM16_MAX , buf );
			}

			if ( tmp >= 0 ) {
				pOutBuffer->SetBufferValue (
					tmp / HSWAVE_PCM16_MAX ,
					cid , sid
				);
			} else {
				pOutBuffer->SetBufferValue (
					( -tmp ) / HSWAVE_PCM16_MIN ,
					cid , sid
				);
			}
		}

	}

	return true;
}


bool HSWave_Normalize ( void * pInBuffer , uint32_t NumberOfSamples , WAVEFORMATEX format , CHSWaveBufferNormalized * pOutBuffer )
{
	CHSWaveBufferUnknown unknown;
	HSWAVE_Unknown *pData = reinterpret_cast<HSWAVE_Unknown*>(pInBuffer);
	if ( unknown.Attach ( pData , NumberOfSamples * format.nBlockAlign) ) {
		return  HSWave_Normalize ( &unknown , format , pOutBuffer );
	}
	return false;
}



bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferPCM8 * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp = min ( 1.0 , max ( -1.0 , tmp ) );

			if ( tmp >= 0 ) {
				tmp = tmp* HSWAVE_PCM8E_MAX;
			} else {
				tmp = (-tmp)* HSWAVE_PCM8E_MIN;
			}

			tmp += 128;

			pOutBuffer->SetBufferValue ( static_cast< HSWAVE_PCM8 >( tmp ) , cid , sid );
		}
	}
	return true;
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferPCM16 * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp = min ( 1.0 , max ( -1.0 , tmp ) );
			if ( tmp >= 0 ) {
				tmp = tmp* HSWAVE_PCM16_MAX;
			} else {
				tmp = ( -tmp )* HSWAVE_PCM16_MIN;
			}
			pOutBuffer->SetBufferValue ( static_cast< HSWAVE_PCM16 >( tmp ) , cid , sid );
		}
	}
	return true;
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferPCM24 * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	HSWAVE_PCM24 pcm24;
	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp = min ( 1.0 , max ( -1.0 , tmp ) );
			if ( tmp >= 0 ) {
				tmp = tmp* HSWAVE_PCM24_MAX;
			} else {
				tmp = ( -tmp )* HSWAVE_PCM24_MIN;
			}

			pcm24 = static_cast< int >( tmp );
			pOutBuffer->SetBufferValue (pcm24 , cid , sid );
		}
	}
	return true;
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferPCM32 * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp = min ( 1.0 , max ( -1.0 , tmp ) );
			if ( tmp >= 0 ) {
				tmp = tmp* HSWAVE_PCM32_MAX;
			} else {
				tmp = ( -tmp )* HSWAVE_PCM32_MIN;
			}
			pOutBuffer->SetBufferValue ( static_cast< HSWAVE_PCM32 >( tmp ) , cid , sid );
		}
	}
	return true;
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferIEEEFloat32 * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp = min ( HSWAVE_IEEE_FLOAT32_MAX , max ( HSWAVE_IEEE_FLOAT32_MIN , tmp ) );

			pOutBuffer->SetBufferValue ( static_cast< HSWAVE_IEEE_FLOAT32 >( tmp ) , cid , sid );
		}
	}
	return true;
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferIEEEFloat64 * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;

	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp = min ( HSWAVE_IEEE_FLOAT64_MAX , max ( HSWAVE_IEEE_FLOAT64_MIN , tmp ) );

			pOutBuffer->SetBufferValue ( static_cast< HSWAVE_IEEE_FLOAT64 >( tmp ) , cid , sid );
		}
	}
	return true;
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferPCMU * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;
	HSWAVE_PCM16  pcm_tmp , proc;
	uint8_t sign , level;
	int proc_buf;
	HSWAVE_PCMU  out_tmp;
	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp = min ( 1.0 , max ( -1.0 , tmp ) );
			if ( tmp >= 0 ) {
				tmp = tmp* HSWAVE_PCM16_MAX;
			} else {
				tmp = ( -tmp )* HSWAVE_PCM16_MIN;
			}

			pcm_tmp = static_cast< HSWAVE_PCM16 >( tmp );

			sign = ( pcm_tmp & 0x8000 ) >> 8;

			if ( sign ) proc_buf = -pcm_tmp;
			else proc_buf = pcm_tmp;
			if ( proc_buf >= 0x7F7B ) {
				proc = 0x7FFF;
			} else {
				proc = proc_buf & 0xFFFF;
				proc += 0x84;
			}

			for ( level = 0; level < 8; level++ ) {
				if ( proc <= HSPCMU_LEVELTABLES [ level ] ) {
					break;
				}
			}

			out_tmp.val = ( proc >> ( level + 3 ) ) & 0x0F;
			out_tmp.val |= ( sign | ( level << 4 ) );

			out_tmp.val = ~out_tmp.val;

			pOutBuffer->SetBufferValue ( out_tmp , cid , sid );
		}
	}

	return true;
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferPCMA * pOutBuffer )
{
	if ( HSWaveBufferPreparation ( pInBuffer , pOutBuffer ) == false ) return false;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	HSWAVE_NORMALIZED tmp;
	HSWAVE_PCM16  pcm_tmp , proc;
	uint8_t sign , level;
	HSWAVE_PCMA out_tmp;
	int proc_buf;
	for ( uint32_t sid = 0; sid < samples; sid++ ) {
		for ( uint8_t cid = 0; cid < channels; cid++ ) {

			tmp = pInBuffer->GetBufferReference ( cid , sid );

			tmp = min ( 1.0 , max ( -1.0 , tmp ) );
			if ( tmp >= 0 ) {
				tmp = tmp* HSWAVE_PCM16_MAX;
			} else {
				tmp = ( -tmp )* HSWAVE_PCM16_MIN;
			}

			pcm_tmp = static_cast< HSWAVE_PCM16 >( tmp );

			sign = ( pcm_tmp & 0x8000 ) >> 8;

			if ( sign ) proc_buf = -pcm_tmp;
			else proc_buf = pcm_tmp;

			if ( proc_buf > 0x7FFF ) {
				proc = 0x7FFF;
			} else {
				proc = proc_buf & 0xFFFF;
			}

			for ( level = 0; level < 8; level++ ) {
				if ( proc <= HSPCMA_LEVELTABLES [ level ] ) {
					break;
				}
			}

			if ( level == 0 ) {
				out_tmp.val = ( proc >> 4 ) & 0x0F;

			} else {
				out_tmp.val = ( proc >> ( level + 3 ) ) & 0x0F;
			}

			out_tmp.val |= ( sign | ( level << 4 ) );

			out_tmp.val = out_tmp.val ^ 0xD5;

			pOutBuffer->SetBufferValue ( out_tmp , cid , sid );
		}
	}

	return true;
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferUnknown * pOutBuffer , WAVEFORMATEX OutFormat )
{
	return HSWave_UnNormalize ( pInBuffer , pOutBuffer , OutFormat.wFormatTag , OutFormat.wBitsPerSample );
}

bool HSWave_UnNormalize ( CHSWaveBufferNormalized * pInBuffer , CHSWaveBufferUnknown * pOutBuffer , uint16_t uFormatTag , uint16_t uBitsPerSample )
{
	if ( pInBuffer == nullptr ) return false;
	if ( pInBuffer->IsReady ( ) == false ) return false;
	if ( ( uBitsPerSample % 8 ) != 0 ) return false;
	
	bool bRet = false;
	void *pPointer;

	uint32_t samples = pInBuffer->CountSamples ( );
	uint8_t channels = pInBuffer->CountChannels ( );

	uint32_t outBlockSize = uBitsPerSample / 8 * channels;

	uint32_t  outByteSize = outBlockSize * samples;

	if ( HSWaveBufferPreparation ( pOutBuffer , outByteSize , 1 ) == false ) return false;

	pPointer = pOutBuffer->GetBufferPointer (0,0 );


	if ( uFormatTag == WAVE_FORMAT_PCM ) {

		switch ( uBitsPerSample ) {
			case  8:
				{
					CHSWaveBufferPCM8 pcm8;
					pcm8.Attach ( ( HSWAVE_PCM8* ) pPointer ,
						samples ,
						channels
					);

					bRet = HSWave_UnNormalize ( pInBuffer , &pcm8 );
				}
				break;
			case 16:
				{
					CHSWaveBufferPCM16 pcm16;
					pcm16.Attach ( ( HSWAVE_PCM16* ) pPointer ,
						samples ,
						channels
					);

					bRet = HSWave_UnNormalize ( pInBuffer , &pcm16 );
				}
				break;
			case 24:
				{
					CHSWaveBufferPCM24 pcm24;
					pcm24.Attach ( ( HSWAVE_PCM24* ) pPointer ,
						samples ,
						channels
					);

					bRet = HSWave_UnNormalize ( pInBuffer , &pcm24 );
				}
				break;
			case 32:
				{
					CHSWaveBufferPCM32 pcm32;
					pcm32.Attach ( ( HSWAVE_PCM32* ) pPointer ,
						samples ,
						channels
					);

					bRet = HSWave_UnNormalize ( pInBuffer , &pcm32 );
				}
				break;
		}


	} else if ( uFormatTag == WAVE_FORMAT_IEEE_FLOAT ) {
		if ( uBitsPerSample == 32 ) {
			CHSWaveBufferIEEEFloat32 ieee_float32;

			ieee_float32.Attach ( ( HSWAVE_IEEE_FLOAT32* ) pPointer ,
				samples ,
				channels
			);

			bRet = HSWave_UnNormalize ( pInBuffer , &ieee_float32 );
		} else if ( uBitsPerSample == 64 ) {

			CHSWaveBufferIEEEFloat64 ieee_float64;

			ieee_float64.Attach ( ( HSWAVE_IEEE_FLOAT64* ) pPointer ,
				samples ,
				channels
			);

			bRet = HSWave_UnNormalize ( pInBuffer , &ieee_float64 );
		}
	} else if ( uFormatTag == WAVE_FORMAT_MULAW ) {
		if ( uBitsPerSample == 8 ) {
			CHSWaveBufferPCMU pcmu;

			pcmu.Attach ( ( HSWAVE_PCMU* ) pPointer ,
				samples ,
				channels
			);

			bRet = HSWave_UnNormalize ( pInBuffer , &pcmu );
		}
	} else if ( uFormatTag == WAVE_FORMAT_ALAW ) {
		if ( uBitsPerSample == 8 ) {
			CHSWaveBufferPCMA pcma;

			pcma.Attach ( ( HSWAVE_PCMA* ) pPointer ,
				samples ,
				channels
			);

			bRet = HSWave_UnNormalize ( pInBuffer , &pcma );
		}
	}

	return bRet;
}


