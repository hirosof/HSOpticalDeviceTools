#include <cstdio>
#include <string>
#include "../../Libraries/CommonLib/CHSOpticalDiscReader.hpp"

void DriveUnitProcess( THSOpticalDriveInfo driveInfo );

int main( void ) {

	SetConsoleTitleA( "OpticalDriveInformationConsole" );


	THSEnumrateOpticalDriveInfo optical_drives_enum;
	if ( CHSOpticalDrive::EnumOpticalDrive( &optical_drives_enum ) == false ) {
		printf( "���w�h���C�u�̗񋓂Ɏ��s���܂���\n" );
		return 0;
	}
	
	if ( optical_drives_enum.uOpticalDriveCount == 0 ) {
		printf( "���w�h���C�u���ڑ�����Ă��܂���B\n" );
		return 0;
	}

	std::string sep( 80, '=' );
	printf( "# OpticalDriveInformationConsole\n\n" );
	printf( "## �h���C�u���X�g\n\n" );
	printf( "|�h���C�u����|�f�o�C�X��|\n" );
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
		printf( "�h���C�u���J���܂���ł����B\n" );
		return;
	}

	CHSOpticalDriveGetConfigCmd cmd( &drive );




}