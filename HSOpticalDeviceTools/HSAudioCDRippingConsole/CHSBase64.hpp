#pragma once

#include <cstdio>
#include <string>
#include <cstdint>
#include <vector>

template <typename CharType, CharType symbol_at_62, CharType symbol_at_63, CharType symbol_at_padding> class CHSBase64T {
public:
	using String = std::basic_string<CharType>;
	using VectorData = std::vector<uint8_t>;

public:

	static String GetBase64CharTable( void ) {
		static String charTable;

		if ( charTable.empty( ) ) {

			for ( CharType c = 'A'; c <= 'Z'; c++ ) {
				charTable.push_back( c );
			}
			for ( CharType c = 'a'; c <= 'z'; c++ ) {
				charTable.push_back( c );
			}

			for ( CharType c = '0'; c <= '9'; c++ ) {
				charTable.push_back( c );
			}

			charTable.push_back( symbol_at_62 );
			charTable.push_back( symbol_at_63 );

		}

		return charTable;
	}

public:


	static String Encode(const void* pData, size_t dataLength ) {

		if ( pData == nullptr ) return String( );
		const uint8_t* pTop = reinterpret_cast<const uint8_t*>( pData );
	
		size_t NumberOfBlocks = dataLength / 3;
		size_t FinalRestData = dataLength % 3;
		if ( FinalRestData != 0 ) NumberOfBlocks++;

		String target;
		const String table = GetBase64CharTable( );
		uint8_t inData[3], intermediateData[4];
		size_t readSize;

		for ( size_t i = 0; i < NumberOfBlocks; i++ ) {

			inData[0] = 0;
			inData[1] = 0;
			inData[2] = 0;

			readSize = 3;
			if ( ( i + 1 ) == NumberOfBlocks ) {
				if ( FinalRestData != 0 ) readSize = FinalRestData;
			}

			for( size_t j = 0; j < readSize; j++ ) {
				inData[j] = *( pTop + i * 3 + j );
			}

			intermediateData[0] = ( inData[0] & 0xFC ) >> 2;
			intermediateData[1] = ( ( inData[0] & 0x03 ) << 4 ) | ( ( inData[1] & 0xF0 ) >> 4 );
			intermediateData[2] = ( ( inData[1] & 0x0F ) << 2 ) | ( ( inData[2] & 0xC0 ) >> 6 );
			intermediateData[3] = inData[2] & 0x3F ;

			switch ( readSize ) {
				case 1:
					target.push_back( table[intermediateData[0]] );
					target.push_back( table[intermediateData[1]] );
					target.push_back( symbol_at_padding );
					target.push_back( symbol_at_padding );
					break;
				case 2:
					target.push_back( table[intermediateData[0]] );
					target.push_back( table[intermediateData[1]] );
					target.push_back( table[intermediateData[2]] );
					target.push_back( symbol_at_padding );
					break;
				case 3:
					target.push_back( table[intermediateData[0]] );
					target.push_back( table[intermediateData[1]] );
					target.push_back( table[intermediateData[2]] );
					target.push_back( table[intermediateData[3]] );
					break;
			}
		}
		return target;
	}

	static String Encode( std::string s ) {
		return Encode( s.c_str( ), s.length( ) * sizeof( char ) );
	}

	static String Encode( std::wstring s ) {
		return Encode( s.c_str( ), s.length( ) * sizeof(wchar_t));
	}

	static size_t Decode( VectorData* pDecoded, const String base64 ) {

		if ( pDecoded == nullptr )return 0;
		if ( base64.empty( ) ) return 0;

		size_t base64Len = base64.length( );
		size_t NumberOfBlocks = base64Len / 4;
		size_t FinalRestData = base64Len % 4;
		if ( FinalRestData != 0 ) NumberOfBlocks++;

		const String table = GetBase64CharTable( );

		uint8_t inData[4],intermediateData[3];
		size_t readSize;

		pDecoded->clear( );

		for ( size_t i = 0; i < NumberOfBlocks; i++ ) {

			inData[0] = 0;
			inData[1] = 0;
			inData[2] = 0;
			inData[3] = 0;

			readSize = 4;
			if ( ( i + 1 ) == NumberOfBlocks ) {
				if ( FinalRestData != 0 ) readSize = FinalRestData;
			}

			for ( size_t j = 0; j < readSize; j++ ) {
				inData[j] = base64[i * 4 + j] ;
				inData[j] = ( inData[j] == symbol_at_padding ) ? 0 : static_cast<uint8_t>(table.find( inData[j] ));
				if ( inData[j] == table.npos ) return 0;
			}

			intermediateData[0] = ( inData[0] << 2 ) | ( ( inData[1] & 0x30 ) >> 4 );
			intermediateData[1] = ( ( inData[1] & 0x0F) << 4 ) | ( ( inData[2] & 0x3C ) >> 2 );
			intermediateData[2] = ( ( inData[2] & 0x03 ) << 6 ) | inData[3];

			switch ( readSize ) {
				case 1:
					pDecoded->push_back( intermediateData[0] );
					break;
				case 2:
					pDecoded->push_back( intermediateData[0] );
					pDecoded->push_back( intermediateData[1] );
					break;
				case 3:
					pDecoded->push_back( intermediateData[0] );
					pDecoded->push_back( intermediateData[1] );
					pDecoded->push_back( intermediateData[2] );
					break;
				case 4:
					pDecoded->push_back( intermediateData[0] );
					pDecoded->push_back( intermediateData[1] );
					pDecoded->push_back( intermediateData[2] );
					break;
			}
		}

		return pDecoded->size();
	}

};

template <typename CharT> using CHSBase64Normal = CHSBase64T<CharT, '+', '/', '='>;
template <typename CharT> using CHSBase64MusicBrainz = CHSBase64T<CharT, '.', '_', '-'>;