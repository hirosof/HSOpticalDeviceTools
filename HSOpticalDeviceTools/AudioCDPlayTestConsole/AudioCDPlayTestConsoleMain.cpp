#include <cstdio>
#include <string>
#include <iostream>
#include <locale>
#include <Windows.h>
#include <CommCtrl.h>
#include <atlbase.h>
#include <atlstr.h>
#include "AudioCDTrackReaderEngine.hpp"

#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"Comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' "	\
	"name='Microsoft.Windows.Common-Controls' "					\
	"version='6.0.0.0' "										\
	"processorArchitecture='*' "								\
	"publicKeyToken='6595b64144ccf1df' "						\
	"language='*'\"")


enum struct RequestAgainFlag {
	None=0,
	DriveSelect,
	TrackSelect
};

struct TProgressDialogData {
	CHSWAVEOUT_CDPlayInformation *pwo;
	CDPlayInformation playInformation;
	AudioCDTrackReaderEngine* pEngine;
	THSSCSI_RawTOC toc;
	bool bReShowDialogFlag;
	bool bFirstShowFlag;
	POINT ptWindow;
	RequestAgainFlag againFlag;
};

void HSShowDialog( TProgressDialogData* pData );
HRESULT CALLBACK TaskDialogProc( HWND hwnd, UINT uNotification, WPARAM wp, LPARAM lp, LONG_PTR dwRefData );

std::string Console_ReadLine( );

RequestAgainFlag DriveProcessEntry( char driveletter );
RequestAgainFlag TOCCheckAndSelect( CHSOpticalDrive* pDrive );
RequestAgainFlag CDPlayMain( CHSOpticalDrive* pDrive, THSSCSI_RawTOCTrackItem track , THSSCSI_RawTOC toc );

int main( void ) {

	SetConsoleTitleA( "AudioCDPlayTestConsole" );
	setlocale( LC_ALL, "Japanese" );

	RequestAgainFlag againFlag = RequestAgainFlag::None;

	do {

		THSEnumrateOpticalDriveInfo optical_drives_enum;
		if ( CHSOpticalDrive::EnumOpticalDrive( &optical_drives_enum ) == false ) {
			printf( "光学ドライブの列挙に失敗しました\n" );
			return 0;
		}

		if ( optical_drives_enum.uOpticalDriveCount == 0 ) {
			printf( "光学ドライブが接続されていません。\n" );
			return 0;
		}

		std::string sep( 80, '-' );

		printf( "【光学ドライブリスト】\n\n" );
		printf( "番号：[ドライブ文字] デバイス名\n" );
		printf( "%s\n", sep.c_str( ) );
		for ( uint8_t id = 0; id < optical_drives_enum.uOpticalDriveCount; id++ ) {

			printf( "%4u：[%c:]", id, optical_drives_enum.Drives[id].Letter );
			if ( optical_drives_enum.Drives[id].bIncludedInfo ) {
				printf( " %s", optical_drives_enum.Drives[id].Info.DeviceName );
			}
			printf( "\n" );
		}
		printf( "%s\n", sep.c_str( ) );
		printf( "\n" );

		uint32_t selectedOpticalDriveNumber = 0;

		if ( optical_drives_enum.uOpticalDriveCount == 1 ) {
			printf( "接続されている光学ドライブは1つでしたので、該当のドライブが自動で選択されました。\n" );
			selectedOpticalDriveNumber = 0;
		} else {

			printf( "上のリストから使用する光学ドライブを番号で指定してください：" );

			while ( true ) {
				(void) scanf_s( "%u", &selectedOpticalDriveNumber );
				(void) Console_ReadLine( );

				if ( selectedOpticalDriveNumber < optical_drives_enum.uOpticalDriveCount ) {
					break;
				}

				printf( "無効な番号が入力されました。指定をやり直してください：" );
			}


		}
		printf( "\n" );
		againFlag = DriveProcessEntry( optical_drives_enum.Drives[selectedOpticalDriveNumber].Letter );
		printf( "\n" );

	} while ( againFlag == RequestAgainFlag::DriveSelect );

	return 0;
}

std::string Console_ReadLine( ) {

	std::string input;

	std::getline( std::cin, input );

	return input;
}


RequestAgainFlag DriveProcessEntry( char driveletter ) {

	CHSOpticalDrive drive;

	std::string sep( 80, '=' );

	printf( "【ドライブの状態チェック】\n" );

	if ( drive.open( driveletter ) == false ) {
		printf( "ドライブを開けませんでした。\n" );
		return RequestAgainFlag::None;
	}

	if ( drive.isTrayOpened( ) ) {
		printf( "ドライブのトレイが開かれています。\n" );
		return RequestAgainFlag::None;
	}

	EHSSCSI_ReadyStatus readyState = drive.checkReady( nullptr );

	switch ( readyState ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "ドライブにアクセスする準備ができています。\n" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "ドライブにアクセスする準備ができていません。\n" );

			break;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "ドライブの状態取得に失敗しました。\n" );

			break;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "ドライブにメディアが挿入されていません。\n" );
			break;
	}

	if ( readyState != EHSSCSI_ReadyStatus::Ready ) {
		return RequestAgainFlag::None;
	}


	printf( "\n【ディスクの状態チェック】\n" );
	CHSOpticalDriveGetConfigCmd cmd( &drive );
	EHSSCSI_ProfileName pn = cmd.getCurrentProfileName( );
	printf( "セットされているメディアの種類：%s\n", cmd.GetProfileNameString( pn ).c_str( ) );


	CHSCompactDiscReader cdreader( &drive );
	if ( cdreader.isCDMediaPresent( ) == false ) {
		printf( "ドライブにセットされているメディアはCD系メディアではありません。\n" );
		return RequestAgainFlag::None;
	}

	THSSCSI_DiscInformation di;
	THSSCSI_InterpretedDiscInformation idi;
	if ( cdreader.readDiscInformation( &di, &idi ) ) {
		if ( di.DiscStatus == 0 ) {
			printf( "ドライブにセットされている%sはブランクメディアです。\n",
				cmd.GetProfileNameString( pn ).c_str( ) );

			return RequestAgainFlag::None;
		}
	}

	printf( "問題なし、TOC情報の確認と選択に移行します。\n\n" );

	return TOCCheckAndSelect( &drive );
}

RequestAgainFlag TOCCheckAndSelect( CHSOpticalDrive* pDrive ) {

	if ( pDrive == nullptr ) 		return RequestAgainFlag::None;

	printf( "【TOC情報】\n" );
	CHSCompactDiscReader cdreader( pDrive );
	THSSCSI_RawTOC rawToc;
	if ( cdreader.readRawTOC( &rawToc, EHSSCSI_AddressFormType::LBA ) == false ) {
		printf( "TOC情報の読み取りに失敗しました\n" );
		return RequestAgainFlag::None;
	}

	bool has_audio2channel_track = false;

	for ( auto& item : rawToc.trackItems ) {
		if ( item.second.TrackType == EHSSCSI_TrackType::Audio2Channel ) {
			has_audio2channel_track = true;
			break;
		}
	}

	if ( !has_audio2channel_track ) {
		printf( "再生に対応している形式のトラックがありません\n" );
		return RequestAgainFlag::None;
	} else {
		printf( "再生に対応している形式のトラックがありました、トラックリストを出力します。\n" );
	}


	RequestAgainFlag againFlag;

	do {

		printf( "\n【再生可能なトラックリスト】\n" );

		std::string separator( 100, '-' );

		printf( "%s\n", separator.c_str( ) );

		printf( "[%7s][Track] : %10s ～ %-10s (%8s)  ", "Session", "開始位置", "終了位置", "長さ" );
		printf( "[%02s : %02s : %02s]\tMiB単位のサイズ\n", "分", "秒", "フレーム" );

		printf( "%s\n", separator.c_str( ) );


		UHSSCSI_AddressData32 address32;
		double  mib_size;
		for ( uint8_t track_no = rawToc.FirstTrackNumber; track_no <= rawToc.LastTrackNumber; track_no++ ) {

			if ( rawToc.trackItems[track_no].TrackType != EHSSCSI_TrackType::Audio2Channel )continue;

			printf( "[%7u][%5u] : %10u ～ %-10u (%8u)  ",
				rawToc.trackItems[track_no].SessionNumber,
				track_no,
				rawToc.trackItems[track_no].TrackStartAddress.u32Value,
				rawToc.trackItems[track_no].TrackEndAddress.u32Value,
				rawToc.trackItems[track_no].TrackLength.u32Value );

			address32 = CHSCompactDiscReader::SplitMSF( rawToc.trackItems[track_no].TrackLength );

			printf( "[%02uM : %02uS : %02uF]\t", address32.urawValues[1], address32.urawValues[2], address32.urawValues[3] );

			mib_size = static_cast<double>( rawToc.trackItems[track_no].TrackLength.u32Value * CHSCompactDiscReader::NormalCDDATrackSectorSize );
			mib_size /= 1024 * 1024;

			printf( "%.2f MiB", mib_size );

			printf( "\n" );

		}
		printf( "%s\n", separator.c_str( ) );

		UINT RippingTrack = 0;

		printf( "\n【再生するトラックの選択】\n" );
		printf( "再生するトラック番号を入力してください：" );
		scanf_s( "%u", &RippingTrack );
		(void) Console_ReadLine( );

		if ( ( RippingTrack < rawToc.FirstTrackNumber ) || ( rawToc.LastTrackNumber < RippingTrack ) ) {
			printf( "\n不正なトラック番号が指定されました。\n" );
			return RequestAgainFlag::None;
		}

		if ( rawToc.trackItems[RippingTrack].TrackType != EHSSCSI_TrackType::Audio2Channel ) {
			printf( "\n再生不可能なトラック番号が指定されました。\n" );
			return RequestAgainFlag::None;
		}

		printf( "\n【再生処理開始】\n" );

		againFlag = CDPlayMain( pDrive, rawToc.trackItems[RippingTrack], rawToc );

	} while ( againFlag == RequestAgainFlag::TrackSelect );

	return againFlag;
}


RequestAgainFlag CDPlayMain( CHSOpticalDrive* pDrive, THSSCSI_RawTOCTrackItem track , THSSCSI_RawTOC toc ) {

	if ( pDrive == nullptr ) return RequestAgainFlag::None;

	RequestAgainFlag againFlag = RequestAgainFlag::None;

	CHSCompactDiscReader cdreader( pDrive );
	cdreader.setSpeedMax( );


	pDrive->spinUp( nullptr, false );

	WAVEFORMATEX wfex;
	wfex.wBitsPerSample = 16;
	wfex.wFormatTag = WAVE_FORMAT_PCM;
	wfex.nSamplesPerSec = 44100;
	wfex.nChannels = 2;
	wfex.nBlockAlign = wfex.wBitsPerSample / 8 * wfex.nChannels;
	wfex.nAvgBytesPerSec = wfex.nBlockAlign * wfex.nSamplesPerSec;
	wfex.cbSize = 0;

	AudioCDTrackReaderEngine engine;
	CHSWAVEOUT_CDPlayInformation wo;
	CDPlayInformation info;
	info.pDrive = pDrive;
	info.track = track;

	wo.SetRepeatType( EHSWAVEOUT_LOOPTYPE::Normal );
	
	if ( wo.Setup(&engine , info ) ) {
		printf( "Setup...done\n" );

		if ( wo.Open( wfex ) ) {
			printf( "Open..done\n" );

			wo.SetVolume( 50 );

			if ( wo.Play( ) ) {
				printf( "\nPlay...done\n\n" );


				TProgressDialogData data;
				data.bFirstShowFlag = true;
				data.playInformation = info;
				data.pwo = &wo;
				data.pEngine = &engine;
				data.toc = toc;
				data.againFlag = RequestAgainFlag::None;

				do {
					data.bReShowDialogFlag = false;
					HSShowDialog( &data );
				} while ( data.bReShowDialogFlag );

				wo.Stop( );

				wo.Close( );


				againFlag = data.againFlag;

			}
		}
	}

	printf( "\n" );
	pDrive->spinDown( nullptr, true );

	return againFlag;
}

enum struct HSTaskDialogCustomButton {
	Test = 200,
	SeekToTop = 100,
	SeekToBack30Sec,
	SeekToBack10Sec,
	SeekToForward10Sec,
	SeekToForward30Sec,
	PlayEnd,
	BackTrack,
	NextTrack,
	Volume,
	TrackSelect,
	DriveChange
};

enum struct HSTaskDialogVolumeCustomButton {
	_25Percents = 300,
	_50Percents,
	_75Percents,
	_100Percents
};

void HSShowDialog( TProgressDialogData* pData ) {

	TASKDIALOGCONFIG tc;
	memset( &tc, 0, sizeof( TASKDIALOGCONFIG ) );
	tc.cbSize = sizeof( TASKDIALOGCONFIG );

	tc.lpCallbackData = (LONG_PTR) pData;
	tc.pfCallback = TaskDialogProc;
	tc.dwFlags = TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER;
	tc.dwFlags |= TDF_EXPAND_FOOTER_AREA | TDF_EXPANDED_BY_DEFAULT;

	tc.dwCommonButtons = 0;
	tc.cxWidth = 360;

	wchar_t trackStr[64];
	THSOpticalDriveDeviceInfo di;

	if ( pData->playInformation.pDrive->getCurrentDeviceInfo( &di ) ) {
		wsprintfW( trackStr, L"[%C:] %S  / Track %02u\n", 
			pData->playInformation.pDrive->getCurrentDriveLetter(),
			di.DeviceName,
			pData->playInformation.track.TrackNumber );

	} else {
		wsprintfW( trackStr, L"Track %02u\n", pData->playInformation.track.TrackNumber );
	}

	tc.pszMainInstruction = trackStr;

	tc.pszWindowTitle = L"再生コントロールダイアログ";
	tc.pszContent = L"";	//コールバック先で更新

	tc.pszCollapsedControlText = L"曲情報表示";
	tc.pszExpandedControlText = L"曲情報非表示";


	TASKDIALOG_BUTTON tb[11];

	tb[0].nButtonID = (int) HSTaskDialogCustomButton::SeekToTop;
	tb[0].pszButtonText = L"最初に戻す";

	tb[1].nButtonID = (int) HSTaskDialogCustomButton::SeekToBack30Sec;
	tb[1].pszButtonText = L"30秒戻す";

	tb[2].nButtonID = (int) HSTaskDialogCustomButton::SeekToBack10Sec;
	tb[2].pszButtonText = L"10秒戻す";

	tb[3].nButtonID = (int) HSTaskDialogCustomButton::SeekToForward10Sec;
	tb[3].pszButtonText = L"10秒進める";

	tb[4].nButtonID = (int) HSTaskDialogCustomButton::SeekToForward30Sec;
	tb[4].pszButtonText = L"30秒進める";

	tb[5].nButtonID = (int) HSTaskDialogCustomButton::Volume;
	tb[5].pszButtonText = L"音量設定";

	tb[6].nButtonID = (int) HSTaskDialogCustomButton::BackTrack;
	tb[6].pszButtonText = L"前のトラック";

	tb[7].nButtonID = (int) HSTaskDialogCustomButton::NextTrack;
	tb[7].pszButtonText = L"次のトラック";

	tb[8].nButtonID = (int) HSTaskDialogCustomButton::TrackSelect;
	tb[8].pszButtonText = L"トラック指定変更";

	tb[9].nButtonID = (int) HSTaskDialogCustomButton::DriveChange;
	tb[9].pszButtonText = L"光学ドライブ変更";

	tb[10].nButtonID = (int) HSTaskDialogCustomButton::PlayEnd;
	tb[10].pszButtonText = L"再生終了";


	tc.cButtons = sizeof( tb ) / sizeof( TASKDIALOG_BUTTON );
	tc.pButtons = tb;


	tc.pszVerificationText = L"一時停止";

	tc.pszExpandedInformation = nullptr;
	

	BOOL check;
	TaskDialogIndirect( &tc, nullptr, nullptr, &check );
}

HRESULT CALLBACK TaskDialogProc( HWND hwnd, UINT uNotification, WPARAM wp, LPARAM lp, LONG_PTR dwRefData ) {

	TProgressDialogData* pData = reinterpret_cast<TProgressDialogData*>( dwRefData );
	CHSWAVEOUT_CDPlayInformation* pwo = pData->pwo;


	if ( uNotification == TDN_CREATED ) {
		SendMessageW( hwnd, TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELONG( 0, 1000 ) );

		if ( pData->bFirstShowFlag == false ) {
			SetWindowPos( hwnd, NULL, pData->ptWindow.x, pData->ptWindow.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE );
		} else {
			pData->bFirstShowFlag = false;
		}

	} else if ( uNotification == TDN_VERIFICATION_CLICKED ) {
		EHSWAVEOUT_STATE state = pwo->GetState( );

		switch ( state ) {
			case EHSWAVEOUT_STATE::Play:
				pwo->Pause( );
				SendMessageW( hwnd, TDM_SET_PROGRESS_BAR_STATE, PBST_PAUSED, 0 );
				break;
			case EHSWAVEOUT_STATE::Pause:
				pwo->Resume( );
				SendMessageW( hwnd, TDM_SET_PROGRESS_BAR_STATE, PBST_NORMAL, 0 );
				break;
		}
	} else if ( uNotification == TDN_BUTTON_CLICKED ) {

		HSTaskDialogCustomButton id = (HSTaskDialogCustomButton) wp;

		switch ( id ) {
			case HSTaskDialogCustomButton::SeekToTop:
				pwo->SeekTo( 0 );
				break;
			case HSTaskDialogCustomButton::SeekToBack30Sec:
				pwo->SeekToBackTime( 30 * 1000 );
				break;
			case HSTaskDialogCustomButton::SeekToBack10Sec:
				pwo->SeekToBackTime( 10 * 1000 );
				break;
			case HSTaskDialogCustomButton::SeekToForward10Sec:
				pwo->SeekToForwardTime( 10 * 1000 );
				break;
			case HSTaskDialogCustomButton::SeekToForward30Sec:
				pwo->SeekToForwardTime( 30 * 1000 );
				break;


			case HSTaskDialogCustomButton::Volume:
				{

					TASKDIALOGCONFIG tc;
					memset( &tc, 0, sizeof( TASKDIALOGCONFIG ) );
					tc.cbSize = sizeof( TASKDIALOGCONFIG );
					tc.dwFlags = TDF_USE_COMMAND_LINKS;
					tc.dwCommonButtons = TDCBF_CANCEL_BUTTON;
					tc.hwndParent = hwnd;
					tc.pszMainInstruction = L"音量を次から選択してください";
					tc.pszWindowTitle = L"音量設定ダイアログ";

					TASKDIALOG_BUTTON tb[4];
					tb[0].nButtonID = (int) HSTaskDialogVolumeCustomButton::_25Percents;
					tb[0].pszButtonText = L"25%";

					tb[1].nButtonID = (int) HSTaskDialogVolumeCustomButton::_50Percents;
					tb[1].pszButtonText = L"50%";

					tb[2].nButtonID = (int) HSTaskDialogVolumeCustomButton::_75Percents;
					tb[2].pszButtonText = L"75%";

					tb[3].nButtonID = (int) HSTaskDialogVolumeCustomButton::_100Percents;
					tb[3].pszButtonText = L"100%";

					tc.cButtons = 4;
					tc.pButtons = tb;
					int select;
					HRESULT hr = TaskDialogIndirect( &tc, &select, nullptr, nullptr );

					if ( SUCCEEDED( hr ) ) {
						if ( select != IDCANCEL ) {
							HSTaskDialogVolumeCustomButton vcb = (HSTaskDialogVolumeCustomButton) select;

							switch ( vcb ) {
								case HSTaskDialogVolumeCustomButton::_25Percents:
									pwo->SetVolume( 25 );
									break;
								case HSTaskDialogVolumeCustomButton::_50Percents:
									pwo->SetVolume( 50 );
									break;
								case HSTaskDialogVolumeCustomButton::_75Percents:
									pwo->SetVolume( 75 );
									break;
								case HSTaskDialogVolumeCustomButton::_100Percents:
									pwo->SetVolume( 100 );
									break;
							}

						}

					}

				}
				break;
			case HSTaskDialogCustomButton::BackTrack:
			case HSTaskDialogCustomButton::NextTrack:

				{
					uint8_t nextPlayTrackNumber = pData->playInformation.track.TrackNumber;
					bool bEnableNextPlayTrack = false;
					if ( id == HSTaskDialogCustomButton::BackTrack ) {
						while ( nextPlayTrackNumber > pData->toc.FirstTrackNumber ) {
							nextPlayTrackNumber--;
							if ( pData->toc.trackItems[nextPlayTrackNumber].TrackType == EHSSCSI_TrackType::Audio2Channel ) {
								bEnableNextPlayTrack = true;
								break;
							}
						}

					} else {
						while ( nextPlayTrackNumber < pData->toc.LastTrackNumber ) {
							nextPlayTrackNumber++;
							if ( pData->toc.trackItems[nextPlayTrackNumber].TrackType == EHSSCSI_TrackType::Audio2Channel ) {
								bEnableNextPlayTrack = true;
								break;
							}
						}
					}
					if ( bEnableNextPlayTrack == false ) return S_FALSE;


					pwo->Stop( );

					pData->playInformation.track = pData->toc.trackItems[nextPlayTrackNumber];

					pwo->ChangeCallBackUserData( pData->playInformation );

					pwo->Play( );

					pData->bReShowDialogFlag = true;

					return S_OK;

				}
			case HSTaskDialogCustomButton::PlayEnd:
				return S_OK;
			case HSTaskDialogCustomButton::TrackSelect:
				pData->againFlag = RequestAgainFlag::TrackSelect;
				return S_OK;
			case HSTaskDialogCustomButton::DriveChange:
				pData->againFlag = RequestAgainFlag::DriveSelect;
				return S_OK;

			default:
				break;
		}

		return S_FALSE;

	} else if ( uNotification == TDN_TIMER ) {

		CAtlStringW  str;
		uint32_t pos_time_ms = pwo->GetCurrentPositionTime( );
		uint32_t length_samples;
		pData->pEngine->GetLength( &length_samples );

		uint32_t length_time_ms = static_cast<uint32_t>(length_samples * 1000.0 / pwo->GetFormat( ).nSamplesPerSec);
		uint32_t pos_time = pos_time_ms / 1000;
		uint32_t length_time = length_time_ms / 1000;

		str.Format( L"再生位置(時間単位) = %02d:%02d:%02d / %02d:%02d:%02d\n",
			pos_time / 3600, pos_time % 3600 / 60, pos_time % 60,
			length_time / 3600, length_time % 3600 / 60, length_time % 60
		);

		str.AppendFormat( L"再生位置(サンプル単位) = %u / %u\n",
			pwo->GetCurrentPosition( ),
			length_samples
		);

		uint32_t PlayPosPermille = pos_time_ms * 1000 / length_time_ms;
		str.AppendFormat( L"再生位置(％) = %d.%d%%\n\n",
			PlayPosPermille / 10, PlayPosPermille % 10
		);

		SendMessageW( hwnd, TDM_SET_ELEMENT_TEXT, TDE_CONTENT, (LPARAM) str.GetString( ) );
		SendMessageW( hwnd, TDM_SET_PROGRESS_BAR_POS, PlayPosPermille, 0 );

	} else if ( uNotification == TDN_DESTROYED ) {

		RECT rect;

		GetWindowRect( hwnd, &rect );

		pData->ptWindow.x = rect.left;
		pData->ptWindow.y = rect.top;

	}
	return S_OK;

}