#pragma once

#include <Windows.h>

//プログラム名
#define PRONAME		"TrayControllerGUI"						

//プログラムバージョン
#define PROVER		"1.0.1.0"							

//プログラム名(テキスト)
#define PRONAME_TXT	TEXT(PRONAME)


#define __UNICODE_STR(str)  L ## str
#define UNICODE_STR(str)  __UNICODE_STR(str)

//プログラム名(テキスト)
#define PRONAME_UNICODE	UNICODE_STR(PRONAME)


//プログラムバージョン(テキスト)
#define PROVER_TXT	TEXT(PROVER)						

