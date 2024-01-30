#include <cstdio>
#include <string>

#include "CHSOpticalDriveExperiment.hpp"
#include "../common/CHSOpticalDriveGetConfigCmd.hpp"


void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res , bool firstNewLine = true);

int main( void ) {

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
	CHSOpticalDriveExperiment driveExp;

	for ( size_t i = 0; i < optical_drives_enum.uOpticalDriveCount; i++ ) {


		printf( "%s\n", sep.c_str( ) );
		printf( "[%c:]", optical_drives_enum.Drives[i].Letter );
		if ( optical_drives_enum.Drives[i].bIncludedInfo ) printf( " %s", optical_drives_enum.Drives[i].Info.DeviceName );
		printf( "\n%s\n", sep.c_str( ) );

		if ( driveExp.open( optical_drives_enum.Drives[i].Letter ) ) {

			HSSCSI_SPTD_RESULT res;
			EHSSCSI_ReadyStatus readyState = driveExp.checkReady( &res );

			ConsoleOut_SPTD_RESULT( res, false );

			switch ( readyState ) {
				case EHSSCSI_ReadyStatus::Ready:
					printf( "\t\t�h���C�u�ɃA�N�Z�X���鏀�����ł��Ă��܂��B\n" );
					driveExp.spinDown( );

					break;
				case EHSSCSI_ReadyStatus::NotReady:
					printf( "\t\t�h���C�u�ɃA�N�Z�X���鏀�����ł��Ă��܂���B\n" );

					break;
				case EHSSCSI_ReadyStatus::FailedGotStatus:
					printf( "\t\t�h���C�u�̏�Ԏ擾�Ɏ��s���܂����B\n" );

					break;
				case EHSSCSI_ReadyStatus::MediumNotPresent:
					printf( "\t\t�h���C�u�Ƀ��f�B�A���}������Ă��܂���B\n" );
					break;
			}

			EHSOD_TrayState tray = driveExp.checkTrayState( &res );

			if ( tray != EHSOD_TrayState::FailedGotStatus ) {

				printf( "[�g���C�̏��]\n" );
				switch ( tray ) {
					case EHSOD_TrayState::Closed:
						printf( "\t�h���C�u�̃g���C�͕����Ă��܂��B\n" );
						break;
					case EHSOD_TrayState::Opened:
						printf( "\t�h���C�u�̃g���C�͊J����Ă��܂��B\n" );
						break;

				}
				printf( "\n" );

			}

			THSSCSI_FeatureInfo info;
			CHSOpticalDriveGetConfigCmd cmd( &driveExp );
			
			size_t size;
			THSSCSI_FeatureInfo fi;
			size = cmd.execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE::All, 0, &fi );
			if ( size > 0 ) {
				printf( "\n[Support Feature Number List]\n" );

				for ( auto info : fi.Descriptors ) {
					printf( "\t0x%02X%02X\n", info.pHeader->FeatureCode[0], info.pHeader->FeatureCode[1] );
				}				
				printf( "\n" );
			}
			

			HSSCSI_Profiles profiles;

			if ( cmd.getSupportProfiles( &profiles ,true) ) {

				printf( "[Support Profiles (Support Media Types)]\n" );
				for ( auto pn: profiles ) {
					printf( "\t0x%04X : %s\n", pn.first , cmd.GetProfileNameString( pn.second ).c_str( )  );
				}
				printf( "\n" );
			}

			if ( driveExp.isReady( ) ) {
				uint16_t cp;
				if ( cmd.getCurrentProfileNumber( &cp ) ) {
					printf( "[CurrentProfile (Current Media Type)] \n\t0x%04X : %s\n", cp, cmd.GetProfileNameString( cp ).c_str( ) );
				}

				printf( "\n" );
			}

			driveExp.close( );
		}
		printf( "\n\n" );

	}

	return 0;
}


void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res, bool firstNewLine ){

	if ( firstNewLine ) printf( "\n" );

	printf( "[Operation Code = 0x%02X]\n\tDeviceIOControlResult:%X , DeviceIOControlLastError:%X\n",
		res.executedOperationCode,
		res.DeviceIOControlResult, res.DeviceIOControlLastError );
	printf( "\tScsiStatus:0x%02X, ", res.scsiStatus.statusByteCode );
	printf( "SK=0x%02X, ASC=0x%02X, ASCQ=0x%02X\n", res.scsiSK, res.scsiASC, res.scsiASCQ );

}
