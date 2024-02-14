#include <cstdio>
#include <string>
#include "../CommonLib/CHSOpticalDiscReader.hpp"

void DriveUnitProcess( char driveLetter );

int main( void ) {

	SetConsoleTitleA( "OpticalDriveInformationConsole" );


	THSEnumrateOpticalDriveInfo optical_drives_enum;
	if ( CHSOpticalDrive::EnumOpticalDrive( &optical_drives_enum ) == false ) {
		printf( "光学ドライブの列挙に失敗しました\n" );
		return 0;
	}
	
	if ( optical_drives_enum.uOpticalDriveCount == 0 ) {
		printf( "光学ドライブが接続されていません。\n" );
		return 0;
	}

	std::string sep( 80, '=' );

	for ( size_t i = 0; i < optical_drives_enum.uOpticalDriveCount; i++ ) {
		printf( "%s\n", sep.c_str( ) );
		printf( "[%c:]", optical_drives_enum.Drives[i].Letter );
		if ( optical_drives_enum.Drives[i].bIncludedInfo ) printf( " %s", optical_drives_enum.Drives[i].Info.DeviceName );
		printf( "\n%s\n", sep.c_str( ) );

		DriveUnitProcess( optical_drives_enum.Drives[i].Letter );

		printf( "\n\n" );
	}


	return 0;
}


void DriveUnitProcess( char driveLetter ) {

	CHSOpticalDrive drive;

	if ( !drive.open( driveLetter ) ) {
		printf( "ドライブを開けませんでした。\n" );
		return;
	}

	EHSSCSI_ReadyStatus readyState = drive.checkReady( nullptr );

	switch ( readyState ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "ドライブの状態：アクセスする準備ができています。\n" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "ドライブの状態：アクセスする準備ができていません。\n" );
			return;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "ドライブの状態：状態取得に失敗しました。\n" );
			return;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "ドライブの状態：メディアが挿入されていません。\n" );
			return;
		default:
			printf( "ドライブの状態：未定義の状態です。\n" );
			return;
	}


	CHSOpticalDriveGetConfigCmd configcmd( &drive );
	EHSSCSI_ProfileName mediaType = configcmd.getCurrentProfileName( );
	printf( "セットされているメディアの種類：%s\n", configcmd.GetProfileNameString( mediaType ).c_str( ) );





}