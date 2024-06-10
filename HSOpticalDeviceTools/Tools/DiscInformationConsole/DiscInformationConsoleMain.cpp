#include <cstdio>
#include <string>
#include <iostream>
#include <locale>
#include <atlbase.h>
#include <atlstr.h>
#include "../../Libraries/CommonLib/CHSCompactDiscReader.hpp"

std::string Console_ReadLine( );

void DriveProcessEntry( char driveletter );

int main( void ) {

	SetConsoleTitleA( "DiscInformationConsole" );
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
	printf( "番号：[ドライブ文字] デバイス表示名\n" );
	printf( "%s\n", sep.c_str( ) );
	for ( uint8_t id = 0; id < optical_drives_enum.uOpticalDriveCount; id++ ) {

		printf( "%4u：[%c:]", id, optical_drives_enum.Drives[id].Letter );
		if ( optical_drives_enum.Drives[id].bIncludedInfo ) {
			printf( " %s", optical_drives_enum.Drives[id].Info.DisplayName );
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





}