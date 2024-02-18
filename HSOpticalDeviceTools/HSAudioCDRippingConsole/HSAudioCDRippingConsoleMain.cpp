#include <cstdio>
#include <string>
#include <iostream>
#include <locale>
#include "../CommonLib/CHSCompactDiscReader.hpp"
#include "wave/HSWAVE.hpp"
#include "hash/HSSHA2.hpp"

std::string Console_ReadLine( );

void DriveProcessEntry( char driveletter );
void TOCCheckAndSelect( CHSOpticalDrive* pDrive );
void RippingMain( CHSWaveWriterW* pWaveWriter, CHSOpticalDrive* pDrive, THSSCSI_RawTOCTrackItem track );

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
	printf( "�ԍ��F[�h���C�u����] �f�o�C�X��\n" );
	printf( "%s\n", sep.c_str( ) );
	for ( uint8_t id = 0; id < optical_drives_enum.uOpticalDriveCount; id++ ) {

		printf( "%4u�F[%c:]", id, optical_drives_enum.Drives[id].Letter );
		if ( optical_drives_enum.Drives[id].bIncludedInfo ) {
			printf( " %s", optical_drives_enum.Drives[id].Info.DeviceName );
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

	TOCCheckAndSelect( &drive );
}



void TOCCheckAndSelect( CHSOpticalDrive* pDrive ) {

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
		printf( "���b�s���O�ɑΉ����Ă���`���̃g���b�N������܂����A�g���b�N���X�g���o�͂��܂��B\n" );

	}

	
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
	hash.Finalize( );
	hash.GetHash( &toc_hash_value );

	printf( "DISC ID  : %s\n", toc_hash_value.ToString( ).c_str( ) );
	printf( "%s\n", separator.c_str( ) );


	UINT RippingTrack = 0;

	printf( "\n�y���b�s���O����g���b�N�̑I���z\n" );
	printf( "���b�s���O����g���b�N�ԍ�����͂��Ă��������F" );
	scanf_s( "%u", &RippingTrack );
	(void) Console_ReadLine( );

	if ( ( RippingTrack < rawToc.FirstTrackNumber ) || ( rawToc.LastTrackNumber < RippingTrack ) ) {
		printf( "\n�s���ȃg���b�N�ԍ����w�肳��܂����B\n" );
		return;
	}

	if ( rawToc.trackItems[RippingTrack].TrackType != EHSSCSI_TrackType::Audio2Channel ) {
		printf( "\n���b�s���O�s�\�ȃg���b�N�ԍ����w�肳��܂����B\n" );
		return;
	}

	printf( "\n�y���b�s���O�󋵁z\n" );
	CHSWaveWriterW wave;
	SYSTEMTIME stf;
	GetLocalTime( &stf );

	wchar_t foldername[] = L"waveout";
	CreateDirectoryW( foldername, nullptr );
	SetCurrentDirectoryW( foldername );

	std::wstring wshash = toc_hash_value.ToWString( );
	CreateDirectoryW( wshash.c_str( ), nullptr );
	SetCurrentDirectoryW( wshash.c_str( ) );

	wchar_t output_file_name[260];
	swprintf_s( output_file_name, L"%04d%02d%02d_%02d%02d%02d_Track-%02u.wav",
		stf.wYear, stf.wMonth, stf.wDay,
		stf.wHour, stf.wMinute, stf.wSecond,
		RippingTrack );

	if ( wave.Create( output_file_name ) ) {

		PCMWAVEFORMAT pcm;
		pcm.wBitsPerSample = 16;
		pcm.wf.wFormatTag = WAVE_FORMAT_PCM;
		pcm.wf.nSamplesPerSec = 44100;
		pcm.wf.nChannels = 2;
		pcm.wf.nBlockAlign = pcm.wBitsPerSample / 8 * pcm.wf.nChannels;
		pcm.wf.nAvgBytesPerSec = pcm.wf.nBlockAlign * pcm.wf.nSamplesPerSec;
		wave.WriteFormatChunkType( pcm );

		RippingMain( &wave, pDrive, rawToc.trackItems[RippingTrack] );

		printf( "\t���b�s���O���������܂����B\n\n[�ۑ���]\n" );

		size_t filename_len = wave.GetCreatedFilePath( nullptr, 0 );
		wchar_t* pname = new wchar_t[filename_len];

		if ( wave.GetCreatedFilePath( pname, filename_len ) > 0 ) {
			printf( "%S", pname );
		}

		delete[]pname;

		printf( "\n" );

		wave.Close( );

	}
}


void RippingMain( CHSWaveWriterW* pWaveWriter, CHSOpticalDrive* pDrive, THSSCSI_RawTOCTrackItem track ) {

	if ( pDrive == nullptr ) return;
	if ( pWaveWriter == nullptr ) return;

	CHSCompactDiscReader cdreader( pDrive );

	UHSSCSI_AddressData32 once_read_size = CHSCompactDiscReader::MakeAddressData32( 0, 1, 0 );
	once_read_size = CHSCompactDiscReader::MergeMSF( once_read_size );

	uint32_t numberOfReadBlocks = track.TrackLength.u32Value / once_read_size.u32Value;
	if ( ( track.TrackLength.u32Value % once_read_size.u32Value ) != 0 ) numberOfReadBlocks++;

	CHSSCSIGeneralBuffer buffer;
	size_t readSuccessSectorLenth;
	UHSSCSI_AddressData32 pos;

	pDrive->spinUp( nullptr, false );
	pWaveWriter->BeginDataChunk( );

	for ( uint32_t block = 0; block < numberOfReadBlocks; block++ ) {
		pos.u32Value = once_read_size.u32Value * block;

		readSuccessSectorLenth = cdreader.readStereoAudioTrack( &buffer,
			track.TrackNumber,
			pos, EHSSCSI_AddressFormType::LBA,
			once_read_size, EHSSCSI_AddressFormType::LBA );

		if ( readSuccessSectorLenth == 0 )break;

		pWaveWriter->AdditionalDataChunkContent( buffer.getBuffer( ),
			static_cast<uint32_t>( readSuccessSectorLenth * CHSCompactDiscReader::NormalCDDATrackSectorSize ));
		
		printf( "\r\t%.2f%% (%zu / %u sectors) complete.", (block+1) * 100.0 / numberOfReadBlocks , 
			once_read_size.u32Value * block + readSuccessSectorLenth  , track.TrackLength.u32Value );

	}
	printf( "\n" );
	pWaveWriter->EndDataChunk( );
	pDrive->spinDown( nullptr, true );
}