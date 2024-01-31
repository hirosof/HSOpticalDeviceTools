#pragma once

#include <Windows.h>
#include <cstdio>
#include <ntddcdrm.h>
#include <string>

#include <set>
#include <tuple>

#include<algorithm>

#include "../common/HSSCSICommandSupport.hpp"

const size_t DHSOpticalDriveVendorIDLength = 8U;
const size_t DHSOpticalDriveProductIDLength = 16U;
const size_t DHSOpticalDriveProductRevisionLevelLength = 4U;
const size_t DHSOpticalDriveDeviceNameLength = 30U;


struct THSOpticalDriveDeviceInfo {
	char VendorID[DHSOpticalDriveVendorIDLength + 1];
	char ProductID[DHSOpticalDriveProductIDLength + 1];
	char ProductRevisionLevel[DHSOpticalDriveProductRevisionLevelLength + 1];
	char DeviceName[DHSOpticalDriveDeviceNameLength + 1];
};


struct THSEnumrateOpticalDriveInfo {
	uint8_t  uOpticalDriveCount;
	struct tagDrive {
		char Letter;
		bool bIncludedInfo;
		THSOpticalDriveDeviceInfo Info;
	}Drives[26];
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
	FailedGodAliment
};

class CHSOpticalDrive {

protected:

	HANDLE hDrive;
	char cOpenedDriveLetter;

public:
	static HANDLE OpenDrive( char driveLetter );
	static bool IsOpticalDrive( const char driveLetter );
	static bool EnumOpticalDrive( THSEnumrateOpticalDriveInfo* pInfo );
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
	EHSSCSI_ReadyStatus checkReady( HSSCSI_SPTD_RESULT *pDetailResult ) const;

	bool ejectMedia( bool asyncWork = true ) const;
	bool loadMedia( bool asyncWork = true ) const;

	bool trayOpen(HSSCSI_SPTD_RESULT* pDetailResult = nullptr, bool asyncWork = true ) const;
	bool trayClose(HSSCSI_SPTD_RESULT* pDetailResult = nullptr,bool asyncWork = true ) const;

	bool isTrayOpened( void) const;
	EHSOD_TrayState checkTrayState( HSSCSI_SPTD_RESULT* pDetailResult )const;


	bool spinUp( HSSCSI_SPTD_RESULT* pDetailResult = nullptr, bool asyncWork = true ) const;
	bool spinDown( HSSCSI_SPTD_RESULT* pDetailResult = nullptr, bool asyncWork = true ) const;


	EHSOD_AlimentMaskType getAlimentMask(ULONG *pRawAlimentMask )const;


};