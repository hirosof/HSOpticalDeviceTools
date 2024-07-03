#include "HSWaveConverter.hpp"

bool HSWaveConv_ReSampling (CHSWaveBufferNormalized * pinBuf ,uint32_t uInSamplingFrequency , 
	CHSWaveBufferNormalized * poutBuf ,	uint32_t uOutSamplingFrequency)
{
	if ( uInSamplingFrequency == 0 ) return false;
	if ( uOutSamplingFrequency == 0 ) return false;

	double f = uInSamplingFrequency;
	f /= uOutSamplingFrequency;

	return HSWaveConv_ReSampling ( pinBuf , poutBuf , f );
}

bool HSWaveConv_ReSampling ( CHSWaveBufferNormalized * pinBuf , 
	CHSWaveBufferNormalized * poutBuf , double FrequencyRate )
{
	if ( pinBuf == nullptr ) return false;
	if ( poutBuf == nullptr ) return false;
	if ( pinBuf->IsReady ( ) == false ) return false;
	if ( FrequencyRate <= 0 )return false;

	uint32_t samples = pinBuf->CountSamples ( );
	uint8_t channels = pinBuf->CountChannels ( );
	uint32_t outsamples = static_cast< uint32_t >( samples / FrequencyRate );

	if ( HSWaveBufferPreparation ( poutBuf , outsamples , channels ) == false ) return false;

	uint32_t level = 1;
	uint32_t level_loop_num = 2 * level + 1;

	double t;
	uint32_t offset;
	HSWAVE_NORMALIZED norm;
	for ( uint32_t out_idx = 0; out_idx < outsamples; out_idx++ ) {
		t = FrequencyRate * out_idx;
		offset = static_cast< uint32_t >( t );
		for ( uint8_t cid = 0; cid < channels; cid++ ) {
			norm = 0;
			for ( int64_t in_idx = ( offset - level ); in_idx <= ( offset + level ); in_idx++ ) {
				if ( ( in_idx >= 0 ) && ( in_idx < samples ) ) {
					norm += pinBuf->GetBufferReference ( cid , static_cast<uint32_t>( in_idx )) * HSWaveMath_Sinc ( M_PI*( t - in_idx ) );
				}
			}
			poutBuf->SetBufferValue ( norm , cid , out_idx );
		}
	}

	return true;
}

uint32_t HSWaveConvCalc_ReSamplingOutSamples ( uint32_t NumberOfInputAllSamples , uint32_t NumberOfProcessBlockSamples , double rate )
{
	if ( (NumberOfProcessBlockSamples > 0) &&(rate>0) ) {
		uint32_t CountProcessBlocks = NumberOfInputAllSamples / NumberOfProcessBlockSamples;
		uint32_t CountProcessBlockResstSamples = NumberOfInputAllSamples % NumberOfProcessBlockSamples;
		uint32_t OutProcessBlockSamples = static_cast< uint32_t >( NumberOfProcessBlockSamples / rate );
		uint32_t OutProcessBlockRestSamples = static_cast< uint32_t >( CountProcessBlockResstSamples / rate );
		return OutProcessBlockSamples * CountProcessBlocks + OutProcessBlockRestSamples;
	}
	return 0;
}

bool HSWaveConv_ReSampling_MT ( CHSWaveBufferNormalized * pinBuf , 
	CHSWaveBufferNormalized * poutBuf , 	double FrequencyRate , 
	CHSWaveBufferNormalized * pinPrevBuf , CHSWaveBufferNormalized * pinNextBuf )
{
	if ( pinBuf == nullptr ) return false;
	if ( poutBuf == nullptr ) return false;
	if ( pinBuf->IsReady ( ) == false ) return false;
	if ( FrequencyRate <= 0 )return false;

	uint32_t samples = pinBuf->CountSamples ( );
	uint8_t channels = pinBuf->CountChannels ( );
	uint32_t outsamples = static_cast< uint32_t >( samples / FrequencyRate );

	if ( HSWaveBufferPreparation ( poutBuf , outsamples , channels ) == false ) return false;

	THSWAVECONV_RESAMPLINGMTDATA data [ HSWAVE_CONV_THREADNUMS ];
	HANDLE hThreads [ HSWAVE_CONV_THREADNUMS ];
	uint32_t ThreadBlockSamples = outsamples /  HSWAVE_CONV_THREADNUMS;

	int threadnums = HSWAVE_CONV_THREADNUMS;

	for ( int s = 0; s < threadnums; s++ ) {

		data [ s ].FrequencyRate = FrequencyRate;
		data [ s ].pinBuf = pinBuf;
		data [ s ].poutBuf = poutBuf;
		data [ s ].pinPrevBuf = pinPrevBuf;
		data [ s ].pinNextBuf = pinNextBuf;

		if ( s == ( threadnums - 1 ) ) {
			data [ s ].ProcessStartIndex = s * ThreadBlockSamples;
			data [ s ].ProcessEndIndex = outsamples - 1;
		} else {
			data [ s ].ProcessStartIndex = s * ThreadBlockSamples;
			data [ s ].ProcessEndIndex = data [ s ].ProcessStartIndex + ThreadBlockSamples - 1;
		}

		hThreads [ s ] = ( HANDLE ) _beginthreadex ( nullptr , 0 ,
			__HSWaveConv_ReSampling_ThreadProc , &data [ s ] , 
			CREATE_SUSPENDED , NULL );
	}

	SYSTEM_INFO si;

	GetSystemInfo ( &si );

	for ( int s = 0; s < threadnums; s++ ) {
		int cpu_thread;
		cpu_thread = s % ( si.dwNumberOfProcessors );
		SetThreadAffinityMask ( hThreads [ s ] ,static_cast<DWORD_PTR> (1i64 << s) );
		ResumeThread ( hThreads [ s ] );
	}


	WaitForMultipleObjects ( threadnums , hThreads , TRUE , INFINITE );

	for ( int s = 0; s < threadnums; s++ ) {
		CloseHandle ( hThreads [ s ] );
	}
	return true;
}

unsigned __stdcall __HSWaveConv_ReSampling_ThreadProc ( void * pData )
{
	THSWAVECONV_RESAMPLINGMTDATA *pInfo = ( THSWAVECONV_RESAMPLINGMTDATA* ) pData;

	uint32_t level = 8;
	uint32_t level_loop_num = 2 * level + 1;

	CHSWaveBufferNormalized * pinBuf = pInfo->pinBuf;
	CHSWaveBufferNormalized * poutBuf = pInfo->poutBuf;
	double FrequencyRate = pInfo->FrequencyRate;

	uint32_t samples = pinBuf->CountSamples ( );
	uint8_t channels = pinBuf->CountChannels ( );

	double t;
	uint32_t offset;
	HSWAVE_NORMALIZED norm;
	double s;
	for ( uint32_t out_idx = pInfo->ProcessStartIndex; out_idx <= pInfo->ProcessEndIndex; out_idx++ ) {
		t = FrequencyRate * out_idx;
		offset = static_cast< uint32_t >( t );
		for ( uint8_t cid = 0; cid < channels; cid++ ) {
			norm = 0;
			for ( int64_t in_idx = ((int64_t) offset - level ); in_idx <= ( offset + level ); in_idx++ ) {
				
				s = HSWaveMath_Sinc ( M_PI*( t - in_idx ) );
				
				if ( in_idx < 0 ) {

					if(pInfo->pinPrevBuf ){
						int64_t idx = pInfo->pinPrevBuf->CountSamples ( ) + in_idx;
						if ( idx >= 0 ) {
							norm += pInfo->pinPrevBuf->GetBufferReference ( cid , static_cast< uint32_t >( idx ) ) * s;
						}
					}
				} else if ( in_idx >= samples ) {

					if(pInfo->pinNextBuf ){
						int64_t idx = in_idx - samples;
						if ( idx < pInfo->pinNextBuf->CountSamples() ) {
							norm += pInfo->pinNextBuf->GetBufferReference ( cid , static_cast< uint32_t >( idx ) ) * s;
						}
					}

				}else{
					norm += pinBuf->GetBufferReference ( cid , static_cast< uint32_t >( in_idx ) ) * s;
				}
			}			
			poutBuf->SetBufferValue ( norm , cid , out_idx );

		}
	}

	_endthreadex ( 0 );
	return 0;
}
