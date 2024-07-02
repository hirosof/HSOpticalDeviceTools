#include <cstdio>
#include <string>
#include <iostream>
#include <locale>
#include <atlbase.h>
#include <atlstr.h>
#include "../../Libraries/CommonLib/CHSCompactDiscReader.hpp"
#include "../../common/CCommandLineParser.hpp"

std::string Console_ReadLine( );


CAtlStringW  SecondsToString( uint32_t seconds );
CAtlStringW GetCommaSplitNumberString( uint64_t number );


void DriveProcessEntry( char driveletter );
void PrintCommonMediaInfomation( CHSOpticalDrive* pDrive );
void PrintCDMediaInfomation( CHSOpticalDrive* pDrive );

bool PrintTOCInformation( CHSOpticalDrive* pDrive, THSSCSI_RawTOC* pRawTOC = nullptr );

void PrintDVDMediaInfomation( CHSOpticalDrive* pDrive );
void PrintBDMediaInfomation( CHSOpticalDrive* pDrive );


CCommandLineParserExW cmdlineParamParser;

int main( void ) {

	SetConsoleTitleA( "DiscInformationConsole" );
	setlocale( LC_ALL, "Japanese" );

	cmdlineParamParser.parse( GetCommandLineW( ) );


	THSEnumrateOpticalDriveInfo optical_drives_enum;
	if ( CHSOpticalDrive::EnumOpticalDrive( &optical_drives_enum ) == false ) {
		printf( "光学ドライブの列挙に失敗しました\n" );
		if ( !cmdlineParamParser.hasNamedOption( L"nowait" ) ) system( "pause" );
		return 0;
	}

	if ( optical_drives_enum.uOpticalDriveCount == 0 ) {
		printf( "光学ドライブが接続されていません。\n" );
		if ( !cmdlineParamParser.hasNamedOption( L"nowait" ) ) system( "pause" );
		return 0;
	}

	std::string sep( 80, '-' );

	printf( "# DiscInformationConsole\n\n" );

	printf( "## ドライブ選択\n\n" );

	printf( "### ドライブリスト\n\n" );
	printf( "|番号|ドライブ|デバイス表示名|\n" );
	printf( "|---|---|---|\n" );

	for ( size_t i = 0; i < optical_drives_enum.uOpticalDriveCount; i++ ) {
		printf( "|%zu|`%c:\\`|", i, optical_drives_enum.Drives[i].Letter );
		if ( optical_drives_enum.Drives[i].bIncludedInfo ) printf( "`%s`", optical_drives_enum.Drives[i].Info.DisplayName );
		printf( "|\n" );
	}
	printf( "\n\n" );



	printf( "### ユーザーによるドライブ指定\n\n" );
	printf( "```\n" );

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
	printf( "```\n\n" );


	printf( "### 指定されたドライブ\n\n```\n" );
	printf( "[%c:\\]", optical_drives_enum.Drives[selectedOpticalDriveNumber].Letter );
	if ( optical_drives_enum.Drives[selectedOpticalDriveNumber].bIncludedInfo ) {
		printf( " %s", optical_drives_enum.Drives[selectedOpticalDriveNumber].Info.DisplayName );
	}
	printf( "\n" );
	printf( "```\n\n" );


	DriveProcessEntry( optical_drives_enum.Drives[selectedOpticalDriveNumber].Letter );
	printf( "\n" );

	if (!cmdlineParamParser.hasNamedOption(L"nowait" ) ) system( "pause" );
	return 0;
}

std::string Console_ReadLine( ) {

	std::string input;

	std::getline( std::cin, input );

	return input;
}

CAtlStringW  SecondsToString( uint32_t seconds ) {

	CAtlStringW str;

	str.Format( L"%02d：%02d", seconds / 60, seconds % 60 );

	return str;
}


CAtlStringW GetCommaSplitNumberString( uint64_t number ) {

	CAtlStringW normalStr;
	normalStr.Format( L"%I64u", number );

	NUMBERFMTW fmt;
	wchar_t comma[] = L",";
	wchar_t dot[] = L".";

	fmt.NumDigits = 0;
	fmt.LeadingZero = 0;
	fmt.Grouping = 3;
	fmt.lpDecimalSep = dot;
	fmt.lpThousandSep = comma;
	fmt.NegativeOrder = 1;

	int reqlength = GetNumberFormatEx( LOCALE_NAME_INVARIANT,
		0,
		normalStr.GetString( ),
		&fmt,
		NULL,
		0
	);

	if ( reqlength == 0 ) return normalStr;

	std::shared_ptr<wchar_t>  target( new wchar_t[reqlength + 1] );

	int result_length = GetNumberFormatEx( LOCALE_NAME_INVARIANT,
		0,
		normalStr.GetString( ),
		&fmt,
		target.get( ),
		reqlength
	);

	if ( result_length == 0 ) return normalStr;

	return CAtlStringW( target.get( ) );
}


void DriveProcessEntry( char driveletter ) {

	CHSOpticalDrive drive;

	std::string sep( 80, '=' );

	printf( "### ドライブの状態チェック\n\n" );

	if ( drive.open( driveletter ) == false ) {
		printf( "```\n" );
		printf( "ドライブを開けませんでした。\n" );
		printf( "```\n" );
		return;
	}

	if ( drive.isTrayOpened( ) ) {
		printf( "```\n" );
		printf( "ドライブのトレイが開かれています。\n" );
		printf( "```\n" );
		return;
	}

	EHSSCSI_ReadyStatus readyState = drive.checkReady( nullptr );

	printf( "```\n" );
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
			printf( "ドライブにディスクが挿入されていません。\n" );
			break;
	}
	printf( "```\n\n" );

	if ( readyState != EHSSCSI_ReadyStatus::Ready ) {
		return;
	}

	CHSOpticalDriveGetConfigCmd cmd( &drive );


	printf( "## 挿入されているディスクの種類\n\n" );


	printf( "```\n" );
	printf( "% s\n",cmd.getCurrentProfileNameString( false ).c_str( ) );
	printf( "```\n\n" );


	PrintCommonMediaInfomation( &drive );
	printf( "\n" );


	EHSSCSI_ProfileFamilyName scsi_pfn = 	cmd.getCurrentProfileFamilyName( );

	switch ( scsi_pfn ) {
		case EHSSCSI_ProfileFamilyName::CD:
			PrintCDMediaInfomation( &drive );
			break;
		case EHSSCSI_ProfileFamilyName::DVD:
			PrintDVDMediaInfomation( &drive );
			break;
		case EHSSCSI_ProfileFamilyName::BD:
			PrintBDMediaInfomation( &drive );
			break;
	}

	printf( "\n" );

}



void PrintCommonMediaInfomation( CHSOpticalDrive* pDrive ) {
	if ( pDrive == nullptr )return;

	printf( "## ディスク情報 (READ DISC INFORMATION Command)\n\n" );

	printf( "* 準備中\n" );


}


void PrintCDMediaInfomation( CHSOpticalDrive* pDrive ) {
	if ( pDrive == nullptr )return;
	CHSCompactDiscReader reader( pDrive );
	THSSCSI_RawTOC toc;

	if ( PrintTOCInformation( pDrive  , &toc) ) {


	}
}

bool PrintTOCInformation( CHSOpticalDrive* pDrive , THSSCSI_RawTOC  *pRawTOC) {

	if ( pDrive == nullptr )return false;
	CHSCompactDiscReader reader( pDrive );

	printf( "## TOC情報 (READ TOC/PMA/ATIP Command：Raw TOC)\n\n" );

	THSSCSI_RawTOC toc;
	if ( !reader.readRawTOC( &toc, EHSSCSI_AddressFormType::LBA ) ) {
		printf( "```\n" );
		printf( "TOC情報の読み取りに失敗しました。\n" );
		printf( "```\n" );
		return false;
	}


	if ( pRawTOC != nullptr ) *pRawTOC = toc;

	printf( "|セッション|トラック|種類|モード|長さ(再生時間)|長さ(ミリ秒)|バイト単位のサイズ(※)|\n" );
	printf( "|---|---|---|---|---|---|---|\n" );
	THSSCSI_RawTOCTrackItem trackItem;
	EHSSCSI_TrackType trackType;
	for ( uint8_t i = toc.FirstTrackNumber; i <= toc.LastTrackNumber; i++ ) {

		trackItem = toc.trackItems[i];

		trackType = trackItem.TrackType;

		printf( "|`%u`|`%u`|",
			trackItem.SessionNumber,
			trackItem.TrackNumber );

		switch ( trackType ) {
			case EHSSCSI_TrackType::Audio2Channel:
				printf( "`オーディオ (2ch)`|" );
				break;
			case EHSSCSI_TrackType::Audio2ChannelWithPreEmphasis:
				printf( "`オーディオ (2ch)`<br>`(プリエンファシス)`|" );
				break;
			case EHSSCSI_TrackType::Audio4Channel:
				printf( "`オーディオ (4ch)`|" );
				break;
			case EHSSCSI_TrackType::Audio4ChannelWithPreEmphasis:
				printf( "`オーディオ (4ch)`<br>`(プリエンファシス)`|" );
				break;
			case EHSSCSI_TrackType::DataUninterrupted:
			case EHSSCSI_TrackType::DataIncrement:
				printf( "`データ`|" );
				break;
			case EHSSCSI_TrackType::Unknown:
			default:
				printf( "`Unknown`|" );
				break;
		}


		EHSSCSI_DiscType tocDiscType = toc.sessionItems[trackItem.SessionNumber].DiscType;
		switch ( tocDiscType ) {
			case EHSSCSI_DiscType::Mode1:
				printf( "`Mode 1`|" );
				break;
			case EHSSCSI_DiscType::CD_I:
				printf( "`CD-I`|" );
				break;
			case EHSSCSI_DiscType::Mode2:
				printf( "`Mode 2`|" );
				break;
			default:
				printf( "`Unknown`|" );
				break;
		}


		UHSSCSI_AddressData32 msf, msf_split;

		msf = trackItem.TrackLength;
		msf_split = CHSCompactDiscReader::SplitMSF( msf );
		printf( "`%02d：%02d.%03d`|`%S`|",
			msf_split.urawValues[1],
			msf_split.urawValues[2],
			msf_split.urawValues[3] * 1000 / 75,
			GetCommaSplitNumberString(msf.u32Value * 1000 / 75).GetString()
		);


		printf( "`%S`|", GetCommaSplitNumberString( msf.u32Value * CHSCompactDiscReader::NormalCDDATrackSectorSize ).GetString() );

		printf( "\n" );

	}

	printf( "\n" );

	printf( "**※ 1 Sector = %zu Bytes (オーディオトラック以外はヘッダーなどのユーザーデータ以外も含みます)**\n", CHSCompactDiscReader::NormalCDDATrackSectorSize );

	printf( "\n" );


	printf( "<details>\n\n" );
	printf( "<summary>LBA単位(セクタ単位)の情報</summary>\n\n" );
	printf( "|トラック|開始位置|終了位置|長さ|\n" );
	printf( "|---|---|---|---|\n" );


	for ( uint8_t i = toc.FirstTrackNumber; i <= toc.LastTrackNumber; i++ ) {

		trackItem = toc.trackItems[i];

		printf( "|`%u`|", trackItem.TrackNumber );

		printf( "`%S`|`%S`|`%S`|",
			GetCommaSplitNumberString( trackItem.TrackStartAddress.u32Value ).GetString( ),
			GetCommaSplitNumberString( trackItem.TrackEndAddress.u32Value ).GetString( ),
			GetCommaSplitNumberString( trackItem.TrackLength.u32Value ).GetString( ) 
		);
		

		printf( "\n" );

	}

	printf( "\n</details>\n\n" );


	printf( "<details>\n\n" );
	printf( "<summary>MSF単位の情報</summary>\n\n" );
	printf( "|トラック|" );	
	printf( "開始位置<br>(Frames)|終了位置<br>(Frames)|長さ<br>(Frames)|" );
	printf("開始位置<br>( MM:SS:FF ) | 終了位置<br>( MM:SS:FF ) | 長さ<br>( MM:SS:FF ) | " );
	printf( "\n" );

	printf( "|---|---|---|---|---|---|---|\n" );


	CAtlStringA frames_str;
	CAtlStringA splitted_frames_str;

	for ( uint8_t i = toc.FirstTrackNumber; i <= toc.LastTrackNumber; i++ ) {

		trackItem = toc.trackItems[i];

		printf( "|`%u`|", trackItem.TrackNumber );

		UHSSCSI_AddressData32 msf, msf_split;

		frames_str.Empty( );
		splitted_frames_str.Empty( );

		msf = CHSCompactDiscReader::LBA_to_MergedMSF( trackItem.TrackStartAddress );
		msf_split = CHSCompactDiscReader::SplitMSF( msf );
		splitted_frames_str.AppendFormat( "`%02d：%02d：%02d`|", msf_split.urawValues[1], msf_split.urawValues[2], msf_split.urawValues[3] );
		frames_str.AppendFormat( "`%S`|", GetCommaSplitNumberString(msf.u32Value).GetString() );

		msf = CHSCompactDiscReader::LBA_to_MergedMSF( trackItem.TrackEndAddress );
		msf_split = CHSCompactDiscReader::SplitMSF( msf );
		splitted_frames_str.AppendFormat( "`%02d：%02d：%02d`|", msf_split.urawValues[1], msf_split.urawValues[2], msf_split.urawValues[3] );
		frames_str.AppendFormat( "`%S`|", GetCommaSplitNumberString( msf.u32Value ).GetString( ) );

		msf = trackItem.TrackLength;
		msf_split = CHSCompactDiscReader::SplitMSF( msf );
		splitted_frames_str.AppendFormat( "`%02d：%02d：%02d`|", msf_split.urawValues[1], msf_split.urawValues[2], msf_split.urawValues[3] );
		frames_str.AppendFormat( "`%S`|", GetCommaSplitNumberString( msf.u32Value ).GetString( ) );


		printf( "%s", frames_str.GetString( ) );
		printf( "%s", splitted_frames_str.GetString( ) );

		printf( "\n" );

	}

	printf( "\n</details>\n" );



	printf( "\n<details>\n\n" );
	printf( "<summary>TOC Track Descriptors(生データ)</summary>\n\n" );
	printf( "|Index|Session<br>Number|CONTROL|ADR|TNO|POINT|Min|Sec|Frame|PMIN|PSEC|PFRAME|" );
	printf( "\n" );

	printf( "|---|---|---|---|---|---|---|---|---|---|---|---|\n" );

	size_t index = 0;
	for ( const THSSCSI_RawTOCRawItem& item : toc.rawItems ) {
		printf( "|`%zu`|`%u`|`%u`|`%u`|`%u`|`0x%02X (%3u)`|`%02u`|`%02u`|`%02u`|`%02u`|`%02u`|`%02u`|\n",
			index,
			item.SessionNumber,
			item.Control,
			item.ADR,
			item.TNO,
			item.POINT,
			item.POINT,
			item.Min,
			item.Sec,
			item.Frame,
			item.PMIN,
			item.PSEC,
			item.PFRAME
		);
		index++;
	}

	printf( "\n</details>\n" );

	return true;
}


void PrintDVDMediaInfomation( CHSOpticalDrive* pDrive ) {
	if ( pDrive == nullptr )return;

}
void PrintBDMediaInfomation( CHSOpticalDrive* pDrive ) {
	if ( pDrive == nullptr )return;


}