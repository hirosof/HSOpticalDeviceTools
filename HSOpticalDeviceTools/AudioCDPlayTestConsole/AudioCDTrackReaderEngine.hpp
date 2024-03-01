#pragma once
#include "../CommonLib/CHSCompactDiscReader.hpp"
#include "CHSWAVEOUT.hpp"

struct CDPlayInformation {
	CHSOpticalDrive* pDrive;
	THSSCSI_RawTOCTrackItem track;
};

using CHSWAVEOUT_CDPlayInformation = CHSWAVEOUT<CDPlayInformation>;

class  AudioCDTrackReaderEngine : public IHSWAVEOUT_CALLBACK<CDPlayInformation> {
private:
	WAVEFORMATEX m_wfex;
	CHSCompactDiscReader m_reader;
public:

	virtual bool OnSetup( UserDataType UserData ) {
		printf( "AudioCDTrackReaderEngine::OnSetup" );
		printf( "\n" );
		this->OnChangeUserData( UserData );
		return true;
	}


	virtual void OnChangeUserData( UserDataType UserData ) {
		printf( "AudioCDTrackReaderEngine::OnChangeUserData" );
		printf( "\n" );
		u_data = UserData;
		m_reader.SetDrive( u_data.pDrive );
	}


	virtual void OnOpenDevice( WAVEFORMATEX wfex ) {
		printf( "AudioCDTrackReaderEngine::OnOpenDevice" );
		printf( "\n" );

		this->m_wfex = wfex;
	}
	virtual void OnCloseDevice( void ) {
		printf( "AudioCDTrackReaderEngine::OnCloseDevice" );
		printf( "\n" );

	}
	virtual void OnPlayDataProcess( void* pPlayData, uint32_t DataSamples ) {
		printf( "AudioCDTrackReaderEngine::OnPlayDataProcess(%u サンプル)", DataSamples );
		printf( "\n" );

	}
	virtual void OnPlayBefore( void ) {
		printf( "AudioCDTrackReaderEngine::OnPlayBefore" );
		printf( "\n" );
		u_data.pDrive->spinUp( nullptr, false );

	}
	virtual void OnPlayAfter( void ) {
		printf( "AudioCDTrackReaderEngine::OnPlayAfter\n" );
		printf( "\n" );

	}
	virtual void OnPlayStop( bool bRepeatFlag, bool UserStopFlag ) {
		printf( "\nAudioCDTrackReaderEngine::OnPlayStop(Repeat:%d , UserStop:%d)\n", bRepeatFlag, UserStopFlag );
		if ( bRepeatFlag ) printf( "\n[リピート再生します。]\n" );
		printf( "\n" );

	}
	virtual void OnPause( void ) {
		printf( "AudioCDTrackReaderEngine::OnPause" );
		printf( "\n" );
		u_data.pDrive->spinDown( nullptr, true );

	}

	virtual void OnBeroreResume( void ) {
		printf( "AudioCDTrackReaderEngine::OnBeroreResume" );
		printf( "\n" );
		u_data.pDrive->spinUp( nullptr, false );
	}

	virtual void OnResume( void ) {
		printf( "AudioCDTrackReaderEngine::OnResume" );
		printf( "\n" );

	}

	virtual bool GetLength( uint32_t* pLength ) {
		size_t size;

		size = u_data.track.TrackLength.u32Value * CHSCompactDiscReader::NormalCDDATrackSectorSize;
		size /= m_wfex.nBlockAlign;

		*pLength = static_cast<uint32_t>( size );
		return true;
	}

	uint32_t OnReadPlayData( void* pPlayData, uint32_t ReadPos, uint32_t ReadSamples ) {
		printf( "\nAudioCDTrackReaderEngine::OnReadPlayData\n");

		UHSSCSI_AddressData32 target_offset , target_size;
		uint32_t sectorSizeU32 = static_cast<uint32_t>( CHSCompactDiscReader::NormalCDDATrackSectorSize );

		target_offset.u32Value = ReadPos * m_wfex.nBlockAlign / sectorSizeU32;
		uint32_t  internalOffset = ReadPos * m_wfex.nBlockAlign % sectorSizeU32;
		
		target_size.u32Value = ReadSamples * m_wfex.nBlockAlign / sectorSizeU32;
		if ( internalOffset > 0 ) target_size.u32Value++;
		if ( ( ReadSamples * m_wfex.nBlockAlign % sectorSizeU32 ) > 0 ) target_size.u32Value++;

		CHSSCSIGeneralBuffer buf;
		size_t loadSize = m_reader.readStereoAudioTrack( 
			&buf, u_data.track.TrackNumber,
			target_offset, EHSSCSI_AddressFormType::LBA,
			target_size, EHSSCSI_AddressFormType::LBA
		);

		size_t loadBytesSize = loadSize * CHSCompactDiscReader::NormalCDDATrackSectorSize;
		size_t reflectionSize =  min(loadBytesSize - internalOffset , ReadSamples * m_wfex.nBlockAlign);

		memcpy( pPlayData, buf.getBufferType<uint8_t*>( ) + internalOffset , reflectionSize );

		printf( "\tRequest Samples Range：%u～%u\n", ReadPos, ReadPos + ReadSamples - 1 );
		printf( "\tLBA offset : %u , LBA read Sectors : %u\n", target_offset.u32Value, target_size.u32Value );
		printf( "\tRequest Top Sample offset  : %u\n", internalOffset );
		printf( "\tReal Read Size ：%zu sectors (%zu Bytes)\n", loadSize, loadBytesSize );
		printf( "\tReflection PlayData Size ：%zu Bytes (%zu Samples)\n", reflectionSize, reflectionSize / m_wfex.nBlockAlign );
		printf( "\n" );
		return static_cast<uint32_t>( reflectionSize );
	}
};

