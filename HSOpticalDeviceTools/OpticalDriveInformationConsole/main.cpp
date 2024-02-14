#include <cstdio>
#include <string>
#include "../CommonLib/CHSOpticalDiscReader.hpp"

void DriveUnitProcess( char driveLetter );

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
		printf( "�h���C�u���J���܂���ł����B\n" );
		return;
	}

	EHSSCSI_ReadyStatus readyState = drive.checkReady( nullptr );

	switch ( readyState ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "�h���C�u�̏�ԁF�A�N�Z�X���鏀�����ł��Ă��܂��B\n" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "�h���C�u�̏�ԁF�A�N�Z�X���鏀�����ł��Ă��܂���B\n" );
			return;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "�h���C�u�̏�ԁF��Ԏ擾�Ɏ��s���܂����B\n" );
			return;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "�h���C�u�̏�ԁF���f�B�A���}������Ă��܂���B\n" );
			return;
		default:
			printf( "�h���C�u�̏�ԁF����`�̏�Ԃł��B\n" );
			return;
	}


	CHSOpticalDriveGetConfigCmd configcmd( &drive );
	EHSSCSI_ProfileName mediaType = configcmd.getCurrentProfileName( );
	printf( "�Z�b�g����Ă��郁�f�B�A�̎�ށF%s\n", configcmd.GetProfileNameString( mediaType ).c_str( ) );





}