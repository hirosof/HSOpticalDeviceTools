#pragma once
#include <stdint.h>
#include <Windows.h>
#include <process.h>
#include "CHSMalloc.hpp"

#pragma comment(lib , "winmm.lib")

template <typename tnUserDataType> class IHSWAVEOUT_CALLBACK {
protected:
	using UserDataType = tnUserDataType;
	tnUserDataType u_data;
public:

	//イベント(オプション)
	virtual bool OnSetup ( tnUserDataType UserData ) {
		this->OnChangeUserData ( UserData );
		return true;
	}
	

	virtual void OnChangeUserData ( tnUserDataType UserData ) {
		u_data = UserData;
	}

	virtual bool GetLength ( uint32_t *pLength ) {
		return false;
	}

	virtual void OnOpenDevice ( WAVEFORMATEX wfex  ){}
	virtual void OnCloseDevice ( void ) {}
	virtual void OnPlayDataProcess ( void *pPlayData , uint32_t DataSamples ) {}
	virtual void OnPlayBefore ( void ) {}
	virtual void OnPlayAfter ( void ) {}
	virtual void OnPlayStop ( bool bRepeatFlag , bool UserStopFlag ) {}
	virtual void OnPause ( void ) {}
	virtual void OnResume( void ) { }
	virtual void OnBeroreResume( void ) { }


	//イベント(実装必須)
	virtual uint32_t OnReadPlayData ( void *pPlayData , uint32_t ReadPos , uint32_t ReadSamples) = 0;




};


enum struct EHSWAVEOUT_STATE {
	Stop = 0,
	Play,
	Pause
};

enum struct EHSWAVEOUT_LOOPTYPE {
	None = 0,
	Normal,
	TwoPoint
};

template <typename tnUserDataType> class CHSWAVEOUT {
public:
	using TUserDataType = tnUserDataType;
	using TCallBackType = IHSWAVEOUT_CALLBACK<TUserDataType>;



private:


	CHSMalloc<char> MemoryForData;

	TCallBackType *pCallBack;	//コールバッククラス
	bool bSetupEnd;		//セットアップ済みかどうか

	WAVEFORMATEX format;	//フォーマット
	HWAVEOUT hwo;			//WAVEOUTハンドル

	WAVEHDR wh[2];			//WAVEHDR構造体×2
	HANDLE hDoneEvent[2];	//イベント×2
	WAVEHDR *pLastWriteInfo;	//現在地点で最後に書き込んだWAVEHDR構造体

	HANDLE hStopEvent;		//停止イベント
	bool bStopByUser;		//停止がユーザーによるものかどうか
	EHSWAVEOUT_STATE State;	//状態
	bool LastDataWriteFlag;	//最終の再生データを書き込んだか

	uint32_t PlayStartPosition; //再生開始位置
	uint32_t NextReadPosition;	//次のデータ読み込み位置
	EHSWAVEOUT_LOOPTYPE LoopType;

	uint32_t LoopStart;
	uint32_t LoopLength;
	bool bLoopRangeOverLoaded;
	bool bTwoPointLoopProcessed;

	double CurrentPitch;
	double CurrentPlaybackRate;

	uint8_t nVolume;


private:

	bool IsSetupEnd ( void ) {
		return bSetupEnd;
	}

	void PlayPreparation ( void ) {
		waveOutReset ( hwo );
		bLoopRangeOverLoaded = false;
		wh [ 0 ].dwUser = 0;
		wh [ 1 ].dwUser = 0;
		this->SetPitch ( CurrentPitch );
		this->SetPlaybackRate ( CurrentPlaybackRate );
		ResetEvent ( hDoneEvent [ 0 ] );
		ResetEvent ( hDoneEvent [ 1 ] );
		ResetEvent ( hStopEvent );

		bTwoPointLoopProcessed = false;

		bStopByUser = false;
		pLastWriteInfo = nullptr;
		LastDataWriteFlag = false;
	}


	void WriteNextPlayDataNormal ( WAVEHDR *pwh ) {

		pwh->dwBufferLength = pCallBack->OnReadPlayData ( pwh->lpData ,
			NextReadPosition , format.nSamplesPerSec );

		pCallBack->OnPlayDataProcess ( pwh->lpData , pwh->dwBufferLength / format.nBlockAlign );

		if ( pwh->dwBufferLength < format.nAvgBytesPerSec ) {
			this->LastDataWriteFlag = true;
		} else {
			NextReadPosition += format.nSamplesPerSec;
		}
	}

	uint32_t GetLoopEnd ( void ) {
		return LoopStart + LoopLength - 1;
	}

	bool WriteNextPlayData ( WAVEHDR *pwh ) {
		if ( LastDataWriteFlag ) return false;
		int WriteType = -1;
		if ( LoopType != EHSWAVEOUT_LOOPTYPE::TwoPoint ) {
			WriteNextPlayDataNormal ( pwh);
			WriteType = 0;
		} else {

			uint32_t LoopEnd = GetLoopEnd ( );
			uint32_t CurrentEnd = NextReadPosition + format.nSamplesPerSec - 1;

			if ( NextReadPosition > LoopEnd ) {
				WriteNextPlayDataNormal ( pwh);
				bLoopRangeOverLoaded = true;
				WriteType = 3;
			}else if ( LoopEnd > CurrentEnd ) {
				WriteNextPlayDataNormal ( pwh);
				WriteType = 1;
			} else {

				bTwoPointLoopProcessed = true;

				uint32_t BeforeLoopLoadSamples = LoopEnd - NextReadPosition + 1;
				uint32_t AfterLoopLoadSamples = format.nSamplesPerSec - BeforeLoopLoadSamples;


				uint32_t BeforeLoopReadSize =  pCallBack->OnReadPlayData ( pwh->lpData ,
					NextReadPosition , BeforeLoopLoadSamples );


				uint32_t AfterLoopReadSize = 0;
				
				if ( AfterLoopLoadSamples > 0 ) {
					AfterLoopReadSize = pCallBack->OnReadPlayData ( pwh->lpData + BeforeLoopReadSize ,
						LoopStart , AfterLoopLoadSamples );
				}


				NextReadPosition = LoopStart + AfterLoopReadSize / format.nBlockAlign;

				pwh->dwBufferLength = BeforeLoopReadSize + AfterLoopReadSize;

				pCallBack->OnPlayDataProcess ( pwh->lpData , pwh->dwBufferLength / format.nBlockAlign );
				WriteType = 2;
			}

		}
		this->pLastWriteInfo = pwh;
		MMRESULT mr =  waveOutWrite ( hwo , pwh , sizeof ( WAVEHDR ) );

		printf ( "waveOutWrite Called.(Ret=%d , Type=%d , LastFlag = %d)\n" , mr , WriteType ,this->LastDataWriteFlag);
		return true;
	}


	static void CALLBACK  s_waveOutProc ( HWAVEOUT hwo , UINT uMsg , DWORD_PTR dwInst , DWORD_PTR dwParam1 , DWORD_PTR dwParam2 ) {
		if ( uMsg == WOM_DONE ) {
			timeSetEvent ( 1 , 0 , s_TimerProc , dwInst , TIME_ONESHOT );
		}
	}

	static void CALLBACK s_TimerProc ( UINT uTimerID , UINT uMsg , DWORD_PTR dwUser , DWORD_PTR dw1 , DWORD_PTR dw2 ) {

		CHSWAVEOUT *p = ( CHSWAVEOUT* ) dwUser;
		p->local_OnTimer ( );
	}



	void CALLBACK local_OnTimer ( void ) {
		for ( int i = 0; i < 2; i++ ) {
			if ( wh [ i ].dwFlags &  WHDR_DONE ) {
				DWORD state = WaitForSingleObject ( this->hDoneEvent [ i ] , 0 );
				if ( state != WAIT_OBJECT_0 ) SetEvent ( hDoneEvent [ i ] );
			}
		}
	}

	static void s_ThreadProc ( void *pArg ) {
		CHSWAVEOUT *p = ( CHSWAVEOUT* ) pArg;
		printf ( "監視スレッド開始(thread id = %u)\n",GetCurrentThreadId ( ) );
		p->local_ThreadProc ( );
		printf ( "監視スレッド終了(thread id = %u)\n" , GetCurrentThreadId ( ) );
		_endthread ( );
	}

	void local_ThreadProc ( void ) {
		HANDLE hEvents [ 3 ];
		hEvents [ 0 ] = this->hDoneEvent [ 0 ];
		hEvents [ 1 ] = this->hDoneEvent [ 1 ];
		hEvents [ 2 ] = this->hStopEvent;
		DWORD  state;
		int index;
		bool bForceEnd;
		while ( true ) {
			state = WaitForMultipleObjects ( 3 , hEvents , FALSE , INFINITE );
			index = state - WAIT_OBJECT_0;
			if ( index == 2 ) break;

			bForceEnd = false;
			this->local_ThreadProc_Done ( &this->wh [ index ] , &bForceEnd);
			if ( bForceEnd ) break;
			ResetEvent ( hEvents [ index ] );
		}
//		pCallBack->OnPlayStop ( false , bStopByUser );
	}

	void local_ThreadProc_Done ( WAVEHDR *lpwh  , bool *pbForceEnd) {
		if ( LastDataWriteFlag || ( lpwh->dwUser != 0 ) ) {
			if ( lpwh == pLastWriteInfo ) {
#if 0
				char times [ 100 ];

				wsprintfA (times, "%u samples\n" , this->GetCurrentPosition ( ) );

				MessageBoxA ( GetConsoleWindow ( ) , times , "" , 0 );
#endif

				bool bRepeat = ( LoopType != EHSWAVEOUT_LOOPTYPE::None ) ? true : false;
				if ( lpwh->dwUser == 0 ) {
					waveOutReset ( hwo );
					bStopByUser = false;
				} else {
					bStopByUser = true;	
					bRepeat = false;
				}
				State = EHSWAVEOUT_STATE::Stop;
				SetEvent ( hStopEvent );

				pCallBack->OnPlayStop ( bRepeat , bStopByUser );
				if ( bRepeat )this->Play ( );
				*pbForceEnd = true;
				
			}
		} else {
			this->WriteNextPlayData ( lpwh );
		}
	}

public:

	CHSWAVEOUT ( ) {
		bSetupEnd = false;
		State = EHSWAVEOUT_STATE::Stop;
		CurrentPitch = 1;
		CurrentPlaybackRate = 1;
		hwo = 0;
		bLoopRangeOverLoaded = false;
		hDoneEvent [ 0 ] = NULL;
		hDoneEvent [ 1 ] = NULL;
		hStopEvent = NULL;
		memset ( &format , 0 , sizeof ( WAVEFORMATEX ) );
		this->nVolume = 100;

	}

	~CHSWAVEOUT ( ){
		this->Close ( );
		if ( hDoneEvent [ 0 ] ) CloseHandle ( hDoneEvent [ 0 ] );
		if ( hDoneEvent [ 1 ] ) CloseHandle ( hDoneEvent [ 1 ] );
		if ( hStopEvent ) CloseHandle ( hStopEvent  );
	}



	bool Setup ( TCallBackType *pCallBackClass , TUserDataType DefaultUserData ) {
		if ( pCallBackClass != nullptr ) {
			if ( pCallBackClass->OnSetup ( DefaultUserData ) ) {
				pCallBack = pCallBackClass;

				hDoneEvent [ 0 ] = CreateEvent ( nullptr , TRUE , FALSE , nullptr );
				if ( hDoneEvent [ 0 ] == NULL ) return false;
				
				hDoneEvent [ 1 ] = CreateEvent ( nullptr , TRUE , FALSE , nullptr );
				if ( hDoneEvent [ 1 ] == NULL ) return false;
				
				hStopEvent = CreateEvent ( nullptr , TRUE , FALSE , nullptr );
				if ( hStopEvent == NULL ) return false;

				bSetupEnd = true;
				return true;
			}
		}
		return false;
	}

	void ChangeCallBackUserData( TUserDataType UserData ) {
		if ( pCallBack ) {
			pCallBack->OnChangeUserData( UserData );
		}
	}

	bool Open ( WAVEFORMATEX wfex , UINT DeviceID = WAVE_MAPPER ) {
		if ( bSetupEnd == false ) return  false;
		if ( this->hwo != NULL ) return false;

		this->format = wfex;

		MMRESULT res = waveOutOpen ( &this->hwo , DeviceID , &format ,
			( DWORD_PTR )this->s_waveOutProc , ( DWORD_PTR )this , CALLBACK_FUNCTION );

		if ( res != MMSYSERR_NOERROR ) return false;

		for ( int i = 0; i < 2; i++ ) {
			WAVEHDR *lpwh = &wh [ i ];

			memset ( lpwh , 0 , sizeof ( WAVEHDR ) );

			lpwh->lpData = MemoryForData.Alloc ( wfex.nAvgBytesPerSec );
			lpwh->dwBufferLength = wfex.nAvgBytesPerSec;
			lpwh->dwFlags = 0;

			waveOutPrepareHeader ( hwo , lpwh , sizeof ( WAVEHDR ) );

			lpwh->dwFlags |= WHDR_BEGINLOOP | WHDR_ENDLOOP;
			lpwh->dwLoops = 1;

		}
		State = EHSWAVEOUT_STATE::Stop;
		pCallBack->OnOpenDevice ( wfex );
		this->SetVolume ( this->nVolume );
		return true;
	}


private:

	bool InnerPlay ( uint32_t  StartPositionSample = 0 , bool bPauseStart = false ) {
		if ( IsSetupEnd ( ) == false ) return false;

		if ( State != EHSWAVEOUT_STATE::Stop ) return false;

		pCallBack->OnPlayBefore ( );

		this->PlayPreparation ( );

		PlayStartPosition = StartPositionSample; //再生開始位置
		NextReadPosition = StartPositionSample;	//次のデータ読み込み位置

		waveOutPause ( hwo );

		for ( int i = 0; i < 2; i++ ) {
			this->WriteNextPlayData ( &wh [ i ] );
		}

		_beginthread ( s_ThreadProc , 0 , this );


		pCallBack->OnPlayAfter ( );
		if ( bPauseStart ) {
			State = EHSWAVEOUT_STATE::Pause;
			pCallBack->OnPause ( );
		} else {
			State = EHSWAVEOUT_STATE::Play;
			waveOutRestart ( hwo );
		}
		return true;
	}

public:
	bool Play ( uint32_t StartPositionSample = 0 ) {
		return InnerPlay ( StartPositionSample , false );
	}



	bool Stop ( void ) {
		if ( this->hwo == NULL ) return false;
		if ( State == EHSWAVEOUT_STATE::Stop ) return false;

		for ( int i = 0; i < 2; i++ ) {
			wh [ i ].dwUser = 1;
		}
		waveOutReset ( hwo );

		WaitForSingleObject ( hStopEvent , INFINITE );

		return true;
	}


	bool SeekTo ( uint32_t SeekPositionSample) {
		if ( this->hwo == NULL ) return false;
		EHSWAVEOUT_STATE s = State;
		if ( s == EHSWAVEOUT_STATE::Stop ) return false;
		this->Stop ( );

		this->InnerPlay ( SeekPositionSample , ( s == EHSWAVEOUT_STATE::Pause ) ? true : false );
		State = s;
		return true;
	}

	bool SeekToBack ( uint32_t BackSeeekSamples ) {
		uint32_t pos = this->GetCurrentPosition ( );
		uint32_t newPos = ( uint32_t ) max ( 0 , ( int64_t ) pos - ( int64_t ) BackSeeekSamples );
		return this->SeekTo ( newPos );
	}

	bool SeekToForward ( uint32_t ForwardSeeekSamples ) {
		uint32_t pos = this->GetCurrentPosition ( );
		uint32_t newPos = pos + ForwardSeeekSamples;
		uint32_t len;
		if ( pCallBack->GetLength ( &len ) ) {
			newPos = min ( len - 1 , newPos );
		}

		return this->SeekTo ( newPos );
	}

	bool SeekToBackTime ( uint32_t BackSeeekTimes ) {
		return this->SeekToBack ( static_cast< uint32_t >( BackSeeekTimes / 1000.0 * format.nSamplesPerSec ) );

	}

	bool SeekToForwardTime ( uint32_t ForwardSeeekTimes ) {
		return this->SeekToForward ( static_cast< uint32_t >( ForwardSeeekTimes / 1000.0 * format.nSamplesPerSec ) );
	}


	bool Pause ( void ) {
		if ( this->hwo == NULL ) return false;
		EHSWAVEOUT_STATE s = State;
		if ( s != EHSWAVEOUT_STATE::Play ) return false;
		waveOutPause ( hwo );
		State = EHSWAVEOUT_STATE::Pause;
		pCallBack->OnPause ( );
		return true;
	}

	bool Resume ( void ) {
		if ( this->hwo == NULL ) return false;
		EHSWAVEOUT_STATE s = State;
		if ( s != EHSWAVEOUT_STATE::Pause ) return false;
		pCallBack->OnBeroreResume( );
		waveOutRestart ( hwo );
		State = EHSWAVEOUT_STATE::Play; 
		pCallBack->OnResume ( );
		return true;
	}


	bool Close ( void ) {
		if ( this->hwo == NULL ) return false;

		this->Stop ( );

		for ( int i = 0; i < 2; i++ ) {
			WAVEHDR *lpwh = &wh [ i ];
			waveOutUnprepareHeader ( hwo , lpwh , sizeof ( WAVEHDR ) );
			MemoryForData.Free ( lpwh->lpData );
		}

		waveOutClose ( hwo );

		hwo = NULL;
		State = EHSWAVEOUT_STATE::Stop;
		pCallBack->OnCloseDevice ( );

		return true;
	}
	
	bool SetRepeatType ( EHSWAVEOUT_LOOPTYPE Type ) {
		if ( Type == EHSWAVEOUT_LOOPTYPE::TwoPoint ) {
			if ( LoopLength == 0 ) return false;
			LoopType = EHSWAVEOUT_LOOPTYPE::TwoPoint;
		} else {
			LoopType = Type;
		}
		return true;
	}

	bool SetPointLoopSample ( uint32_t Start , uint32_t Length ) {
		if ( Length == 0 ) return false;
		this->LoopStart = Start;
		this->LoopLength = Length;
		return true;
	}

	uint32_t  GetTotalPosition ( void ) {
		MMTIME mt;
		mt.wType = TIME_SAMPLES;
		if ( this->hwo == NULL ) return 0;
		if ( waveOutGetPosition ( this->hwo , &mt , sizeof ( MMTIME ) ) != MMSYSERR_NOERROR ) {
			return 0;
		}
		return mt.u.sample;
	}
	uint32_t  GetCurrentPosition ( void ) {

		uint32_t time_included_startpoint = this->PlayStartPosition + this->GetTotalPosition ( );
		//printf ( "time_included_startpoint : %u\n" , time_included_startpoint );
		if ( this->LoopType == EHSWAVEOUT_LOOPTYPE::TwoPoint ) {
			if ( time_included_startpoint < LoopStart ) return time_included_startpoint;
			if ( bLoopRangeOverLoaded ) return time_included_startpoint;
			uint32_t buf = time_included_startpoint - LoopStart;
			return buf % LoopLength + LoopStart;
		} else {
			return time_included_startpoint;
		}

	}

	bool SetCurrentPosition ( uint32_t PositionSample ) {
		return this->SeekTo(PositionSample);
	}

	bool SetCurrentPositionTime ( uint32_t Position ) {
		return this->SetCurrentPosition ( static_cast< uint32_t >( Position / 1000.0 * format.nSamplesPerSec ) );
	}


	uint32_t  GetCurrentPositionTime ( void ) {
		__int64 t = this->GetCurrentPosition();
		return static_cast< uint32_t >( ( t * 1000 ) / format.nSamplesPerSec );
	}

	uint32_t  GetTotalPositionTime ( void ) {
		__int64 t = this->GetTotalPosition ( );
		return static_cast< uint32_t >( ( t * 1000 ) / format.nSamplesPerSec );
	}



	WAVEFORMATEX GetFormat ( void ) {
		return format;
	}

	EHSWAVEOUT_STATE GetState ( void ) {
		return this->State;
	}


	bool IsSupportedPitch ( void ) {
		if ( this->hwo == NULL ) return false;

		WAVEOUTCAPS woc;
		MMRESULT mr = waveOutGetDevCaps ( ( UINT_PTR ) hwo , &woc , sizeof ( WAVEOUTCAPS ) );
		if ( mr == MMSYSERR_NOERROR ) {
			return (woc.dwSupport & WAVECAPS_PITCH) ? true : false;
		}
		return false;


	}


	bool IsSupportedPlaybackRate ( void ) {
		if ( this->hwo == NULL ) return false;

		WAVEOUTCAPS woc;
		MMRESULT mr = waveOutGetDevCaps (( UINT_PTR)hwo , &woc , sizeof ( WAVEOUTCAPS ) );
		if ( mr == MMSYSERR_NOERROR ) {
			return ( woc.dwSupport & WAVECAPS_PLAYBACKRATE ) ? true : false;
		}
		return false;


	}

	bool SetPitch ( double pitch ) {
		if ( this->hwo == NULL ) return false;
		DWORD  base = 0x00010000;
		DWORD  pitchvalue = static_cast< DWORD >( base *pitch );
		MMRESULT mr = waveOutSetPitch ( hwo , pitchvalue );
		if ( mr == MMSYSERR_NOERROR ) {
			CurrentPitch = pitch;
			return true;
		}
		return false;
	}

	bool SetPlaybackRate ( double rate ) {
		if ( this->hwo == NULL ) return false;
		DWORD  base = 0x00010000;
		DWORD  ratevalue = static_cast< DWORD >( base *rate );
		MMRESULT mr = waveOutSetPlaybackRate ( hwo , ratevalue );
		if ( mr == MMSYSERR_NOERROR ) {
			CurrentPlaybackRate = rate;
			return true;
		}
		return false;
	}


	uint8_t GetVolume ( void ) {
		return this->nVolume;
	}


	void SetVolume ( uint8_t Volume ) {
		this->nVolume = Volume;
		if ( this->nVolume > 100 ) this->nVolume = 100;
		if ( this->hwo != NULL ) {
			uint16_t value = (uint16_t)(( this->nVolume / 100.0 ) * 0xFFFF);
			waveOutSetVolume ( this->hwo , value | ( value << 16 ) );
		}
	}

};

using IHSWAVEOUT_CALLBACKDefault = IHSWAVEOUT_CALLBACK<void*>;
using CHSWAVEOUTDefault = CHSWAVEOUT<void*>;