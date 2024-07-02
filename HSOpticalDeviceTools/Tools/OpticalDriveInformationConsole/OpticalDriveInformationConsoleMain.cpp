#include <cstdio>
#include <string>
#include <atlstr.h>
#include "../../Libraries/CommonLib/CHSOpticalDiscReader.hpp"
#include "../../common/CCommandLineParser.hpp"


void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res, bool firstNewLine = false, std::string prefix = "" );

void DriveUnitProcess( THSOpticalDriveInfo driveInfo, const CAtlStringW displayDriveName );

void PrintDeviceBasicInfo( CHSOpticalDrive* pDrive, const THSOpticalDriveInfo* pDriveInfo = nullptr );
void PrintFunctionsInformation( CHSOpticalDrive* pDrive );

void PrintSupportProfiles( CHSOpticalDrive* pDrive );
void PrintSupportFeatureCodeList( CHSOpticalDrive* pDrive );

void PrintDriveIndependentDebugInformation( void );

CCommandLineParserExW cmdlineParamParser;

int main( void ) {

	SetConsoleTitleA( "OpticalDriveInformationConsole" );

	cmdlineParamParser.parse( GetCommandLineW( ) );

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
	printf( "|�h���C�u|�f�o�C�X�\����|\n" );
	printf( "|---|---|\n" );



	for ( size_t i = 0; i < optical_drives_enum.uOpticalDriveCount; i++ ) {
		printf( "|`%c:\\`|", optical_drives_enum.Drives[i].Letter );
		if ( optical_drives_enum.Drives[i].bIncludedInfo ) printf( "`%s`", optical_drives_enum.Drives[i].Info.DisplayName );
		printf( "|\n" );
	}
	printf( "\n\n" );

	CAtlStringW displayDriveName, displayDriveNameForSummaryTag;

	for ( size_t i = 0; i < optical_drives_enum.uOpticalDriveCount; i++ ) {

		displayDriveName.Format( L"[`%C:\\`]", optical_drives_enum.Drives[i].Letter );

		if ( optical_drives_enum.Drives[i].bIncludedInfo ) displayDriveName.AppendFormat( L" %S", optical_drives_enum.Drives[i].Info.DisplayName );

		displayDriveNameForSummaryTag = displayDriveName;
		displayDriveNameForSummaryTag.Remove( '`' );


		printf( "<details>\n\n" );
		printf( "<summary>%S</summary>\n\n" , displayDriveNameForSummaryTag.GetString() );

		DriveUnitProcess( optical_drives_enum.Drives[i], displayDriveName);

		if ( cmdlineParamParser.hasNamedOption( L"drivewait" ) && ((i+1)!=optical_drives_enum.uOpticalDriveCount)) {
			system( "pause" );
		}

		printf( "\n</details>\n\n" );

	}
#if _DEBUG
	PrintDriveIndependentDebugInformation( );
	printf( "\n\n" );
#endif

	if ( ( !cmdlineParamParser.hasNamedOption( L"endnopause" ) ) && ( !cmdlineParamParser.hasNamedOption( L"endnowait" ) ) ) {
		system( "pause" );
	}

	return 0;
}

void ConsoleOut_SPTD_RESULT( HSSCSI_SPTD_RESULT res, bool firstNewLine, std::string prefix ) {
	if ( firstNewLine ) printf( "\n" );
	printf( "%s[SPTD Raw Results]\n", prefix.c_str( ) );
	printf( "%sOperationCode�F0x%02X\n", prefix.c_str( ), res.executedOperationCode );
	printf( "%sDeviceIOControlResult�F0x%02X\n", prefix.c_str( ), res.DeviceIOControlResult );
	printf( "%sDeviceIOControlLastError�F0x%02X\n", prefix.c_str( ), res.DeviceIOControlLastError );
	printf( "%sScsiStatus�F0x%02X\n", prefix.c_str( ), res.scsiStatus.statusByteCode );
	printf( "%sSK�F0x%02X\n", prefix.c_str( ), res.scsiSK );
	printf( "%sASC�F0x%02X\n", prefix.c_str( ), res.scsiASC );
	printf( "%sASCQ�F0x%02X\n", prefix.c_str( ), res.scsiASCQ );
	printf( "\n" );
}

void DriveUnitProcess( THSOpticalDriveInfo driveInfo, const CAtlStringW displayDriveName ) {

	CHSOpticalDrive drive;


	printf( "## %S", displayDriveName.GetString( ) );
	
	printf( "\n\n" );


	if ( !drive.open( driveInfo.Letter ) ) {
		printf( "�h���C�u���J���܂���ł����B\n" );
		return;
	}

	PrintDeviceBasicInfo( &drive, &driveInfo );
	printf( "\n" );

	PrintFunctionsInformation( &drive );
	printf( "\n" );

	PrintSupportProfiles( &drive );
	printf( "\n" );

	PrintSupportFeatureCodeList( &drive );

}


void PrintDeviceBasicInfo(CHSOpticalDrive* pDrive, const THSOpticalDriveInfo* pDriveInfo ) {
	if ( pDrive == nullptr ) return;
	if ( pDriveInfo == nullptr ) return;

	printf( "### ��{���\n\n" );
	printf( "|���ږ�|�l|\n" );
	printf( "|---|---|\n" );

	if ( pDriveInfo->bIncludedInfo ) {
		printf( "|�f�o�C�X�\����|`%s`|\n", pDriveInfo->Info.DisplayName );
		printf( "|�x���_�[ID|`%s`|\n", pDriveInfo->Info.VendorID );
		printf( "|�v���_�N�gID|`%s`|\n", pDriveInfo->Info.ProductID );
		printf( "|�v���_�N�g���r�W�������x��|`%s`|\n", pDriveInfo->Info.ProductRevisionLevel );

	}

	CHSOpticalDriveGetConfigCmd cmd( pDrive );
	THSSCSI_FeatureDescriptor_DriveSerialNumber dsn;
	if ( cmd.getFeatureDriveSerialNumber( &dsn ) ) {
		printf( "|�V���A���i���o�[|`%s`|\n", dsn.SerialNumber );
	}


	EHSSCSI_ConnectInterfaceName busName = pDrive->getBusType( );
	EHSSCSI_ConnectInterfaceName physicalInterfaceName = cmd.getPhysicalInterfaceStandardName( );

	if ( busName == physicalInterfaceName ) {
		printf( "|�ڑ��C���^�t�F�[�X|`%s`|\n", 
			HSSCSI_GetConnectInterfaceNameStringByName(physicalInterfaceName).c_str( ) );
	} else {
		printf( "|�ڑ��C���^�t�F�[�X|`%s (%s)`|\n", 
			HSSCSI_GetConnectInterfaceNameStringByName( busName ).c_str( ),
			HSSCSI_GetConnectInterfaceNameStringByName( physicalInterfaceName ).c_str( ) );
	}

	EHSSCSI_ReadyStatus readyStatus = pDrive->checkReady( nullptr );
	printf( "|�h���C�u�̏��|" );
	switch ( readyStatus ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "�h���C�u�ɃA�N�Z�X���鏀�����o���Ă��܂��B" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "�h���C�u�ɃA�N�Z�X���鏀�����o���Ă��܂���B" );
			break;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "�h���C�u�̏�Ԃ̎擾�Ɏ��s���܂����B" );
			break;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "���f�B�A���h���C�u�ɃZ�b�g����Ă��܂���B" );
			break;
	}
	printf( "|\n" );


	EHSOD_TrayState trayState = pDrive->checkTrayState( );
	printf( "|�g���C�̏��|" );
	switch ( trayState ) {
		case EHSOD_TrayState::Closed:
			printf( "�g���C�͕����Ă��܂��B" );
			break;
		case EHSOD_TrayState::Opened:
			printf( "�g���C���J����Ă��܂��B" );
			break;
		case EHSOD_TrayState::FailedGotStatus:
			printf( "�g���C�̏�Ԃ̎擾�Ɏ��s���܂����B" );
			break;

	}
	printf( "|\n" );

	printf( "|���f�B�A���Z�b�g����Ă��邩|%s|\n", pDrive->isMediaPresent( ) ? "�͂�" : "������" );

	
}

void PrintFunctionsInformation( CHSOpticalDrive* pDrive ) {

	if ( pDrive == nullptr ) return;

	CHSOpticalDriveGetConfigCmd cmd( pDrive );
	THSSCSI_FeatureDescriptor_CDRead fd_cdread;
	if ( !cmd.getFeatureCDRead( &fd_cdread ) ) {
		fd_cdread.CDText = false;
	}
	THSSCSI_FeatureDescriptor_RemovableMedium fd_removableMedium;
	if ( !cmd.getFeatureRemovableMedium( &fd_removableMedium ) ) {
		fd_removableMedium.Eject = false;
		fd_removableMedium.Load = false;
		fd_removableMedium.Lock = false;
	}

	printf( "### �@�\���\n\n" );
	printf( "|���ږ�|�l|\n" );
	printf( "|---|---|\n" );
	printf( "|READ TOC/PMA/ATIP�R�}���h�ł�CD-TEXT�̓ǂݍ��݂��T�|�[�g���邩|%s|\n", ( fd_cdread.CDText ) ? "�͂�" : "������" );
	printf( "|START STOP UNIT�R�}���h�ł̃g���C�����o�����T�|�[�g���邩|%s|\n", ( fd_removableMedium.Eject ) ? "�͂�" : "������" );
	printf( "|START STOP UNIT�R�}���h�ł̃g���C�����߂����T�|�[�g���邩|%s|\n", ( fd_removableMedium.Load ) ? "�͂�" : "������" );
	printf( "|PREVENT ALLOW MEDIUM REMOVAL�R�}���h�ł̃g���C�̃��b�N���T�|�[�g���邩|%s|\n", ( fd_removableMedium.Lock ) ? "�͂�" : "������" );


}

void PrintSupportProfiles( CHSOpticalDrive* pDrive ) {
	if ( pDrive == nullptr ) return;


	printf( "### �Ή��v���t�@�C���ꗗ(�Ή����f�B�A�ꗗ)\n\n" );

	CHSOpticalDriveGetConfigCmd cmd( pDrive );
	HSSCSI_Profiles profiles;

	if ( cmd.getSupportProfiles( &profiles ) == false ) {
		printf( "```\n" );
		printf( "���̎擾�Ɏ��s���܂���\n" );
		printf( "```\n" );
	} else {

		uint16_t currentProfile = 0;
		cmd.getCurrentProfileNumber( &currentProfile );

		printf( "|�v���t�@�C��ID|�v���t�@�C����(���f�B�A��)|���f�B�A�t�@�~���[��|\n" );
		printf( "|---|---|---|\n" );

		bool bCurrent;
		for ( auto& info : profiles ) {
			if ( info.second == EHSSCSI_ProfileName::RemovableDisk )continue;

			bCurrent = ( info.first == currentProfile );

			//�v���t�@�C��ID
			printf( "|" );
			if ( bCurrent ) printf( "<ins>**" );
			printf( "0x%04X",info.first);
			if ( bCurrent ) printf( "**</ins>" );
			printf( "|" );

			//�v���t�@�C����
			if ( bCurrent ) printf( "<ins>**" );
			printf( "`%s`", cmd.GetProfileNameString( info.second, false ).c_str( ) );
			if ( bCurrent ) printf( "**</ins>" );
			printf( "|" );
			
			//�v���t�@�C���t�@�~���[��
			if ( bCurrent ) printf( "<ins>**" );
			printf( "`%s`", cmd.GetProfileFamilyNameString( info.second ).c_str( ) );
			if ( bCurrent ) printf( "**</ins>" );
			printf( "|\n" );
		}
	}

}


void PrintSupportFeatureCodeList( CHSOpticalDrive* pDrive ) {
	if ( pDrive == nullptr ) return;

	printf( "### �Ή�FeatureCode�ꗗ\n\n" );

	CHSOpticalDriveGetConfigCmd cmd( pDrive );
	THSSCSI_FeatureInfo info;

	if ( cmd.execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE::All, 0, &info ) !=0) {
		printf( "|Feature Code|Feature Name|\n" );
		printf( "|---|---|\n" );
		for ( auto item : info.Descriptors ) {
			auto it = HSSCSI_FeatureNameStrings.find( item.pHeader->FeatureCode );
			if ( it != HSSCSI_FeatureNameStrings.end( ) ) {
				printf( "|" );
				if ( item.pHeader->Current ) printf( "<ins>**" );
				printf( "0x%04X", item.pHeader->FeatureCode );
				if ( item.pHeader->Current ) printf( "**</ins>" );
				printf( "|" );
				if ( item.pHeader->Current ) printf( "<ins>**" );
				printf( "`%s`", it->second.c_str( ) );
				if ( item.pHeader->Current ) printf( "**</ins>" );
				printf( "|\n" );
			}
		}
	} else {
		printf( "```\n" );
		printf( "���̎擾�Ɏ��s���܂���\n" );
		printf( "```\n" );
	}
}


void PrintDriveIndependentDebugInformation( void ) {

	printf( "## Debug���\n\n" );
	printf( "### ��`�ς�FeatureCode�ꗗ\n\n" );


	printf( "|Feature Code|Feature Name|\n" );
	printf( "|---|---|\n" );


	for ( auto item : HSSCSI_FeatureNameStrings ) {
		printf( "|0x%04X|`%s`|\n", item.first ,item.second.c_str());
	}

}

