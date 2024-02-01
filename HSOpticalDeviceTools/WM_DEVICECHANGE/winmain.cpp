/*----------------------------------------------------------------------------
�v���O������:Template
�o�[�W����:1.0.1.0
�v���O�����T�v�F
--------------------------------------------------------------------------------
*/
/*-----------------------�C���N���[�h��------------------------*/
#include <windows.h>
#include <cstdio>
/*------------------------------------------------------------*/
/*-----------------�}�j�t�F�X�g�o�^----------------------------*/
#pragma comment(linker,"\"/manifestdependency:type='win32' "	\
	"name='Microsoft.Windows.Common-Controls' "					\
	"version='6.0.0.0' "										\
	"processorArchitecture='*' "								\
	"publicKeyToken='6595b64144ccf1df' "						\
	"language='*'\"")
/*-------------------------------------------------------------*/
/*-------------------�v���g�^�C�v�錾��------------------------*/

/*
�A�v���P�[�V�����T�|�[�g
*/
ATOM RegisterAppWindowClass( HINSTANCE hInstance, const TCHAR* lpszClassName, WNDPROC funcWndProc, COLORREF bgColor = RGB( 255, 255, 255 ), int ResMenuID = 0, int ResIconID = 0 );
HWND CreateAppWindow( HINSTANCE hInstance, const TCHAR* lpszClassName, const TCHAR* lpszTitle, DWORD WindowStyle, int x, int y, int ClientWidth, int ClientHeight, int ShowState );


BOOL InitAppWindowClass( HINSTANCE hInstance );
BOOL InitAppWindow( HINSTANCE hInstance, int ShowState );

BOOL InitMainWindow( HINSTANCE hInstance, int ShowState );

/* ���C���E�B���h�E�v���V�[�W���[ */
LRESULT CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp );

/*-------------------------------------------------------------*/
/*-------------------------�}�N����`��------------------------*/
//�v���O������
#define PRONAME		"WM_DEVICECHANGE"						

//�v���O�����o�[�W����
#define PROVER		"1.0.1.0"							

//�v���O������(�e�L�X�g)
#define PRONAME_TXT	TEXT(PRONAME)

//�v���O�����o�[�W����(�e�L�X�g)
#define PROVER_TXT	TEXT(PROVER)						




//----------------------------------------------------------------
//���C���E�B���h�E�̃N���C�A���g�T�C�Y�i���j
#define MAINWINDOW_CLIENT_WIDTH		800					

//���C���E�B���h�E�̃N���C�A���g�T�C�Y�i�����j
#define MAINWINDOW_CLIENT_HEIGHT	600					

//���C���E�B���h�E���T�C�Y�ς��ǂ���(0:�Œ�A ����ȊO�F��)
#define MAINWINDOW_SIZE_VARIABLE	1					

//���C���E�B���h�E�̔w�i�F
#define MAINWINDOW_BACKGROUNDCOLOR	RGB(255,255,255)	

//���C���E�B���h�E�̏����^�C�g��
#define MAINWINDOW_DEFAULTTITLE		PRONAME_TXT			

//���C���E�B���h�E�̃E�B���h�E�N���X��
#define MAINWINDOW_CLASSNAME		PRONAME_TXT			


#ifdef _DEBUG
//�R���\�[���E�B���h�E�̃E�B���h�E�^�C�g��
#define CONSOLEWINDOW_TITLE	TEXT(PRONAME ## " (Debug Console)")
#endif

/*-------------------------------------------------------------*/
/*-------------------�O���[�o���ϐ���`��----------------------*/
HINSTANCE hInstance = NULL;
/*-------------------------------------------------------------*/

/*----------------------------���C���֐���`��--------------------------------------------*/
int WINAPI WinMain( _In_ HINSTANCE hCurInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPSTR lpsCmdLine, _In_ int nCmdShow ) {
	MSG msg;
	BOOL bRet;

	//COM������
	if ( FAILED( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) ) ) {
		MessageBox( NULL, TEXT( "CoInitializeEx�G���[" ), TEXT( "Error" ), MB_OK );
		return 0;
	}

#ifdef _DEBUG

	if ( AllocConsole( ) == FALSE ) {
		MessageBox( NULL, TEXT( "AllocConsole�G���[" ), TEXT( "Error" ), MB_OK );
		return 0;
	}


	SetConsoleTitle( CONSOLEWINDOW_TITLE );

	FILE* fpConsoleOut;
	freopen_s( &fpConsoleOut, "CONOUT$", "w", stdout );

#endif
	//�C���X�^���X�n���h�����O���[�o���ϐ��ɃR�s�[
	hInstance = hCurInst;

	//�E�B���h�E�N���X�̓o�^
	if ( !InitAppWindowClass( hCurInst ) ) return FALSE;

	//�E�B���h�E�쐬
	if ( !InitAppWindow( hCurInst, nCmdShow ) ) return FALSE;

	//���b�Z�[�W���[�v
	while ( ( bRet = GetMessage( &msg, NULL, 0, 0 ) ) ) {
		if ( bRet == -1 ) {
			MessageBox( NULL, TEXT( "GetMessage�G���[" ), TEXT( "Error" ), MB_OK );
			break;
		} else {
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

#ifdef _DEBUG
	FreeConsole( );
#endif

	//COM ���
	CoUninitialize( );
	return (int) msg.wParam;
}

ATOM RegisterAppWindowClass( HINSTANCE hInstance, const  TCHAR* lpszClassName, WNDPROC funcWndProc, COLORREF bgColor, int ResMenuID, int ResIconID ) {
	WNDCLASSEX wc; //WNDCLASSEX�\����
	wc.cbSize = sizeof( WNDCLASSEX );
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc = funcWndProc; //�v���V�[�W����
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance; //�C���X�^���X
	if ( ResIconID == 0 ) {
		wc.hIcon = (HICON) LoadImage( NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED );
	} else {
		wc.hIcon = (HICON) LoadImage( hInstance, MAKEINTRESOURCE( ResIconID ), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED );
	}
	wc.hIconSm = wc.hIcon;
	wc.hCursor = (HCURSOR) LoadImage( NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED );
	wc.hbrBackground = CreateSolidBrush( bgColor );
	wc.lpszMenuName = MAKEINTRESOURCE( ResMenuID ); //���j���[��
	wc.lpszClassName = lpszClassName;
	return RegisterClassEx( &wc );
}

HWND CreateAppWindow( HINSTANCE hInstance, const TCHAR* lpszClassName, const TCHAR* lpszTitle, DWORD WindowStyle, int x, int y, int ClientWidth, int ClientHeight, int ShowState ) {

	WNDCLASSEX wc; //WNDCLASSEX�\����
	wc.cbSize = sizeof( WNDCLASSEX );

	if ( GetClassInfoEx( hInstance, lpszClassName, &wc ) == FALSE ) return NULL;

	int Width, Height;
	RECT adustRect;
	adustRect.left = 0;
	adustRect.top = 0;
	adustRect.right = ClientWidth;
	adustRect.bottom = ClientHeight;
	if ( AdjustWindowRectEx( &adustRect, WindowStyle, ( wc.lpszMenuName ) ? TRUE : FALSE, 0 ) ) {
		Width = adustRect.right - adustRect.left;
		Height = adustRect.bottom - adustRect.top;
	} else {
		Width = ClientWidth;
		Height = ClientHeight;
	}

	HWND hWnd = CreateWindowEx( NULL, lpszClassName, lpszTitle, WindowStyle, x, y, Width, Height, NULL, NULL, hInstance, NULL );

	if ( hWnd == NULL )return NULL;


	//�E�B���h�E�\��
	ShowWindow( hWnd, ShowState );

	//�E�B���h�E�X�V
	UpdateWindow( hWnd );

	return hWnd;
}



BOOL InitMainWindow( HINSTANCE hInstance, int ShowState ) {

	int WindowStyle;

	//�E�B���h�E�X�^�C��
	WindowStyle = WS_OVERLAPPEDWINDOW;

	//�E�B���h�E�T�C�Y�Œ�Ȃ�΃I�[�o�[���b�v�E�B���h�E����
	//�T�C�Y�ύX���E�ƍő剻�{�^���𖳌����ɂ���
	if ( MAINWINDOW_SIZE_VARIABLE == 0 )
		WindowStyle &= ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;

	HWND hwnd = CreateAppWindow( hInstance, MAINWINDOW_CLASSNAME, MAINWINDOW_DEFAULTTITLE,
		WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, MAINWINDOW_CLIENT_WIDTH, MAINWINDOW_CLIENT_HEIGHT, ShowState );
	if ( hwnd == NULL )return FALSE;
	return TRUE;
}

BOOL InitAppWindowClass( HINSTANCE hInstance ) {
	ATOM atMain = RegisterAppWindowClass( hInstance, MAINWINDOW_CLASSNAME, MainWndProc, MAINWINDOW_BACKGROUNDCOLOR );
	if ( atMain == NULL )return FALSE;
	return TRUE;
}

BOOL InitAppWindow( HINSTANCE hInstance, int ShowState ) {
	return InitMainWindow( hInstance, ShowState );
}


/*
���C���E�B���h�E�v���V�[�W���[
*/

LRESULT CALLBACK MainWndProc( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ) {
	//�N���C�A���g�T�C�Y�ۊǏꏊ
	RECT rcClientSize;

	//�N���C�A���g�T�C�Y�擾
	GetClientRect( hwnd, &rcClientSize );

	//���b�Z�[�W���Ƃ̏���
	switch ( msg ) {
		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;
		default:
			return DefWindowProc( hwnd, msg, wp, lp );
	}
	return 0;
}