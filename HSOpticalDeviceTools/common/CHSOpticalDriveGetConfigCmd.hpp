#pragma once

#include "CHSOpticalDrive.hpp"


enum struct EHSSCSI_GET_CONFIGURATION_RT_TYPE {
	All = 0,
	Current,
	Once
};


struct THSSCSI_FeatureHeader {
	uint8_t  DataLength[4];
	uint16_t Reserved;
	uint8_t CurrentProfile[2];
};

struct THSSCSI_FeatureDescriptorHeader {
	uint8_t FeatureCode[2];
	uint8_t Current : 1;
	uint8_t Persistent : 1;
	uint8_t Version : 4;
	uint8_t Reserved : 2;
	uint8_t AdditionalLength;
};

struct THSSCSI_FeatureDescriptorInfo {
	THSSCSI_FeatureDescriptorHeader* pHeader;
	uint8_t* pAdditionalData;
};


struct  THSSCSI_FeatureInfo {
	HSSCSI_SPTD_ResponseRawData raw;
	size_t rawSize;
	THSSCSI_FeatureHeader* pHeader;
	std::vector<THSSCSI_FeatureDescriptorInfo> Descriptors;
};


struct THSSCSI_ProfileDescriptor {
	uint8_t  ProfileNumber[2];
	uint8_t CurrentP : 1;
	uint8_t Reserved1 : 7;
	uint8_t Reserved2;
};


enum struct  EHSSCSI_ProfileName {
	Unknown = 0,	//•s–¾
	RemovableDisk,
	CD_ROM,	//CD-ROM
	CD_R,	//CD-R
	CD_RW,	//CD-RW
	DVD_ROM,	//DVD-ROM
	DVD_R,	//DVD-R
	DVD_R_DL_Sequential,	//DVD-R DL
	DVD_R_DL_Jump,	//DVD-R DL
	DVD_RW_Restricted,	//DVD-RW
	DVD_RW_Sequential,	//DVD-RW
	DVD_RAM,	//DVD_RAM
	DVD_Plus_R,	//DVD+R
	DVD_Plus_R_DL,	//DVD+R DL
	DVD_Plus_RW,	//DVD+RW
	BD_ROM,	//BD-ROM
	BD_R_SRM,	//BD-R
	BD_R_RRM,	//BD-R
	BD_RE,	//BD-RE
};

using HSSCSI_ProfilesItem = std::pair<uint16_t,EHSSCSI_ProfileName>;
using HSSCSI_Profiles = std::vector<HSSCSI_ProfilesItem>;

using HSSCSI_ProfilesNumberVector = std::vector<uint16_t>;
using HSSCSI_ProfilesNameVector = std::vector<EHSSCSI_ProfileName>;


class CHSOpticalDriveGetConfigCmd {
private:

	CHSOpticalDrive* mpDrive;

	bool executeRawGeneralCommand( THSSCSI_CommandData* pData ) const;
	size_t  executeRawGetConfigCmd( EHSSCSI_GET_CONFIGURATION_RT_TYPE type, uint16_t startFeatureNumber,
		HSSCSI_SPTD_ResponseRawData* pRawResponseData, HSSCSI_SPTD_RESULT* pDetailResult = nullptr );


public:
	CHSOpticalDriveGetConfigCmd( );
	CHSOpticalDriveGetConfigCmd( CHSOpticalDrive* pDrive );
	void SetDrive( CHSOpticalDrive* pDrive );

	size_t  execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE type, uint16_t startFeatureNumber,
		THSSCSI_FeatureInfo* pInfo, HSSCSI_SPTD_RESULT* pDetailResult = nullptr );

	bool getCurrentProfileNumber( uint16_t* pCurrentProfile );


	bool getSupportProfileNumbers( HSSCSI_ProfilesNumberVector* pProfiles );
	bool getSupportProfileNames( HSSCSI_ProfilesNameVector* pProfiles );

	bool getSupportProfiles( HSSCSI_Profiles* pProfiles , bool bIncludeUnknown=false);

	static EHSSCSI_ProfileName GetProfileName( uint16_t profileNumber );
	static std::string GetProfileNameString( uint16_t profileNumber );
	static std::string GetProfileNameString( EHSSCSI_ProfileName profileName );


};