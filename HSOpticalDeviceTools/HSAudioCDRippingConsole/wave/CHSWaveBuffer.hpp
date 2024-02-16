//2016/12/13
//2016/12/14
//2016/12/15


#pragma once
#include <Windows.h>
#include <stdint.h>
#include <stdexcept>

const uint8_t HSWAVEBUFFER_DEFAULT_CHANNELS = 1;

template <typename T> class CHSWaveBuffer {
private:
	enum struct eMode {
		None = 0 ,
		Allocated ,
		Attached
	};

	eMode m_Mode;
	T *pm_Data;
	uint8_t m_NumberOfChannels;
	uint32_t m_NumberOfSamples;

	void LocalInit( void ) {
		this->pm_Data = nullptr;
		this->m_Mode = eMode::None;
		this->m_NumberOfSamples = 0;
		this->m_NumberOfChannels = 0;
	}


	uint32_t CalcBufferIndex ( uint8_t ChannelIndex , uint32_t SampleIndex ) {
		return SampleIndex * this->m_NumberOfChannels + ChannelIndex;
	}

public:

	CHSWaveBuffer ( ) {
		this->LocalInit ( );
	}


	CHSWaveBuffer ( uint32_t NumberOfSamples , uint8_t NumberOfChannels = HSWAVEBUFFER_DEFAULT_CHANNELS) {
		this->LocalInit ( );
	}

	~CHSWaveBuffer ( ){
		this->Free ( );
	}

	bool IsReady ( void ) {
		return ( this->pm_Data != nullptr );
	}

	bool IsAllocatedReady ( void ) {
		return this->IsReady ( ) && ( m_Mode == eMode::Allocated );
	}
	bool IsAttachedReady ( void ) {
		return this->IsReady ( ) && ( m_Mode == eMode::Attached );
	}


	bool Alloc ( uint32_t NumberOfSamples , uint8_t NumberOfChannels = HSWAVEBUFFER_DEFAULT_CHANNELS ) {
		if ( NumberOfSamples == 0 ) return false;
		if ( NumberOfChannels == 0 ) return false;
		if ( this->IsReady ( ) ) return false;
		this->pm_Data = reinterpret_cast< T* >( HeapAlloc ( GetProcessHeap ( ) ,
			HEAP_ZERO_MEMORY , sizeof ( T ) *  NumberOfSamples * NumberOfChannels ) );
		if ( this->pm_Data != nullptr ) {
			this->m_NumberOfChannels = NumberOfChannels;
			this->m_NumberOfSamples = NumberOfSamples;
			this->m_Mode = eMode::Allocated;
			return true;
		}
		return false;
	}

	bool Attach ( T *lpBuffer , uint32_t NumberOfSamples , uint8_t NumberOfChannels = HSWAVEBUFFER_DEFAULT_CHANNELS ) {
		if ( NumberOfSamples == 0 ) return false;
		if ( NumberOfChannels == 0 ) return false;
		if ( this->IsReady ( ) ) return false;
		this->pm_Data = lpBuffer;
		if ( this->pm_Data!=nullptr ) {
			this->m_NumberOfChannels = NumberOfChannels;
			this->m_NumberOfSamples = NumberOfSamples;
			this->m_Mode = eMode::Attached;
			return true;
		}
		return false;
	}


	bool ReAlloc ( uint32_t NumberOfNewSamples , uint8_t NumberOfNewChannels = HSWAVEBUFFER_DEFAULT_CHANNELS ) {
		if ( NumberOfNewChannels == 0 ) return false;
		if ( NumberOfNewSamples == 0 ) return false;
		if ( this->IsAllocatedReady ( ) ) {
			void *pnewbuf;
			
			if ( this->CountElements ( ) == ( NumberOfNewChannels * NumberOfNewSamples ) ) {
				pnewbuf = reinterpret_cast<void*>(this->pm_Data);
			} else {
				pnewbuf = HeapReAlloc ( GetProcessHeap ( ) , HEAP_ZERO_MEMORY ,
					this->pm_Data , sizeof ( T ) *  NumberOfNewSamples*NumberOfNewChannels );
			}
			if ( pnewbuf != nullptr ) {
				this->m_NumberOfChannels = NumberOfNewChannels;
				this->m_NumberOfSamples = NumberOfNewSamples;
				this->pm_Data = reinterpret_cast< T* >( pnewbuf );
				return true;
			}
		}
		return false;
	}

	bool ReAttach ( T *pNewBuffer , T **ppOldBuffer , uint32_t NumberOfNewSamples , uint8_t NumberOfNewChannels = HSWAVEBUFFER_DEFAULT_CHANNELS ) {
		if ( NumberOfNewChannels == 0 ) return false;
		if ( NumberOfNewSamples == 0 ) return false;
		if ( pNewBuffer == nullptr ) return false;

		if ( this->IsAttachedReady ( ) ) {
			if ( ppOldBuffer != nullptr ) *ppOldBuffer = this->pm_Data;
			this->pm_Data = pNewBuffer;
			this->m_NumberOfChannels = NumberOfNewChannels;
			this->m_NumberOfSamples = NumberOfNewSamples;
			return true;
		}
		return false;
	}
	bool ReAttach ( T *pNewBuffer , uint32_t NumberOfNewSamples , uint8_t NumberOfNewChannels = HSWAVEBUFFER_DEFAULT_CHANNELS ) {
		return this->ReAttach ( pNewBuffer , nullptr , NumberOfNewSamples , NumberOfNewChannels );
	}


	bool Free ( void ) {
		if ( this->IsAllocatedReady ( ) ) {
			if ( HeapFree ( GetProcessHeap ( ) , NULL , this->pm_Data ) ) {
				this->LocalInit ( );
				return true;
			}
		}
		return false;
	}


	bool Detach ( T **ppBuffer = nullptr ) {
		if ( this->IsAttachedReady ( ) ) {
			if ( ppBuffer ) *ppBuffer = this->pm_Data;
			this->LocalInit ( );
			return true;
		}
		return false;
	}


	uint32_t CountElements ( void ) {
		if ( this->IsReady ( ) ) {
			return this->m_NumberOfChannels * this->m_NumberOfSamples;
		}
		return 0;
	}

	uint8_t CountChannels ( void ) {
		if ( this->IsReady ( ) ) {
			return this->m_NumberOfChannels;
		}
		return 0;
	}

	uint32_t CountSamples ( void ) {
		if ( this->IsReady ( ) ) {
			return this->m_NumberOfSamples;
		}
		return 0;
	}

	uint32_t GetBufferSizeBytes ( void ) {
		return static_cast< uint32_t >( this->CountElements ( ) * sizeof ( T ) );
	}

	bool SetBufferValue ( T Value , uint8_t ChannelIndex , uint32_t SampleIndex ) {
		T *pLocal = this->GetBufferPointer ( ChannelIndex , SampleIndex );
		if(pLocal !=nullptr ){
			*pLocal = Value;
			return true;
		}
		return false;
	}

	bool GetBufferValue ( T *pValue , uint8_t ChannelIndex , uint32_t SampleIndex ) {
		if ( pValue == nullptr ) return false;
		T *pLocal = this->GetBufferPointer ( ChannelIndex , SampleIndex );
		if(pLocal !=nullptr ){
			*pValue = *pLocal;
			return true;
		}
		return false;
	}


	bool SetElementValue ( T Value , uint32_t Index ) {
		T *pLocal = this->GetElementPointer ( Index );
		if ( pLocal != nullptr ) {
			*pLocal = Value;
			return true;
		}
	}
	bool GetElementValue ( T *pValue , uint32_t Index ) {
		if ( pValue == nullptr ) return false;
		T *pLocal = this->GetElementPointer ( Index );
		if(pLocal !=nullptr ){
			*pLocal = *pValue;
			return true;
		}
		return true;
	}



	T& GetBufferReference ( uint8_t ChannelIndex , uint32_t SampleIndex ) {
		T *pLocal = this->GetBufferPointer ( ChannelIndex , SampleIndex );
		if ( pLocal == nullptr ) throw std::runtime_error ("");
		return *pLocal;
	}
	T& GetElementReference ( uint8_t Index ) {
		T *pLocal = this->GetElementPointer ( Index );
		if ( pLocal == nullptr ) throw std::runtime_error ("");
		return *pLocal;		
	}

	T* GetBufferPointer ( uint8_t ChannelIndex , uint32_t SampleIndex ) {
		if ( ChannelIndex >= this->m_NumberOfChannels ) return nullptr;
		if ( SampleIndex >= this->m_NumberOfSamples ) return nullptr;
		if ( this->IsReady ( ) ) {
			uint32_t index = this->CalcBufferIndex ( ChannelIndex , SampleIndex );
			return this->pm_Data + index;
		}
		return nullptr;
	}
	T* GetElementPointer ( uint8_t Index ) {
		if ( this->IsReady ( ) ) {
			if ( Index < this->CountElements ( ) ) {
				return this->pm_Data + Index;
			}
		}
		return nullptr;
	}

	T& operator[] ( uint32_t idx ) {
		return this->GetElementReference ( idx );
	}

	T& operator()( uint8_t ChannelIndex , uint32_t SampleIndex ) {
		return this->GetBufferReference ( ChannelIndex , SampleIndex );
	}

};


