#include <cstdio>
#include <string>
#include <iostream>
#include <locale>
#include <atlbase.h>
#include <atlstr.h>
#include <unordered_map>

#include "../../Libraries/CommonLib/CHSOpticalDriveGetConfigCmd.hpp"
#include "../../Libraries/CommonLib/CHSOpticalDrive.hpp"
#include "../../common/CCommandLineParser.hpp"


#define APPNAME "TrayController"


enum struct ProcessMode {
	Unspecified=0,
	Invalid,
	Info,
	Status,
	Eject,
	Load,
	ReLoad,
	Lock,
	Unlock
};


enum struct DoTrayState {
	Open = 0,
	Close
};


const std::unordered_map<std::string, ProcessMode>  modeMap {
	{"info" , ProcessMode::Info },
	{"status" , ProcessMode::Status },
	{"eject" , ProcessMode::Eject },
	{"load" , ProcessMode::Load },
	{"open" , ProcessMode::Eject },
	{"close" , ProcessMode::Load },
	{"reload" , ProcessMode::ReLoad },
	{"lock" , ProcessMode::Lock },
	{"unlock" , ProcessMode::Unlock },

};


const std::vector < std::pair<std::string, ProcessMode>>  DescriptionToModeArray {
	{"トレイに関する対応情報とトレイの状態を取得する" , ProcessMode::Info },
	{"トレイの状態を取得する" , ProcessMode::Status },
	{"トレイを開く" , ProcessMode::Eject },
	{"トレイを閉じる" , ProcessMode::Load },
	{"トレイを開いて再度閉じる" , ProcessMode::ReLoad },
	{"トレイをロックする" , ProcessMode::Lock },
	{"トレイのロックを解除する" , ProcessMode::Unlock },

};


std::unordered_map<ProcessMode, std::string> ModeToDescriptionMap;
CCommandLineParserExA cmdParser;

std::string Console_ReadLine( );
ProcessMode GetModeByString(const std::string s );
char DriveSelect( void );
ProcessMode ModeSelect( void);
void DriveMain( const char DriveLetter, const ProcessMode Mode);
bool  TrayOpenOrClose( CHSOpticalDrive* pDrive, DoTrayState state, std::string* pMessage=nullptr );
int ProcessMain( void );

int main( void ) {

	SetConsoleTitleA( APPNAME );
	setlocale( LC_ALL, "Japanese" );

	cmdParser.parse( GetCommandLineA( ) );

	ProcessMain( );

	if ( !cmdParser.hasNamedOption( "nowait" ) ) {
		printf( "\n\n" );
		system( "pause" );
	}

	return 0;
}

int ProcessMain( void ) {
	ModeToDescriptionMap.clear( );
	for ( auto& current : DescriptionToModeArray ) {
		ModeToDescriptionMap[current.second] = current.first;
	}


	const std::string dualsep( 80, '=' );


	printf( "%s\n%s\n%s\n\n", dualsep.c_str( ), APPNAME, dualsep.c_str( ) );

	if ( cmdParser.hasNamedOption( "help" ) || cmdParser.hasNamedOption( "?" ) ) {

		printf( "%s Help\n\n準備中\n", APPNAME );

		return 0;
	}


	printf( "【コマンドライン引数情報】\n\n" );

	char drive = '\0';
	if ( cmdParser.hasNamedOption( "drive" ) ) {
		drive = cmdParser.getNamedOptionTypeValue<char>( "drive", 0 );

		if ( islower( drive ) ) drive -= 32;

		if ( ( drive < 'A' ) || ( drive > 'Z' ) ) {
			printf( "\tドライブの指定が不正です。処理を中断します。\n" );
			return 0;
		}

		if ( !CHSOpticalDrive::IsOpticalDrive( drive ) ) {
			printf( "\t指定された%cドライブは光学ドライブではありませんでした。処理を中断します。\n", drive );
			return 0;
		}

		printf( "\tコマンドライン引数にて「%cドライブ」が指定されました。\n", drive );

	} else {
		printf( "\tコマンドライン引数にてドライブは指定されませんでした。コンソールへの入力によるドライブ選択となります。\n" );
	}

	ProcessMode mode = ProcessMode::Unspecified;
	if ( cmdParser.hasNamedOption( "mode" ) ) {
		std::string inMode = cmdParser.getNamedOptionValue( "mode", 0 );
		mode = GetModeByString( inMode );
		if ( mode == ProcessMode::Invalid ) {
			printf( "\t動作モードの指定が不正です。処理を中断します。\n" );
			return 0;
		}
		printf( "\tコマンドライン引数にて「%s」が動作モードと指定されました。\n", ModeToDescriptionMap[mode].c_str( ) );
	} else {
		printf( "\tコマンドライン引数にて動作モードは指定されませんでした。コンソールへの入力によるモード選択となります。\n" );
	}

	printf( "\n" );


	if ( drive == '\0' ) {
		drive = DriveSelect( );
		if ( drive == '\0' ) {
			return 0;
		} else {
			printf( "\n" );
		}
	}

	if ( mode == ProcessMode::Unspecified ) {
		mode = ModeSelect( );
		if ( mode == ProcessMode::Unspecified ) {
			return 0;
		} else {
			printf( "\n" );
		}
	}

	DriveMain( drive, mode );

	return 0;
}

ProcessMode GetModeByString( const std::string s ) {

	auto it = modeMap.find( s );

	if ( it == modeMap.end( ) ) return ProcessMode::Invalid;

	return it->second;
}


std::string Console_ReadLine( ) {

	std::string input;

	std::getline( std::cin, input );

	return input;
}



char DriveSelect( void ) {

	THSEnumrateOpticalDriveInfo optical_drives_enum;
	if ( CHSOpticalDrive::EnumOpticalDrive( &optical_drives_enum ) == false ) {
		printf( "光学ドライブの列挙に失敗しました\n" );
		return 0;
	}

	if ( optical_drives_enum.uOpticalDriveCount == 0 ) {
		printf( "光学ドライブが接続されていません。\n" );
		return 0;
	}

	std::string sep( 80, '-' );

	printf( "【光学ドライブ選択画面】\n\n" );
	printf( "\t%s\n", sep.c_str( ) );
	printf( "\t番号：[ドライブ文字] デバイス表示名\n" );
	printf( "\t%s\n", sep.c_str( ) );
	for ( uint8_t id = 0; id < optical_drives_enum.uOpticalDriveCount; id++ ) {

		printf( "\t%4u：[%c:\\]", id, optical_drives_enum.Drives[id].Letter );
		if ( optical_drives_enum.Drives[id].bIncludedInfo ) {
			printf( " %s", optical_drives_enum.Drives[id].Info.DisplayName );
		}
		printf( "\n" );
	}
	printf( "\t%s\n", sep.c_str( ) );
	printf( "\n" );

	int32_t selectedOpticalDriveNumber = 0;

	if ( optical_drives_enum.uOpticalDriveCount == 1 ) {
		printf( "\t接続されている光学ドライブは1つでしたので、該当のドライブが自動で選択されました。\n" );
		selectedOpticalDriveNumber = 0;
	} else {

		printf( "\t上のリストから使用する光学ドライブを番号で指定してください。\n\tなお、中断する場合は-1以下の数値を入力してください。\n\t" );

		while ( true ) {
			(void) scanf_s( "%d", &selectedOpticalDriveNumber );
			(void) Console_ReadLine( );

			if ( selectedOpticalDriveNumber <= -1 ) {
				printf( "\n\t中断を示す値が入力されました。中断します。" );
				return '\0';
			}

			if ( selectedOpticalDriveNumber < optical_drives_enum.uOpticalDriveCount ) {
				break;
			}

			printf( "\t無効な番号が入力されました。指定をやり直してください：" );
		}

	}

	return optical_drives_enum.Drives[selectedOpticalDriveNumber].Letter;
}

ProcessMode ModeSelect( void ) {


	std::string sep( 80, '-' );

	printf( "【モード選択画面】\n\n" );
	printf( "\t%s\n", sep.c_str( ) );
	printf( "\t番号：モード名\n" );
	printf( "\t%s\n", sep.c_str( ) );
	for ( size_t id = 0; id < DescriptionToModeArray.size( ); id++ ) {

		printf( "\t%4zu：%s\n", id, DescriptionToModeArray[id].first.c_str() );
	}
	printf( "\t%s\n", sep.c_str( ) );
	printf( "\n" );


	int32_t selectedModeNumber = 0;


	printf( "\t上のリストから実行する機能を番号で指定してください。\n\tなお、中断する場合は-1以下の数値を入力してください。\n\t" );

	while ( true ) {
		(void) scanf_s( "%d", &selectedModeNumber );
		(void) Console_ReadLine( );

		if ( selectedModeNumber <= -1 ) {
			printf( "\n\t中断を示す値が入力されました。中断します。" );
			return ProcessMode::Unspecified;
		}

		if ( selectedModeNumber < DescriptionToModeArray.size() ) {
			break;
		}

		printf( "\t無効な番号が入力されました。指定をやり直してください：" );
	}

	return DescriptionToModeArray[selectedModeNumber].second;
}


void DriveMain( const char DriveLetter, const ProcessMode Mode ) {

	CHSOpticalDrive drive;

	if ( !drive.open( DriveLetter ) ) {
		printf( "【エラー】\n\t指定された%cドライブを開けませんでした\n"  ,DriveLetter);
		return;
	}

	THSOpticalDriveDeviceInfo info;

	printf( "【指定されたドライブ】\n\tドライブ：%c:\\\n", DriveLetter );
	if ( drive.getCurrentDeviceInfo( &info ) ) {
		printf( "\tデバイス表示名：%s\n", info.DisplayName );
	}
	

	printf( "\n【選択された処理】\n" );
	printf( "\t%s\n", ModeToDescriptionMap[Mode].c_str( ) );


	CHSOpticalDriveGetConfigCmd cmd( &drive );

	THSSCSI_FeatureDescriptor_RemovableMedium  fd_rm;

	printf( "\n【処理実行部】\n" );

	if ( !cmd.getFeatureRemovableMedium( &fd_rm ) ) {
		printf( "\tドライブの対応情報の取得に失敗しました。処理を中断します。\n" );
		return;
	}

	printf( "\n" );

	std::string resultMessage;
	EHSOD_TrayState trayState = drive.checkTrayState( );
	switch ( Mode ) {
		case ProcessMode::Info:
			printf( "\t[対応情報]\n" );
			printf( "\t\tトレイを開く命令をサポートするか：%s\n", ( fd_rm.Eject ) ? "はい" : "いいえ" );
			printf( "\t\tトレイを閉じる命令をサポートするか：%s\n", ( fd_rm.Load ) ? "はい" : "いいえ" );
			printf( "\t\tトレイをロックする/ロックを解除する命令をサポートするか：%s\n", ( fd_rm.Lock ) ? "はい" : "いいえ" );
			printf( "\n" );

			/* ProcessMode::Info は ProcessMode::Status の処理も行うため、fallthroughする */
		case ProcessMode::Status:
			printf( "\t[トレイの状態]\n" );

			switch ( trayState ) {
				case EHSOD_TrayState::Closed:
					printf( "\t\tトレイは閉じられています。\n" );
					break;
				case EHSOD_TrayState::Opened:
					printf( "\t\tトレイは開かれています。\n" );
					break;
				case EHSOD_TrayState::FailedGotStatus:
					printf( "\t\tトレイの状態取得に失敗しました。。\n" );
					break;
			}

			break;
		case ProcessMode::Eject:
			printf( "\t[トレイを開く処理の状況と結果]\n" );

			if ( trayState == EHSOD_TrayState::Opened ) {
				printf( "\t既ににトレイが開かれています。\n" );
			} else {
				if ( !fd_rm.Eject ) {
					printf( "\tドライブはトレイを開く命令をサポートしていません。処理を中止します。\n" );
				} else {
					printf( "\tトレイを開いています..." );
					resultMessage.clear( );
					TrayOpenOrClose( &drive, DoTrayState::Open, &resultMessage );
					if ( resultMessage.empty( ) ) {
						printf( "失敗しました。(詳細不明)\n" );
					} else {
						printf( "%s\n", resultMessage.c_str( ) );
					}
				}
			}

			break;
		case ProcessMode::Load:
			printf( "\t[トレイを閉じる処理の状況と結果]\n" );
			if ( trayState == EHSOD_TrayState::Closed ) {
				printf( "\t既ににトレイが閉じられています。\n" );

			} else {
				if ( !fd_rm.Load ) {
					printf( "\tドライブはトレイを閉じる命令をサポートしていません。処理を中止します。\n" );
				} else {
					printf( "\tトレイを閉じています..." );
					resultMessage.clear( );
					TrayOpenOrClose( &drive, DoTrayState::Close, &resultMessage );
					if ( resultMessage.empty( ) ) {
						printf( "失敗しました。(詳細不明)\n" );
					} else {
						printf( "%s\n", resultMessage.c_str( ) );
					}
				}
			}
			break;
		case ProcessMode::ReLoad:
			printf( "\t[トレイを開いて再度閉じる処理の状況と結果]\n" );

			if ( !fd_rm.Eject ) {
				printf( "\tドライブはトレイを開く命令をサポートしていません。処理を中止します。\n" );
				break;
			}

			if ( !fd_rm.Load ) {
				printf( "\tドライブはトレイを閉じる命令をサポートしていません。処理を中止します。\n" );
				break;
			}

			if ( trayState == EHSOD_TrayState::Opened ) {
				printf( "\t既ににトレイが開かれていますので、開く処理はスキップします。\n" );
			} else {

				printf( "\tトレイを開いています..." );
				resultMessage.clear( );
				bool bret = TrayOpenOrClose( &drive, DoTrayState::Open, &resultMessage );
				if ( resultMessage.empty( ) ) {
					printf( "失敗しました。(詳細不明)\n" );
				} else {
					printf( "%s\n", resultMessage.c_str( ) );
				}

				if ( !bret ) break;

				uint32_t waitTime = 5;
				for ( uint32_t i = 0; i < waitTime; i++ ) {
					printf( "\r\t%u秒待機しています...", waitTime - i );
					Sleep( 1000 );
				}
				printf( "\r\t待機時間は終了しました。\n" );

			}
			printf( "\t続いて、トレイを閉じています..." );
			resultMessage.clear( );
			TrayOpenOrClose( &drive, DoTrayState::Close, &resultMessage );
			if ( resultMessage.empty( ) ) {
				printf( "失敗しました。(詳細不明)\n" );
			} else {
				printf( "%s\n", resultMessage.c_str( ) );
			}

			break;
		case ProcessMode::Lock:
			printf( "\t[トレイのロック処理結果]\n" );
			if ( !fd_rm.Lock ) {
				printf( "\tドライブはトレイをロックする命令をサポートしていません。処理を中止します。\n" );
			} else {
				HSSCSI_SPTD_RESULT res;
				bool bret =  drive.trayLock( &res );

				if ( bret ) {
					printf( "\tトレイのロック設定は正常に終了しました。\n" );
				} else {
					printf( "\tトレイのロック設定は失敗しました。 (詳細：SCSIStatus=0x%02X, SK=0x%02X, ASC=0x%02X, ASCQ=0x%02X)\n",
						res.scsiStatus.statusByteCode, res.scsiSK, res.scsiASC, res.scsiASCQ );
				}
			}

			break;
		case ProcessMode::Unlock:
			printf( "\t[トレイのロック解除処理結果]\n" );
			if ( !fd_rm.Lock ) {
				printf( "\tドライブはトレイのロックを解除する命令をサポートしていません。処理を中止します。\n" );
			} else {
				HSSCSI_SPTD_RESULT res;
				bool bret = drive.trayUnlock( &res );

				if ( bret ) {
					printf( "\tトレイのロック解除設定は正常に終了しました。\n" );
				} else {
					printf( "\tトレイのロック解除設定は失敗しました。 (詳細：SCSIStatus=0x%02X, SK=0x%02X, ASC=0x%02X, ASCQ=0x%02X)\n",
						res.scsiStatus.statusByteCode, res.scsiSK, res.scsiASC, res.scsiASCQ );
				}
			}
			break;
		default:
			break;
	}

}

bool  TrayOpenOrClose( CHSOpticalDrive* pDrive, DoTrayState state, std::string* pMessage ) {
	if ( pDrive ) {
		HSSCSI_SPTD_RESULT res;
		if ( state == DoTrayState::Open ) {
			pDrive->trayOpen( &res, false );
		} else {
			pDrive->trayClose( &res, false );
		}
		CAtlStringA resultmes;

		if ( HSSCSIStatusToStatusCode( res.scsiStatus ) != EHSSCSIStatusCode::Good ) {
			resultmes.Format( "失敗しました。 (詳細：SCSIStatus=0x%02X, SK=0x%02X, ASC=0x%02X, ASCQ=0x%02X)",
				res.scsiStatus.statusByteCode, res.scsiSK, res.scsiASC, res.scsiASCQ );
		} else {
			resultmes.Format( "成功しました。" );
		}

		if ( pMessage ) {
			pMessage->clear( );
			pMessage->append( resultmes.GetString( ) );
		}

		return HSSCSIStatusToStatusCode( res.scsiStatus ) == EHSSCSIStatusCode::Good;
	}
	return false;
}


