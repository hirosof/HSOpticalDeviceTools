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
	{"�g���C�Ɋւ���Ή����ƃg���C�̏�Ԃ��擾����" , ProcessMode::Info },
	{"�g���C�̏�Ԃ��擾����" , ProcessMode::Status },
	{"�g���C���J��" , ProcessMode::Eject },
	{"�g���C�����" , ProcessMode::Load },
	{"�g���C���J���čēx����" , ProcessMode::ReLoad },
	{"�g���C�����b�N����" , ProcessMode::Lock },
	{"�g���C�̃��b�N����������" , ProcessMode::Unlock },

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

		printf( "%s Help\n\n������\n", APPNAME );

		return 0;
	}


	printf( "�y�R�}���h���C���������z\n\n" );

	char drive = '\0';
	if ( cmdParser.hasNamedOption( "drive" ) ) {
		drive = cmdParser.getNamedOptionTypeValue<char>( "drive", 0 );

		if ( islower( drive ) ) drive -= 32;

		if ( ( drive < 'A' ) || ( drive > 'Z' ) ) {
			printf( "\t�h���C�u�̎w�肪�s���ł��B�����𒆒f���܂��B\n" );
			return 0;
		}

		if ( !CHSOpticalDrive::IsOpticalDrive( drive ) ) {
			printf( "\t�w�肳�ꂽ%c�h���C�u�͌��w�h���C�u�ł͂���܂���ł����B�����𒆒f���܂��B\n", drive );
			return 0;
		}

		printf( "\t�R�}���h���C�������ɂāu%c�h���C�u�v���w�肳��܂����B\n", drive );

	} else {
		printf( "\t�R�}���h���C�������ɂăh���C�u�͎w�肳��܂���ł����B�R���\�[���ւ̓��͂ɂ��h���C�u�I���ƂȂ�܂��B\n" );
	}

	ProcessMode mode = ProcessMode::Unspecified;
	if ( cmdParser.hasNamedOption( "mode" ) ) {
		std::string inMode = cmdParser.getNamedOptionValue( "mode", 0 );
		mode = GetModeByString( inMode );
		if ( mode == ProcessMode::Invalid ) {
			printf( "\t���샂�[�h�̎w�肪�s���ł��B�����𒆒f���܂��B\n" );
			return 0;
		}
		printf( "\t�R�}���h���C�������ɂāu%s�v�����샂�[�h�Ǝw�肳��܂����B\n", ModeToDescriptionMap[mode].c_str( ) );
	} else {
		printf( "\t�R�}���h���C�������ɂē��샂�[�h�͎w�肳��܂���ł����B�R���\�[���ւ̓��͂ɂ�郂�[�h�I���ƂȂ�܂��B\n" );
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
		printf( "���w�h���C�u�̗񋓂Ɏ��s���܂���\n" );
		return 0;
	}

	if ( optical_drives_enum.uOpticalDriveCount == 0 ) {
		printf( "���w�h���C�u���ڑ�����Ă��܂���B\n" );
		return 0;
	}

	std::string sep( 80, '-' );

	printf( "�y���w�h���C�u�I����ʁz\n\n" );
	printf( "\t%s\n", sep.c_str( ) );
	printf( "\t�ԍ��F[�h���C�u����] �f�o�C�X�\����\n" );
	printf( "\t%s\n", sep.c_str( ) );
	for ( uint8_t id = 0; id < optical_drives_enum.uOpticalDriveCount; id++ ) {

		printf( "\t%4u�F[%c:\\]", id, optical_drives_enum.Drives[id].Letter );
		if ( optical_drives_enum.Drives[id].bIncludedInfo ) {
			printf( " %s", optical_drives_enum.Drives[id].Info.DisplayName );
		}
		printf( "\n" );
	}
	printf( "\t%s\n", sep.c_str( ) );
	printf( "\n" );

	int32_t selectedOpticalDriveNumber = 0;

	if ( optical_drives_enum.uOpticalDriveCount == 1 ) {
		printf( "\t�ڑ�����Ă�����w�h���C�u��1�ł����̂ŁA�Y���̃h���C�u�������őI������܂����B\n" );
		selectedOpticalDriveNumber = 0;
	} else {

		printf( "\t��̃��X�g����g�p������w�h���C�u��ԍ��Ŏw�肵�Ă��������B\n\t�Ȃ��A���f����ꍇ��-1�ȉ��̐��l����͂��Ă��������B\n\t" );

		while ( true ) {
			(void) scanf_s( "%d", &selectedOpticalDriveNumber );
			(void) Console_ReadLine( );

			if ( selectedOpticalDriveNumber <= -1 ) {
				printf( "\n\t���f�������l�����͂���܂����B���f���܂��B" );
				return '\0';
			}

			if ( selectedOpticalDriveNumber < optical_drives_enum.uOpticalDriveCount ) {
				break;
			}

			printf( "\t�����Ȕԍ������͂���܂����B�w�����蒼���Ă��������F" );
		}

	}

	return optical_drives_enum.Drives[selectedOpticalDriveNumber].Letter;
}

ProcessMode ModeSelect( void ) {


	std::string sep( 80, '-' );

	printf( "�y���[�h�I����ʁz\n\n" );
	printf( "\t%s\n", sep.c_str( ) );
	printf( "\t�ԍ��F���[�h��\n" );
	printf( "\t%s\n", sep.c_str( ) );
	for ( size_t id = 0; id < DescriptionToModeArray.size( ); id++ ) {

		printf( "\t%4zu�F%s\n", id, DescriptionToModeArray[id].first.c_str() );
	}
	printf( "\t%s\n", sep.c_str( ) );
	printf( "\n" );


	int32_t selectedModeNumber = 0;


	printf( "\t��̃��X�g������s����@�\��ԍ��Ŏw�肵�Ă��������B\n\t�Ȃ��A���f����ꍇ��-1�ȉ��̐��l����͂��Ă��������B\n\t" );

	while ( true ) {
		(void) scanf_s( "%d", &selectedModeNumber );
		(void) Console_ReadLine( );

		if ( selectedModeNumber <= -1 ) {
			printf( "\n\t���f�������l�����͂���܂����B���f���܂��B" );
			return ProcessMode::Unspecified;
		}

		if ( selectedModeNumber < DescriptionToModeArray.size() ) {
			break;
		}

		printf( "\t�����Ȕԍ������͂���܂����B�w�����蒼���Ă��������F" );
	}

	return DescriptionToModeArray[selectedModeNumber].second;
}


void DriveMain( const char DriveLetter, const ProcessMode Mode ) {

	CHSOpticalDrive drive;

	if ( !drive.open( DriveLetter ) ) {
		printf( "�y�G���[�z\n\t�w�肳�ꂽ%c�h���C�u���J���܂���ł���\n"  ,DriveLetter);
		return;
	}

	THSOpticalDriveDeviceInfo info;

	printf( "�y�w�肳�ꂽ�h���C�u�z\n\t�h���C�u�F%c:\\\n", DriveLetter );
	if ( drive.getCurrentDeviceInfo( &info ) ) {
		printf( "\t�f�o�C�X�\�����F%s\n", info.DisplayName );
	}
	

	printf( "\n�y�I�����ꂽ�����z\n" );
	printf( "\t%s\n", ModeToDescriptionMap[Mode].c_str( ) );


	CHSOpticalDriveGetConfigCmd cmd( &drive );

	THSSCSI_FeatureDescriptor_RemovableMedium  fd_rm;

	printf( "\n�y�������s���z\n" );

	if ( !cmd.getFeatureRemovableMedium( &fd_rm ) ) {
		printf( "\t�h���C�u�̑Ή����̎擾�Ɏ��s���܂����B�����𒆒f���܂��B\n" );
		return;
	}

	printf( "\n" );

	std::string resultMessage;
	EHSOD_TrayState trayState = drive.checkTrayState( );
	switch ( Mode ) {
		case ProcessMode::Info:
			printf( "\t[�Ή����]\n" );
			printf( "\t\t�g���C���J�����߂��T�|�[�g���邩�F%s\n", ( fd_rm.Eject ) ? "�͂�" : "������" );
			printf( "\t\t�g���C����閽�߂��T�|�[�g���邩�F%s\n", ( fd_rm.Load ) ? "�͂�" : "������" );
			printf( "\t\t�g���C�����b�N����/���b�N���������閽�߂��T�|�[�g���邩�F%s\n", ( fd_rm.Lock ) ? "�͂�" : "������" );
			printf( "\n" );

			/* ProcessMode::Info �� ProcessMode::Status �̏������s�����߁Afallthrough���� */
		case ProcessMode::Status:
			printf( "\t[�g���C�̏��]\n" );

			switch ( trayState ) {
				case EHSOD_TrayState::Closed:
					printf( "\t\t�g���C�͕����Ă��܂��B\n" );
					break;
				case EHSOD_TrayState::Opened:
					printf( "\t\t�g���C�͊J����Ă��܂��B\n" );
					break;
				case EHSOD_TrayState::FailedGotStatus:
					printf( "\t\t�g���C�̏�Ԏ擾�Ɏ��s���܂����B�B\n" );
					break;
			}

			break;
		case ProcessMode::Eject:
			printf( "\t[�g���C���J�������̏󋵂ƌ���]\n" );

			if ( trayState == EHSOD_TrayState::Opened ) {
				printf( "\t���ɂɃg���C���J����Ă��܂��B\n" );
			} else {
				if ( !fd_rm.Eject ) {
					printf( "\t�h���C�u�̓g���C���J�����߂��T�|�[�g���Ă��܂���B�����𒆎~���܂��B\n" );
				} else {
					printf( "\t�g���C���J���Ă��܂�..." );
					resultMessage.clear( );
					TrayOpenOrClose( &drive, DoTrayState::Open, &resultMessage );
					if ( resultMessage.empty( ) ) {
						printf( "���s���܂����B(�ڍוs��)\n" );
					} else {
						printf( "%s\n", resultMessage.c_str( ) );
					}
				}
			}

			break;
		case ProcessMode::Load:
			printf( "\t[�g���C����鏈���̏󋵂ƌ���]\n" );
			if ( trayState == EHSOD_TrayState::Closed ) {
				printf( "\t���ɂɃg���C�������Ă��܂��B\n" );

			} else {
				if ( !fd_rm.Load ) {
					printf( "\t�h���C�u�̓g���C����閽�߂��T�|�[�g���Ă��܂���B�����𒆎~���܂��B\n" );
				} else {
					printf( "\t�g���C����Ă��܂�..." );
					resultMessage.clear( );
					TrayOpenOrClose( &drive, DoTrayState::Close, &resultMessage );
					if ( resultMessage.empty( ) ) {
						printf( "���s���܂����B(�ڍוs��)\n" );
					} else {
						printf( "%s\n", resultMessage.c_str( ) );
					}
				}
			}
			break;
		case ProcessMode::ReLoad:
			printf( "\t[�g���C���J���čēx���鏈���̏󋵂ƌ���]\n" );

			if ( !fd_rm.Eject ) {
				printf( "\t�h���C�u�̓g���C���J�����߂��T�|�[�g���Ă��܂���B�����𒆎~���܂��B\n" );
				break;
			}

			if ( !fd_rm.Load ) {
				printf( "\t�h���C�u�̓g���C����閽�߂��T�|�[�g���Ă��܂���B�����𒆎~���܂��B\n" );
				break;
			}

			if ( trayState == EHSOD_TrayState::Opened ) {
				printf( "\t���ɂɃg���C���J����Ă��܂��̂ŁA�J�������̓X�L�b�v���܂��B\n" );
			} else {

				printf( "\t�g���C���J���Ă��܂�..." );
				resultMessage.clear( );
				bool bret = TrayOpenOrClose( &drive, DoTrayState::Open, &resultMessage );
				if ( resultMessage.empty( ) ) {
					printf( "���s���܂����B(�ڍוs��)\n" );
				} else {
					printf( "%s\n", resultMessage.c_str( ) );
				}

				if ( !bret ) break;

				uint32_t waitTime = 5;
				for ( uint32_t i = 0; i < waitTime; i++ ) {
					printf( "\r\t%u�b�ҋ@���Ă��܂�...", waitTime - i );
					Sleep( 1000 );
				}
				printf( "\r\t�ҋ@���Ԃ͏I�����܂����B\n" );

			}
			printf( "\t�����āA�g���C����Ă��܂�..." );
			resultMessage.clear( );
			TrayOpenOrClose( &drive, DoTrayState::Close, &resultMessage );
			if ( resultMessage.empty( ) ) {
				printf( "���s���܂����B(�ڍוs��)\n" );
			} else {
				printf( "%s\n", resultMessage.c_str( ) );
			}

			break;
		case ProcessMode::Lock:
			printf( "\t[�g���C�̃��b�N��������]\n" );
			if ( !fd_rm.Lock ) {
				printf( "\t�h���C�u�̓g���C�����b�N���閽�߂��T�|�[�g���Ă��܂���B�����𒆎~���܂��B\n" );
			} else {
				HSSCSI_SPTD_RESULT res;
				bool bret =  drive.trayLock( &res );

				if ( bret ) {
					printf( "\t�g���C�̃��b�N�ݒ�͐���ɏI�����܂����B\n" );
				} else {
					printf( "\t�g���C�̃��b�N�ݒ�͎��s���܂����B (�ڍׁFSCSIStatus=0x%02X, SK=0x%02X, ASC=0x%02X, ASCQ=0x%02X)\n",
						res.scsiStatus.statusByteCode, res.scsiSK, res.scsiASC, res.scsiASCQ );
				}
			}

			break;
		case ProcessMode::Unlock:
			printf( "\t[�g���C�̃��b�N������������]\n" );
			if ( !fd_rm.Lock ) {
				printf( "\t�h���C�u�̓g���C�̃��b�N���������閽�߂��T�|�[�g���Ă��܂���B�����𒆎~���܂��B\n" );
			} else {
				HSSCSI_SPTD_RESULT res;
				bool bret = drive.trayUnlock( &res );

				if ( bret ) {
					printf( "\t�g���C�̃��b�N�����ݒ�͐���ɏI�����܂����B\n" );
				} else {
					printf( "\t�g���C�̃��b�N�����ݒ�͎��s���܂����B (�ڍׁFSCSIStatus=0x%02X, SK=0x%02X, ASC=0x%02X, ASCQ=0x%02X)\n",
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
			resultmes.Format( "���s���܂����B (�ڍׁFSCSIStatus=0x%02X, SK=0x%02X, ASC=0x%02X, ASCQ=0x%02X)",
				res.scsiStatus.statusByteCode, res.scsiSK, res.scsiASC, res.scsiASCQ );
		} else {
			resultmes.Format( "�������܂����B" );
		}

		if ( pMessage ) {
			pMessage->clear( );
			pMessage->append( resultmes.GetString( ) );
		}

		return HSSCSIStatusToStatusCode( res.scsiStatus ) == EHSSCSIStatusCode::Good;
	}
	return false;
}


