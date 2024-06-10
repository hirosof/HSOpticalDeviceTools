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
		printf( "���w�h���C�u�̗񋓂Ɏ��s���܂���\n" );
		return 0;
	}

	if ( optical_drives_enum.uOpticalDriveCount == 0 ) {
		printf( "���w�h���C�u���ڑ�����Ă��܂���B\n" );
		return 0;
	}

	std::string sep( 80, '-' );

	printf( "�y���w�h���C�u���X�g�z\n\n" );
	printf( "�ԍ��F[�h���C�u����] �f�o�C�X�\����\n" );
	printf( "%s\n", sep.c_str( ) );
	for ( uint8_t id = 0; id < optical_drives_enum.uOpticalDriveCount; id++ ) {

		printf( "%4u�F[%c:]", id, optical_drives_enum.Drives[id].Letter );
		if ( optical_drives_enum.Drives[id].bIncludedInfo ) {
			printf( " %s", optical_drives_enum.Drives[id].Info.DisplayName );
		}
		printf( "\n" );
	}
	printf( "%s\n", sep.c_str( ) );
	printf( "\n" );

	uint32_t selectedOpticalDriveNumber = 0;

	if ( optical_drives_enum.uOpticalDriveCount == 1 ) {
		printf( "�ڑ�����Ă�����w�h���C�u��1�ł����̂ŁA�Y���̃h���C�u�������őI������܂����B\n" );
		selectedOpticalDriveNumber = 0;
	} else {

		printf( "��̃��X�g����g�p������w�h���C�u��ԍ��Ŏw�肵�Ă��������F" );

		while ( true ) {
			(void) scanf_s( "%u", &selectedOpticalDriveNumber );
			(void) Console_ReadLine( );

			if ( selectedOpticalDriveNumber < optical_drives_enum.uOpticalDriveCount ) {
				break;
			}

			printf( "�����Ȕԍ������͂���܂����B�w�����蒼���Ă��������F" );
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

	printf( "�y�h���C�u�̏�ԃ`�F�b�N�z\n" );

	if ( drive.open( driveletter ) == false ) {
		printf( "�h���C�u���J���܂���ł����B\n" );
		return;
	}

	if ( drive.isTrayOpened( ) ) {
		printf( "�h���C�u�̃g���C���J����Ă��܂��B\n" );
		return;
	}

	EHSSCSI_ReadyStatus readyState = drive.checkReady( nullptr );

	switch ( readyState ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "�h���C�u�ɃA�N�Z�X���鏀�����ł��Ă��܂��B\n" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "�h���C�u�ɃA�N�Z�X���鏀�����ł��Ă��܂���B\n" );

			break;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "�h���C�u�̏�Ԏ擾�Ɏ��s���܂����B\n" );

			break;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "�h���C�u�Ƀ��f�B�A���}������Ă��܂���B\n" );
			break;
	}

	if ( readyState != EHSSCSI_ReadyStatus::Ready ) {
		return;
	}





}