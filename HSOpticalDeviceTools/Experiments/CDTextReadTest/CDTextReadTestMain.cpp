#include <cstdio>
#include <string>
#include <iostream>
#include <locale>
#include "../../Libraries/CommonLib/CHSCompactDiscReader.hpp"

std::string Console_ReadLine( );

void DriveProcessEntry( char driveletter );
void DiscProcess( CHSOpticalDrive* pDrive );

int main( void ) {

	SetConsoleTitleA( "CDTextReadTest" );
	setlocale( LC_ALL, "Japanese" );

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
	DriveProcessEntry( optical_drives_enum.Drives[selectedOpticalDriveNumber].Letter );
	printf( "\n" );

	system( "pause" );
	return 0;
}

std::string Console_ReadLine( ) {

	std::string input;

	std::getline( std::cin, input );

	return input;
}


void DriveProcessEntry( char driveletter ) {

	CHSOpticalDrive drive;

	std::string sep( 80, '=' );

	printf( "【ドライブの状態チェック】\n" );

	if ( drive.open( driveletter ) == false ) {
		printf( "ドライブを開けませんでした。\n" );
		return;
	}

	if ( drive.isTrayOpened( ) ) {
		printf( "ドライブのトレイが開かれています。\n" );
		return;
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
		return;
	}



	printf( "\n【ディスクの状態チェック】\n" );
	CHSOpticalDriveGetConfigCmd cmd( &drive );
	EHSSCSI_ProfileName pn = cmd.getCurrentProfileName( );
	printf( "セットされているメディアの種類：%s\n", cmd.GetProfileNameString( pn ).c_str( ) );


	CHSCompactDiscReader cdreader( &drive );
	if ( cdreader.isCDMediaPresent( ) == false ) {
		printf( "ドライブにセットされているメディアはCD系メディアではありません。\n" );
		return;
	}

	THSSCSI_DiscInformation di;
	THSSCSI_InterpretedDiscInformation idi;
	if ( cdreader.readDiscInformation( &di, &idi ) ) {
		if ( di.DiscStatus == 0 ) {
			printf( "ドライブにセットされている%sはブランクメディアです。\n",
				cmd.GetProfileNameString( pn ).c_str( ) );

			return;
		}
	}

	printf( "\n\n" );

	DiscProcess( &drive );
}



void DiscProcess( CHSOpticalDrive* pDrive ) {

	if ( pDrive == nullptr ) return;

	printf( "【TOC情報】\n" );
	CHSCompactDiscReader cdreader( pDrive );
	THSSCSI_RawTOC rawToc;
	if ( cdreader.readRawTOC( &rawToc, EHSSCSI_AddressFormType::LBA ) == false ) {
		printf( "TOC情報の読み取りに失敗しました\n" );
		return;
	}

	bool has_audio2channel_track = false;

	for ( auto& item : rawToc.trackItems ) {
		if ( item.second.TrackType == EHSSCSI_TrackType::Audio2Channel ) {
			has_audio2channel_track = true;
			break;
		}
	}

	if ( !has_audio2channel_track ) {
		printf( "一般的なステレオチャンネルのオーディオトラックがありません\n" );
		return;
	} else {
		printf( "一般的なステレオチャンネルのオーディオトラックがありました、トラックリストを出力します。\n" );

	}


	printf( "\n【トラックリスト】\n" );

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


	printf( "CD-TEXT読み込み中..\n" );

	THSSCSI_CDTEXT_Information info;
	if ( cdreader.readCDText( &info ) == EHSSCSI_CDText_ReadResult::Success) {
		printf( "done\n\n" );


		for ( size_t i = 0; i < info.NumberOfBlocks; i++ ) {

			printf( "[CD-TEXT Block %zu]\n", i );

			printf( "isDoubleByteCharatorCode\n\t%s\n",( info.parsedItems[i].isDoubleByteCharatorCode ) ? "true" : "false" );

			printf( "\nAlbum\n" );
			printf( "\tName : %s\n", info.parsedItems[i].album.Name.c_str( ) );
			printf( "\tPerformerName : %s\n", info.parsedItems[i].album.PerformerName.c_str( ) );
			printf( "\tSongWriterName : %s\n", info.parsedItems[i].album.SongWriterName.c_str( ) );
			printf( "\tComposerName : %s\n", info.parsedItems[i].album.ComposerName.c_str( ) );
			printf( "\tArrangerName : %s\n", info.parsedItems[i].album.ArrangerName.c_str( ) );
			
			for ( uint8_t track_no = rawToc.FirstTrackNumber; track_no <= rawToc.LastTrackNumber; track_no++ ) {

				THSSCSI_CDTEXT_ParsedItemNames& trackItem = info.parsedItems[i].trackTitles[track_no];

				printf( "Track %u\n", track_no );
				printf( "\tName : %s\n", trackItem.Name.c_str( ) );
				printf( "\tPerformerName : %s\n", trackItem.PerformerName.c_str( ) );
				printf( "\tSongWriterName : %s\n", trackItem.SongWriterName.c_str( ) );
				printf( "\tComposerName : %s\n", trackItem.ComposerName.c_str( ) );
				printf( "\tArrangerName : %s\n", trackItem.ArrangerName.c_str( ) );


			}

			printf( "\n" );

		}



	} else {
		printf( "failed\n\n" );

	}

}