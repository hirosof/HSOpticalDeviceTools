#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include <atlbase.h>
#include <atlstr.h>
#include "cstdio"
#include "resource.h"
#include "AppDefine.hpp"
#include "../../Libraries/CommonLib/CHSOpticalDriveGetConfigCmd.hpp"
#include "../../Libraries/CommonLib/CHSOpticalDrive.hpp"

class CMainWindowDlgProcess {
private:
	CMainWindowDlgProcess( );
	~CMainWindowDlgProcess( );

	HWND hwnd;
	THSEnumrateOpticalDriveInfo optical_drives_enum;

	char GetSelectDriveLetter( void );


public:

	static CMainWindowDlgProcess* GetInstance( void );

	INT_PTR ProcEntry( HWND hDlg, UINT msg, WPARAM wp, LPARAM lp );

	void OnInitDialog( void );

	INT_PTR OnCommand( WPARAM wp, LPARAM lp );


	uint8_t UpdateOpticalDriveList( void );

	void ShowSupportInfomation( void );
	void ShowTrayState( void );
	void ShowInsertedMediaType( void );

	void TrayOpenProcess( void );
	void TrayCloseProcess( void );
	void TrayLockProcess( void );
	void TrayUnlockProcess( void );

};


