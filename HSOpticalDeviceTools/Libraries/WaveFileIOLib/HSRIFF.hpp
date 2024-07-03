
/*


	2016/12/07
	2016/12/08
	2016/12/09


*/
#pragma once
#include <stdint.h>
#include <Windows.h>

#include <vector>
#include <map>
#include <string>
#include <Shlwapi.h>
#include <typeinfo>
#pragma comment(lib,"Shlwapi.lib")

/*
	INFO型LISTチャンクメンバーの識別子定義部
*/
#define HSRIFF_FOURCC_LIST_INFO_ISRC "ISRC" //メディア元
#define HSRIFF_FOURCC_LIST_INFO_ICRD "ICRD" //作成日
#define HSRIFF_FOURCC_LIST_INFO_IPRD "IPRD" //アルバム名
#define HSRIFF_FOURCC_LIST_INFO_INAM "INAM" //曲名
#define HSRIFF_FOURCC_LIST_INFO_IGNR "IGNR" //ジャンル
#define HSRIFF_FOURCC_LIST_INFO_ICMT "ICMT" //コメント
#define HSRIFF_FOURCC_LIST_INFO_IART "IART" //アーティスト
#define HSRIFF_FOURCC_LIST_INFO_ICMS "ICMS" //著作権代理人
#define HSRIFF_FOURCC_LIST_INFO_ICOP "ICOP" //著作権情報
#define HSRIFF_FOURCC_LIST_INFO_IENG "IENG" //録音エンジニア
#define HSRIFF_FOURCC_LIST_INFO_ISFT "ISFT" //ソフトウェア
#define HSRIFF_FOURCC_LIST_INFO_IKEY "IKEY" //キーワード
#define HSRIFF_FOURCC_LIST_INFO_ITCH "ITCH" //エンコード技術者
#define HSRIFF_FOURCC_LIST_INFO_ISBJ "ISBJ" //タイトル
#define HSRIFF_FOURCC_LIST_INFO_ITRK "ITRK" //トラック番号

/*
	グローバル変数(定数)定義部
*/

const int HSRIFF_FIRSTSUBCHUNK_OFFSET = 12;

const uint32_t HSRIFF_MAGICNUMBER = 0x46464952; // "RIFF"を示す
const uint32_t HSRIFF_MAGICNUMBER_LISTCHUNK = 0x5453494c; // "LIST"を示す
const uint32_t HSRIFF_MAGICNUMBER_DATACHUNK = 0x61746164; // "data"を示す
const uint32_t HSRIFF_MAGICNUMBER_FMTCHUNK = 0x20746d66; // "fmt "を示す

struct THSRiffChunkHeader {
	char Name [ 4 ];	//チャンク名
	uint32_t DataSize;	//チャンクデータサイズ
};

struct THSRiffChunkInfo {
	THSRiffChunkHeader Header;
	uint32_t Position;	//チャンクの位置
	char Type [ 4 ];	//種類(LISTチャンクのみ使用)
};


//チャンクテーブル
using CHSRiffChunkTable = std::vector<THSRiffChunkInfo>;
using CHSListChunkTable = std::map<std::string , CHSRiffChunkTable>;


bool HSMakeRiffChunkFourCC(const char *lpChunkBaseName , std::string *lpChunkFourCC );
int32_t HSScanRiffChunkTable (const char *lpChunkName , CHSRiffChunkTable *pTable );

template <typename Type> class __CHSRiffReaderBaseTemplate {
protected:
	using T = Type;
	virtual bool AdditionalCheckProcess ( void ) {
		return true;
	}


protected:
	HANDLE hFile;
	uint32_t FileSize;
	T szFilePath [ MAX_PATH ];
	char FileType [ 4 ];


	CHSRiffChunkTable m_ChunkTable;
	CHSListChunkTable m_ListChunkTable;

	uint32_t rawRead ( void *lpBuffer , uint32_t ReadPosition , uint32_t ReadSize ) {
		LARGE_INTEGER li;
		li.QuadPart = ReadPosition;

		if ( SetFilePointerEx ( this->hFile , li , NULL , FILE_BEGIN ) ) {
			DWORD loaded_size;
			if ( ReadFile ( this->hFile , lpBuffer , ReadSize , &loaded_size , NULL ) ) {
				return loaded_size;
			}
		}
		return 0;
	}

	bool CheckProcess ( void ) {
		uint32_t magicnumber;
		uint32_t headersize;
		LARGE_INTEGER li;
		if ( this->rawRead ( &magicnumber , 0 , 4 ) == 4 ) {
			if ( magicnumber == HSRIFF_MAGICNUMBER ) {
			//	printf ( "Magic Number OK\n" );
				if ( this->rawRead ( &headersize , 4 , 4 ) == 4 ) {
					if ( GetFileSizeEx ( this->hFile , &li ) ) {
						if ( ( headersize + 8 ) == li.QuadPart ) {
						//	printf ( "Size Check OK\n" );
							this->FileSize = headersize + 8;
							if ( this->rawRead ( this->FileType , 8 , 4 ) == 4 ) {
								return true;
							}
						}
					}
				}
			}
		}
		return false;
	}



	bool MakeChunkTableInner ( uint32_t offset , CHSRiffChunkTable  *pMember , uint32_t LimitPos ) {

		THSRiffChunkInfo info;

		uint32_t pos = offset;
		uint32_t magic_number;
		bool bRet = true;

		do {
			if ( this->rawRead ( &info.Header , pos , 8 ) != 8 ) {
				bRet = false;
				break;
			}
			info.Position = pos;
			magic_number = *reinterpret_cast< uint32_t* >( info.Header.Name );
			if ( magic_number == HSRIFF_MAGICNUMBER_LISTCHUNK ) {
				if ( this->rawRead ( info.Type , pos + 8 , 4 ) == 4 ) {
					pMember->push_back ( info );

					char type [ 5 ];
					type [ 4 ] = 0;
					*reinterpret_cast< uint32_t* >( type ) = *reinterpret_cast< uint32_t* >( info.Type );

					CHSRiffChunkTable &ListMember = m_ListChunkTable [ type ];
					uint32_t dataoffset = info.Position + 8;

					if ( MakeChunkTableInner ( dataoffset + 4 , &ListMember , dataoffset + info.Header.DataSize - 1 ) == false ) {
						bRet = false;
						break;
					}
				} else {
					bRet = false;
					break;
				}
			} else {
				*reinterpret_cast< uint32_t* >( info.Type ) = 0;
				pMember->push_back ( info );
			}


			pos = info.Position + info.Header.DataSize + 8;
			if ( info.Header.DataSize % 2 ) pos++;

		} while ( pos <= LimitPos );
		return bRet;
	}

	bool MakeChunkTable ( void ) {
		m_ChunkTable.clear ( );
		m_ListChunkTable.clear ( );
		return MakeChunkTableInner ( HSRIFF_FIRSTSUBCHUNK_OFFSET ,
			&m_ChunkTable , this->FileSize - 1 );
	}

public:

	__CHSRiffReaderBaseTemplate ( ) {
		this->hFile = NULL;
	}

	__CHSRiffReaderBaseTemplate ( T *lpszPath ) {
		this->hFile = NULL;
		this->Open ( lpszPath );
	}

	~__CHSRiffReaderBaseTemplate ( ) {
		this->Close ( );
	}

	bool Open (const T *lpszFilePath ) {
		if ( this->OpenProcess ( lpszFilePath ) ) {
			//printf ( "Opened File\n" );
			if ( this->CheckProcess ( ) ) {
				//printf ( "Checked File\n" );
				if ( this->MakeChunkTable ( ) ) {
				//	printf ( "Made Chunk Table\n" );

					if ( this->AdditionalCheckProcess ( ) ) {
					//	printf ( "Additional Checked\n" );

						return true;
					}
				}
			}
			this->Close ( );
		}
		
		return false;
	}


	bool IsOpened ( void ) {
		return ( this->hFile != NULL );
	}

	bool CheckType ( char *lpType ) {
		std::string base;
		if ( HSMakeRiffChunkFourCC ( lpType , &base ) ) {
			uint32_t typeint = *reinterpret_cast< const uint32_t* >( base.c_str ( ) );
			return ( typeint == *reinterpret_cast< uint32_t* >( this->FileType ) );
		}
		return false;
	}

	bool Close ( void ) {
		if ( this->IsOpened ( ) ) {
			if ( CloseHandle ( this->hFile ) ) {
				this->hFile = NULL;
				return true;
			}
		}
		return false;
	}

	bool GetChunkTable ( CHSRiffChunkTable *pTable ) {
		if ( pTable != nullptr ) {
			*pTable = this->m_ChunkTable;
			return true;
		}
		return false;
	}

	bool GetChunkInfo ( char *lpChunkName , THSRiffChunkInfo *pInfo ) {
		if ( pInfo == nullptr ) return false;
		int pos = HSScanRiffChunkTable ( lpChunkName , &this->m_ChunkTable );
		if ( pos >= 0 ) {
			*pInfo = this->m_ChunkTable [ pos ];
			return true;
		}

		return false;
	}


	bool HasChunk ( char *lpChunkName ) {
		THSRiffChunkInfo info;
		return this->GetChunkInfo(lpChunkName , &info);
	}

	uint32_t GetChunkSize ( char *lpChunkName ) {
		THSRiffChunkInfo info;
		if ( this->GetChunkInfo ( lpChunkName , &info ) ) {
			return info.Header.DataSize;
		}
		return 0;
	}

	uint32_t GetChunkPos ( char *lpChunkName ) {
		THSRiffChunkInfo info;
		if ( this->GetChunkInfo ( lpChunkName , &info ) ) {
			return info.Position;
		}
		return 0;
	}


	uint32_t ReadChunkData ( char *lpChunkName , void *lpData, uint32_t offset  , uint32_t readsize ) {
		if ( lpData == nullptr ) return 0;
		THSRiffChunkInfo info;
		if ( this->GetChunkInfo ( lpChunkName , &info ) ) {
			uint32_t datapos = info.Position + 8;
			if ( offset < info.Header.DataSize ) {
				uint32_t pos = datapos + offset;
				uint32_t len = info.Header.DataSize - offset;
				if ( len > readsize ) len = readsize;
				return this->rawRead ( lpData , pos , len );
			}
		}
		return 0;
	}

	bool GetListChunkMemberTable ( char *lpType , CHSRiffChunkTable *pTable ) {
		std::string idStr;
		if ( HSMakeRiffChunkFourCC ( lpType , &idStr ) ) {
			auto it = this->m_ListChunkTable.find ( idStr );
			if ( it != this->m_ListChunkTable.end ( ) ) {
				*pTable = it->second;
				return true;
			}
		}
		return false;
	}

	bool HasListChunk ( char *lpType ) {
		CHSRiffChunkTable ct;
		return this->GetListChunkMemberTable ( lpType , &ct );
	}

	bool HasListChunkMember ( char *lpType , char *lpMemberChunkName ) {
		THSRiffChunkInfo *pInfo;
		return this->GetListChunkMemberInfo(lpType , lpMemberChunkName , &pInfo);
	}

	bool GetListChunkMemberInfo ( char *lpTypeName , char *lpChunkName , THSRiffChunkInfo *pInfo ) {
		if ( pInfo == nullptr ) return false;
		CHSRiffChunkTable table;
		if ( this->GetListChunkMemberTable ( lpTypeName , &table ) ) {
			int idx = HSScanRiffChunkTable ( lpChunkName , &table );
			if ( idx >= 0 ) {
				*pInfo = table [ idx ];
				return true;
			}
		}
		return false;
	}


	uint32_t GetListChunkMemberSize ( char *lpTypeName , char *lpChunkName ) {
		THSRiffChunkInfo info;
		if ( this->GetListChunkMemberInfo ( lpTypeName , lpChunkName , &info ) ) {
			return info.Header.DataSize;
		}
		return 0;
	}

	uint32_t GetListChunkMemberPos ( char *lpTypeName , char *lpChunkName ) {
		THSRiffChunkInfo info;
		if ( this->GetListChunkMemberInfo ( lpTypeName , lpChunkName , &info ) ) {
			return info.Position;
		}
		return 0;
	}


	uint32_t ReadListChunkMemberData (char *lpTypeName ,  char *lpChunkName  , void *lpData,uint32_t offset  , uint32_t readsize ) {
		if ( lpData == nullptr ) return 0;
		THSRiffChunkInfo info;
		if ( this->GetListChunkMemberInfo ( lpTypeName , lpChunkName , &info ) ) {
			uint32_t datapos = info.Position + 8;
			if ( offset < info.Header.DataSize ) {
				uint32_t pos = datapos + offset;
				uint32_t len = info.Header.DataSize - offset;
				if ( len > readsize ) len = readsize;
				return this->rawRead ( lpData , pos , len );
			}
		}
		return 0;

	}

	uint32_t CustomRead ( void *lpData , uint32_t Position , uint32_t ReadSize ) {
		if ( lpData == nullptr ) return 0;
		if ( Position >= this->FileSize ) return 0;
		return this->rawRead ( lpData , Position , ReadSize );
	}

protected:

	virtual bool OpenProcess (const T *lpszFileTargetPath ) {
		if ( lpszFileTargetPath == nullptr ) {
			return false;
		}

		if ( this->IsOpened ( ) ) {
			return false;
		}

		memset ( this->szFilePath , 0 , sizeof ( this->szFilePath ) );

		if ( typeid( T ) == typeid( char ) ) {

			char *lpszFilePath = (char*)lpszFileTargetPath;
			char *lpszInnerFilePath = ( char* )this->szFilePath;

			if ( PathFileExistsA ( lpszFilePath ) == FALSE ) {
				return false;
			}

			HANDLE hNextFile;

			hNextFile = CreateFileA ( lpszFilePath , GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
			if ( hNextFile == INVALID_HANDLE_VALUE ) {
				return false;
			}

			this->hFile = hNextFile;

			if ( PathIsRelativeA ( lpszFilePath ) ) {
				if ( GetFullPathNameA ( lpszFilePath , MAX_PATH , lpszInnerFilePath , NULL ) == 0 ) {
					lstrcpynA ( lpszInnerFilePath , lpszFilePath , MAX_PATH );
				}
			} else {
				lstrcpynA ( lpszInnerFilePath , lpszFilePath , MAX_PATH );
			}

			return true;


		} else if ( typeid( T ) == typeid( wchar_t ) ) {
			wchar_t *lpszFilePath = (wchar_t*)lpszFileTargetPath;
			wchar_t *lpszInnerFilePath = ( wchar_t* )this->szFilePath;
			if ( PathFileExistsW ( lpszFilePath ) == FALSE ) {
				return false;
			}

			HANDLE hNextFile;

			hNextFile = CreateFileW ( lpszFilePath , GENERIC_READ , FILE_SHARE_READ , NULL , OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL );
			if ( hNextFile == INVALID_HANDLE_VALUE ) {
				return false;
			}

			this->hFile = hNextFile;

			if ( PathIsRelativeW ( lpszFilePath ) ) {
				if ( GetFullPathNameW ( lpszFilePath , MAX_PATH , lpszInnerFilePath , NULL ) == 0 ) {
					lstrcpynW ( lpszInnerFilePath , lpszFilePath , MAX_PATH );
				}
			} else {
				lstrcpynW ( lpszInnerFilePath , lpszFilePath , MAX_PATH );
			}

			return true;
		}

		return false;
	}

public:
	virtual uint32_t GetOpenedFilePath ( T *lpszFileTargetPath , size_t size ) {
		if ( typeid( T ) == typeid( char ) ) {

			char *lpszFilePath = (char*)lpszFileTargetPath;
			char *lpszInnerFilePath = ( char* )this->szFilePath;

			if ( ( size == 0 ) || ( lpszFileTargetPath == nullptr ) ) {
				return lstrlenA ( lpszInnerFilePath ) + 1;
			}
			lstrcpynA ( lpszFilePath , lpszInnerFilePath , size );
			return lstrlenA ( lpszFilePath ) + 1;

		} else if ( typeid( T ) == typeid( wchar_t ) ) {
			wchar_t *lpszFilePath = (wchar_t*)lpszFileTargetPath;
			wchar_t *lpszInnerFilePath = ( wchar_t* )this->szFilePath;
			if ( ( size == 0 ) || ( lpszFileTargetPath == nullptr ) ) {
				return lstrlenW ( lpszInnerFilePath ) + 1;
			}
			lstrcpynW ( lpszFilePath , lpszInnerFilePath , size );
			return lstrlenW ( lpszFilePath ) + 1;
		}
		return 0;
	}


};


using CHSRiffReaderA = __CHSRiffReaderBaseTemplate<char>;
using CHSRiffReaderW = __CHSRiffReaderBaseTemplate<wchar_t>;


template <typename Type> class __CHSRiffWriterBaseTemplate {
protected:
	using T = Type;
	enum struct WriteMode {
		None = 0 ,
		ChunkWrite ,
		ListChunkWrite ,
		ListMemberChunkWrite
	};

	virtual bool CreateAdditionalProcess ( void ) { return true; }


protected:
	HANDLE hFile;
	WriteMode  Mode;
	T szFilePath [ MAX_PATH ];
	THSRiffChunkInfo m_ChunkInfo;
	THSRiffChunkInfo m_ListMemberChunkInfo;


	bool InitializeFile ( void ) {

		LARGE_INTEGER li;
		li.QuadPart = 0;
		if ( SetFilePointerEx ( this->hFile , li , NULL , FILE_BEGIN ) ) {

			if ( SetEndOfFile ( this->hFile ) ) {

				uint32_t blank [ 2 ] = { 0,0 };

				uint32_t root = HSRIFF_MAGICNUMBER;

				DWORD wrote_size;

				BOOL bRet = WriteFile ( this->hFile , &root , 4 , &wrote_size , NULL );

				if ( ( bRet == TRUE ) && ( wrote_size == 4 ) ) {
					bRet = WriteFile ( this->hFile , blank , 8 , &wrote_size , NULL );
					if ( ( bRet == TRUE ) && ( wrote_size == 8 ) ) {
						return true;
					}
				}

			}

		}

		return false;
	}
public:

	__CHSRiffWriterBaseTemplate ( ) {
		hFile = NULL;
		this->Mode = WriteMode::None;
		m_ChunkInfo.Header.Name[0] = '\0';
		m_ListMemberChunkInfo.Header.Name[0] = '\0';
		szFilePath[0] = '\0';

	}

	~__CHSRiffWriterBaseTemplate ( ) {
		this->Close ( );
	}

	bool Create (const T *lpszFilePath ) {
		if ( this->CreateRiffFile ( lpszFilePath ) ) {
			this->Mode = WriteMode::None;
			if ( this->InitializeFile ( ) ) {
				if ( this->CreateAdditionalProcess ( ) ) {
					return true;
				}

			}
		}
		this->Close ( );

		return false;
	}


	bool WriteRiffChunkSize ( void ) {
		if ( this->IsCreated ( ) ) {

			LARGE_INTEGER pos , len;

			uint32_t data;

			pos.QuadPart = 4;

			if ( GetFileSizeEx ( this->hFile , &len ) ) {
				data = static_cast< uint32_t >( len.QuadPart - 8 );
				if ( SetFilePointerEx ( this->hFile , pos , NULL , FILE_BEGIN ) ) {
					DWORD wrotesize;

					if ( WriteFile ( this->hFile , &data , 4 , &wrotesize , NULL ) ) {

						if ( wrotesize == 4 ) {
							return true;
						}
					}
				}
			}
		}

		return false;
	}

	bool WriteRiffType (const char *lpszType ) {
		if ( this->IsCreated ( ) ) {
			LARGE_INTEGER pos;
			pos.QuadPart = 8;
			std::string data;
			if ( HSMakeRiffChunkFourCC ( lpszType , &data ) ) {
				if ( SetFilePointerEx ( this->hFile , pos , NULL , FILE_BEGIN ) ) {
					DWORD wrotesize;

					if ( WriteFile ( this->hFile , data.c_str ( ) , 4 , &wrotesize , NULL ) ) {
						if ( wrotesize == 4 ) {
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	bool Close (void ) {
		if ( this->IsCreated ( ) ) {

			if ( this->Mode == WriteMode::ListMemberChunkWrite ) this->EndListMemberChunk ( );
			if ( this->Mode == WriteMode::ListChunkWrite ) this->EndListChunk ( );
			if ( this->Mode == WriteMode::ChunkWrite ) this->EndChunk ( );

			if ( this->WriteRiffChunkSize ( ) ) {
				if ( CloseHandle ( this->hFile ) ) {
					this->hFile = NULL;
					return true;
				}
			}
		}
		return false;
	}

	bool IsCreated ( void ) {
		return ( this->hFile != NULL );
	}



	bool BeginChunk (const char *lpChunkName ) {
		if ( this->IsCreated ( ) ) {
			if ( this->Mode == WriteMode::None ) {
				LARGE_INTEGER zero , pos;
				zero.QuadPart = 0;
				if ( SetFilePointerEx ( this->hFile , zero , &pos , FILE_END ) ) {
					std::string cn;
					if ( HSMakeRiffChunkFourCC ( lpChunkName , &cn ) ) {
						DWORD wrote_size;
						BOOL bRet;
						bRet = WriteFile ( this->hFile , cn.c_str ( ) , 4 , &wrote_size , NULL );
						if ( ( bRet == TRUE ) && ( wrote_size == 4 ) ) {
							uint32_t blank = 0;
							bRet = WriteFile ( this->hFile , &blank , 4 , &wrote_size , NULL );
							if ( ( bRet == TRUE ) && ( wrote_size == 4 ) ) {
								this->Mode = WriteMode::ChunkWrite;
								memcpy ( this->m_ChunkInfo.Header.Name , cn.c_str ( ) , 4 );
								this->m_ChunkInfo.Header.DataSize = 0;
								this->m_ChunkInfo.Position = static_cast< uint32_t >( pos.QuadPart );					
								memset ( this->m_ChunkInfo.Type , 0 , 4 );
								return true;
							}
						}
					}
				}
			}
		}
		return false;
	}
	bool AdditionalChunkData (const void *lpData , uint32_t DataSize ) {
		if ( lpData == nullptr ) return false;
		if ( DataSize == 0 ) return false;
		if ( this->IsCreated ( ) ) {
			if ( this->Mode != WriteMode::None ) {
				LARGE_INTEGER zero;
				zero.QuadPart = 0;
				if ( SetFilePointerEx ( this->hFile , zero , NULL , FILE_END ) ) {
					DWORD wrote_size;
					BOOL bRet;
					bRet = WriteFile ( this->hFile ,lpData , DataSize , &wrote_size , NULL );
					if (bRet ) {
						this->m_ChunkInfo.Header.DataSize += wrote_size;
						if ( this->Mode == WriteMode::ListMemberChunkWrite ) {
							this->m_ListMemberChunkInfo.Header.DataSize += wrote_size;
						}
						return ( wrote_size == DataSize );
					}
				}
			}
		}
		return false;
	}

	bool AdditionalChunkStringData ( const char *lpszString , bool bWriteNullCharactor = true ) {

		if ( lpszString == nullptr ) return false;

		size_t len = lstrlenA ( lpszString );

		if ( bWriteNullCharactor ) len++;

		return this->AdditionalChunkData ( lpszString , static_cast<uint32_t>(len * sizeof(char)));

	}
	bool AdditionalChunkStringData ( const wchar_t *lpszString , bool bWriteNullCharactor = true ) {
		if ( lpszString == nullptr ) return false;
		size_t len = lstrlenW ( lpszString );
		if ( bWriteNullCharactor ) len++;
		return this->AdditionalChunkData ( lpszString ,static_cast<uint32_t>( len * sizeof(wchar_t)));
	}

	template <typename U> bool AdditionalChunkTypeData ( U typeData ) {
		return this->AdditionalChunkData ( &typeData , sizeof ( U ) );
	}

	

	bool EndChunk ( void ) {
		if ( this->IsCreated ( ) ) {
			if ( this->Mode == WriteMode::ChunkWrite ) {
				LARGE_INTEGER pos;
				pos.QuadPart = this->m_ChunkInfo.Position + 4;
				if ( SetFilePointerEx ( this->hFile , pos , NULL , FILE_BEGIN ) ) {
					DWORD wrote_size;
					BOOL bRet;
					bRet = WriteFile ( this->hFile , &this->m_ChunkInfo.Header.DataSize , 4 , &wrote_size , NULL );
					if ( ( bRet == TRUE)&&(wrote_size ==4)) {

						bool bReturn = false;

						if ( this->m_ChunkInfo.Header.DataSize % 2 ) {
							char padding = 0;
							LARGE_INTEGER last;
							last.QuadPart = 0;
							if ( SetFilePointerEx ( this->hFile , last , NULL , FILE_END ) ) {
								bRet = WriteFile ( this->hFile , &padding , 1 , &wrote_size , NULL );
								if ( ( bRet == TRUE ) && ( wrote_size == 1 ) ) {
									bReturn = true;
								}
							}
						} else {
							bReturn = true;
						}

						if ( bReturn ) {
							this->Mode = WriteMode::None;
							return true;
						}
					}
				}

			}
		}
		return false;
	}


	bool WriteChunk (const char *lpChunkName , const void *lpData , uint32_t DataSize ) {
		if ( this->BeginChunk ( lpChunkName ) ) {
			if ( this->AdditionalChunkData ( lpData , DataSize ) ) {
				if ( this->EndChunk ( ) ) {
					return true;
				}
			}
		}
		return false;
	}
	template <typename U> 	bool WriteChunkType ( const char *lpChunkName , U typeData ) {
		return this->WriteChunk ( lpChunkName , &typeData , sizeof ( U ) );
	}

	bool WriteChunkString (const char *lpChunkName , const char *lpszString , bool bWriteNullCharactor = true ) {
		if ( this->BeginChunk ( lpChunkName ) ) {
			if ( this->AdditionalChunkStringData ( lpszString , bWriteNullCharactor ) ) {
				if ( this->EndChunk ( ) ) {
					return true;
				}
			}
		}
		return false;
	}

	bool WriteChunkString ( char *lpChunkName , const wchar_t *lpszString , bool bWriteNullCharactor = true ) {
		if ( this->BeginChunk ( lpChunkName ) ) {
			if ( this->AdditionalChunkStringData ( lpszString , bWriteNullCharactor ) ) {
				if ( this->EndChunk ( ) ) {
					return true;
				}
			}
		}
		return false;
	}

	bool BeginListChunk (const char *lpType ) {
		if ( this->BeginChunk ( "LIST" ) ) {
			std::string typeStr;
			if ( HSMakeRiffChunkFourCC ( lpType , &typeStr ) ) {
				if ( this->AdditionalChunkData ( typeStr.c_str ( ) , 4 ) ) {
					memcpy ( this->m_ChunkInfo.Type , typeStr.c_str ( ) , 4 );
					this->Mode = WriteMode::ListChunkWrite;
				}
			}
		}
		return false;
	}

	bool EndListChunk ( void ) {
		if ( this->IsCreated ( ) ) {
			if ( this->Mode == WriteMode::ListChunkWrite ) {
				this->Mode = WriteMode::ChunkWrite;
				return this->EndChunk ( );
			}
		}
		return false;
	}
	bool BeginListMemberChunk (const char *lpChunkName ) {
		if ( this->IsCreated ( ) ) {
			if ( this->Mode == WriteMode::ListChunkWrite ) {
				LARGE_INTEGER zero , pos;
				zero.QuadPart = 0;
				if ( SetFilePointerEx ( this->hFile , zero , &pos , FILE_END ) ) {
					std::string membername;
					uint32_t membersize = 0;
					if ( HSMakeRiffChunkFourCC ( lpChunkName , &membername ) ) {
						if ( this->AdditionalChunkData ( membername.c_str ( ) , 4 ) ) {
							if ( this->AdditionalChunkData ( &membersize , 4 ) ) {
								this->Mode = WriteMode::ListMemberChunkWrite;
								memcpy ( this->m_ListMemberChunkInfo.Header.Name , membername.c_str ( ) , 4 );
								this->m_ListMemberChunkInfo.Header.DataSize = 0;
								this->m_ListMemberChunkInfo.Position = static_cast< uint32_t >( pos.QuadPart );
								memset ( this->m_ChunkInfo.Type , 0 , 4 );
								return true;
							}
						}
					}
				}
			}
		}	
		return false;
	}
	bool AdditionalListMemberChunkData ( const void *lpData , uint32_t size ) {
		if ( this->IsCreated ( ) ) {
			if ( this->Mode == WriteMode::ListMemberChunkWrite ) {
				if ( this->AdditionalChunkData ( lpData , size ) ) {
					return true;
				}
			}
		}
		return false;
	}

	template <typename U> bool AdditionalListMemberChunkTypeData ( U typeData ) {
		return this->AdditionalListMemberChunkData ( &typeData , sizeof ( U ) );
	}


	bool AdditionalListMemberChunkStringData ( const char *lpszString , bool bWriteNullCharactor = true ) {
		if ( this->IsCreated ( ) ) {
			if ( this->Mode == WriteMode::ListMemberChunkWrite ) {
				if ( this->AdditionalChunkStringData ( lpszString , bWriteNullCharactor ) ) {
					return true;
				}
			}
		}
		return false;
	}
	bool AdditionalListMemberChunkStringData ( const wchar_t *lpszString , bool bWriteNullCharactor = true ) {
		if ( this->IsCreated ( ) ) {
			if ( this->Mode == WriteMode::ListMemberChunkWrite ) {
				if ( this->AdditionalChunkStringData ( lpszString , bWriteNullCharactor ) ) {
					return true;
				}
			}
		}
		return false;
	}




	bool EndListMemberChunk ( void ) {
		if ( this->IsCreated ( ) ) {
			if ( this->Mode == WriteMode::ListMemberChunkWrite ) {
				LARGE_INTEGER pos;
				pos.QuadPart = this->m_ListMemberChunkInfo.Position + 4;
				if ( SetFilePointerEx ( this->hFile , pos , NULL , FILE_BEGIN ) ) {
					DWORD wrote_size;
					BOOL bRet;
					bRet = WriteFile ( this->hFile , &this->m_ListMemberChunkInfo.Header.DataSize , 4 , &wrote_size , NULL );
					if ( ( bRet == TRUE ) && ( wrote_size == 4 ) ) {
						bool bReturn = false;
						if ( this->m_ListMemberChunkInfo.Header.DataSize % 2 ) {
							char padding = 0;
							bReturn = this->AdditionalChunkData ( &padding , 1 );
						} else {
							bReturn = true;
						}
						if ( bReturn ) {
							this->Mode = WriteMode::ListChunkWrite;
							return true;
						}
					}
				}
			}
		}
		return false;
	}


	bool WriteListMemberChunk ( char *lpChunkName , const void *lpData , uint32_t DataSize ) {
		if ( this->BeginListMemberChunk ( lpChunkName ) ) {
			if ( this->AdditionalListMemberChunkData ( lpData , DataSize ) ) {
				if ( this->EndListMemberChunk ( ) ) {
					return true;
				}
			}
		}
		return false;
	}

	template <typename U> bool WriteListMemberChunkType ( char *lpChunkName , U typeData ) {
		return this->WriteListMemberChunk ( lpChunkName , &typeData , sizeof ( U ) );
	}

	bool WriteListMemberChunkString (const char *lpChunkName , const char *lpszString , bool bWriteNullCharactor = true ) {
		if ( this->BeginListMemberChunk ( lpChunkName ) ) {
			if ( this->AdditionalListMemberChunkStringData ( lpszString , bWriteNullCharactor ) ) {
				if ( this->EndListMemberChunk ( ) ) {
					return true;
				}
			}
		}
		return false;
	}

	bool WriteListMemberChunkString ( const char *lpChunkName , const wchar_t *lpszString , bool bWriteNullCharactor = true ) {
		if ( this->BeginListMemberChunk ( lpChunkName ) ) {
			if ( this->AdditionalListMemberChunkStringData ( lpszString , bWriteNullCharactor ) ) {
				if ( this->EndListMemberChunk ( ) ) {
					return true;
				}
			}
		}
		return false;
	}



private:

	bool __CreateRiffFileA ( char * lpszFilePath ) {
		if ( lpszFilePath == nullptr ) {
			return false;
		}

		if ( this->IsCreated ( ) ) {
			return false;
		}

		HANDLE hNextFile;

		hNextFile = CreateFileA ( lpszFilePath , GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ , NULL , OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL );
		if ( hNextFile == INVALID_HANDLE_VALUE ) {
			return false;
		}

		this->hFile = hNextFile;

		memset ( this->szFilePath , 0 , sizeof ( this->szFilePath ) );


		if ( PathIsRelativeA ( lpszFilePath ) ) {
			if ( GetFullPathNameA ( lpszFilePath , MAX_PATH , (char*)this->szFilePath , NULL ) == 0 ) {
				lstrcpynA ( ( char* )this->szFilePath , lpszFilePath , MAX_PATH );
			}
		} else {
			lstrcpynA ( ( char* )this->szFilePath , lpszFilePath , MAX_PATH );
		}

		return true;
	}


	bool __CreateRiffFileW ( wchar_t * lpszFilePath ) {
		if ( lpszFilePath == nullptr ) {
			return false;
		}

		if ( this->IsCreated ( ) ) {
			return false;
		}

		HANDLE hNextFile;

		hNextFile = CreateFileW ( lpszFilePath , GENERIC_READ | GENERIC_WRITE , FILE_SHARE_READ , NULL , OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL );
		if ( hNextFile == INVALID_HANDLE_VALUE ) {
			return false;
		}

		this->hFile = hNextFile;

		memset ( this->szFilePath , 0 , sizeof ( this->szFilePath ) );


		if ( PathIsRelativeW ( lpszFilePath ) ) {
			if ( GetFullPathNameW ( lpszFilePath , MAX_PATH , ( wchar_t* )this->szFilePath , NULL ) == 0 ) {
				lstrcpynW ( ( wchar_t* )this->szFilePath , lpszFilePath , MAX_PATH );
			}
		} else {
			lstrcpynW ( ( wchar_t* ) this->szFilePath , lpszFilePath , MAX_PATH );
		}

		return true;
	}

	uint32_t __GetCreatedFilePathA ( char * lpszFilePath , size_t size ) {
		if ( ( size == 0 ) || ( lpszFilePath == nullptr ) ) {
			return lstrlenA ( (char*)this->szFilePath ) + 1;
		}
		lstrcpynA ( lpszFilePath , (char*)this->szFilePath , static_cast<int>( size ) );
		return lstrlenA ( lpszFilePath ) + 1;
	}


	uint32_t __GetCreatedFilePathW ( wchar_t * lpszFilePath , size_t size ) {
		if ( ( size == 0 ) || ( lpszFilePath == nullptr ) ) {
			return lstrlenW ( ( wchar_t* )this->szFilePath ) + 1;
		}
		lstrcpynW ( lpszFilePath , ( wchar_t* )this->szFilePath , static_cast<int>( size) );
		return lstrlenW ( lpszFilePath ) + 1;
	}


protected:
	virtual bool CreateRiffFile (const T *lpszFilePath ) {
		if ( typeid( T ) == typeid( char )) {
			return this->__CreateRiffFileA ( ( char* ) lpszFilePath );
		} else if ( typeid( T ) == typeid( wchar_t ) ) {
			return this->__CreateRiffFileW ( ( wchar_t* ) lpszFilePath );
		}
		return false;
	}

public:
	virtual uint32_t GetCreatedFilePath ( T *lpszFilePath , size_t size ) {
		if ( typeid( T ) == typeid( char ) ) {
			return this->__GetCreatedFilePathA ( ( char* ) lpszFilePath , size );
		} else if ( typeid( T ) == typeid( wchar_t ) ) {
			return this->__GetCreatedFilePathW ( ( wchar_t* ) lpszFilePath ,size);
		}
		return false;
	}

};


using CHSRiffWriterA = __CHSRiffWriterBaseTemplate<char>;
using CHSRiffWriterW = __CHSRiffWriterBaseTemplate<wchar_t>;
