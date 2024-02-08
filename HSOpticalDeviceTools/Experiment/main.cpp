#include <cstdio>
#include <string>

#include "CHSOpticalDriveExperiment.hpp"
#include "../CommonLib/CHSOpticalDriveGetConfigCmd.hpp"


void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res , bool firstNewLine = true , std::string prefix="" );
void ConsoleOut_FeatureDescriptorHeader( THSSCSI_FeatureDescriptorHeader* pHeader, bool firstNewLine = true, std::string prefix = "" );

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


			printf( "[�h���C�u�̏��]\n" );

			switch ( readyState ) {
				case EHSSCSI_ReadyStatus::Ready:
					printf( "\t�h���C�u�ɃA�N�Z�X���鏀�����ł��Ă��܂��B\n" );
					driveExp.spinDown( );

					break;
				case EHSSCSI_ReadyStatus::NotReady:
					printf( "\t�h���C�u�ɃA�N�Z�X���鏀�����ł��Ă��܂���B\n" );

					break;
				case EHSSCSI_ReadyStatus::FailedGotStatus:
					printf( "\t�h���C�u�̏�Ԏ擾�Ɏ��s���܂����B\n" );

					break;
				case EHSSCSI_ReadyStatus::MediumNotPresent:
					printf( "\t�h���C�u�Ƀ��f�B�A���}������Ă��܂���B\n" );
					break;
			}

			ConsoleOut_SPTD_RESULT( res, true ,"\t\t" );


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

			CHSOpticalDriveGetConfigCmd cmd( &driveExp );

			HSSCSI_Profiles profiles;

			if ( cmd.getSupportProfiles( &profiles ,true) ) {

				printf( "[�Ή����f�B�A�̎��]\n" );
				for ( auto pn: profiles ) {
					if ( pn.second != EHSSCSI_ProfileName::RemovableDisk ) {
						printf( "\t0x%04X : %s\n", pn.first, cmd.GetProfileNameString( pn.second, false ).c_str( ) );
					}
				}
				printf( "\n" );
			}

			if ( driveExp.isReady( ) ) {
				uint16_t cp;
				if ( cmd.getCurrentProfileNumber( &cp ) ) {
					printf( "[�}������Ă��郁�f�B�A�̎��] \n\t0x%04X : %s\n\n", cp, cmd.GetProfileNameString( cp ).c_str( ) );
				}

				printf( "[�}������Ă��郁�f�B�A�̑�܂��Ȏ��] \n\t%s\n", cmd.getCurrentProfileRoughTypeString( ).c_str( ) );

				printf( "\n" );
			}

			ULONG am;
			EHSOD_AlimentMaskType amt = driveExp.getAlimentMask( &am );
			if ( amt!= EHSOD_AlimentMaskType::FailedGodAliment) {
				printf( "[AlimentMask]\n\t%d", (int) amt );
				switch ( amt ) {
					case EHSOD_AlimentMaskType::ByteAliment:
						printf( " : ByteAliment" );
						break;
					case EHSOD_AlimentMaskType::WordAliment:
						printf( " : WordAliment" );
						break;
					case EHSOD_AlimentMaskType::DwordAliment:
						printf( " : DwordAliment" );
						break;
					case EHSOD_AlimentMaskType::DoubleDwordAliment:
						printf( " : DoubleDwordAliment" );
						break;
				}
				printf( " (Raw : 0x%08X)\n\n", am );
			}

			DWORD max_trans_size;
			if ( driveExp.getMaxTransferLength( &max_trans_size ) ) {
				printf( "[MaxTransferLength]\n\t%u bytes\n", max_trans_size );
			}
		

			size_t size;
			THSSCSI_FeatureInfo fi;
			size = cmd.execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE::All, 0, &fi );
			if ( size > 0 ) {
				printf( "\n[Support Feature Code List]\n" );

				printf( "\t" );

				for ( size_t i = 0; i < fi.Descriptors.size( ); i++ ) {

					printf( "0x%02X%02X",
						fi.Descriptors[i].pHeader->FeatureCode[0],
						fi.Descriptors[i].pHeader->FeatureCode[1] );

					if ( ( i + 1 ) != ( fi.Descriptors.size( ) ) )printf( ", " );
					if ( ( i + 1 ) % 10 == 0 ) printf( "\n\t" );
				}

				printf( "\n\n" );
			}


			THSSCSI_FeatureDescriptor_CDRead cdread_feature;
			if ( cmd.getCDReadFeatureDescriptor( &cdread_feature ) ) {

				printf( "[CD Read Feature Descriptor]\n" );
				
				ConsoleOut_FeatureDescriptorHeader( &cdread_feature.header, false, "\t" );
				printf( "\n\tCD-Text : %d\n", cdread_feature.CDText );
				printf( "\tC2 Flags : %d\n", cdread_feature.C2Flags );
				printf( "\tDAP : %d\n", cdread_feature.DAP );

				printf( "\n" );

			}


			THSSCSI_FeatureDescriptor_RemovableMedium rem_media_feature;
			if ( cmd.getRemovableMediumFeatureDescriptor( &rem_media_feature ) ) {

				printf( "\n[Removable Medium Feature Descriptor]\n" );
				ConsoleOut_FeatureDescriptorHeader( &rem_media_feature.header, false, "\t" );
				printf( "\n\tLock : %d\n", rem_media_feature.Lock );
				printf( "\tDBML : %d\n", rem_media_feature.DBML );
				printf( "\tPvnt Jmpr : %d\n", rem_media_feature.Pvnt_Jmpr );
				printf( "\tEject : %d\n", rem_media_feature.Eject );
				printf( "\tLoad : %d\n", rem_media_feature.Load );
				printf( "\tLoading Mechanism Type : %d\n", rem_media_feature.Loading_Mechanism_Type );
				printf( "\n" );

			}

			THSSCSI_FeatureDescriptor_DriveSerialNumber driveSerialNumber_desc;
			if ( cmd.getDriveSerialNumberFeatureDescriptor( &driveSerialNumber_desc) ) {
				printf( "\n[Drive Serial Number Feature Descriptor]\n" );
				ConsoleOut_FeatureDescriptorHeader( &driveSerialNumber_desc.header, false, "\t" );
				printf( "\n\tSerial Number : %s\n", driveSerialNumber_desc.SerialNumber );
			}





			driveExp.close( );
		}
		printf( "\n\n" );

	}

	return 0;
}


void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res, bool firstNewLine, std::string prefix ){
	if ( firstNewLine ) printf( "\n" );
	printf( "%s[Raw Results]\n", prefix.c_str() );
	printf( "%sOperationCode�F0x%02X\n", prefix.c_str( ), res.executedOperationCode );
	printf( "%sDeviceIOControlResult�F0x%02X\n", prefix.c_str( ), res.DeviceIOControlResult );
	printf( "%sDeviceIOControlLastError�F0x%02X\n", prefix.c_str( ), res.DeviceIOControlLastError );
	printf( "%sScsiStatus�F0x%02X\n", prefix.c_str( ), res.scsiStatus.statusByteCode );
	printf( "%sSK�F0x%02X\n", prefix.c_str( ), res.scsiSK );
	printf( "%sASC�F0x%02X\n", prefix.c_str( ), res.scsiASC );
	printf( "%sASCQ�F0x%02X\n", prefix.c_str( ), res.scsiASCQ );
	printf( "\n" );
}

void ConsoleOut_FeatureDescriptorHeader( THSSCSI_FeatureDescriptorHeader* pHeader, bool firstNewLine, std::string prefix ) {
	if ( pHeader == nullptr )return;
	if ( firstNewLine ) printf( "\n" );
	printf( "%sHeader:\n", prefix.c_str( ) );
	printf( "%s\tFeature Code : 0x%02X%02X\n", prefix.c_str( ),pHeader->FeatureCode[0], pHeader->FeatureCode[1] );
	printf( "%s\tCurrent : %d\n", prefix.c_str( ), pHeader->Current );
	printf( "%s\tPersistent : %d\n", prefix.c_str( ),pHeader->Persistent );
	printf( "%s\tVersion : %d\n", prefix.c_str( ),pHeader->Version );
	printf( "%s\tAdditionalLength : %d\n", prefix.c_str( ),pHeader->AdditionalLength );
}
