#pragma once
#include "CHSOpticalDrive.hpp"
#include "CHSOpticalDriveGetConfigCmd.hpp"


#pragma pack(push , 1)

struct THSSCSI_ReadCapacityResponse {
	uint32_t LogicalBlockAddress;
	uint32_t BlockLengthInBytes;
};


struct THSSCSI_OPCTableEntry {
	uint16_t Speed;
	uint8_t VendorSpecificOPCValues[6];
};

struct THSSCSI_DiscInformation {
	uint16_t DiscInformationLength;
	uint8_t DiscStatus : 2;
	uint8_t StateOfLastSession : 2;
	bool Erasable : 1;
	uint8_t DiscInformationDataType : 3;

	uint8_t NumberOfFirstTrackOnDisc;

	uint8_t NumberOfSessionsLSB;
	uint8_t FirstTrackNumberInLastSessionLSB;
	uint8_t LastTrackNumberInLastSessionLSB;

	uint8_t BGFormatStatus : 2;
	uint8_t Legacy : 1;
	uint8_t Reserved : 1;
	uint8_t DAC_V : 1;
	uint8_t URU : 1;
	uint8_t DBC_V: 1;
	uint8_t DID_V : 1;
	uint8_t DiscType;

	uint8_t NumberOfSessionsMSB;
	uint8_t FirstTrackNumberInLastSessionMSB;
	uint8_t LastTrackNumberInLastSessionMSB;

	uint32_t DiscIdentification;
	uint32_t LastSessionLeadInStartAddress;
	uint32_t LastPossibleLeadOutStartAddress;

	uint8_t DiscBarCode[8];
	uint8_t DiscApplicationCode;
	uint8_t NumberOfOPCTables;
	THSSCSI_OPCTableEntry OPCTable[255];
};


struct THSSCSI_InterpretedDiscInformation {
	uint16_t DiscInformationLength;
	uint16_t NumberOfSessions;
	uint16_t FirstTrackNumberInLastSession;
	uint16_t LastTrackNumberInLastSession;
	uint32_t DiscIdentification;
	uint32_t LastSessionLeadInStartAddress;
	uint32_t LastPossibleLeadOutStartAddress;
};


struct THSSCSI_TrackResourcesInformation {
	uint16_t DiscInformationLength;
	uint8_t Reserved1 : 5;
	uint8_t DiscInformationDataType : 3;
	uint8_t Reserved2;
	uint16_t MaxPossibleNumberOfTracksOnDisc;
	uint16_t NumberOfAssignedTracksOnDisc;
	uint16_t MaxPossibleNumberOfAppendableTracksOnDisc;
	uint16_t CurrentNumberOfAppendableTracksOnDisc;
};


struct THSSCSI_POWResourcesDiscInformation {
	uint16_t DiscInformationLength;
	uint8_t Reserved1 : 5;
	uint8_t DiscInformationDataType : 3;
	uint8_t Reserved2;
	uint32_t RemainingPOWReplacements;
	uint32_t RemainingPOWReallocationMapEntries;
	uint32_t NumberOfRemainingPOWUpdates;
};





#pragma pack(pop)

class  CHSOpticalDiscReader {
protected:
	CHSOpticalDrive* mp_Drive;
	CHSOpticalDriveGetConfigCmd m_cmd;


	bool executeRawCommand( THSSCSI_CommandData* pData ) const;


public:

	CHSOpticalDiscReader( );
	CHSOpticalDiscReader( CHSOpticalDrive* pDrive );
	void SetDrive( CHSOpticalDrive* pDrive );


	bool  readCapacity( THSSCSI_ReadCapacityResponse* pres );
	bool  readDiscInformation( THSSCSI_DiscInformation *pInfo,  THSSCSI_InterpretedDiscInformation* pInterpretedInfo = nullptr);
	bool  readTrackResourcesInformation( THSSCSI_TrackResourcesInformation* pInfo );
	bool  readPOWResourcesDiscInformation( THSSCSI_POWResourcesDiscInformation* pInfo );
};