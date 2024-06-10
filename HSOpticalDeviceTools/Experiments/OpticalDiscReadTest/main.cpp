#include <cstdio>
#include <string>
#include "../../Libraries/CommonLib/CHSCompactDiscReader.hpp"

void DriveUnitProcess( char driveLetter );
void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res, bool firstNewLine = true, std::string prefix = "" );
void ConsoleWrite_AddressData32( UHSSCSI_AddressData32 address, EHSSCSI_AddressFormType type );

int main( void ) {

	SetConsoleTitleA( "OpticalDiscReadTest" );


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
		if ( optical_drives_enum.Drives[i].bIncludedInfo ) printf( " %s", optical_drives_enum.Drives[i].Info.DisplayName );
		printf( "\n%s\n", sep.c_str( ) );

		DriveUnitProcess( optical_drives_enum.Drives[i].Letter );

		printf( "\n\n" );
	}

	//printf( "sizeof(THSSCSI_DiscInformationHeader) : %zu\n", sizeof( THSSCSI_DiscInformation ) );

	return 0;
}


void DriveUnitProcess( char driveLetter ) {

	CHSOpticalDrive drive;

	if ( !drive.open( driveLetter ) ) {
		printf( "�h���C�u���J���܂���ł����B\n" );
		return;
	}

	printf( "sizeof THSSCSI_MediaEventStatus: %zu\n", sizeof( THSSCSI_MediaEventStatus ) );
	EHSOD_TrayState tray = drive.checkTrayState( );
	printf( "[�g���C�̏��]  " );
	switch ( tray ) {
		case EHSOD_TrayState::Closed:
			printf( "�����Ă��܂��B" );
			break;
		case EHSOD_TrayState::Opened:
			printf( "�J����Ă��܂��B" );
			break;
		case EHSOD_TrayState::FailedGotStatus:
			printf( "��Ԃ̎擾�Ɏ��s���܂����B" );
			break;
		default:
			printf( "����`�̏�Ԃł��B" );
			break;

	}
	printf( "\n" );


	EHSSCSI_ReadyStatus readyState = drive.checkReady( nullptr );
	printf( "[�h���C�u�̏��] " );
	switch ( readyState ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "�A�N�Z�X���鏀�����ł��Ă��܂��B" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "�A�N�Z�X���鏀�����ł��Ă��܂���B" );
			break;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "��Ԏ擾�Ɏ��s���܂����B" );
			break;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "���f�B�A���}������Ă��܂���B" );
			break;
		default:
			printf( "����`�̏�Ԃł��B" );
			break;
	}
	printf( "\n" );

	if ( readyState != EHSSCSI_ReadyStatus::Ready ) return;

	CHSOpticalDriveGetConfigCmd configcmd( &drive );
	EHSSCSI_ProfileName mediaType = configcmd.getCurrentProfileName( );
	printf( "[�Z�b�g����Ă��郁�f�B�A�̎��] %s\n", configcmd.GetProfileNameString( mediaType , false ).c_str() );

	CHSCompactDiscReader reader( &drive );

#if 1
	THSSCSI_ReadCapacityResponse cap;
	if ( reader.readCapacity( &cap ) ) {

		printf( "\n[READ CAPACITY Command]\n" );
		printf( "\tLogicalBlockAddress�F%u\n", cap.LogicalBlockAddress );
		printf( "\tBlockLengthInBytes�F%u\n", cap.BlockLengthInBytes);
	}
#endif


	//THSSCSI_TrackResourcesInformation tri;
	//memset( &tri, 0, sizeof( tri ) );

#if 1
	THSSCSI_DiscInformation di;
	THSSCSI_InterpretedDiscInformation idi;


	if ( reader.readDiscInformation( &di , &idi ) ) {

		printf( "\n[READ DISC INFORMATION (Datatype:0) Command]\n" );
		printf( "\tDiscInformationLength�F%u\n", idi.DiscInformationLength );
		printf( "\tDiscStatus : %u\n", di.DiscStatus );
		printf( "\tStateOfLastSession : %u\n", di.StateOfLastSession );
		printf( "\tErasable : %u\n", di.Erasable );
		printf( "\tDiscInformationDataType�F%u\n", di.DiscInformationDataType );
		printf( "\tNumberOfFirstTrackOnDisc : %u\n", di.NumberOfFirstTrackOnDisc );
		printf( "\tNumberOfSessions : %u\n", idi.NumberOfSessions );
		printf( "\tFirstTrackNumberInLastSession : %u\n", idi.FirstTrackNumberInLastSession );
		printf( "\tLastTrackNumberInLastSession : %u\n", idi.LastTrackNumberInLastSession );

		printf( "\tBGFormatStatus : %u\n", di.BGFormatStatus );
		printf( "\tDAC_V : %u\n", di.DAC_V);
		printf( "\tURU : %u\n", di.URU );
		printf( "\tDBC_V : %u\n", di.DBC_V );
		printf( "\tDID_V : %u\n", di.DID_V );
		printf( "\tDiscType : %u\n", di.DiscType );


		printf( "\tDiscIdentification : 0x%08X (%u) \n", idi.DiscIdentification, idi.DiscIdentification );
		printf( "\tLastSessionLeadInStartAddress : 0x%08X (%u) \n", idi.LastSessionLeadInStartAddress , idi.LastSessionLeadInStartAddress );
		printf( "\tLastPossibleLeadOutStartAddress : 0x%08X (%u) \n", idi.LastPossibleLeadOutStartAddress , idi.LastPossibleLeadOutStartAddress );

		printf( "\tDisc Bar Code : " );
		for ( int i = 0; i < 8; i++ ) printf( "%02X ", di.DiscBarCode[i] );
		printf( "\n" );

		printf( "\tDiscApplicationCode : %u\n", di.DiscApplicationCode );
		printf( "\tNumberOfOPCTables : %u\n", di.NumberOfOPCTables );
	}

#endif
	if ( reader.isCDMediaPresent( )) {

		if ( reader.setSpeedMax(  ) ) {
			printf( "max speed OK\n" );
		}
		//drive.spinUp( nullptr, false );
		//drive.setPowerState( 3  , nullptr , false);

#if 0
		THSSCSI_FormattedTOC toc;

		if ( reader.readFormmatedTOC( &toc, EHSSCSI_AddressFormType::MergedMSF ) ) {

			printf( "\n[READ TOC/PMA/ATIP Command (Format=0000b:Formatted TOC, AppDefinedAddressType=" );
			
			switch ( toc.AddressType ) {
				case EHSSCSI_AddressFormType::SplittedMSF:
					printf( "SplittedMSF)]\n" );
					break;
				case EHSSCSI_AddressFormType::MergedMSF:
					printf( "MergedMSF)]\n" );
					break;
				case EHSSCSI_AddressFormType::LBA:
					printf( "LBA)]\n" );
					break;
			}

			printf( "\t�g���b�N���F%u\n", toc.CountOfTracks );
			printf( "\t�g���b�N���F\n" );
			
			for ( auto &it : toc.items ) {
				printf( "\t\tTrack %d:\n", it.first );
				printf( "\t\t\tADR�F%u\n", it.second.ADR );
				printf( "\t\t\tControl�F%u\n", it.second.Control );
				printf( "\t\t\tTrackType�F" );

			
				switch ( it.second.TrackType ) {
					case EHSSCSI_TrackType::Audio2Channel:
						printf( "Audio2Channel\n" );
						break;
					case EHSSCSI_TrackType::Audio2ChannelWithPreEmphasis:
						printf( "Audio2ChannelWithPreEmphasis\n" );
						break;
					case EHSSCSI_TrackType::Audio4Channel:
						printf( "Audio4Channel\n" );
						break;
					case EHSSCSI_TrackType::Audio4ChannelWithPreEmphasis:
						printf( "Audio4ChannelWithPreEmphasis\n" );
						break;
					case EHSSCSI_TrackType::DataUninterrupted:
						printf( "DataUninterrupted\n" );
						break;
					case EHSSCSI_TrackType::DataIncrement:
						printf( "DataIncrement\n" );
						break;
					case EHSSCSI_TrackType::Unknown:
						printf( "Unknown\n" );
						break;
				}


				printf( "\t\t\tPermittedDigitalCopy : %s\n", ( it.second.PermittedDigitalCopy ) ? "true" : "false" );

				switch ( toc.AddressType ) {
					case EHSSCSI_AddressFormType::SplittedMSF:
						printf( "\t\t\tTrackStartAddress : %02uM:%02uS:%02uF\n", it.second.TrackStartAddress.urawValues[1],
							it.second.TrackStartAddress.urawValues[2],
							it.second.TrackStartAddress.urawValues[3]
						);


						printf( "\t\t\tTrackEndAddress : %02uM:%02uS:%02uF\n", it.second.TrackEndAddress.urawValues[1],
							it.second.TrackEndAddress.urawValues[2],
							it.second.TrackEndAddress.urawValues[3]
						);

						printf( "\t\t\tLength : %02uM:%02uS:%02uF\n", it.second.TrackLength.urawValues[1],
							it.second.TrackLength.urawValues[2],
							it.second.TrackLength.urawValues[3]
						);

						break;
					case EHSSCSI_AddressFormType::MergedMSF:
						printf( "\t\t\tTrackStartAddress : %u\n", it.second.TrackStartAddress.u32Value);
						printf( "\t\t\tTrackEndAddress : %u\n", it.second.TrackEndAddress.u32Value );
						printf( "\t\t\tLength : %u\n", it.second.TrackLength.u32Value );

						break;
					case EHSSCSI_AddressFormType::LBA:
						printf( "\t\t\tTrackStartAddress : %d\n", it.second.TrackStartAddress.i32Value);
						printf( "\t\t\tTrackEndAddress : %d\n", it.second.TrackEndAddress.i32Value );
						printf( "\t\t\tLength : %d Sectors\n", it.second.TrackLength.i32Value );
						break;
					default:
						break;
				}

				printf( "\n" );
			}

			switch ( toc.AddressType ) {
				case EHSSCSI_AddressFormType::SplittedMSF:
					printf( "\tPointOfLeadOutAreaStart: %02uM:%02uS:%02uF\n", toc.PointOfLeadOutAreaStart.urawValues[1], toc.PointOfLeadOutAreaStart.urawValues[2], toc.PointOfLeadOutAreaStart.urawValues[3] );

					break;
				case EHSSCSI_AddressFormType::MergedMSF:
					printf( "\tPointOfLeadOutAreaStart : %u\n", toc.PointOfLeadOutAreaStart.u32Value );
					break;
				case EHSSCSI_AddressFormType::LBA:
					printf( "\tPointOfLeadOutAreaStart : %d\n", toc.PointOfLeadOutAreaStart.i32Value );
					break;
				default:
					break;
			}

			printf( "\n" );
		}
#endif
#if 0
		THSSCSI_RawTOC rtoc;

		if ( reader.readRawTOC( &rtoc, EHSSCSI_AddressFormType::LBA ) ) {

			printf( "\n[READ TOC/PMA/ATIP Command (Format=0010b:Raw TOC, AppDefinedAddressType=" );

			switch ( rtoc.AddressType ) {
				case EHSSCSI_AddressFormType::SplittedMSF:
					printf( "SplittedMSF)]\n" );
					break;
				case EHSSCSI_AddressFormType::MergedMSF:
					printf( "MergedMSF)]\n" );
					break;
				case EHSSCSI_AddressFormType::LBA:
					printf( "LBA)]\n" );
					break;
			}

			printf( "\t���Z�b�V�������F%u\n", rtoc.CountOfSessions );
			printf( "\t���g���b�N���F%u\n", rtoc.CountOfTracks );

			printf( "\n" );


			for ( auto session : rtoc.sessionItems ) {
				printf( "\t[�Z�b�V���� %u]\n", session.second.SessionNumber );
				printf( "\t\t�g���b�N�ԍ��͈́F%u �` %u\n", session.second.FirstTrackNumber, session.second.LastTrackNumber );
				printf( "\t\tDiscType�F" );
				switch ( session.second.DiscType ) {
					case EHSSCSI_DiscType::Mode1:
						printf( "Mode-1" );
						break;
					case EHSSCSI_DiscType::CD_I:
						printf( "CD-I" );
						break;
					case EHSSCSI_DiscType::Mode2:
						printf( "Mode-2" );
						break;
				}
				
				printf( "\n\n" );


				for ( uint8_t track_no = session.second.FirstTrackNumber; track_no <= session.second.LastTrackNumber; track_no++ ) {
					printf( "\t\t[�g���b�N %u]\n", track_no );

					auto& track = rtoc.trackItems[track_no];



					printf( "\t\t\tADR�F%u\n", track.ADR );
					printf( "\t\t\tControl�F%u\n", track.Control );
					printf( "\t\t\tTrackType�F" );


					switch ( track.TrackType ) {
						case EHSSCSI_TrackType::Audio2Channel:
							printf( "Audio2Channel\n" );
							break;
						case EHSSCSI_TrackType::Audio2ChannelWithPreEmphasis:
							printf( "Audio2ChannelWithPreEmphasis\n" );
							break;
						case EHSSCSI_TrackType::Audio4Channel:
							printf( "Audio4Channel\n" );
							break;
						case EHSSCSI_TrackType::Audio4ChannelWithPreEmphasis:
							printf( "Audio4ChannelWithPreEmphasis\n" );
							break;
						case EHSSCSI_TrackType::DataUninterrupted:
							printf( "DataUninterrupted\n" );
							break;
						case EHSSCSI_TrackType::DataIncrement:
							printf( "DataIncrement\n" );
							break;
						case EHSSCSI_TrackType::Unknown:
							printf( "Unknown\n" );
							break;
					}


					printf( "\t\t\tPermittedDigitalCopy : %s\n", ( track.PermittedDigitalCopy ) ? "true" : "false" );
						
					printf( "\t\t\tTrackStartAddress : " );
					ConsoleWrite_AddressData32( track.TrackStartAddress, rtoc.AddressType );


					printf( "\n\t\t\tTrackEndAddress�F" );
					ConsoleWrite_AddressData32( track.TrackEndAddress, rtoc.AddressType );


					printf( "\n\t\t\tLength : " );
					ConsoleWrite_AddressData32( track.TrackLength, rtoc.AddressType );


					printf( "\n\n" );

				}
				printf( "\n\t\t���[�h�A�E�g�̊J�n�ʒu�F" );
				ConsoleWrite_AddressData32( session.second.PointOfLeadOutAreaStart, rtoc.AddressType );
				printf( "\n\n" );



			}
		}
#endif

#if 0
		CHSSCSIGeneralBuffer buf;

		drive.spinUp( nullptr, false );

		reader.readAudioTrack( &buf, 2, CHSCompactDiscReader::MakeAddressData32( 0,5 , 0 ), EHSSCSI_AddressFormType::SplittedMSF,
			CHSCompactDiscReader::MakeAddressData32( 0, 5, 0 ), EHSSCSI_AddressFormType::SplittedMSF );

		drive.spinDown( );
#endif

	
	}

}


void ConsoleWrite_AddressData32( UHSSCSI_AddressData32 address, EHSSCSI_AddressFormType type ) {

	switch ( type ) {
		case EHSSCSI_AddressFormType::SplittedMSF:
			printf( "%02uM�F%02uS�F%02uF", address.urawValues[1], address.urawValues[2], address.urawValues[3] );
			break;
		case EHSSCSI_AddressFormType::MergedMSF:
			printf( "%u", address.u32Value );
			break;
		case EHSSCSI_AddressFormType::LBA:
			printf( "%d", address.i32Value );
			break;
	}

}

void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res, bool firstNewLine, std::string prefix ) {
	if ( firstNewLine ) printf( "\n" );
	printf( "%s[Raw Results]\n", prefix.c_str( ) );
	printf( "%sOperationCode�F0x%02X\n", prefix.c_str( ), res.executedOperationCode );
	printf( "%sDeviceIOControlResult�F0x%02X\n", prefix.c_str( ), res.DeviceIOControlResult );
	printf( "%sDeviceIOControlLastError�F0x%02X\n", prefix.c_str( ), res.DeviceIOControlLastError );
	printf( "%sScsiStatus�F0x%02X\n", prefix.c_str( ), res.scsiStatus.statusByteCode );
	printf( "%sSK�F0x%02X\n", prefix.c_str( ), res.scsiSK );
	printf( "%sASC�F0x%02X\n", prefix.c_str( ), res.scsiASC );
	printf( "%sASCQ�F0x%02X\n", prefix.c_str( ), res.scsiASCQ );
	printf( "\n" );
}