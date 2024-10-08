#include <cstdio>
#include <string>

#include "CHSOpticalDriveExperiment.hpp"
#include "../../Libraries/CommonLib/CHSOpticalDriveGetConfigCmd.hpp"


void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res , bool firstNewLine = true , std::string prefix="" );
void ConsoleOut_FeatureDescriptorHeader( THSSCSI_FeatureDescriptorHeader* pHeader, bool firstNewLine = true, std::string prefix = "" );


void ConsoleOut_GUITHREADINFO( const GUITHREADINFO& info );



int main( void ) {

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
	CHSOpticalDriveExperiment driveExp;

	for ( size_t i = 0; i < optical_drives_enum.uOpticalDriveCount; i++ ) {


		printf( "%s\n", sep.c_str( ) );
		printf( "[%c:]", optical_drives_enum.Drives[i].Letter );
		if ( optical_drives_enum.Drives[i].bIncludedInfo ) printf( " %s", optical_drives_enum.Drives[i].Info.DisplayName );
		printf( "\n%s\n", sep.c_str( ) );

		if ( driveExp.open( optical_drives_enum.Drives[i].Letter ) ) {

			HSSCSI_SPTD_RESULT res;
			EHSSCSI_ReadyStatus readyState = driveExp.checkReady( &res );


			printf( "[ドライブの状態]\n" );

			switch ( readyState ) {
				case EHSSCSI_ReadyStatus::Ready:
					printf( "\tドライブにアクセスする準備ができています。\n" );
					break;
				case EHSSCSI_ReadyStatus::NotReady:
					printf( "\tドライブにアクセスする準備ができていません。\n" );

					break;
				case EHSSCSI_ReadyStatus::FailedGotStatus:
					printf( "\tドライブの状態取得に失敗しました。\n" );

					break;
				case EHSSCSI_ReadyStatus::MediumNotPresent:
					printf( "\tドライブにメディアが挿入されていません。\n" );
					break;
			}

			ConsoleOut_SPTD_RESULT( res, true ,"\t\t" );


			EHSOD_TrayState tray = driveExp.checkTrayState();

			if ( tray != EHSOD_TrayState::FailedGotStatus ) {

				printf( "[トレイの状態]\n" );
				switch ( tray ) {
					case EHSOD_TrayState::Closed:
						printf( "\tドライブのトレイは閉じられています。\n" );
						break;
					case EHSOD_TrayState::Opened:
						printf( "\tドライブのトレイは開かれています。\n" );
						break;

				}
				printf( "\n" );

			}

			CHSOpticalDriveGetConfigCmd cmd( &driveExp );

			HSSCSI_Profiles profiles;

			if ( cmd.getSupportProfiles( &profiles ,true) ) {

				printf( "[対応メディアの種類]\n" );
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
					printf( "[挿入されているメディアの種類] \n\t0x%04X : %s\n\n", cp, cmd.GetProfileNameString( cp ).c_str( ) );
				}

				printf( "[挿入されているメディアのファミリー名] \n\t%s\n", cmd.getCurrentProfileFamilyNameString( ).c_str( ) );

				printf( "\n" );
			}

			ULONG am;
			EHSOD_AlimentMaskType amt = driveExp.getAlimentMask( &am );
			if ( amt!= EHSOD_AlimentMaskType::FailedGotAliment) {
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

					printf( "0x%04X",
						fi.Descriptors[i].pHeader->FeatureCode );

					printf( " (%s)", ( fi.Descriptors[i].pHeader->Current ) ? "E" : "D" );

					if ( ( i + 1 ) != ( fi.Descriptors.size( ) ) )printf( ", " );
					if ( ( i + 1 ) % 10 == 0 ) printf( "\n\t" );
				}

				printf( "\n\n" );
			}


			THSSCSI_FeatureDescriptor_CDRead cdread_feature;
			if ( cmd.getFeatureCDRead( &cdread_feature ) ) {

				printf( "[CD Read Feature Descriptor]\n" );
				
				ConsoleOut_FeatureDescriptorHeader( &cdread_feature.header, false, "\t" );
				printf( "\n\tCD-Text : %d\n", cdread_feature.CDText );
				printf( "\tC2 Flags : %d\n", cdread_feature.C2Flags );
				printf( "\tDAP : %d\n", cdread_feature.DAP );

				printf( "\n" );

			}


			THSSCSI_FeatureDescriptor_RemovableMedium rem_media_feature;
			if ( cmd.getFeatureRemovableMedium( &rem_media_feature ) ) {

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
			if ( cmd.getFeatureDriveSerialNumber( &driveSerialNumber_desc) ) {
				printf( "\n[Drive Serial Number Feature Descriptor]\n" );
				ConsoleOut_FeatureDescriptorHeader( &driveSerialNumber_desc.header, false, "\t" );
				printf( "\n\tSerial Number : %s\n", driveSerialNumber_desc.SerialNumber );
			}


			THSSCSI_MediaEventStatus mes;
			HSSCSI_SPTD_RESULT mes_res;

			if ( driveExp.getMediaEventStatus( &mes, &mes_res ) ) {
				printf( "\n[GET EVENT STATUS NOTIFICATION (Media)]\n" );
				printf( "\tNotificationClass: 0x%02X\n", mes.header.NotificationClass );
				printf( "\tSupportedEventClass : 0x%02X\n", mes.header.SupportedEventClass );
				printf( "\tEventCode : 0x%X\n", mes.EventCode);
				printf( "\tDoorOrTrayOpen : %s\n", (mes.DoorOrTrayOpen) ? "true" : "false" );
				printf( "\tMediaPresent : %s\n", ( mes.MediaPresent ) ? "true" : "false" );
			}

			THSSCSI_MechanismStatus ms;
			HSSCSI_SPTD_RESULT ms_res;
			if ( driveExp.getMechanismStatus( &ms, &ms_res ) ) {
				printf( "\n[getMechanismStatus]\n" );
				printf( "\tDoorOpen : %s\n", (ms.DoorOpen) ? "true" : "false" );
			}



			driveExp.close( );
		}
		printf( "\n\n" );

	}



	GUITHREADINFO foregroundGUIInfo;
	GUITHREADINFO currentGUIInfo;

	foregroundGUIInfo.cbSize = currentGUIInfo.cbSize = sizeof( GUITHREADINFO );

	if ( GetGUIThreadInfo( NULL, &foregroundGUIInfo ) ) {
		printf( "[foreground]\n" );
		ConsoleOut_GUITHREADINFO( foregroundGUIInfo );
		printf( "\n" );
	}

	DWORD dww;
	if ( GetGUIThreadInfo( GetWindowThreadProcessId(GetConsoleWindow() , &dww ), &currentGUIInfo ) ) {
		printf( "[currentGUIInfo]\n" );
		ConsoleOut_GUITHREADINFO( currentGUIInfo );
		printf( "dww = 0x%X\n" , dww );
		printf( "\n" );
	}


	printf( "DesktopWindow : %p\n", GetDesktopWindow( ) );
	printf( "GetActiveWindow : %p\n", GetActiveWindow( ) );
	printf( "GetForegroundWindow : %p\n", GetForegroundWindow( ) );
	printf( "GetConsoleWindow : %p\n", GetConsoleWindow( ) );
	printf( "GetActiveWindowParent : %p\n", GetParent(GetActiveWindow( )) );
	printf( "GetForegroundWindowParent : %p\n", GetParent( GetForegroundWindow( )) );
	printf( "GetConsoleWindowParent : %p\n", GetParent( GetConsoleWindow( ) ));
	printf( "GetWindowLongPtr (GWL_STYLE) : %I64d\n", GetWindowLongPtr( GetConsoleWindow( ), GWL_STYLE ) );
	printf( "GetFocus : %p\n", GetFocus() );
	printf( "\n" );
	MessageBoxA( NULL, "", "", MB_OK );

	printf( "DesktopWindow : %p\n", GetDesktopWindow( ) );
	printf( "GetActiveWindow : %p\n", GetActiveWindow( ) );
	printf( "GetForegroundWindow : %p\n", GetForegroundWindow( ) );
	printf( "GetConsoleWindow : %p\n", GetConsoleWindow( ) );
	printf( "GetActiveWindowParent : %p\n", GetParent( GetActiveWindow( ) ) );
	printf( "GetForegroundWindowParent : %p\n", GetParent( GetForegroundWindow( ) ) );
	printf( "GetConsoleWindowParent : %p\n", GetParent( GetConsoleWindow( ) ) );
	printf( "GetWindowLongPtr (GWL_STYLE) : %I64d\n", GetWindowLongPtr( GetConsoleWindow( ), GWL_STYLE ) );
	printf( "GetFocus : %p\n", GetFocus( ) );

	
	return 0;
}

void ConsoleOut_GUITHREADINFO( const GUITHREADINFO& info ) {
	printf( "cbSize = %u\n", info.cbSize );
	printf( "flags = %u\n", info.flags );
	printf( "hwndActive = %p\n", info.hwndActive );
	printf( "hwndCapture = %p\n", info.hwndCapture );
	printf( "hwndCaret = %p\n", info.hwndCaret );
	printf( "hwndFocus = %p\n", info.hwndFocus );
	printf( "hwndMenuOwner = %p\n", info.hwndMenuOwner );
	printf( "hwndMoveSize = %p\n", info.hwndMoveSize );
	printf( "rcCaret = (%d,%d)-(%d,%d)\n", info.rcCaret.left, info.rcCaret.top, info.rcCaret.right, info.rcCaret.bottom );
}

void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res, bool firstNewLine, std::string prefix ){
	if ( firstNewLine ) printf( "\n" );
	printf( "%s[Raw Results]\n", prefix.c_str() );
	printf( "%sOperationCode：0x%02X\n", prefix.c_str( ), res.executedOperationCode );
	printf( "%sDeviceIOControlResult：0x%02X\n", prefix.c_str( ), res.DeviceIOControlResult );
	printf( "%sDeviceIOControlLastError：0x%02X\n", prefix.c_str( ), res.DeviceIOControlLastError );
	printf( "%sScsiStatus：0x%02X\n", prefix.c_str( ), res.scsiStatus.statusByteCode );
	printf( "%sSK：0x%02X\n", prefix.c_str( ), res.scsiSK );
	printf( "%sASC：0x%02X\n", prefix.c_str( ), res.scsiASC );
	printf( "%sASCQ：0x%02X\n", prefix.c_str( ), res.scsiASCQ );
	printf( "\n" );
}

void ConsoleOut_FeatureDescriptorHeader( THSSCSI_FeatureDescriptorHeader* pHeader, bool firstNewLine, std::string prefix ) {
	if ( pHeader == nullptr )return;
	if ( firstNewLine ) printf( "\n" );
	printf( "%sHeader:\n", prefix.c_str( ) );
	printf( "%s\tFeature Code : 0x%04X\n", prefix.c_str( ),pHeader->FeatureCode );
	printf( "%s\tCurrent : %d\n", prefix.c_str( ), pHeader->Current );
	printf( "%s\tPersistent : %d\n", prefix.c_str( ),pHeader->Persistent );
	printf( "%s\tVersion : %d\n", prefix.c_str( ),pHeader->Version );
	printf( "%s\tAdditionalLength : %d\n", prefix.c_str( ),pHeader->AdditionalLength );
}
