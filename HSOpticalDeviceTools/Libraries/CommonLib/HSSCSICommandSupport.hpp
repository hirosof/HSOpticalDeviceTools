#pragma once

#include "HSSCSICommandDefine.hpp"

EHSSCSIStatusCode  HSSCSIStatusToStatusCode(const UHSSCSIStatus status );
bool  HSSCSI_InitializeCommandData( THSSCSI_CommandData *pData );

template <typename T> uint16_t HSSCSI_InverseEndian16( T value ) {
	uint16_t out = 0;
	uint8_t* pByteIn = reinterpret_cast<uint8_t*>( &value );
	uint8_t* pByteOut = reinterpret_cast<uint8_t*>( &out );

	size_t size = sizeof( out ) / sizeof( uint8_t );

	for ( size_t i = 0; i < size; i++ ) {
		*( pByteOut + i ) = *( pByteIn + ( size - 1 - i ));
	}

	return out;
}

template <typename T> uint16_t HSSCSI_InverseEndian16Pointer( T* pvalue ) {
	if ( pvalue == nullptr ) return 0;
	return HSSCSI_InverseEndian16( *reinterpret_cast<uint16_t*>( pvalue ) );
}

template <typename T> uint32_t HSSCSI_InverseEndian32( T value ) {
	uint32_t out = 0;
	uint8_t* pByteIn = reinterpret_cast<uint8_t*>( &value );
	uint8_t* pByteOut = reinterpret_cast<uint8_t*>( &out );

	size_t size = sizeof( out ) / sizeof( uint8_t );

	for ( size_t i = 0; i < size; i++ ) {
		*( pByteOut + i ) = *( pByteIn + ( size - 1 - i ) );
	}

	return out;

}

template <typename T> uint32_t HSSCSI_InverseEndian32Pointer( T* pvalue ) {
	if ( pvalue == nullptr ) return 0;
	return HSSCSI_InverseEndian32( *reinterpret_cast<uint32_t*>( pvalue ) );
}

template <typename T> uint64_t HSSCSI_InverseEndian64( T value ) {
	uint64_t out = 0;
	uint8_t* pByteIn = reinterpret_cast<uint8_t*>( &value );
	uint8_t* pByteOut = reinterpret_cast<uint8_t*>( &out );

	size_t size = sizeof( out ) / sizeof( uint8_t );

	for ( size_t i = 0; i < size; i++ ) {
		*( pByteOut + i ) = *( pByteIn + ( size - 1 - i ) );
	}

	return out;

}

template <typename T> uint64_t HSSCSI_InverseEndian64Pointer( T* pvalue ) {
	if ( pvalue == nullptr ) return 0;
	return HSSCSI_InverseEndian64( *reinterpret_cast<uint64_t*>( pvalue ) );
}


std::string HSSCSI_GetConnectInterfaceNameStringByName(const EHSSCSI_ConnectInterfaceName name );