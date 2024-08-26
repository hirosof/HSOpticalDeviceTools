#include "MainWindowDlgProcess.hpp"

CMainWindowDlgProcess::CMainWindowDlgProcess( ) {
	this->hwnd = NULL;
	this->optical_drives_enum.uOpticalDriveCount = 0;
}

CMainWindowDlgProcess::~CMainWindowDlgProcess( ) {
}

char CMainWindowDlgProcess::GetSelectDriveLetter( void ) {
	LPARAM index = SendDlgItemMessage( hwnd, IDC_CB_OPTICAL_DRIVE_LIST, CB_GETCURSEL, 0, 0 );

	if ( index == CB_ERR ) {
		return 0;
	}

	return this->optical_drives_enum.Drives[index].Letter;
}

CMainWindowDlgProcess* CMainWindowDlgProcess::GetInstance( void ) {
	static  CMainWindowDlgProcess  instance;
	return &instance;
}

INT_PTR CMainWindowDlgProcess::ProcEntry( HWND hDlg, UINT msg, WPARAM wp, LPARAM lp ) {

	switch ( msg ) {
		case WM_INITDIALOG:
			this->hwnd = hDlg;
			this->OnInitDialog( );
			return TRUE;
		case WM_COMMAND:
			return this->OnCommand( wp, lp );
		case WM_CLOSE:
			DestroyWindow( hDlg );
			return TRUE;
	}
	return FALSE;
}

void CMainWindowDlgProcess::OnInitDialog( void ) {

	this->UpdateOpticalDriveList( );



}

INT_PTR CMainWindowDlgProcess::OnCommand( WPARAM wp, LPARAM lp ) {

	switch ( LOWORD( wp ) ) {
		case IDC_UPDATE_OPTICAL_DRIVE_LIST_BUTTON:
			this->UpdateOpticalDriveList( );
			MessageBox( NULL, TEXT( "���w�h���C�u���X�g���X�V���܂����B" ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
			break;
		case IDC_SHOW_SUPPORT_INFO_BUTTON:
			this->ShowSupportInfomation( );
			break;
		case IDC_SHOW_TRAY_STATE_BUTTON:
			this->ShowTrayState( );
			break;
		case IDC_SHOW_INSERTED_MEDIA_TYPE:
			this->ShowInsertedMediaType( );
			break;
		case IDC_TRAY_OPEN_BUTTON:
			this->TrayOpenProcess( );
			break;
		case IDC_TRAY_CLOSE_BUTTON:
			this->TrayCloseProcess( );
			break;
		case IDC_TRAY_LOCK_BUTTON:
			this->TrayLockProcess( );
			break;
		case IDC_TRAY_UNLOCK_BUTTON:
			this->TrayUnlockProcess( );
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

uint8_t CMainWindowDlgProcess::UpdateOpticalDriveList( void ) {


	char selectedDrive = '\0';

	if ( optical_drives_enum.uOpticalDriveCount != 0 ) {
		LRESULT beforeIndex = SendDlgItemMessage( hwnd, IDC_CB_OPTICAL_DRIVE_LIST, CB_GETCURSEL, 0, 0 );

		if ( beforeIndex != CB_ERR ) {
			if ( beforeIndex < optical_drives_enum.uOpticalDriveCount ) {
				selectedDrive = optical_drives_enum.Drives[beforeIndex].Letter;
			}
		}
	}

	LRESULT beforeItemCount = SendDlgItemMessage( hwnd, IDC_CB_OPTICAL_DRIVE_LIST, CB_GETCOUNT, 0, 0 );

	if ( beforeItemCount == CB_ERR ) beforeItemCount = 0;

	for ( int i = 0; i < beforeItemCount; i++ ) {
		SendDlgItemMessage( hwnd, IDC_CB_OPTICAL_DRIVE_LIST, CB_DELETESTRING, 0, 0 );
	}

	THSEnumrateOpticalDriveInfo new_optical_drives_enum;

	if ( CHSOpticalDrive::EnumOpticalDrive( &new_optical_drives_enum ) ) {

		CAtlStringW driveItem;
		size_t newIndex = 0;


		for ( size_t i = 0; i < new_optical_drives_enum.uOpticalDriveCount; i++ ) {


			driveItem.Format( L"[%C:\\]", new_optical_drives_enum.Drives[i].Letter );

			if ( new_optical_drives_enum.Drives[i].bIncludedInfo ) {
				driveItem.AppendFormat( L" %S", new_optical_drives_enum.Drives[i].Info.DisplayName );
			}

			SendDlgItemMessage( hwnd, IDC_CB_OPTICAL_DRIVE_LIST, CB_ADDSTRING, 0,
				reinterpret_cast<LPARAM>( driveItem.GetString( ) ) );

			if ( new_optical_drives_enum.Drives[i].Letter == selectedDrive ) {
				newIndex = i;
			}

		}
		this->optical_drives_enum = new_optical_drives_enum;
		SendDlgItemMessage( hwnd, IDC_CB_OPTICAL_DRIVE_LIST, CB_SETCURSEL, newIndex, 0 );
		return this->optical_drives_enum.uOpticalDriveCount;
	}

	return 0;
}

void CMainWindowDlgProcess::ShowSupportInfomation( void ) {


	char dl = this->GetSelectDriveLetter( );

	if ( dl == 0 )return;

	CHSOpticalDrive drive;

	if ( drive.open( dl ) == false ) return;

	CHSOpticalDriveGetConfigCmd cmd( &drive );

	THSSCSI_FeatureDescriptor_RemovableMedium  fd_rm;

	if ( !cmd.getFeatureRemovableMedium( &fd_rm ) ) {
		MessageBox( hwnd, TEXT( "�h���C�u�̑Ή����̎擾�Ɏ��s���܂����B" ), PRONAME_TXT, MB_OK | MB_ICONERROR );
	} else {
		CAtlString str;
		str.Format( TEXT( "[�g���C���J�����߂��T�|�[�g���邩�H]\n%s\n\n" ), ( fd_rm.Eject ) ? TEXT( "�͂�" ) : TEXT( "������" ) );
		str.AppendFormat( TEXT( "[�g���C����閽�߂��T�|�[�g���邩�H]\n%s\n\n" ), ( fd_rm.Load ) ? TEXT( "�͂�" ) : TEXT( "������" ) );
		str.AppendFormat( TEXT( "[�g���C�����b�N����/���b�N���������閽�߂��T�|�[�g���邩�H]\n%s" ), ( fd_rm.Lock ) ? TEXT( "�͂�" ) : TEXT( "������" ) );
		MessageBox( hwnd, str.GetString( ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
	}
}

void CMainWindowDlgProcess::ShowTrayState( void ) {

	char dl = this->GetSelectDriveLetter( );

	if ( dl == 0 )return;

	CHSOpticalDrive drive;

	if ( drive.open( dl ) == false ) return;


	EHSOD_TrayState state = drive.checkTrayState( );
	CAtlString str;


	switch ( state ) {
		case EHSOD_TrayState::Closed:
			str.SetString( TEXT("�g���C�͕����Ă��܂��B") );
			break;
		case EHSOD_TrayState::Opened:
			str.SetString( TEXT( "�g���C�͊J����Ă��܂��B") );
			break;
		case EHSOD_TrayState::FailedGotStatus:
			str.SetString( TEXT( "�g���C�̏�Ԏ擾�Ɏ��s���܂����B") );
			break;
	}


	if ( str.GetLength( ) > 0 ) {
		MessageBox( hwnd, str.GetString( ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
	}

}

void CMainWindowDlgProcess::ShowInsertedMediaType( void ) {

	char dl = this->GetSelectDriveLetter( );

	if ( dl == 0 )return;

	CHSOpticalDrive drive;

	if ( drive.open( dl ) == false ) return;

	if ( !drive.isMediaPresent( ) ) {
		MessageBox( hwnd, TEXT( "���݃h���C�u�ɂ̓��f�B�A���}������Ă��܂���B" ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
		return;
	}


	CHSOpticalDriveGetConfigCmd cmd( &drive );
	std::string pnstr = cmd.getCurrentProfileNameString( false );

	CAtlStringW str;

	str.Format( L"���݃h���C�u�ɂ͈ȉ��̃��f�B�A���}������Ă��܂��B\n\n%S", pnstr.c_str( ) );

	MessageBoxW( hwnd, str.GetString( ), PRONAME_UNICODE, MB_OK | MB_ICONINFORMATION );

}

void CMainWindowDlgProcess::TrayOpenProcess( void ) {
	char dl = this->GetSelectDriveLetter( );

	if ( dl == 0 )return;

	CHSOpticalDrive drive;

	if ( drive.open( dl ) == false ) return;


	CHSOpticalDriveGetConfigCmd cmd( &drive );
	THSSCSI_FeatureDescriptor_RemovableMedium  fd_rm;
	if ( cmd.getFeatureRemovableMedium( &fd_rm ) ) {
		if ( !fd_rm.Eject ) {
			MessageBox( hwnd, TEXT( "�h���C�u�̓g���C���J�����߂��T�|�[�g���Ă��܂���B" ), PRONAME_TXT, MB_OK | MB_ICONERROR );
			return;
		}
	}

	if ( drive.checkTrayState( ) == EHSOD_TrayState::Opened ) {
		MessageBox( hwnd, TEXT( "�h���C�u�̃g���C�͊��ɊJ����Ă��܂��B" ), PRONAME_TXT, MB_OK |MB_ICONINFORMATION );
		return;
	}


	HSSCSI_SPTD_RESULT detailResult;
	if ( drive.trayOpen( &detailResult, false ) ) {
		MessageBox( hwnd, TEXT( "�h���C�u�̃g���C���J���܂����B" ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
	} else {

		CAtlStringW str;
		str.Format( L"�h���C�u�̃g���C���J���������ɃG���[���������܂����B\n\n" );
		str.AppendFormat( L"[�ڍ�]\n" );
		str.AppendFormat( L"SCSIStatus = 0x%02X (%S)\nSK = 0x%02X\nASC = 0x%02X\nASCQ = 0x%02X",
			detailResult.scsiStatus.rawValue,HSSCSIStatusToString( detailResult.scsiStatus ).c_str( ), 
			detailResult.scsiSK, detailResult.scsiASC, detailResult.scsiASCQ );
		MessageBoxW( hwnd, str.GetString(), PRONAME_UNICODE, MB_OK | MB_ICONERROR );
	}
}

void CMainWindowDlgProcess::TrayCloseProcess( void ) {
	char dl = this->GetSelectDriveLetter( );

	if ( dl == 0 )return;

	CHSOpticalDrive drive;

	if ( drive.open( dl ) == false ) return;

	CHSOpticalDriveGetConfigCmd cmd( &drive );
	THSSCSI_FeatureDescriptor_RemovableMedium  fd_rm;
	if ( cmd.getFeatureRemovableMedium( &fd_rm ) ) {
		if ( !fd_rm.Load) {
			MessageBox( hwnd, TEXT( "�h���C�u�̓g���C����閽�߂��T�|�[�g���Ă��܂���B" ), PRONAME_TXT, MB_OK | MB_ICONERROR );
			return;
		}
	}

	if ( drive.checkTrayState( ) == EHSOD_TrayState::Closed ) {
		MessageBox( hwnd, TEXT( "�h���C�u�̃g���C�͊��ɕ����Ă��܂��B" ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
		return;
	}


	HSSCSI_SPTD_RESULT detailResult;
	if ( drive.trayClose( &detailResult, true ) ) {
		MessageBox( hwnd, TEXT( "�h���C�u�̃g���C����܂����B" ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
	} else {
		CAtlStringW str;
		str.Format( L"�h���C�u�̃g���C����鏈�����ɃG���[���������܂����B\n\n" );
		str.AppendFormat( L"[�ڍ�]\n" );
		str.AppendFormat( L"SCSIStatus = 0x%02X (%S)\nSK = 0x%02X\nASC = 0x%02X\nASCQ = 0x%02X",
			detailResult.scsiStatus.rawValue, HSSCSIStatusToString( detailResult.scsiStatus ).c_str( ),
			detailResult.scsiSK, detailResult.scsiASC, detailResult.scsiASCQ );
		MessageBoxW( hwnd, str.GetString( ), PRONAME_UNICODE, MB_OK | MB_ICONERROR );

	}
}

void CMainWindowDlgProcess::TrayLockProcess( void ) {
	char dl = this->GetSelectDriveLetter( );

	if ( dl == 0 )return;

	CHSOpticalDrive drive;

	if ( drive.open( dl ) == false ) return;

	CHSOpticalDriveGetConfigCmd cmd( &drive );
	THSSCSI_FeatureDescriptor_RemovableMedium  fd_rm;
	if ( cmd.getFeatureRemovableMedium( &fd_rm ) ) {
		if ( !fd_rm.Lock ) {
			MessageBox( hwnd, TEXT( "�h���C�u�̓g���C�̃��b�N������T�|�[�g���Ă��܂���B" ), PRONAME_TXT, MB_OK | MB_ICONERROR );
			return;
		}
	}

	HSSCSI_SPTD_RESULT detailResult;
	if ( drive.trayLock( &detailResult ) ) {
		MessageBox( hwnd, TEXT( "�h���C�u�̃g���C�����b�N���܂����B" ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
	} else {
		CAtlStringW str;
		str.Format( L"�h���C�u�̃g���C�����b�N���鏈�����ɃG���[���������܂����B\n\n" );
		str.AppendFormat( L"[�ڍ�]\n" );
		str.AppendFormat( L"SCSIStatus = 0x%02X (%S)\nSK = 0x%02X\nASC = 0x%02X\nASCQ = 0x%02X",
			detailResult.scsiStatus.rawValue, HSSCSIStatusToString( detailResult.scsiStatus ).c_str( ),
			detailResult.scsiSK, detailResult.scsiASC, detailResult.scsiASCQ );
		MessageBoxW( hwnd, str.GetString( ), PRONAME_UNICODE, MB_OK | MB_ICONERROR );

	}
}

void CMainWindowDlgProcess::TrayUnlockProcess( void ) {
	char dl = this->GetSelectDriveLetter( );

	if ( dl == 0 )return;

	CHSOpticalDrive drive;

	if ( drive.open( dl ) == false ) return;

	CHSOpticalDriveGetConfigCmd cmd( &drive );
	THSSCSI_FeatureDescriptor_RemovableMedium  fd_rm;
	if ( cmd.getFeatureRemovableMedium( &fd_rm ) ) {
		if ( !fd_rm.Lock ) {
			MessageBox( hwnd, TEXT( "�h���C�u�̓g���C�̃��b�N������T�|�[�g���Ă��܂���B" ), PRONAME_TXT, MB_OK | MB_ICONERROR );
			return;
		}
	}

	HSSCSI_SPTD_RESULT detailResult;
	if ( drive.trayUnlock( &detailResult ) ) {
		MessageBox( hwnd, TEXT( "�h���C�u�̃g���C�̃��b�N���������܂����B" ), PRONAME_TXT, MB_OK | MB_ICONINFORMATION );
	} else {
		CAtlStringW str;
		str.Format( L"�h���C�u�̃g���C�̃��b�N���������鏈�����ɃG���[���������܂����B\n\n" );
		str.AppendFormat( L"[�ڍ�]\n" );
		str.AppendFormat( L"SCSIStatus = 0x%02X (%S)\nSK = 0x%02X\nASC = 0x%02X\nASCQ = 0x%02X",
			detailResult.scsiStatus.rawValue, HSSCSIStatusToString( detailResult.scsiStatus ).c_str( ),
			detailResult.scsiSK, detailResult.scsiASC, detailResult.scsiASCQ );
		MessageBoxW( hwnd, str.GetString( ), PRONAME_UNICODE, MB_OK | MB_ICONERROR );
	}
}

