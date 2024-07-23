#pragma once

#include <Windows.h>
#include <cstdio>
#include <ntddcdrm.h>
#include <string>

#include <set>
#include <tuple>

#include<algorithm>

#include "HSSCSICommandSupport.hpp"

const size_t DHSOpticalDriveVendorIDLength = 8U;
const size_t DHSOpticalDriveProductIDLength = 16U;
const size_t DHSOpticalDriveProductRevisionLevelLength = 4U;
const size_t DHSOpticalDriveDeviceNameLength = 30U;


#pragma pack(push , 1)

struct THSOpticalDriveDeviceInfo {
	INQUIRYDATA raw;
	char VendorID[DHSOpticalDriveVendorIDLength + 1];
	char ProductID[DHSOpticalDriveProductIDLength + 1];
	char ProductRevisionLevel[DHSOpticalDriveProductRevisionLevelLength + 1];
	char DisplayName[DHSOpticalDriveDeviceNameLength + 1];
};


struct THSOpticalDriveInfo {
	char Letter;
	bool bIncludedInfo;
	THSOpticalDriveDeviceInfo Info;
};


struct THSEnumrateOpticalDriveInfo {
	uint8_t  uOpticalDriveCount;
	THSOpticalDriveInfo Drives[26];
};


enum struct EHSOD_TrayState {
	Closed=0,
	Opened,
	FailedGotStatus
};



enum struct EHSOD_AlimentMaskType {
	ByteAliment = 0,
	WordAliment,
	DwordAliment,
	DoubleDwordAliment,
	UnknownAliment,
	FailedGotAliment
};





struct THSSCSI_EventStatusHeader {
	uint8_t Length[2];
	uint8_t NotificationClass : 3;
	uint8_t Reserved : 4;
	bool NEA : 1;
	uint8_t SupportedEventClass;
};



struct THSSCSI_MediaEventStatus {
	THSSCSI_EventStatusHeader header;
	uint8_t EventCode : 4;
	uint8_t Reserved1 : 4;
	bool DoorOrTrayOpen : 1;
	bool MediaPresent : 1;
	uint8_t Reserved2 : 6;
	uint8_t StartSlot;
	uint8_t EndSlot;
};


struct THSSCSI_SlotTable {
	bool Change : 1;
	uint8_t Reserved1 : 6;
	bool DiscPresent : 1;
	bool CWP : 1;
	bool CWP_V : 1;
	uint8_t Reserved2 : 6;
	uint8_t Reserved3[2];
};


struct THSSCSI_MechanismStatus {
	uint8_t CurrentSlotLowOrder5bit : 5;
	uint8_t ChangerState:2;
	uint8_t Fault : 1;
	uint8_t CurrentSlotHighOrder3bit : 3;
	uint8_t Reserved1 : 1;
	bool DoorOpen : 1;
	uint8_t MechanismState : 3;
	uint8_t CurrentLBALegacy[3];
	uint8_t NumberOfSlotsAvailable;
	uint8_t LengthOfSlotTable[2];
	THSSCSI_SlotTable SlotTable[255];
};


#pragma pack(pop)


class CHSOpticalDrive {

protected:

	HANDLE hDrive;
	char cOpenedDriveLetter;

public:
	static HANDLE OpenDrive( char driveLetter );
	static bool IsOpticalDrive( const char driveLetter );
	static bool EnumOpticalDrive( THSEnumrateOpticalDriveInfo* pInfo );
	static bool GetFirstOpticalDriveInfo( THSOpticalDriveInfo *pInfo );
	static bool GetDeviceInfo( const char opticalDriveLetter, THSOpticalDriveDeviceInfo* pInfo );
	static bool GetDeviceInfoFromHandle( HANDLE hOpticalDrive, THSOpticalDriveDeviceInfo* pInfo );

	CHSOpticalDrive( void );

	~CHSOpticalDrive( );


	bool open( const char opticalDriveLetter );

	bool getCurrentDeviceInfo( THSOpticalDriveDeviceInfo* pInfo ) const;
	char getCurrentDriveLetter( void ) const;

	bool close( void );

	HANDLE getHandle( void ) const;


	bool isOpened( void ) const;

	bool executeCommand( THSSCSI_CommandData* pData) const;


	bool isReady( void ) const;
	EHSSCSI_ReadyStatus checkReady( HSSCSI_SPTD_RESULT *pDetailResult = nullptr ) const;


	bool isMediaPresent( void )const;



	bool ejectMedia( bool asyncWork = true ) const;
	bool loadMedia( bool asyncWork = true ) const;

	bool trayOpen( HSSCSI_SPTD_RESULT* pDetailResult = nullptr, bool asyncWork = true ) const;
	bool trayClose( HSSCSI_SPTD_RESULT* pDetailResult = nullptr, bool asyncWork = true ) const;

	bool trayLock( HSSCSI_SPTD_RESULT* pDetailResult = nullptr ) const;
	bool trayUnlock( HSSCSI_SPTD_RESULT* pDetailResult = nullptr ) const;

	bool isTrayOpened( void) const;
	EHSOD_TrayState checkTrayState(void )const;


	bool spinUp( HSSCSI_SPTD_RESULT* pDetailResult = nullptr, bool asyncWork = true ) const;
	bool spinDown( HSSCSI_SPTD_RESULT* pDetailResult = nullptr, bool asyncWork = true ) const;
	bool setPowerState(uint8_t condition ,  HSSCSI_SPTD_RESULT* pDetailResult = nullptr, bool asyncWork = true ) const;


	EHSOD_AlimentMaskType getAlimentMask(ULONG *pRawAlimentMask )const;
	bool getMaxTransferLength(DWORD *pMaxTransferLength )const;
	EHSSCSI_ConnectInterfaceName getBusType( STORAGE_BUS_TYPE *pRawBusType = nullptr)const;

	bool getMechanismStatus( THSSCSI_MechanismStatus* pStatus, HSSCSI_SPTD_RESULT* pDetailResult ) const;
	bool getMediaEventStatus( THSSCSI_MediaEventStatus* pStatus, HSSCSI_SPTD_RESULT* pDetailResult )const;




};