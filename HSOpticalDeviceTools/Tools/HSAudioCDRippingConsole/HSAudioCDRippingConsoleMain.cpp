#include <cstdio>
#include <string>
#include <iostream>
#include <locale>
#include <set>
#include <atlbase.h>
#include <atlstr.h>
#include "../../Libraries/CommonLib/CHSCompactDiscReader.hpp"
#include "../../common/CHSBase64.hpp"
#include "../../Libraries/WaveFileIOLib/HSWAVE.hpp"
#include "../../Libraries/HashLib/HSSHA1.hpp"
#include "../../Libraries/HashLib/HSSHA2.hpp"
#include "../../Libraries/InternalLib/RangeSpecifyStringParser.hpp"

#pragma comment(lib,"winmm.lib")

std::string Console_ReadLine( );

void DriveProcessEntry( char driveletter );
void DiscProcess( CHSOpticalDrive* pDrive );
void RippingMain( CHSWaveWriterW* pWaveWriter, CHSOpticalDrive* pDrive, THSSCSI_RawTOCTrackItem track , bool *pCancelled = nullptr);

int main( void ) {

	SetConsoleTitleA( "HSAudioCDRippingConsole" );
	setlocale( LC_ALL, "Japanese" );
	
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

	printf( "�y���w�h���C�u���X�g�z\n\n" );
	printf( "�ԍ��F[�h���C�u����] �f�o�C�X�\����\n" );
	printf( "%s\n", sep.c_str( ) );
	for ( uint8_t id = 0; id < optical_drives_enum.uOpticalDriveCount; id++ ) {

		printf( "%4u�F[%c:]", id, optical_drives_enum.Drives[id].Letter );
		if ( optical_drives_enum.Drives[id].bIncludedInfo ) {
			printf( " %s", optical_drives_enum.Drives[id].Info.DisplayName );
		}
		printf( "\n" );
	}
	printf( "%s\n", sep.c_str( ) );
	printf( "\n" );

	uint32_t selectedOpticalDriveNumber = 0;

	if ( optical_drives_enum.uOpticalDriveCount == 1 ) {
		printf( "�ڑ�����Ă�����w�h���C�u��1�ł����̂ŁA�Y���̃h���C�u�������őI������܂����B\n" );
		selectedOpticalDriveNumber = 0;
	} else {

		printf( "��̃��X�g����g�p������w�h���C�u��ԍ��Ŏw�肵�Ă��������F" );

		while ( true ) {
			(void) scanf_s( "%u", &selectedOpticalDriveNumber );
			(void) Console_ReadLine( );

			if ( selectedOpticalDriveNumber < optical_drives_enum.uOpticalDriveCount ) {
				break;
			}

			printf( "�����Ȕԍ������͂���܂����B�w�����蒼���Ă��������F" );
		}


	}
	printf( "\n" );
	DriveProcessEntry( optical_drives_enum.Drives[selectedOpticalDriveNumber].Letter );
	printf( "\n" );

	system( "pause" );
	return 0;
}

std::string Console_ReadLine( ) {

	std::string input;

	std::getline( std::cin, input );

	return input;
}


void DriveProcessEntry( char driveletter ) {

	CHSOpticalDrive drive;

	std::string sep( 80, '=' );

	printf( "�y�h���C�u�̏�ԃ`�F�b�N�z\n" );

	if ( drive.open( driveletter ) == false ) {
		printf( "�h���C�u���J���܂���ł����B\n" );
		return;
	}

	if ( drive.isTrayOpened( ) ) {
		printf( "�h���C�u�̃g���C���J����Ă��܂��B\n" );
		return;
	}

	EHSSCSI_ReadyStatus readyState = drive.checkReady( nullptr );

	switch ( readyState ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "�h���C�u�ɃA�N�Z�X���鏀�����ł��Ă��܂��B\n" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "�h���C�u�ɃA�N�Z�X���鏀�����ł��Ă��܂���B\n" );

			break;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "�h���C�u�̏�Ԏ擾�Ɏ��s���܂����B\n" );

			break;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "�h���C�u�Ƀ��f�B�A���}������Ă��܂���B\n" );
			break;
	}

	if ( readyState != EHSSCSI_ReadyStatus::Ready ) {
		return;
	}



	printf( "\n�y�f�B�X�N�̏�ԃ`�F�b�N�z\n" );
	CHSOpticalDriveGetConfigCmd cmd( &drive );
	EHSSCSI_ProfileName pn = cmd.getCurrentProfileName( );
	printf( "�Z�b�g����Ă��郁�f�B�A�̎�ށF%s\n", cmd.GetProfileNameString( pn ).c_str( ) );


	CHSCompactDiscReader cdreader( &drive );
	if ( cdreader.isCDMediaPresent( ) == false ) {
		printf( "�h���C�u�ɃZ�b�g����Ă��郁�f�B�A��CD�n���f�B�A�ł͂���܂���B\n");
		return;
	}

	THSSCSI_DiscInformation di;
	THSSCSI_InterpretedDiscInformation idi;
	if ( cdreader.readDiscInformation( &di, &idi ) ) {
		if ( di.DiscStatus == 0 ) {
			printf( "�h���C�u�ɃZ�b�g����Ă���%s�̓u�����N���f�B�A�ł��B\n" ,
				cmd.GetProfileNameString( pn ).c_str( ));

			return;
		}
	}

	printf( "���Ȃ��ATOC���̊m�F�ƑI���Ɉڍs���܂��B\n\n" );

	DiscProcess( &drive );
}



void DiscProcess( CHSOpticalDrive* pDrive ) {

	if ( pDrive == nullptr ) return;

	printf( "�yTOC���z\n" );
	CHSCompactDiscReader cdreader( pDrive );
	THSSCSI_RawTOC rawToc;
	if ( cdreader.readRawTOC( &rawToc, EHSSCSI_AddressFormType::LBA ) == false ) {
		printf( "TOC���̓ǂݎ��Ɏ��s���܂���\n" );
		return;
	}

	bool has_audio2channel_track = false;

	for ( auto& item : rawToc.trackItems ) {
		if ( item.second.TrackType == EHSSCSI_TrackType::Audio2Channel ) {
			has_audio2channel_track = true;
			break;
		}
	}

	if ( !has_audio2channel_track ) {
		printf( "���b�s���O�ɑΉ����Ă���`���̃g���b�N������܂���\n" );
		return;
	} else {
		printf( "���b�s���O�ɑΉ����Ă���`���̃g���b�N������܂����A�����𑱍s���܂��B\n" );
	}

	THSSCSI_CDTEXT_Information cdtext;
	printf( "\n�yCD-TEXT���擾�z\n" );

	printf( "�X�s���A�b�v���Ă��܂�..." );
	pDrive->spinUp( nullptr, false );
	printf( "����\n\n" );

	printf( "CD-TEXT�����f�o�C�X�ɏƉ�Ă��܂�..." );
	EHSSCSI_CDText_ReadResult cdtextReadResult = cdreader.readCDText( &cdtext );

	if ( cdtextReadResult == EHSSCSI_CDText_ReadResult::Success ) {
		if ( cdtext.hasItems ) {
			printf( "�������܂����B\n" );
		} else {
			printf( "CD-TEXT���������Ă��܂���ł����B\n" );
		}
	} else if ( cdtextReadResult == EHSSCSI_CDText_ReadResult::TimeOut ) {
		cdtext.hasItems = false;
		printf( "�^�C���A�E�g���܂����B\n" );
	} else {
		cdtext.hasItems = false;
		printf( "�^�C���A�E�g�ȊO�̗v���Ŏ��s���܂����B\n" );
	}
	
	printf( "\n�X�s���_�E�����Ă��܂�..." );
	pDrive->spinDown( nullptr, false );
	printf( "����\n\n" );


	printf( "\n�y���b�s���O�\�ȃg���b�N���X�g�z\n" );

	std::string separator( 100, '-' );

	printf( "%s\n", separator.c_str( ) );

	printf( "[%7s][Track] : %10s �` %-10s (%8s)  ","Session", "�J�n�ʒu", "�I���ʒu", "����" );
	printf( "[%02s : %02s : %02s]\tMiB�P�ʂ̃T�C�Y\n", "��", "�b", "�t���[��" );

	printf( "%s\n", separator.c_str( ) );


	UHSSCSI_AddressData32 address32;
	double  mib_size;
	for ( uint8_t track_no = rawToc.FirstTrackNumber; track_no <= rawToc.LastTrackNumber; track_no++ ) {

		if ( rawToc.trackItems[track_no].TrackType != EHSSCSI_TrackType::Audio2Channel )continue;

		printf( "[%7u][%5u] : %10u �` %-10u (%8u)  ", 
			rawToc.trackItems[track_no].SessionNumber,
			track_no, 
			rawToc.trackItems[track_no].TrackStartAddress.u32Value,
			rawToc.trackItems[track_no].TrackEndAddress.u32Value,
			rawToc.trackItems[track_no].TrackLength.u32Value );

		address32 = CHSCompactDiscReader::SplitMSF( rawToc.trackItems[track_no].TrackLength );

		printf( "[%02uM : %02uS : %02uF]\t", address32.urawValues[1], address32.urawValues[2], address32.urawValues[3]);
		
		mib_size = static_cast<double>(rawToc.trackItems[track_no].TrackLength.u32Value * CHSCompactDiscReader::NormalCDDATrackSectorSize);
		mib_size /= 1024 * 1024;

		printf( "%.2f MiB", mib_size );

		printf( "\n" );

	}
	printf( "%s\n", separator.c_str( ) );

	hirosof::Hash::CSHA256 hash;
	hirosof::Hash::CSHA256Value toc_hash_value;;

	for ( auto item : rawToc.rawItems ) {
		hash.Put( item );
	}

	if ( cdtext.hasItems ) {
		for (auto item : cdtext.rawItems) {
			hash.Put( item );
		}
	}

	hash.Finalize( );
	hash.GetHash( &toc_hash_value );

	std::string toc_str = cdreader.GetTOCStringStatic( &rawToc );

	printf( "Raw TOC Hash:\n\t%s\n\n", toc_hash_value.ToString( ).c_str( ) );

	printf( "TOC String :\n\t%s\n\n", toc_str.c_str());

	hirosof::Hash::CSHA1 mbdiscid_sha1;
	hirosof::Hash::CSHA1Value mbdiscid_sha1_value;
	std::string musicBrainzDiscIDSource = cdreader.GetMusicBrainzDiscIDSourceStatic( &rawToc );
	mbdiscid_sha1.Compute( musicBrainzDiscIDSource.c_str( ) );
	mbdiscid_sha1.GetHash( &mbdiscid_sha1_value );


	hirosof::Hash::CSHA1Value::ElementType mbdiscid_sha1_raw[hirosof::Hash::CSHA1Value::m_ElementSize];
	for ( size_t i = 0; i < hirosof::Hash::CSHA1Value::m_ElementSize; i++ ) {
		mbdiscid_sha1_raw[i] = HSSCSI_InverseEndian32(mbdiscid_sha1_value.GetWordValue( i ));
	}
	
	printf( "getMusicBrainzDiscIDSourceHash Binary Value:\n\t" );
	for ( size_t i = 0; i < mbdiscid_sha1_value.Count( ); i++ ) {
		printf( "%02X ", mbdiscid_sha1_value.GetValue( i ) );
	}
	printf( "\n\n" );

	std::string MusicBrainzDiscID = CHSBase64MusicBrainz<char>::Encode( mbdiscid_sha1_raw, sizeof( mbdiscid_sha1_raw ) );
	printf( "MusicBrainz DiscID:\n\t" );
	printf( "%s\n\n", MusicBrainzDiscID.c_str( ) );

	CAtlStringW MusicBrainzDiscIDInfoURLBase;

	MusicBrainzDiscIDInfoURLBase.Format( L"https://musicbrainz.org/ws/2/discid/%S?toc=%S&inc=recordings", MusicBrainzDiscID.c_str( ), toc_str.c_str( ) );


	printf( "MusicBrainz DiscID Info URL (Result Type = json):\n\t" );
	printf( "%S&fmt=json\n\n", MusicBrainzDiscIDInfoURLBase.GetString( ) );

	printf( "MusicBrainz DiscID Info URL (Result Type = xml):\n\t" );
	printf( "%S&fmt=xml\n", MusicBrainzDiscIDInfoURLBase.GetString( ) );

	uint8_t cdtextBlockID = 0;

	if ( cdtext.hasItems ) {
		for ( uint8_t i = 0; i < cdtext.NumberOfBlocks; i++ ) {
			if ( cdtext.parsedItems[i].isDoubleByteCharatorCode ) {
				cdtextBlockID = i;
				break;
			}
		}

		size_t songNameMaxLen = 0;
		size_t currentSongNameLen;
		for ( uint8_t track_no = rawToc.FirstTrackNumber; track_no <= rawToc.LastTrackNumber; track_no++ ) {
			currentSongNameLen = cdtext.parsedItems[cdtextBlockID].trackTitles[track_no].Name.length( );
			songNameMaxLen = max( songNameMaxLen, currentSongNameLen );
		}

		printf( "\n�yCD-TEXT���o�́z\n" );
		printf( "\n�A���o���� : %s\n", cdtext.parsedItems[cdtextBlockID].album.Name.c_str( ) );
		printf( "�A���o���A�[�e�B�X�g : %s\n", cdtext.parsedItems[cdtextBlockID].album.PerformerName.c_str( ) );
		printf( "\n%s\n", separator.c_str( ) );
		printf( "[Track] %-*s\t�A�[�e�B�X�g��\n", static_cast<int>( songNameMaxLen ), "�Ȗ�" );

		printf( "%s\n", separator.c_str( ) );

		for ( uint8_t track_no = rawToc.FirstTrackNumber; track_no <= rawToc.LastTrackNumber; track_no++ ) {
			printf( "[%5u] %-*s\t%s\n", track_no, static_cast<int>( songNameMaxLen ),
				cdtext.parsedItems[cdtextBlockID].trackTitles[track_no].Name.c_str( ),
				cdtext.parsedItems[cdtextBlockID].trackTitles[track_no].PerformerName.c_str( )
			);
		}
		printf( "%s\n", separator.c_str( ) );

		printf( "\n" );

	}

	printf( "\n�y���b�s���O����g���b�N�̑I���z\n" );
	printf( "\n\t[�g���b�N�w���]\n" );
	printf( "\t��1) �g���b�N1�����b�s���O����ꍇ�F1\n" );
	printf( "\t��2) �g���b�N1��3�����b�s���O����ꍇ�F1,3\n" );
	printf( "\t��3) �g���b�N1����3�����b�s���O����ꍇ�F1-3\n" );
	printf( "\t��4) �g���b�N2����6�ƃg���b�N10�����b�s���O����ꍇ�F2-6,10\n" );
	printf( "\t��5) �S�g���b�N�����b�s���O����ꍇ�Fall\n" );

	printf( "\n\t���b�s���O����g���b�N���w�肵�Ă������� (��L�̂悤�ɕ����w��\�ł�)�F" );
	std::string rippingTrackRangesRaw = Console_ReadLine( );
	std::set<uint32_t>  specifyTracks;

	if ( lstrcmpiA( rippingTrackRangesRaw.c_str( ), "all" ) == 0 ) {
		for ( uint8_t track_no = rawToc.FirstTrackNumber; track_no <= rawToc.LastTrackNumber; track_no++ ) {
			specifyTracks.insert( track_no );
		}
	} else {
		RangeValueTypeUVector  rippingTrackRangeValues = RangeSpecifyStringParseUnsigned( rippingTrackRangesRaw );
		for ( RangeValueTypeU v : rippingTrackRangeValues ) {
			for ( uint32_t track_no = v.first; track_no <= v.second; track_no++ ) {
				if ( track_no < rawToc.FirstTrackNumber ) continue;
				if ( track_no > rawToc.LastTrackNumber ) break;
				specifyTracks.insert( track_no );
			}
		}
	}

	if ( specifyTracks.empty( ) ) {
		printf( "\n�L���ȃg���b�N�ԍ������͂���܂���ł����B\n" );
		return;
	}


	printf( "\n�y���b�s���O���s���g���b�N�ԍ����X�g�z\n" );

	uint32_t count = 0 , numberOfUnit=22;
	bool afterNewline = false;
	for ( auto it = specifyTracks.begin( ); it != specifyTracks.end( ); it++ ) {
		count++;
		afterNewline = false;

		if ( ( it == specifyTracks.begin( ) ) || ( ( count % numberOfUnit ) == 1 ) ) {
			printf( "\t" );

		} else if ( ( count % numberOfUnit ) != 1 ) {
			printf( ", " );
		}

		printf( "%02u" , *it);

		if ( ( count % numberOfUnit ) == 0 ) {
			printf( "\n" );
			afterNewline = true;
		}

	}

	if(!afterNewline ) 	printf( "\n" );

	wchar_t foldername[] = L"waveout";
	CreateDirectoryW( foldername, nullptr );
	SetCurrentDirectoryW( foldername );

	std::wstring wshash = toc_hash_value.ToWString( );
	CreateDirectoryW( wshash.c_str( ), nullptr );
	SetCurrentDirectoryW( wshash.c_str( ) );

	DWORD  sizeOfCurrentDirectoryPath = GetCurrentDirectoryW( 0, nullptr );
	std::shared_ptr<wchar_t> currentDirectory( new wchar_t[sizeOfCurrentDirectoryPath] );
	GetCurrentDirectoryW( sizeOfCurrentDirectoryPath, currentDirectory.get( ) );

	std::vector<std::pair<uint32_t, std::wstring>>  ripping_target_track_files;
	std::pair<uint32_t, std::wstring> ripping_target_track_file_item;
	uint32_t RippingTrack = 0;


	wchar_t output_file_name[260];
	bool isDriveLockSupport = false;
	CHSOpticalDriveGetConfigCmd cmd( pDrive );
	THSSCSI_FeatureDescriptor_RemovableMedium  fd_rm;


	if ( cmd.getFeatureRemovableMedium( &fd_rm ) ) {
		isDriveLockSupport = fd_rm.Lock;
	}



	printf( "\n�y���b�s���O�󋵁z\n" );

	printf( "\n\t�������ɃL�����Z������ꍇ��ESC�L�[�������Ă��������B\n" );

	printf( "\n\t[�O����]\n" );

	if ( isDriveLockSupport ) {
		printf( "\n\t\t�g���C�����b�N���Ă��܂�..." );
		if ( pDrive->trayLock( ) ) printf( "����" );
		else printf( "���s" );
	}

	printf( "\n\t\t�X�s���A�b�v���Ă��܂�..." );
	pDrive->spinUp( nullptr, false );
	printf( "����\n\n" );


	DWORD stTotalStartMS, stTotalTimeSec,stTotalTimeMS;
	stTotalStartMS = timeGetTime( );
	
	SYSTEMTIME stf;
	EHSSCSI_ReadyStatus driveStatus;
	bool breakProcess;
	count = 0;
	bool rippingProcessCancelled = false;

	for ( auto it = specifyTracks.begin( ); it != specifyTracks.end( ); it++ ) {
		RippingTrack = *it;

		count++;
		printf( "\n\t[Track %02u] (%u/%zu)\n", RippingTrack  , count ,specifyTracks.size());


		breakProcess = false;

		driveStatus = pDrive->checkReady(  );

		switch ( driveStatus ) {
			case EHSSCSI_ReadyStatus::Ready:
				break;
			case EHSSCSI_ReadyStatus::NotReady:
				printf( "\n\t\t�����O�ɂ�����h���C�u�̏�Ԋm�F�ɂāA�h���C�u�ɃA�N�Z�X�ł��Ȃ���ԂɂȂ������Ƃ��m�F���܂����B\n" );
				breakProcess = true;
				break;
			case EHSSCSI_ReadyStatus::FailedGotStatus:
				printf( "\n\t\t�����O�ɂ�����h���C�u�̏�Ԏ擾�Ɏ��s���܂����B\n" );
				breakProcess = true;
				break;
			case EHSSCSI_ReadyStatus::MediumNotPresent:
				printf( "\n\t\t�����O�ɂ�����h���C�u�̏�Ԋm�F�ɂāA�f�B�X�N���C�W�F�N�g���ꂽ���Ƃ��m�F���܂����B\n" );
				breakProcess = true;
				break;
			default:
				break;
		}
		
		if ( breakProcess ) {
			printf( "\t\t�Ȍ�̏����𒆒f���܂��B\n" );
			break;
		}

		if ( rawToc.trackItems[RippingTrack].TrackType != EHSSCSI_TrackType::Audio2Channel ) {
			printf( "\n\t\t���b�s���O�s�\�ȃg���b�N�Ȃ��߃X�L�b�v���܂�\n" );
			continue;
		}

		GetLocalTime( &stf );

		swprintf_s( output_file_name, L"%04d%02d%02d_%02d%02d%02d_Track-%02u.wav",
			stf.wYear, stf.wMonth, stf.wDay,
			stf.wHour, stf.wMinute, stf.wSecond,
			RippingTrack );

		printf( "\n\t\t�o�̓t�@�C�����F%S\n", output_file_name );

		CHSWaveWriterW wave;
		if ( wave.Create( output_file_name ) ) {


			wave.BeginListChunk( "INFO" );

			if ( cdtext.hasItems ) {
				wave.WriteListMemberChunkString( HSRIFF_FOURCC_LIST_INFO_IPRD,
					cdtext.parsedItems[cdtextBlockID].album.Name.c_str( ),
					true );
				wave.WriteListMemberChunkString( HSRIFF_FOURCC_LIST_INFO_INAM,
					cdtext.parsedItems[cdtextBlockID].trackTitles[RippingTrack].Name.c_str( ),
					true );
				wave.WriteListMemberChunkString( HSRIFF_FOURCC_LIST_INFO_IART,
					cdtext.parsedItems[cdtextBlockID].trackTitles[RippingTrack].PerformerName.c_str( ),
					true );

				printf( "\n\t\tCD-TEXT�F\n" );
				printf( "\t\t\t�A���o�����F%s\n", cdtext.parsedItems[cdtextBlockID].album.Name.c_str( ) );
				printf( "\t\t\t�Ȗ��F%s\n", cdtext.parsedItems[cdtextBlockID].trackTitles[RippingTrack].Name.c_str( ) );
				printf( "\t\t\t�A�[�e�B�X�g���F%s\n", cdtext.parsedItems[cdtextBlockID].trackTitles[RippingTrack].PerformerName.c_str( ) );

				printf( "\n" );
			}

			char TrackNumberText[3];
			sprintf_s( TrackNumberText, "%u", RippingTrack );
			wave.WriteListMemberChunkString( HSRIFF_FOURCC_LIST_INFO_ITRK, TrackNumberText, true );

			wave.WriteListMemberChunkString( "ITOC", toc_str.c_str( ), true );
			wave.WriteListMemberChunkString( HSRIFF_FOURCC_LIST_INFO_ICMT, toc_hash_value.ToString( ).c_str( ), true );
			wave.WriteListMemberChunkString( HSRIFF_FOURCC_LIST_INFO_ISFT, "HSAudioCDRippingConsole" , true );


			wave.EndListChunk( );

			PCMWAVEFORMAT pcm;
			pcm.wBitsPerSample = 16;
			pcm.wf.wFormatTag = WAVE_FORMAT_PCM;
			pcm.wf.nSamplesPerSec = 44100;
			pcm.wf.nChannels = 2;
			pcm.wf.nBlockAlign = pcm.wBitsPerSample / 8 * pcm.wf.nChannels;
			pcm.wf.nAvgBytesPerSec = pcm.wf.nBlockAlign * pcm.wf.nSamplesPerSec;
			wave.WriteFormatChunkType( pcm );


			RippingMain( &wave, pDrive, rawToc.trackItems[RippingTrack], &rippingProcessCancelled );

			wave.Close( );

			ripping_target_track_file_item.first = RippingTrack;
			ripping_target_track_file_item.second = output_file_name;
			ripping_target_track_files.push_back( ripping_target_track_file_item );

			
			if ( rippingProcessCancelled ) {
				printf( "\n\t\t�L�����Z������܂����A�Ȍ�̃��b�s���O�𒆎~���܂��B\n" );
				break;
			}
		}

	}

	stTotalTimeMS = timeGetTime( ) - stTotalStartMS;
	stTotalTimeSec = stTotalTimeMS / 1000;


	printf( "\n\t[�㏈��]\n" );
	printf( "\n\t\t�X�s���_�E�����Ă��܂�..." );
	pDrive->spinDown( nullptr, false );
	printf( "����" );
	if ( isDriveLockSupport ) {
		printf( "\n\t\t�g���C�̃��b�N���������Ă��܂�..." );
		if ( pDrive->trayUnlock( ) ) printf( "����" );
		else printf( "���s" );
	}
	printf( "\n\n" );


	printf( "\n�y���b�s���O���ʁz\n\n" );

	printf( "\t[���b�s���O�̑����v����]\n" );
	printf( "\t\t%02u��%02u�b%03u\n\n", stTotalTimeSec / 60, stTotalTimeSec % 60, stTotalTimeMS % 1000 );

	printf( "\t[�o�͐�t�H���_]\n" );
	printf( "\t\t%S\n\n", currentDirectory.get( ) );

	printf( "\t[�g���b�N�Əo�͐�t�@�C�����ꗗ]\n" );
	for ( auto& item : ripping_target_track_files ) {
		printf( "\t\t[Track %02u]�F%S\n", item.first, item.second.c_str( ) );
	}

	if ( rippingProcessCancelled ) {
		printf( "\n\t\t�� ����L�����Z������܂����̂ŁA�ȏ�̃��X�g�ɂ���Ō�̃t�@�C����\n\t\t�� �g���b�N�̓r���܂łƂȂ��Ă��܂��B\n" );
	}

	
}


void RippingMain( CHSWaveWriterW* pWaveWriter, CHSOpticalDrive* pDrive, THSSCSI_RawTOCTrackItem track, bool* pCancelled ) {

	if ( pDrive == nullptr ) return;
	if ( pWaveWriter == nullptr ) return;

	CHSCompactDiscReader cdreader( pDrive );
	cdreader.setSpeedMax( );

	UHSSCSI_AddressData32 once_read_size = CHSCompactDiscReader::MakeAddressData32( 0, 1, 0 );
	once_read_size = CHSCompactDiscReader::MergeMSF( once_read_size );

	uint32_t numberOfReadBlocks = track.TrackLength.u32Value / once_read_size.u32Value;
	if ( ( track.TrackLength.u32Value % once_read_size.u32Value ) != 0 ) numberOfReadBlocks++;

	CHSSCSIGeneralBuffer buffer;
	size_t readSuccessSectorLength;
	UHSSCSI_AddressData32 pos;

	pWaveWriter->BeginDataChunk( );

	DWORD startTime = timeGetTime( );
	DWORD processTime , processTimeSec;
	size_t currentPositionSector;
	uint32_t speed_sector_per_sec;
	uint32_t rest_time_ms, rest_time_sec;

	for ( uint32_t block = 0; block < numberOfReadBlocks; block++ ) {

		if ( pCancelled ) {
			if ( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 ) {
				*pCancelled = true;
				break;
			}
		}

		pos.u32Value = once_read_size.u32Value * block;

		readSuccessSectorLength = cdreader.readStereoAudioTrack( &buffer,
			track.TrackNumber,
			pos, EHSSCSI_AddressFormType::LBA,
			once_read_size, EHSSCSI_AddressFormType::LBA );

		if ( readSuccessSectorLength == 0 ) {
			break;
		}

		pWaveWriter->AdditionalDataChunkContent( buffer.getBuffer( ),
			static_cast<uint32_t>( readSuccessSectorLength * CHSCompactDiscReader::NormalCDDATrackSectorSize ) );

		currentPositionSector = pos.u32Value + readSuccessSectorLength;
		printf( "\r\t\t�i���󋵁F%.2f%%���� (%zu / %u sectors)", ( block + 1 ) * 100.0 / numberOfReadBlocks,
			currentPositionSector, track.TrackLength.u32Value );

		processTime = timeGetTime( ) - startTime;

		if ( processTime > 0 ) {
			speed_sector_per_sec = static_cast<uint32_t>( currentPositionSector * 1000.0 / processTime );
			rest_time_ms= static_cast<uint32_t>( ( track.TrackLength.u32Value - currentPositionSector ) * 1000.0/ speed_sector_per_sec );

			processTimeSec = processTime / 1000;
			rest_time_sec = rest_time_ms / 1000;

			printf( "[�c�萄�莞��=%02d��%02d�b%03d][�o�ߎ���=%02d��%02d�b%03d]",
				rest_time_sec / 60, rest_time_sec % 60, rest_time_ms % 1000,
				processTimeSec / 60, processTimeSec % 60, processTime % 1000
			);

		}

	}

	printf( "\n" );
	pWaveWriter->EndDataChunk( );
}