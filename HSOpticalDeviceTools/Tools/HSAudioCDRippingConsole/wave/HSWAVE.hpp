//2016-12-09
//2016-12-10
//2016-12-11

#pragma once

#include "HSRIFF.hpp"
#include <mmreg.h>


/*
	ˆÈ‰ºƒNƒ‰ƒX’è‹`
*/
//__CHSRiffReaderBaseTemplate<Type>
//__CHSRiffReaderBaseTemplate<Type>



template <typename Type> class __CHSWaveReaderBaseTemplate : public __CHSRiffReaderBaseTemplate<Type> {
private:
	union LocalFormatData {
		WAVEFORMATEX wfex;
		PCMWAVEFORMAT pcm_format;
		ADPCMWAVEFORMAT adpcm_format;
		IMAADPCMWAVEFORMAT imaadpcm_format;
		WAVEFORMATEXTENSIBLE extensible_format;
	};

	THSRiffChunkInfo formatChunkInfo;
	THSRiffChunkInfo dataChunkInfo;
	LocalFormatData localFormatData;
	bool AdditionalCheckProcess ( void ) {
		if ( this->CheckType ( "WAVE" ) ) {
			if ( this->GetChunkInfo ( "fmt" , &this->formatChunkInfo ) ) {
				if ( this->GetChunkInfo ( "data" , &this->dataChunkInfo ) ) {
					memset ( &this->localFormatData , 0 , sizeof ( LocalFormatData ) );
					if ( this->ReadFormatChunk ( &this->localFormatData , 0 , sizeof ( LocalFormatData ) ) ) {
						return true;
					}
				}
			}
		}
		return false;
	}
	
	void __WaveReaderInit ( void ) {
		memset ( &this->formatChunkInfo , 0 , sizeof ( THSRiffChunkInfo ) );
		memset ( &this->dataChunkInfo , 0 , sizeof ( THSRiffChunkInfo ) );
	}
public:


	__CHSWaveReaderBaseTemplate ( )  : __CHSRiffReaderBaseTemplate<Type> (){
		this->__WaveReaderInit ( );
	}
	
	__CHSWaveReaderBaseTemplate (Type *lpszFilePath )  : __CHSRiffReaderBaseTemplate<Type>(){
		this->__WaveReaderInit ( );
		this->Open ( lpszFilePath );
	}

	uint32_t GetDataChunkSize ( void ) {
		return this->dataChunkInfo.Header.DataSize;
	}

	uint32_t ReadDataChunk ( void *lpData , uint32_t offset , uint32_t readsize ) {
		return this->ReadChunkData ( "data" , lpData , offset , readsize );
	}

	uint32_t GetFormatChunkSize ( void ) {
		return this->formatChunkInfo.Header.DataSize;
	}

	uint32_t ReadFormatChunk ( void *lpData , uint32_t offset , uint32_t readsize ) {
		return this->ReadChunkData ( "fmt" , lpData , offset , readsize );
	}

	bool HasExtendedFormatData ( void ) {
		return ( this->localFormatData.wfex.cbSize != 0 );
	}
	
	uint32_t GetExtendedFormatDataSize ( void ) {
		return  this->localFormatData.wfex.cbSize;
	}

	uint32_t ReadExtendedFormatData ( void *lpData , uint32_t offset , uint32_t readsize ) {
		return this->ReadChunkData ( "fmt" , lpData , offset + sizeof(WAVEFORMATEX) , readsize );
	}

	
	bool GetFormat ( WAVEFORMATEX *lpwfex ) {
		if ( lpwfex ) {
			*lpwfex = this->localFormatData.wfex;
			return true;
		}
		return false;
	}

	bool GetFormat ( PCMWAVEFORMAT *lppcm_format ) {
		if ( lppcm_format ) {
			*lppcm_format = this->localFormatData.pcm_format;
			return true;
		}
		return false;
	}
	bool GetFormat ( ADPCMWAVEFORMAT *lpadpcm_format ) {

		if ( lpadpcm_format ) {
			*lpadpcm_format = this->localFormatData.adpcm_format;
			return true;
		}
		return false;
	}
	bool GetFormat ( IMAADPCMWAVEFORMAT *lpimaadpcm_format ) {
		if ( lpimaadpcm_format ) {
			*lpimaadpcm_format = this->localFormatData.imaadpcm_format;
			return true;
		}
		return false;
	}

	bool GetFormat ( WAVEFORMATEXTENSIBLE *lpextensible_format ) {
		if ( lpextensible_format ) {
			*lpextensible_format = this->localFormatData.extensible_format;
			return true;
		}
		return false;
	}


};

using CHSWaveReaderA = __CHSWaveReaderBaseTemplate<char>;
using CHSWaveReaderW = __CHSWaveReaderBaseTemplate<wchar_t>;

//__CHSRiffWriterBaseTemplate<Type>
template <typename Type> class __CHSWaveWriterBaseTemplate : public __CHSRiffWriterBaseTemplate<Type> {

private:

	virtual bool CreateAdditionalProcess ( void ) {
		return this->WriteRiffType("WAVE");
	}

public:

	
	bool BeginDataChunk ( void ) {
		return this->BeginChunk ( "data" );
	}


	bool AdditionalDataChunkContent ( void *lpData , uint32_t size ) {
		uint32_t currentChunk = *reinterpret_cast< uint32_t* >( this->m_ChunkInfo.Header.Name );
		if ( currentChunk == HSRIFF_MAGICNUMBER_DATACHUNK ) {
			return this->AdditionalChunkData ( lpData , size );
		}
		return false;
	}


	bool EndDataChunk ( void ) {
		uint32_t currentChunk = *reinterpret_cast< uint32_t* >( this->m_ChunkInfo.Header.Name );
		if ( currentChunk == HSRIFF_MAGICNUMBER_DATACHUNK ) {
			return this->EndChunk ( );
		}
		return false;
	}
	


	bool WriteFormatChunk ( WAVEFORMATEX wfex ) {
		return this->WriteFormatChunkType ( wfex );
	}
	bool WriteFormatChunk ( PCMWAVEFORMAT pcm_format ) {
		return this->WriteFormatChunkType ( pcm_format );
	}
	bool WriteFormatChunk ( ADPCMWAVEFORMAT adpcm_format ) {
		return this->WriteFormatChunkType ( adpcm_format );
	}
	bool WriteFormatChunk ( IMAADPCMWAVEFORMAT imaadpcm_format ) {
		return this->WriteFormatChunkType ( imaadpcm_format );
	}

	bool WriteFormatChunk ( WAVEFORMATEXTENSIBLE extensible_format ) {
		return this->WriteFormatChunkType ( extensible_format );
	}

	bool WriteFormatChunkCostom ( void *lpFormat , uint32_t size ) {
		return this->WriteChunk ( "fmt" , lpFormat , size );
	}

	template<typename U> bool WriteFormatChunkType ( U Format ) {
		return this->WriteChunkType( "fmt" , Format );
	}


};


using CHSWaveWriterA = __CHSWaveWriterBaseTemplate<char>;
using CHSWaveWriterW = __CHSWaveWriterBaseTemplate<wchar_t>;




