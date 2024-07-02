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
	printf( "|ドライブ|デバイス表示名|\n" );
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
	printf( "%sOperationCode：0x%02X\n", prefix.c_str( ), res.executedOperationCode );
	printf( "%sDeviceIOControlResult：0x%02X\n", prefix.c_str( ), res.DeviceIOControlResult );
	printf( "%sDeviceIOControlLastError：0x%02X\n", prefix.c_str( ), res.DeviceIOControlLastError );
	printf( "%sScsiStatus：0x%02X\n", prefix.c_str( ), res.scsiStatus.statusByteCode );
	printf( "%sSK：0x%02X\n", prefix.c_str( ), res.scsiSK );
	printf( "%sASC：0x%02X\n", prefix.c_str( ), res.scsiASC );
	printf( "%sASCQ：0x%02X\n", prefix.c_str( ), res.scsiASCQ );
	printf( "\n" );
}

void DriveUnitProcess( THSOpticalDriveInfo driveInfo, const CAtlStringW displayDriveName ) {

	CHSOpticalDrive drive;


	printf( "## %S", displayDriveName.GetString( ) );
	
	printf( "\n\n" );


	if ( !drive.open( driveInfo.Letter ) ) {
		printf( "ドライブを開けませんでした。\n" );
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

	printf( "### 基本情報\n\n" );
	printf( "|項目名|値|\n" );
	printf( "|---|---|\n" );

	if ( pDriveInfo->bIncludedInfo ) {
		printf( "|デバイス表示名|`%s`|\n", pDriveInfo->Info.DisplayName );
		printf( "|ベンダーID|`%s`|\n", pDriveInfo->Info.VendorID );
		printf( "|プロダクトID|`%s`|\n", pDriveInfo->Info.ProductID );
		printf( "|プロダクトリビジョンレベル|`%s`|\n", pDriveInfo->Info.ProductRevisionLevel );

	}

	CHSOpticalDriveGetConfigCmd cmd( pDrive );
	THSSCSI_FeatureDescriptor_DriveSerialNumber dsn;
	if ( cmd.getFeatureDriveSerialNumber( &dsn ) ) {
		printf( "|シリアルナンバー|`%s`|\n", dsn.SerialNumber );
	}


	EHSSCSI_ConnectInterfaceName busName = pDrive->getBusType( );
	EHSSCSI_ConnectInterfaceName physicalInterfaceName = cmd.getPhysicalInterfaceStandardName( );

	if ( busName == physicalInterfaceName ) {
		printf( "|接続インタフェース|`%s`|\n", 
			HSSCSI_GetConnectInterfaceNameStringByName(physicalInterfaceName).c_str( ) );
	} else {
		printf( "|接続インタフェース|`%s (%s)`|\n", 
			HSSCSI_GetConnectInterfaceNameStringByName( busName ).c_str( ),
			HSSCSI_GetConnectInterfaceNameStringByName( physicalInterfaceName ).c_str( ) );
	}

	EHSSCSI_ReadyStatus readyStatus = pDrive->checkReady( nullptr );
	printf( "|ドライブの状態|" );
	switch ( readyStatus ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "ドライブにアクセスする準備が出来ています。" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "ドライブにアクセスする準備が出来ていません。" );
			break;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "ドライブの状態の取得に失敗しました。" );
			break;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "メディアがドライブにセットされていません。" );
			break;
	}
	printf( "|\n" );


	EHSOD_TrayState trayState = pDrive->checkTrayState( );
	printf( "|トレイの状態|" );
	switch ( trayState ) {
		case EHSOD_TrayState::Closed:
			printf( "トレイは閉じられています。" );
			break;
		case EHSOD_TrayState::Opened:
			printf( "トレイが開かれています。" );
			break;
		case EHSOD_TrayState::FailedGotStatus:
			printf( "トレイの状態の取得に失敗しました。" );
			break;

	}
	printf( "|\n" );

	printf( "|メディアがセットされているか|%s|\n", pDrive->isMediaPresent( ) ? "はい" : "いいえ" );

	
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

	printf( "### 機能情報\n\n" );
	printf( "|項目名|値|\n" );
	printf( "|---|---|\n" );
	printf( "|READ TOC/PMA/ATIPコマンドでのCD-TEXTの読み込みをサポートするか|%s|\n", ( fd_cdread.CDText ) ? "はい" : "いいえ" );
	printf( "|START STOP UNITコマンドでのトレイ引き出しをサポートするか|%s|\n", ( fd_removableMedium.Eject ) ? "はい" : "いいえ" );
	printf( "|START STOP UNITコマンドでのトレイ引き戻しをサポートするか|%s|\n", ( fd_removableMedium.Load ) ? "はい" : "いいえ" );
	printf( "|PREVENT ALLOW MEDIUM REMOVALコマンドでのトレイのロックをサポートするか|%s|\n", ( fd_removableMedium.Lock ) ? "はい" : "いいえ" );


}

void PrintSupportProfiles( CHSOpticalDrive* pDrive ) {
	if ( pDrive == nullptr ) return;


	printf( "### 対応プロファイル一覧(対応メディア一覧)\n\n" );

	CHSOpticalDriveGetConfigCmd cmd( pDrive );
	HSSCSI_Profiles profiles;

	if ( cmd.getSupportProfiles( &profiles ) == false ) {
		printf( "```\n" );
		printf( "情報の取得に失敗しました\n" );
		printf( "```\n" );
	} else {

		uint16_t currentProfile = 0;
		cmd.getCurrentProfileNumber( &currentProfile );

		printf( "|プロファイルID|プロファイル名(メディア名)|メディアファミリー名|\n" );
		printf( "|---|---|---|\n" );

		bool bCurrent;
		for ( auto& info : profiles ) {
			if ( info.second == EHSSCSI_ProfileName::RemovableDisk )continue;

			bCurrent = ( info.first == currentProfile );

			//プロファイルID
			printf( "|" );
			if ( bCurrent ) printf( "<ins>**" );
			printf( "0x%04X",info.first);
			if ( bCurrent ) printf( "**</ins>" );
			printf( "|" );

			//プロファイル名
			if ( bCurrent ) printf( "<ins>**" );
			printf( "`%s`", cmd.GetProfileNameString( info.second, false ).c_str( ) );
			if ( bCurrent ) printf( "**</ins>" );
			printf( "|" );
			
			//プロファイルファミリー名
			if ( bCurrent ) printf( "<ins>**" );
			printf( "`%s`", cmd.GetProfileFamilyNameString( info.second ).c_str( ) );
			if ( bCurrent ) printf( "**</ins>" );
			printf( "|\n" );
		}
	}

}


void PrintSupportFeatureCodeList( CHSOpticalDrive* pDrive ) {
	if ( pDrive == nullptr ) return;

	printf( "### 対応FeatureCode一覧\n\n" );

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
		printf( "情報の取得に失敗しました\n" );
		printf( "```\n" );
	}
}


void PrintDriveIndependentDebugInformation( void ) {

	printf( "## Debug情報\n\n" );
	printf( "### 定義済みFeatureCode一覧\n\n" );


	printf( "|Feature Code|Feature Name|\n" );
	printf( "|---|---|\n" );


	for ( auto item : HSSCSI_FeatureNameStrings ) {
		printf( "|0x%04X|`%s`|\n", item.first ,item.second.c_str());
	}

}

