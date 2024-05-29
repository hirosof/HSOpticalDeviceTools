#pragma once

#include "CHSOpticalDrive.hpp"

#pragma pack(push , 1)


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

#pragma warning( push )
#pragma warning( disable:26495 )
struct  THSSCSI_FeatureInfo {
	HSSCSI_SPTD_ResponseRawData raw;
	size_t rawSize;
	THSSCSI_FeatureHeader* pHeader;
	std::vector<THSSCSI_FeatureDescriptorInfo> Descriptors;
};
#pragma warning(pop)

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

enum struct EHSSCSI_ProfileFamilyName {
	Unknown = 0,
	CD,
	DVD,
	BD
};


//CD Read Feature Descriptor (Feature Code = 0x001E)
struct THSSCSI_FeatureDescriptor_CDRead {
	THSSCSI_FeatureDescriptorHeader header;
	bool CDText : 1;
	bool C2Flags : 1;
	uint8_t Reserved : 3;
	bool DAP : 1;
};


//Removable Medium Feature Descriptor (Feature Code = 0x0003)
struct THSSCSI_FeatureDescriptor_RemovableMedium {
	THSSCSI_FeatureDescriptorHeader header;
	bool Lock : 1;
	bool DBML : 1;
	bool Pvnt_Jmpr : 1;
	bool Eject : 1;
	bool Load : 1;
	uint8_t Loading_Mechanism_Type : 3;
	uint8_t Reserved[3];
};

// Drive Serial Number Feature Descriptor (Feature Code = 0x0108)
struct THSSCSI_FeatureDescriptor_DriveSerialNumber {
	THSSCSI_FeatureDescriptorHeader header;
	unsigned char SerialNumber[256];
};


using HSSCSI_ProfilesItem = std::pair<uint16_t,EHSSCSI_ProfileName>;
using HSSCSI_Profiles = std::vector<HSSCSI_ProfilesItem>;

using HSSCSI_ProfilesNumberVector = std::vector<uint16_t>;
using HSSCSI_ProfilesNameVector = std::vector<EHSSCSI_ProfileName>;



#pragma pack(pop)


class CHSOpticalDriveGetConfigCmd {
private:

	CHSOpticalDrive* mpDrive;

	bool executeRawGeneralCommand( THSSCSI_CommandData* pData ) const;
	size_t  executeRawGetConfigCmd( EHSSCSI_GET_CONFIGURATION_RT_TYPE type, uint16_t startFeatureNumber,
		HSSCSI_SPTD_ResponseRawData* pRawResponseData, HSSCSI_SPTD_RESULT* pDetailResult = nullptr ) const;


public:
	CHSOpticalDriveGetConfigCmd( );
	CHSOpticalDriveGetConfigCmd( CHSOpticalDrive* pDrive );
	void setDrive( CHSOpticalDrive* pDrive );

	size_t  execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE type, uint16_t startFeatureNumber,
		THSSCSI_FeatureInfo* pInfo, HSSCSI_SPTD_RESULT* pDetailResult = nullptr ) const;

	bool getCurrentProfileNumber( uint16_t* pCurrentProfile ) const;


	bool getSupportProfileNumbers( HSSCSI_ProfilesNumberVector* pProfiles ) const;
	bool getSupportProfileNames( HSSCSI_ProfilesNameVector* pProfiles ) const;

	bool getSupportProfiles( HSSCSI_Profiles* pProfiles , bool bIncludeUnknown=false) const;

	static EHSSCSI_ProfileName GetProfileName( uint16_t profileNumber );
	static std::string GetProfileNameString( uint16_t profileNumber , bool groupOfSameType = true);
	static std::string GetProfileNameString( EHSSCSI_ProfileName profileName , bool groupOfSameType = true );

	EHSSCSI_ProfileName getCurrentProfileName( void ) const;
	EHSSCSI_ProfileFamilyName getCurrentProfileFamilyName(void) const;

	std::string getCurrentProfileFamilyNameString( void ) const;

	static std::string GetProfileFamilyNameString( EHSSCSI_ProfileFamilyName profileFamily );


	template <typename T> bool getGeneralFeatureDescriptor( T* pDesc , uint16_t targetFeatureNumber )const {
		if ( pDesc == nullptr ) return false;
		THSSCSI_FeatureInfo info;
		if ( this->execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE::Once, targetFeatureNumber, &info ) == 0 )return false;
		THSSCSI_FeatureDescriptorInfo descInfo = info.Descriptors[0];
		memcpy( pDesc, descInfo.pHeader, sizeof( T ) );
		return true;
	}

	bool getCDReadFeatureDescriptor( THSSCSI_FeatureDescriptor_CDRead* pDesc )const;
	bool getRemovableMediumFeatureDescriptor( THSSCSI_FeatureDescriptor_RemovableMedium* pDesc ) const;
	bool getDriveSerialNumberFeatureDescriptor( THSSCSI_FeatureDescriptor_DriveSerialNumber* pDesc ) const;

};


