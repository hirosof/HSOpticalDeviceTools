#pragma once

#include <map>
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
	uint16_t FeatureCode;
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


// Core Feature Descriptor (Feature Code = 0x0001)
struct THSSCSI_FeatureDescriptor_Core {
	THSSCSI_FeatureDescriptorHeader header;
	uint32_t PhysicalInterfaceStandard;
	bool DBE : 1;
	bool INQ2 : 1;
};




const std::map<const uint16_t, const std::string>  HSSCSI_FeatureNameStrings {
	{0x0000,"Profile List"},
	{0x0001,"Core"},
	{0x0002,"Morphing"},
	{0x0003,"Removable Medium"},
	{0x0004,"Write Protect"},
	{0x0010,"Random Readable"},
	{0x001D,"Multi-Read"},
	{0x001E,"CD Read"},
	{0x001F,"DVD Read"},
	{0x0020,"Random Writable"},
	{0x0021,"Incremental Streaming Writable"},
	{0x0022,"Sector Erasable"},
	{0x0023,"Formattable"},
	{0x0024,"Hardware Defect Management"},
	{0x0025,"Write Once"},
	{0x0026,"Restricted Overwrite"},
	{0x0027,"CD-RW CAV Write"},
	{0x0028,"MRW"},
	{0x0029,"Enhanced Defect Reporting"},
	{0x002A,"DVD+RW"},
	{0x002B,"DVD+R"},
	{0x002C,"Rigid Restricted Overwrite"},
	{0x002D,"CD Track at Once"},
	{0x002E,"CD Mastering"},
	{0x002F,"DVD-R/-RW Write"},
	{0x0033,"Layer Jump Recording"},
	{0x0034,"LJ Rigid Restricted Overwrite"},
	{0x0035,"Stop Long Operation"},
	{0x0037,"CD-RW Media Write Support"},
	{0x0038,"BD-R POW"},
	{0x003A,"DVD+RW Dual Layer"},
	{0x003B,"DVD+R Dual Layer"},
	{0x0040,"BD Read Feature"},
	{0x0041,"BD Write Feature"},
	{0x0042,"TSR"},
	{0x0050,"HD DVD Read"},
	{0x0051,"HD DVD Write"},
	{0x0052,"HD DVD-RW Fragment Recording"},
	{0x0080,"Hybrid Disc"},
	{0x0100,"Power Management"},
	{0x0101,"SMART"},
	{0x0102,"Embedded Changer"},
	{0x0103,"CD Audio External Play"},
	{0x0104,"Microcode Upgrade"},
	{0x0105,"Timeout"},
	{0x0106,"DVD-CSS"},
	{0x0107,"Real Time Streaming"},
	{0x0108,"Drive Serial Number"},
	{0x0109,"Media Serial Number"},
	{0x010A,"Disc Control Blocks"},
	{0x010B,"DVD CPRM"},
	{0x010C,"Firmware Information"},
	{0x010D,"AACS"},
	{0x010E,"DVD CSS Managed Recording"},
	{0x0110,"VCPS"},
	{0x0113,"SecurDisc"},
	{0x0142,"OSSC Feature"},
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





	bool getSupportProfileNumbers( HSSCSI_ProfilesNumberVector* pProfiles ) const;
	bool getSupportProfileNames( HSSCSI_ProfilesNameVector* pProfiles ) const;

	bool getSupportProfiles( HSSCSI_Profiles* pProfiles , bool bIncludeUnknown=false) const;

	static EHSSCSI_ProfileName GetProfileName( uint16_t profileNumber );
	static std::string GetProfileNameString( uint16_t profileNumber , bool groupOfSameType = true);
	static std::string GetProfileNameString( EHSSCSI_ProfileName profileName , bool groupOfSameType = true );


	bool getCurrentProfileNumber( uint16_t* pCurrentProfile ) const;
	bool getCurrentProfile( HSSCSI_ProfilesItem* pCurrentProfileItem ) const;
	EHSSCSI_ProfileName getCurrentProfileName( void ) const;
	std::string getCurrentProfileNameString( bool groupOfSameType = true ) const;


	EHSSCSI_ProfileFamilyName getCurrentProfileFamilyName(void) const;
	std::string getCurrentProfileFamilyNameString( void ) const;



	static EHSSCSI_ProfileFamilyName GetProfileFamilyName( EHSSCSI_ProfileName profileName );
	static std::string GetProfileFamilyNameString( EHSSCSI_ProfileFamilyName profileFamily );
	static std::string GetProfileFamilyNameString( EHSSCSI_ProfileName profileName );


	template <typename T> bool getGeneralFeatureDescriptor( T* pDesc , uint16_t targetFeatureNumber )const {
		if ( pDesc == nullptr ) return false;
		THSSCSI_FeatureInfo info;
		if ( this->execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE::Once, targetFeatureNumber, &info ) == 0 )return false;
		THSSCSI_FeatureDescriptorInfo descInfo = info.Descriptors[0];
		memcpy( pDesc, descInfo.pHeader, sizeof( T ) );
		return true;
	}

	bool getFeatureCDRead( THSSCSI_FeatureDescriptor_CDRead* pDesc )const;
	bool getFeatureRemovableMedium( THSSCSI_FeatureDescriptor_RemovableMedium* pDesc ) const;
	bool getFeatureDriveSerialNumber( THSSCSI_FeatureDescriptor_DriveSerialNumber* pDesc ) const;

	EHSSCSI_ConnectInterfaceName getPhysicalInterfaceStandardName( void )const;
	std::string getPhysicalInterfaceStandardNameString( void )const;


};


