#include "CHSSCSIGeneralBuffer.hpp"

void CHSSCSIGeneralBuffer::LocalInit( void ) {
	this->pBuffer = nullptr;
	this->bufferSize = 0;

}

CHSSCSIGeneralBuffer::CHSSCSIGeneralBuffer( ) {
	this->LocalInit( );
}

CHSSCSIGeneralBuffer::~CHSSCSIGeneralBuffer( ) {
	this->free( );
}

bool CHSSCSIGeneralBuffer::isReady( void ) {
	return ( this->pBuffer != nullptr );
}

bool CHSSCSIGeneralBuffer::alloc( size_t size ) {
	if ( this->isReady( ) ) return false;
	this->pBuffer = HeapAlloc( GetProcessHeap( ),HEAP_ZERO_MEMORY, size);

	if ( this->pBuffer != nullptr ) {
		this->bufferSize = size;
		return true;
	}

	return false;
}

bool CHSSCSIGeneralBuffer::realloc( size_t size ) {
	if ( this->isReady( ) ) {
		void* pnewbuf = HeapReAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, this->pBuffer, size );
		if ( pnewbuf ) {
			this->bufferSize = size;
			this->pBuffer = pnewbuf;
			return true;
		}
	}
	return false;
}

bool CHSSCSIGeneralBuffer::free( void ) {
	if ( this->isReady( ) ) {
		if ( HeapFree( GetProcessHeap( ), NULL, this->pBuffer ) ) {
			this->LocalInit( );
			return true;
		}
	}
	return false;
}

bool CHSSCSIGeneralBuffer::prepare( size_t size ) {
	if ( this->pBuffer ) {
		return this->realloc( size );
	} else {
		return this->alloc( size );
	}
}

void* CHSSCSIGeneralBuffer::getBuffer( void ) const {
	return this->pBuffer;
}


