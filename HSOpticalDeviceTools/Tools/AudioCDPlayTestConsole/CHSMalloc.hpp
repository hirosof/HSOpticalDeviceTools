#pragma once

#include <unordered_set>

template <typename T> class CHSMalloc { 
public:
	using TPointer = T*;
	using TBase = T;

private:
	//解放忘れを防ぐために確保したバッファのポインタを保存する
	std::unordered_set<void*>  AllocSet;
	HANDLE hHeap;
public:
	
	CHSMalloc ( ) {
		hHeap = GetProcessHeap ( );
	}

	~CHSMalloc ( ){
		this->AllFree ( );
	}

	bool IsManaged ( TPointer pval ) {
		return ( AllocSet.find ( pval ) != AllocSet.end ( ) );
	}

	TPointer Alloc ( size_t NumberOfElements ) {
		if ( NumberOfElements == 0 ) return nullptr;
		size_t s = sizeof ( T ) * NumberOfElements;
		void*  pval = HeapAlloc ( hHeap , HEAP_ZERO_MEMORY , s );
		if ( pval != nullptr ) AllocSet.insert ( pval );
		return static_cast< TPointer >( pval );
	}


	bool ReAlloc ( TPointer *ppval , size_t  NumberOfNewElements ) {
		if ( ppval == nullptr ) return false;
		if ( this->IsManaged(*ppval) ) {
			size_t s = sizeof ( T ) * NumberOfNewElements;
			void *pret = HeapReAlloc ( hHeap , HEAP_ZERO_MEMORY , *ppval , s );
			if ( pret ) {
				if ( pret != *ppval ) {
					this->AllocSet.erase (*ppval );
					this->AllocSet.insert ( pret );
					*ppval = pret;
				}
				return true;
			}
		}
		return false;
	}
	
	size_t GetSize ( TPointer pval ) {
		if ( this->IsManaged ( pval ) ) {
			return HeapSize ( hHeap , NULL , pval );
		}
		return 0;
	}

	bool Free ( TPointer pval ) {
		if ( this->IsManaged ( pval ) ) {
			BOOL bRet = HeapFree ( hHeap , NULL , pval );
			if ( bRet ) {
				this->AllocSet.erase ( pval );
				return true;
			}
		}
		return false;
	}


	bool AllFree ( void ) {
		auto bufSet = this->AllocSet;
		bool bAllFreeSucceed = true;
		if ( bufSet.size ( ) > 0 ) {
			for ( auto it = bufSet.begin ( ); it != bufSet.end ( ); it++ ) {
				if ( this->Free ( static_cast< TPointer >( *it ) ) == false ) {
					bAllFreeSucceed = false;
				}
			}
		}
		return bAllFreeSucceed;
	}

};

template <typename T> class CHSMallocFromPointerType {
};

template <typename T> class CHSMallocFromPointerType<T*> : public CHSMalloc<T>{};
