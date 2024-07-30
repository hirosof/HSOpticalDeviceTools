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
		printf( "光学ドライブの列挙に失敗しました\n" );
		return 0;
	}

	if ( optical_drives_enum.uOpticalDriveCount == 0 ) {
		printf( "光学ドライブが接続されていません。\n" );
		return 0;
	}

	std::string sep( 80, '-' );

	printf( "【光学ドライブリスト】\n\n" );
	printf( "番号：[ドライブ文字] デバイス表示名\n" );
	printf( "%s\n", sep.c_str( ) );
	for ( uint8_t id = 0; id < optical_drives_enum.uOpticalDriveCount; id++ ) {

		printf( "%4u：[%c:]", id, optical_drives_enum.Drives[id].Letter );
		if ( optical_drives_enum.Drives[id].bIncludedInfo ) {
			printf( " %s", optical_drives_enum.Drives[id].Info.DisplayName );
		}
		printf( "\n" );
	}
	printf( "%s\n", sep.c_str( ) );
	printf( "\n" );

	uint32_t selectedOpticalDriveNumber = 0;

	if ( optical_drives_enum.uOpticalDriveCount == 1 ) {
		printf( "接続されている光学ドライブは1つでしたので、該当のドライブが自動で選択されました。\n" );
		selectedOpticalDriveNumber = 0;
	} else {

		printf( "上のリストから使用する光学ドライブを番号で指定してください：" );

		while ( true ) {
			(void) scanf_s( "%u", &selectedOpticalDriveNumber );
			(void) Console_ReadLine( );

			if ( selectedOpticalDriveNumber < optical_drives_enum.uOpticalDriveCount ) {
				break;
			}

			printf( "無効な番号が入力されました。指定をやり直してください：" );
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

	printf( "【ドライブの状態チェック】\n" );

	if ( drive.open( driveletter ) == false ) {
		printf( "ドライブを開けませんでした。\n" );
		return;
	}

	if ( drive.isTrayOpened( ) ) {
		printf( "ドライブのトレイが開かれています。\n" );
		return;
	}

	EHSSCSI_ReadyStatus readyState = drive.checkReady( nullptr );

	switch ( readyState ) {
		case EHSSCSI_ReadyStatus::Ready:
			printf( "ドライブにアクセスする準備ができています。\n" );
			break;
		case EHSSCSI_ReadyStatus::NotReady:
			printf( "ドライブにアクセスする準備ができていません。\n" );

			break;
		case EHSSCSI_ReadyStatus::FailedGotStatus:
			printf( "ドライブの状態取得に失敗しました。\n" );

			break;
		case EHSSCSI_ReadyStatus::MediumNotPresent:
			printf( "ドライブにメディアが挿入されていません。\n" );
			break;
	}

	if ( readyState != EHSSCSI_ReadyStatus::Ready ) {
		return;
	}



	printf( "\n【ディスクの状態チェック】\n" );
	CHSOpticalDriveGetConfigCmd cmd( &drive );
	EHSSCSI_ProfileName pn = cmd.getCurrentProfileName( );
	printf( "セットされているメディアの種類：%s\n", cmd.GetProfileNameString( pn ).c_str( ) );


	CHSCompactDiscReader cdreader( &drive );
	if ( cdreader.isCDMediaPresent( ) == false ) {
		printf( "ドライブにセットされているメディアはCD系メディアではありません。\n");
		return;
	}

	THSSCSI_DiscInformation di;
	THSSCSI_InterpretedDiscInformation idi;
	if ( cdreader.readDiscInformation( &di, &idi ) ) {
		if ( di.DiscStatus == 0 ) {
			printf( "ドライブにセットされている%sはブランクメディアです。\n" ,
				cmd.GetProfileNameString( pn ).c_str( ));

			return;
		}
	}

	printf( "問題なし、TOC情報の確認と選択に移行します。\n\n" );

	DiscProcess( &drive );
}



void DiscProcess( CHSOpticalDrive* pDrive ) {

	if ( pDrive == nullptr ) return;

	printf( "【TOC情報】\n" );
	CHSCompactDiscReader cdreader( pDrive );
	THSSCSI_RawTOC rawToc;
	if ( cdreader.readRawTOC( &rawToc, EHSSCSI_AddressFormType::LBA ) == false ) {
		printf( "TOC情報の読み取りに失敗しました\n" );
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
		printf( "リッピングに対応している形式のトラックがありません\n" );
		return;
	} else {
		printf( "リッピングに対応している形式のトラックがありました、処理を続行します。\n" );
	}

	THSSCSI_CDTEXT_Information cdtext;
	printf( "\n【CD-TEXT情報取得】\n" );

	printf( "スピンアップしています..." );
	pDrive->spinUp( nullptr, false );
	printf( "完了\n\n" );

	printf( "CD-TEXT情報をデバイスに照会しています..." );
	EHSSCSI_CDText_ReadResult cdtextReadResult = cdreader.readCDText( &cdtext );

	if ( cdtextReadResult == EHSSCSI_CDText_ReadResult::Success ) {
		if ( cdtext.hasItems ) {
			printf( "成功しました。\n" );
		} else {
			printf( "CD-TEXT情報を持っていませんでした。\n" );
		}
	} else if ( cdtextReadResult == EHSSCSI_CDText_ReadResult::TimeOut ) {
		cdtext.hasItems = false;
		printf( "タイムアウトしました。\n" );
	} else {
		cdtext.hasItems = false;
		printf( "タイムアウト以外の要因で失敗しました。\n" );
	}
	
	printf( "\nスピンダウンしています..." );
	pDrive->spinDown( nullptr, false );
	printf( "完了\n\n" );


	printf( "\n【リッピング可能なトラックリスト】\n" );

	std::string separator( 100, '-' );

	printf( "%s\n", separator.c_str( ) );

	printf( "[%7s][Track] : %10s ～ %-10s (%8s)  ","Session", "開始位置", "終了位置", "長さ" );
	printf( "[%02s : %02s : %02s]\tMiB単位のサイズ\n", "分", "秒", "フレーム" );

	printf( "%s\n", separator.c_str( ) );


	UHSSCSI_AddressData32 address32;
	double  mib_size;
	for ( uint8_t track_no = rawToc.FirstTrackNumber; track_no <= rawToc.LastTrackNumber; track_no++ ) {

		if ( rawToc.trackItems[track_no].TrackType != EHSSCSI_TrackType::Audio2Channel )continue;

		printf( "[%7u][%5u] : %10u ～ %-10u (%8u)  ", 
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

		printf( "\n【CD-TEXT情報出力】\n" );
		printf( "\nアルバム名 : %s\n", cdtext.parsedItems[cdtextBlockID].album.Name.c_str( ) );
		printf( "アルバムアーティスト : %s\n", cdtext.parsedItems[cdtextBlockID].album.PerformerName.c_str( ) );
		printf( "\n%s\n", separator.c_str( ) );
		printf( "[Track] %-*s\tアーティスト名\n", static_cast<int>( songNameMaxLen ), "曲名" );

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

	printf( "\n【リッピングするトラックの選択】\n" );
	printf( "\n\t[トラック指定例]\n" );
	printf( "\t例1) トラック1をリッピングする場合：1\n" );
	printf( "\t例2) トラック1と3をリッピングする場合：1,3\n" );
	printf( "\t例3) トラック1から3をリッピングする場合：1-3\n" );
	printf( "\t例4) トラック2から6とトラック10をリッピングする場合：2-6,10\n" );
	printf( "\t例5) 全トラックをリッピングする場合：all\n" );

	printf( "\n\tリッピングするトラックを指定してください (上記のように複数指定可能です)：" );
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
		printf( "\n有効なトラック番号が入力されませんでした。\n" );
		return;
	}


	printf( "\n【リッピングを行うトラック番号リスト】\n" );

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



	printf( "\n【リッピング状況】\n" );

	printf( "\n\t処理中にキャンセルする場合はESCキーを押してください。\n" );

	printf( "\n\t[前処理]\n" );

	if ( isDriveLockSupport ) {
		printf( "\n\t\tトレイをロックしています..." );
		if ( pDrive->trayLock( ) ) printf( "成功" );
		else printf( "失敗" );
	}

	printf( "\n\t\tスピンアップしています..." );
	pDrive->spinUp( nullptr, false );
	printf( "完了\n\n" );


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
				printf( "\n\t\t処理前におけるドライブの状態確認にて、ドライブにアクセスできない状態になったことを確認しました。\n" );
				breakProcess = true;
				break;
			case EHSSCSI_ReadyStatus::FailedGotStatus:
				printf( "\n\t\t処理前におけるドライブの状態取得に失敗しました。\n" );
				breakProcess = true;
				break;
			case EHSSCSI_ReadyStatus::MediumNotPresent:
				printf( "\n\t\t処理前におけるドライブの状態確認にて、ディスクがイジェクトされたことを確認しました。\n" );
				breakProcess = true;
				break;
			default:
				break;
		}
		
		if ( breakProcess ) {
			printf( "\t\t以後の処理を中断します。\n" );
			break;
		}

		if ( rawToc.trackItems[RippingTrack].TrackType != EHSSCSI_TrackType::Audio2Channel ) {
			printf( "\n\t\tリッピング不可能なトラックなためスキップします\n" );
			continue;
		}

		GetLocalTime( &stf );

		swprintf_s( output_file_name, L"%04d%02d%02d_%02d%02d%02d_Track-%02u.wav",
			stf.wYear, stf.wMonth, stf.wDay,
			stf.wHour, stf.wMinute, stf.wSecond,
			RippingTrack );

		printf( "\n\t\t出力ファイル名：%S\n", output_file_name );

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

				printf( "\n\t\tCD-TEXT：\n" );
				printf( "\t\t\tアルバム名：%s\n", cdtext.parsedItems[cdtextBlockID].album.Name.c_str( ) );
				printf( "\t\t\t曲名：%s\n", cdtext.parsedItems[cdtextBlockID].trackTitles[RippingTrack].Name.c_str( ) );
				printf( "\t\t\tアーティスト名：%s\n", cdtext.parsedItems[cdtextBlockID].trackTitles[RippingTrack].PerformerName.c_str( ) );

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
				printf( "\n\t\tキャンセルされました、以後のリッピングを中止します。\n" );
				break;
			}
		}

	}

	stTotalTimeMS = timeGetTime( ) - stTotalStartMS;
	stTotalTimeSec = stTotalTimeMS / 1000;


	printf( "\n\t[後処理]\n" );
	printf( "\n\t\tスピンダウンしています..." );
	pDrive->spinDown( nullptr, false );
	printf( "完了" );
	if ( isDriveLockSupport ) {
		printf( "\n\t\tトレイのロックを解除しています..." );
		if ( pDrive->trayUnlock( ) ) printf( "成功" );
		else printf( "失敗" );
	}
	printf( "\n\n" );


	printf( "\n【リッピング結果】\n\n" );

	printf( "\t[リッピングの総所要時間]\n" );
	printf( "\t\t%02u分%02u秒%03u\n\n", stTotalTimeSec / 60, stTotalTimeSec % 60, stTotalTimeMS % 1000 );

	printf( "\t[出力先フォルダ]\n" );
	printf( "\t\t%S\n\n", currentDirectory.get( ) );

	printf( "\t[トラックと出力先ファイル名一覧]\n" );
	for ( auto& item : ripping_target_track_files ) {
		printf( "\t\t[Track %02u]：%S\n", item.first, item.second.c_str( ) );
	}

	if ( rippingProcessCancelled ) {
		printf( "\n\t\t※ 今回キャンセルされましたので、以上のリストにある最後のファイルは\n\t\t※ トラックの途中までとなっています。\n" );
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
		printf( "\r\t\t進捗状況：%.2f%%完了 (%zu / %u sectors)", ( block + 1 ) * 100.0 / numberOfReadBlocks,
			currentPositionSector, track.TrackLength.u32Value );

		processTime = timeGetTime( ) - startTime;

		if ( processTime > 0 ) {
			speed_sector_per_sec = static_cast<uint32_t>( currentPositionSector * 1000.0 / processTime );
			rest_time_ms= static_cast<uint32_t>( ( track.TrackLength.u32Value - currentPositionSector ) * 1000.0/ speed_sector_per_sec );

			processTimeSec = processTime / 1000;
			rest_time_sec = rest_time_ms / 1000;

			printf( "[残り推定時間=%02d分%02d秒%03d][経過時間=%02d分%02d秒%03d]",
				rest_time_sec / 60, rest_time_sec % 60, rest_time_ms % 1000,
				processTimeSec / 60, processTimeSec % 60, processTime % 1000
			);

		}

	}

	printf( "\n" );
	pWaveWriter->EndDataChunk( );
}