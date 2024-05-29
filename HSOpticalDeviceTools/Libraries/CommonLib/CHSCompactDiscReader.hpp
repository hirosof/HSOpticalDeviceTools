#include "CHSOpticalDiscReader.hpp"
#include <map>
#pragma pack(push , 1)

enum struct  EHSSCSI_AddressFormType {
	SplittedMSF = 0,
	MergedMSF,
	LBA
};


enum struct EHSSCSI_TrackType {
	Audio2Channel = 0,
	Audio2ChannelWithPreEmphasis,
	Audio4Channel,
	Audio4ChannelWithPreEmphasis,
	DataUninterrupted,
	DataIncrement,
	Unknown
};

enum struct EHSSCSI_DiscType {
	Mode1 = 0,
	CD_I,
	Mode2
};

union UHSSCSI_AddressData32 {
	uint8_t urawValues [4] ;
	int8_t irawValues[4];
	uint32_t u32Value;
	int32_t i32Value;
};

struct THSSCSI_TOC_PMA_ATIP_ResponseHeader {
	uint16_t DataLength;
	uint8_t FirstNumberField;
	uint8_t LastNumberField;
};

struct THSSCSI_FormattedTOCRawItem {
	uint8_t reserved1;
	uint8_t Control : 4;
	uint8_t ADR : 4;
	uint8_t TrackNumber;
	uint8_t reserved2;
	UHSSCSI_AddressData32 TrackStartAddress;
};


struct THSSCSI_FormattedTOCInterpretedItem {
	uint8_t TrackNumber;
	uint8_t Control;
	uint8_t ADR;
	EHSSCSI_TrackType TrackType;
	bool PermittedDigitalCopy;
	UHSSCSI_AddressData32 TrackStartAddress;
	UHSSCSI_AddressData32 TrackEndAddress;
	UHSSCSI_AddressData32 TrackLength;
};

#pragma warning( push )
#pragma warning( disable:26495 )

struct THSSCSI_FormattedTOC {
	EHSSCSI_AddressFormType AddressType;
	THSSCSI_TOC_PMA_ATIP_ResponseHeader header;
	std::vector< THSSCSI_FormattedTOCRawItem> rawItems;
	std::map<uint8_t ,  THSSCSI_FormattedTOCInterpretedItem> items;
	uint8_t StartTrackNumber;
	uint8_t EndTrackNumber;
	uint8_t CountOfTracks;
	UHSSCSI_AddressData32 PointOfLeadOutAreaStart;
};

#pragma warning(pop)

struct THSSCSI_RawTOCRawItem {
	uint8_t SessionNumber;
	uint8_t Control : 4;
	uint8_t ADR : 4;
	uint8_t TNO;
	uint8_t POINT;
	uint8_t Min;
	uint8_t Sec;
	uint8_t Frame;
	uint8_t Zero;
	uint8_t PMIN;
	uint8_t PSEC;
	uint8_t PFRAME;
};


struct THSSCSI_RawTOCSessionItem {
	uint8_t SessionNumber;
	uint8_t FirstTrackNumber;
	uint8_t LastTrackNumber;
	EHSSCSI_DiscType DiscType;
	UHSSCSI_AddressData32 PointOfLeadOutAreaStart;
};

struct THSSCSI_RawTOCTrackItem {
	uint8_t TrackNumber;
	uint8_t SessionNumber;
	uint8_t Control;
	uint8_t ADR;
	EHSSCSI_TrackType TrackType;
	bool PermittedDigitalCopy;
	UHSSCSI_AddressData32 TrackStartAddress;
	UHSSCSI_AddressData32 TrackEndAddress;
	UHSSCSI_AddressData32 TrackLength;
};


#pragma warning( push )
#pragma warning( disable:26495 )

struct THSSCSI_RawTOC {
	EHSSCSI_AddressFormType AddressType;
	THSSCSI_TOC_PMA_ATIP_ResponseHeader header;
	std::vector< THSSCSI_RawTOCRawItem> rawItems;
	std::map< uint8_t, THSSCSI_RawTOCSessionItem> sessionItems;
	std::map<uint8_t ,  THSSCSI_RawTOCTrackItem> trackItems;
	uint8_t FirstTrackNumber;
	uint8_t LastTrackNumber;
	uint8_t CountOfTracks;
	uint8_t FirstSessionNumber;
	uint8_t LastSessionNumber;
	uint8_t CountOfSessions;
};

#pragma warning(pop)


struct THSSCSI_CDTEXT_PackData {
	uint8_t PackIndicator1;
	uint8_t PackIndicator2;
	uint8_t PackIndicator3;
	uint8_t CharacterPosition : 4;
	uint8_t BlockNumber : 3;
	bool isDoubleByteCharacterCode : 1;

	union {
		uint8_t single[12];
		uint16_t dual[6];
	}TextDataField;

	uint16_t CRC_OR_Reserved;
};

struct THSSCSI_CDTEXT_ParsedItemNames {
	std::string Name;
	std::string PerformerName;
	std::string SongWriterName;
	std::string ComposerName;
	std::string ArrangerName;
};




struct THSSCSI_CDTEXT_ParsedNames {
	bool isDoubleByteCharatorCode;
	THSSCSI_CDTEXT_ParsedItemNames album;
	std::map< uint8_t, THSSCSI_CDTEXT_ParsedItemNames> trackTitles;
};


enum struct EHSSCSI_CDText_ReadResult {
	Success = 0,
	Failed,
	InvalidParameter,
	NotReady,
	NotFound,
	NotSupported,
	TimeOut
};

struct THSSCSI_CDTEXT_Information {
	THSSCSI_TOC_PMA_ATIP_ResponseHeader header;
	bool hasItems;
	bool validHeader;
	std::vector< THSSCSI_CDTEXT_PackData> rawItems;
	std::map< size_t, THSSCSI_CDTEXT_ParsedNames> parsedItems;
	uint8_t NumberOfBlocks;
};




#pragma pack(pop)

class CHSCompactDiscReader : public CHSOpticalDiscReader {

private:
	static const UINT16  CRCTABLE[256];
	static const uint8_t NumberOfCDTextReadTry;
	static bool Crc16( void* lpData, size_t size, UINT16* lpCRC16 );


public:

	static const size_t NormalCDDATrackSectorSize;

	CHSCompactDiscReader( );
	CHSCompactDiscReader( CHSOpticalDrive* pDrive );

	static  UHSSCSI_AddressData32 MergeMSF( UHSSCSI_AddressData32 address );
	static  UHSSCSI_AddressData32 InverseEndianAddressData32( UHSSCSI_AddressData32 address );

	static  UHSSCSI_AddressData32 SplitMSF( UHSSCSI_AddressData32 address );
	static  UHSSCSI_AddressData32 SplittedMSF_to_LBA( UHSSCSI_AddressData32 address );
	static  UHSSCSI_AddressData32 LBA_to_SplittedMSF( UHSSCSI_AddressData32 address );
	static  UHSSCSI_AddressData32 MergedMSF_to_LBA( UHSSCSI_AddressData32 address );
	static  UHSSCSI_AddressData32 LBA_to_MergedMSF( UHSSCSI_AddressData32 address );

	static UHSSCSI_AddressData32 MakeAddressData32( uint32_t value );
	static UHSSCSI_AddressData32 MakeAddressData32( uint8_t  m, uint8_t s, uint8_t f );

	static EHSSCSI_TrackType GetTrackTypeFromControl( uint8_t control, bool* pPermittedDigitalCopy = nullptr );


	bool isCDMediaPresent( void ) const;
	bool isSupportedCDText( void ) const;

	bool readFormmatedTOC( THSSCSI_FormattedTOC* pInfo, EHSSCSI_AddressFormType addressType = EHSSCSI_AddressFormType::LBA )const;
	bool readRawTOC( THSSCSI_RawTOC* pInfo, EHSSCSI_AddressFormType addressType = EHSSCSI_AddressFormType::LBA )const;

	EHSSCSI_CDText_ReadResult readCDText( THSSCSI_CDTEXT_Information* pInfo )const;



	size_t readStereoAudioTrack( CHSSCSIGeneralBuffer* pBuffer, uint8_t track_number, UHSSCSI_AddressData32 offset, EHSSCSI_AddressFormType offsetAddressType, UHSSCSI_AddressData32  readSize, EHSSCSI_AddressFormType readSizeAddressType )const;

	std::string  getTOCString( const char joinChar = '+' )const;
	static std::string  GetTOCStringStatic(const  THSSCSI_RawTOC *pToc , const char joinChar = '+' );

	std::string  getMusicBrainzDiscIDSource( void )const;
	static std::string  GetMusicBrainzDiscIDSourceStatic( const  THSSCSI_RawTOC *pToc );

	bool  setSpeedMax( HSSCSI_SPTD_RESULT* pResult = nullptr )const;

};