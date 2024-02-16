#pragma once
#include <Windows.h>
#include <cstdio>


class CHSSCSIGeneralBuffer {

private:
	void* pBuffer;

	size_t bufferSize;

	void LocalInit( void );

public:
		CHSSCSIGeneralBuffer( );
		~CHSSCSIGeneralBuffer( );


		bool isReady( void );

		bool alloc( size_t size );
		bool realloc( size_t  size );

		bool free( void );

		bool prepare( size_t size );

		void* getBuffer( void ) const;

		template <typename T> T getBufferType( void ) const {
			return reinterpret_cast<T>( this->getBuffer( ) );
		}

};