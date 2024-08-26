#include "MainWindowDlgProcess.hpp"

#pragma comment(linker,"\"/manifestdependency:type='win32' "	\
	"name='Microsoft.Windows.Common-Controls' "						\
	"version='6.0.0.0' "																		\
	"processorArchitecture='*' "														\
	"publicKeyToken='6595b64144ccf1df' "										\
	"language='*'\"")


#ifdef _DEBUG
//コンソールウィンドウのウィンドウタイトル
#define CONSOLEWINDOW_TITLE	TEXT(PRONAME ## " (Debug Console)")
#endif



INT_PTR MainWindowDlgProc( HWND hDlg, UINT msg, WPARAM wp, LPARAM lp );

HINSTANCE hInstance = NULL;

int WINAPI WinMain( _In_ HINSTANCE hCurInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPSTR lpsCmdLine, _In_ int nCmdShow ) {
	MSG msg {};
	msg.wParam = 0;

	BOOL bRet;


	if ( CHSOpticalDrive::CountOfOpticalDrive( ) == 0 ) {
		MessageBox( NULL, TEXT( "光学ドライブが接続されていません。\nプログラムを終了します。" ), PRONAME_TXT, MB_OK | MB_ICONERROR );
		return 0;
	}


	if ( FAILED( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) ) ) {
		MessageBox( NULL, TEXT( "CoInitializeExエラー" ), TEXT( "Error" ), MB_OK );
		return 0;
	}

#ifdef _DEBUG

	if ( AllocConsole( ) == FALSE ) {
		MessageBox( NULL, TEXT( "AllocConsoleエラー" ), TEXT( "Error" ), MB_OK );
		return 0;
	}


	SetConsoleTitle( CONSOLEWINDOW_TITLE );

	FILE* fpConsoleOut;
	freopen_s( &fpConsoleOut, "CONOUT$", "w", stdout );

#endif
	hInstance = hCurInst;


	HWND hMainWindowDlg = CreateDialog( hInstance, MAKEINTRESOURCE( IDD_MAIN_WINDOW_DIALOG ), NULL, MainWindowDlgProc );

	if ( hMainWindowDlg != NULL ) {

		while ( ( bRet = GetMessage( &msg, NULL, 0, 0 ) ) ) {
			if ( bRet == -1 ) {
				MessageBox( NULL, TEXT( "GetMessageエラー" ), TEXT( "Error" ), MB_OK| MB_ICONERROR );
				break;
			} else {
				if (!IsDialogMessage( hMainWindowDlg, &msg ) ){
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
			}
		}
	} else {
		MessageBox( NULL, TEXT( "メインウィンドウの作成に失敗しました" ), TEXT( "Error" ), MB_OK | MB_ICONERROR );
	}

#ifdef _DEBUG
	FreeConsole( );
#endif

	//COM 解放
	CoUninitialize( );
	return (int) msg.wParam;
}


INT_PTR MainWindowDlgProc( HWND hDlg, UINT msg, WPARAM wp, LPARAM lp ) {
	switch ( msg ) {
		case WM_DESTROY:
			PostQuitMessage( 0 );
			return TRUE;
		default:
			return CMainWindowDlgProcess::GetInstance()->ProcEntry(hDlg,msg,wp,lp);
	}
}