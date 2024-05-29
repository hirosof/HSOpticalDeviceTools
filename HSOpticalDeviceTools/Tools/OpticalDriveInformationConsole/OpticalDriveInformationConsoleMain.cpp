#include <cstdio>
#include <string>
#include "../../Libraries/CommonLib/CHSOpticalDiscReader.hpp"

void DriveUnitProcess( THSOpticalDriveInfo driveInfo );

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
	printf( "# OpticalDriveInformationConsole\n\n" );
	printf( "## ドライブリスト\n\n" );
	printf( "|ドライブ文字|デバイス名|\n" );
	printf( "|---|---|\n" );

	for ( size_t i = 0; i < optical_drives_enum.uOpticalDriveCount; i++ ) {
		printf( "|`%c:\\`|", optical_drives_enum.Drives[i].Letter );
		if ( optical_drives_enum.Drives[i].bIncludedInfo ) printf( "`%s`", optical_drives_enum.Drives[i].Info.DeviceName );
		printf( "|\n" );
	}

	printf( "\n" );




	for ( size_t i = 0; i < optical_drives_enum.uOpticalDriveCount; i++ ) {
		printf( "## [%c:\\]", optical_drives_enum.Drives[i].Letter );
		if ( optical_drives_enum.Drives[i].bIncludedInfo ) printf( " %s", optical_drives_enum.Drives[i].Info.DeviceName );
		printf( "\n\n" );

		DriveUnitProcess( optical_drives_enum.Drives[i] );

		printf( "\n\n" );
	}


	return 0;
}


void DriveUnitProcess( THSOpticalDriveInfo driveInfo ) {

	CHSOpticalDrive drive;

	if ( !drive.open( driveInfo.Letter ) ) {
		printf( "ドライブを開けませんでした。\n" );
		return;
	}

	CHSOpticalDriveGetConfigCmd cmd( &drive );




}