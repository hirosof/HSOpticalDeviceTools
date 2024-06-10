#include "CHSOpticalDriveGetConfigCmd.hpp"

bool CHSOpticalDriveGetConfigCmd::executeRawGeneralCommand( THSSCSI_CommandData* pData ) const {
    return ( this->mpDrive != nullptr ) ? this->mpDrive->executeCommand( pData ) : false;
}


size_t CHSOpticalDriveGetConfigCmd::executeRawGetConfigCmd( EHSSCSI_GET_CONFIGURATION_RT_TYPE type,
    uint16_t startFeatureNumber, HSSCSI_SPTD_ResponseRawData* pRawResponseData, HSSCSI_SPTD_RESULT* pDetailResult ) const{

    if ( pRawResponseData == nullptr ) return 0;

    THSSCSI_CommandData data;
    if ( !HSSCSI_InitializeCommandData( &data ) )return 0;

    uint8_t headerOnly[8];
    memset( headerOnly, 0, sizeof( headerOnly ) );

    data.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
    data.pSPTDStruct->DataBuffer = headerOnly;
    data.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( headerOnly ) );
    data.pSPTDStruct->CdbLength = 10;

    data.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_GET_CONFIGURATION;
    data.pSPTDStruct->Cdb[2] = ( startFeatureNumber & 0xFF00 ) >> 8;
    data.pSPTDStruct->Cdb[3] = startFeatureNumber & 0xFF;
    data.pSPTDStruct->Cdb[8] = static_cast<uint8_t>( sizeof( headerOnly ) );

    switch ( type ) {
        case EHSSCSI_GET_CONFIGURATION_RT_TYPE::All:
            data.pSPTDStruct->Cdb[1] = 0;
            break;
        case EHSSCSI_GET_CONFIGURATION_RT_TYPE::Current:
            data.pSPTDStruct->Cdb[1] = 1;

            break;
        case EHSSCSI_GET_CONFIGURATION_RT_TYPE::Once:
            data.pSPTDStruct->Cdb[1] = 2;
            break;
        default:
            return 0;
    }


    if ( this->executeRawGeneralCommand( &data ) == false ) {
        return 0;
    }

    if ( data.result.DeviceIOControlResult == FALSE ) {
        return 0;
    }

    size_t size = ( headerOnly[0] << 24 ) | ( headerOnly[1] << 16 ) | ( headerOnly[2] << 8 ) | ( headerOnly[3] ) + 4;


    HSSCSI_SPTD_ResponseRawData resp( new HSSCSI_SPTD_ResponseRawDataType[size] );

    data.pSPTDStruct->DataBuffer = resp.get( );
    data.pSPTDStruct->DataTransferLength = static_cast<ULONG>( size );

    data.pSPTDStruct->Cdb[7] = ( size & 0xFF00 ) >> 8;
    data.pSPTDStruct->Cdb[8] = size & 0xFF;

    if ( this->executeRawGeneralCommand( &data ) == false ) {
        return 0;
    }

    if ( data.result.DeviceIOControlResult == FALSE ) {
        return 0;
    }


    size = ( resp[0] << 24 ) | ( resp[1] << 16 ) | ( resp[2] << 8 ) | ( resp[3] ) + 4;

    *pRawResponseData = std::move( resp );

    if ( pDetailResult ) {
        *pDetailResult = data.result;
        pDetailResult->resultSize = 0;
    }

    return  size;
}



CHSOpticalDriveGetConfigCmd::CHSOpticalDriveGetConfigCmd( ) {
	this->setDrive( nullptr );

}

CHSOpticalDriveGetConfigCmd::CHSOpticalDriveGetConfigCmd( CHSOpticalDrive* pDrive ) {
	this->setDrive( pDrive );
}

void CHSOpticalDriveGetConfigCmd::setDrive( CHSOpticalDrive* pDrive ) {
	this->mpDrive = pDrive;
}


size_t CHSOpticalDriveGetConfigCmd::execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE type, 
	uint16_t startFeatureNumber, THSSCSI_FeatureInfo* pInfo, HSSCSI_SPTD_RESULT* pDetailResult ) const{


    if ( pInfo == nullptr ) return 0;

    size_t result_size = this->executeRawGetConfigCmd( type, startFeatureNumber, &pInfo->raw, pDetailResult );

    if ( result_size == 0 )return 0;

    pInfo->rawSize = result_size;

    uint8_t* pTop = pInfo->raw.get( );
    uint8_t* pFirstDesc = pTop + sizeof( THSSCSI_FeatureHeader );
    pInfo->pHeader = reinterpret_cast<THSSCSI_FeatureHeader*>( pTop );
    pInfo->Descriptors.clear( );

    size_t desc_size = pInfo->rawSize - sizeof( THSSCSI_FeatureHeader );
    size_t offset = 0;

    THSSCSI_FeatureDescriptorInfo desc;
    while ( offset < desc_size ) {
        desc.pHeader = reinterpret_cast<THSSCSI_FeatureDescriptorHeader*>( pFirstDesc + offset );
        desc.pHeader->FeatureCode = HSSCSI_InverseEndian16( desc.pHeader->FeatureCode );
        if ( desc.pHeader->AdditionalLength != 0 ) {
            desc.pAdditionalData = pFirstDesc + offset + sizeof( THSSCSI_FeatureDescriptorHeader );
        } else {
            desc.pAdditionalData = nullptr;
        }
        pInfo->Descriptors.push_back( desc );
        offset += sizeof( THSSCSI_FeatureDescriptorHeader ) + desc.pHeader->AdditionalLength;
    }

    return pInfo->Descriptors.size( );
}

bool CHSOpticalDriveGetConfigCmd::getSupportProfileNumbers( HSSCSI_ProfilesNumberVector* pProfiles ) const {
    if ( pProfiles == nullptr ) return false;

    THSSCSI_FeatureInfo info;
    size_t size = this->execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE::Once, 0, &info );

    if ( size > 0 ) {
        pProfiles->clear( );

        THSSCSI_ProfileDescriptor* pProf;
        size_t  profDescNums;
        for ( auto desc : info.Descriptors ) {

            if ( desc.pHeader->FeatureCode != 0 )continue;

            pProf = reinterpret_cast<THSSCSI_ProfileDescriptor*>( desc.pAdditionalData );
            profDescNums = desc.pHeader->AdditionalLength / sizeof( THSSCSI_ProfileDescriptor );
            for ( size_t i = 0; i < profDescNums; i++ ) {
                pProfiles->push_back( ( ( pProf + i )->ProfileNumber[0] << 8 ) | ( ( pProf + i )->ProfileNumber[1] ) );
            }

        }

        std::sort( pProfiles->begin( ), pProfiles->end( ) );

        return true;
    }
    return false;
}

bool CHSOpticalDriveGetConfigCmd::getSupportProfileNames( HSSCSI_ProfilesNameVector* pProfiles ) const{
    if ( pProfiles == nullptr ) return false;
    HSSCSI_ProfilesNumberVector  pnv;

    if ( this->getSupportProfileNumbers( &pnv ) ) {
        pProfiles->clear( );
        EHSSCSI_ProfileName name;
        for ( auto current : pnv ) {
            name = this->GetProfileName( current );
            if ( name != EHSSCSI_ProfileName::Unknown ) {
                pProfiles->push_back( name );
            }
        }
        return true;
    }
    return false;
}

bool CHSOpticalDriveGetConfigCmd::getSupportProfiles( HSSCSI_Profiles* pProfiles, bool bIncludeUnknown ) const{
    if ( pProfiles == nullptr ) return false;
    HSSCSI_ProfilesNumberVector  pnv;
    if ( this->getSupportProfileNumbers( &pnv) ) {
        pProfiles->clear( );
        EHSSCSI_ProfileName name;
        for ( auto current : pnv ) {
            name = this->GetProfileName( current );
            if ( bIncludeUnknown ||( name != EHSSCSI_ProfileName::Unknown) ) {
                pProfiles->push_back( std::make_pair( current , name ) );
            }

        }
        return true;
    }
    return false;
}

EHSSCSI_ProfileName CHSOpticalDriveGetConfigCmd::GetProfileName( uint16_t profileNumber ) {

    switch ( profileNumber & 0xFFFF ) {
        case 0x0002:
            return EHSSCSI_ProfileName::RemovableDisk;

            //CD系メディア
        case 0x0008:
            return EHSSCSI_ProfileName::CD_ROM;
        case 0x0009:
            return EHSSCSI_ProfileName::CD_R;
        case 0x000A:
            return EHSSCSI_ProfileName::CD_RW;

            //DVD-系メディア
        case 0x0010:
            return EHSSCSI_ProfileName::DVD_ROM;
        case 0x0011:
            return EHSSCSI_ProfileName::DVD_R;
        case 0x0012:
            return EHSSCSI_ProfileName::DVD_RAM;
        case 0x0013:
            return EHSSCSI_ProfileName::DVD_RW_Restricted;
        case 0x0014:
            return EHSSCSI_ProfileName::DVD_RW_Sequential;
        case 0x0015:
            return EHSSCSI_ProfileName::DVD_R_DL_Sequential;
        case 0x0016:
            return EHSSCSI_ProfileName::DVD_R_DL_Jump;

            //DVD+系メディア
        case 0x001A:
            return EHSSCSI_ProfileName::DVD_Plus_RW;
        case 0x001B:
            return EHSSCSI_ProfileName::DVD_Plus_R;
        case 0x002B:
            return EHSSCSI_ProfileName::DVD_Plus_R_DL;

            //BD系メディア
        case 0x0040:
            return EHSSCSI_ProfileName::BD_ROM;
        case 0x0041:
            return EHSSCSI_ProfileName::BD_R_SRM;
        case 0x0042:
            return EHSSCSI_ProfileName::BD_R_RRM;
        case 0x0043:
            return EHSSCSI_ProfileName::BD_RE;

        default:
            return EHSSCSI_ProfileName::Unknown;
    }
}

std::string CHSOpticalDriveGetConfigCmd::GetProfileNameString( uint16_t profileNumber , bool groupOfSameType ) {
    EHSSCSI_ProfileName pn = CHSOpticalDriveGetConfigCmd::GetProfileName( profileNumber );
    return CHSOpticalDriveGetConfigCmd::GetProfileNameString( pn , groupOfSameType );
}

std::string CHSOpticalDriveGetConfigCmd::GetProfileNameString( EHSSCSI_ProfileName profileName, bool groupOfSameType ) {
    switch ( profileName ) {
        case EHSSCSI_ProfileName::RemovableDisk:
            return std::string( "Removable Disk" );

            //CD系メディア
        case EHSSCSI_ProfileName::CD_ROM:
            return std::string( "CD-ROM" );
        case EHSSCSI_ProfileName::CD_R:
            return std::string( "CD-R" );
        case EHSSCSI_ProfileName::CD_RW:
            return std::string( "CD-RW" );

            //DVD-系メディア
        case EHSSCSI_ProfileName::DVD_ROM:
            return std::string( "DVD-ROM" );
        case EHSSCSI_ProfileName::DVD_R:
            return std::string( "DVD-R" );
        case EHSSCSI_ProfileName::DVD_R_DL_Jump:
            return (groupOfSameType) ? std::string( "DVD-R DL" ) :  std::string( "DVD-R DL [Jump Recording]" );
        case EHSSCSI_ProfileName::DVD_R_DL_Sequential:
            return (groupOfSameType) ? std::string( "DVD-R DL" ) :  std::string( "DVD-R DL [Sequential Recording]" );
        case EHSSCSI_ProfileName::DVD_RW_Restricted:
            return (groupOfSameType) ? std::string( "DVD-RW" ) :  std::string( "DVD-RW [Restricted Overwrite]" );
        case EHSSCSI_ProfileName::DVD_RW_Sequential:
            return (groupOfSameType) ? std::string( "DVD-RW" ) :  std::string( "DVD-RW [Sequential Recording]" );

        case EHSSCSI_ProfileName::DVD_RAM:
            return std::string( "DVD-RAM" );

            //DVD+系メディア
        case EHSSCSI_ProfileName::DVD_Plus_R:
            return std::string( "DVD+R" );
        case EHSSCSI_ProfileName::DVD_Plus_R_DL:
            return std::string( "DVD+R DL" );
        case EHSSCSI_ProfileName::DVD_Plus_RW:
            return std::string( "DVD+RW" );

            //BD系メディア
        case EHSSCSI_ProfileName::BD_ROM:
            return std::string( "BD-ROM" );
        case EHSSCSI_ProfileName::BD_R_SRM:
            return (groupOfSameType) ? std::string( "BD-R" ) :  std::string( "BD-R [Sequential Recording Mode]" );
        case EHSSCSI_ProfileName::BD_R_RRM:
            return (groupOfSameType) ? std::string( "BD-R" ) :  std::string( "BD-R [Random Recording Mode]" );
        case EHSSCSI_ProfileName::BD_RE:
            return std::string( "BD-RE" );

    }
    return std::string( "Unknown" );
}


bool CHSOpticalDriveGetConfigCmd::getCurrentProfileNumber( uint16_t* pCurrentProfile ) const {
    if ( pCurrentProfile == nullptr ) return false;

    THSSCSI_CommandData data;
    if ( !HSSCSI_InitializeCommandData( &data ) )return false;

    THSSCSI_FeatureHeader headerOnly;

    memset( &headerOnly, 0, sizeof( THSSCSI_FeatureHeader ) );

    data.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
    data.pSPTDStruct->DataBuffer = &headerOnly;
    data.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( THSSCSI_FeatureHeader ) );
    data.pSPTDStruct->CdbLength = 10;

    data.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_GET_CONFIGURATION;
    data.pSPTDStruct->Cdb[2] = 0;
    data.pSPTDStruct->Cdb[3] = 0;
    data.pSPTDStruct->Cdb[8] = static_cast<uint8_t>( sizeof( THSSCSI_FeatureHeader ) );
    data.pSPTDStruct->Cdb[1] = 2;

    if ( this->executeRawGeneralCommand( &data ) == false ) {
        return false;
    }

    if ( data.result.DeviceIOControlResult == FALSE ) {
        return false;
    }
    size_t size = ( headerOnly.DataLength[0] << 24 ) | ( headerOnly.DataLength[1] << 16 );
    size |= ( headerOnly.DataLength[2] << 8 ) | headerOnly.DataLength[3];
    size += 4;
    if ( size >= 8 ) {
        *pCurrentProfile = ( headerOnly.CurrentProfile[0] << 8 ) | ( headerOnly.CurrentProfile[1] );
        return true;
    }

    return false;
}

bool CHSOpticalDriveGetConfigCmd::getCurrentProfile( HSSCSI_ProfilesItem* pCurrentProfileItem ) const {

    if ( pCurrentProfileItem == nullptr ) return false;

    if ( this->getCurrentProfileNumber( &pCurrentProfileItem->first ) ) {
        pCurrentProfileItem->second = GetProfileName( pCurrentProfileItem->first );
        return true;
    }

    return false;
}



EHSSCSI_ProfileName CHSOpticalDriveGetConfigCmd::getCurrentProfileName( void ) const {
    uint16_t pn;
    if ( this->getCurrentProfileNumber( &pn ) ) {
        return this->GetProfileName( pn );
    }
    return EHSSCSI_ProfileName::Unknown;
}

EHSSCSI_ProfileFamilyName CHSOpticalDriveGetConfigCmd::getCurrentProfileFamilyName( void ) const {
    return this->GetProfileFamilyName( this->getCurrentProfileName( ) );
}

std::string CHSOpticalDriveGetConfigCmd::getCurrentProfileFamilyNameString( void ) const {
    return this->GetProfileFamilyNameString(this->getCurrentProfileFamilyName());
}

EHSSCSI_ProfileFamilyName CHSOpticalDriveGetConfigCmd::GetProfileFamilyName( EHSSCSI_ProfileName profileName ) {
    switch ( profileName ) {
        case EHSSCSI_ProfileName::CD_ROM:
        case EHSSCSI_ProfileName::CD_R:
        case EHSSCSI_ProfileName::CD_RW:
            return EHSSCSI_ProfileFamilyName::CD;

        case EHSSCSI_ProfileName::DVD_ROM:
        case EHSSCSI_ProfileName::DVD_R:
        case EHSSCSI_ProfileName::DVD_R_DL_Sequential:
        case EHSSCSI_ProfileName::DVD_R_DL_Jump:
        case EHSSCSI_ProfileName::DVD_RW_Restricted:
        case EHSSCSI_ProfileName::DVD_RW_Sequential:
        case EHSSCSI_ProfileName::DVD_RAM:
        case EHSSCSI_ProfileName::DVD_Plus_R:
        case EHSSCSI_ProfileName::DVD_Plus_R_DL:
        case EHSSCSI_ProfileName::DVD_Plus_RW:
            return EHSSCSI_ProfileFamilyName::DVD;

        case EHSSCSI_ProfileName::BD_ROM:
        case EHSSCSI_ProfileName::BD_R_SRM:
        case EHSSCSI_ProfileName::BD_R_RRM:
        case EHSSCSI_ProfileName::BD_RE:
            return EHSSCSI_ProfileFamilyName::BD;
    }
    return EHSSCSI_ProfileFamilyName::Unknown;

}

std::string CHSOpticalDriveGetConfigCmd::GetProfileFamilyNameString( EHSSCSI_ProfileFamilyName profileFamily ) {

    switch ( profileFamily ) {

        case EHSSCSI_ProfileFamilyName::CD:
            return std::string( "CD" );
        case EHSSCSI_ProfileFamilyName::DVD:
            return std::string( "DVD" );
        case EHSSCSI_ProfileFamilyName::BD:
            return std::string( "BD" );
    }

    return std::string( "Unknown" );
}

std::string CHSOpticalDriveGetConfigCmd::GetProfileFamilyNameString( EHSSCSI_ProfileName profileName ) {
    return GetProfileFamilyNameString( GetProfileFamilyName( profileName ) );
}

bool CHSOpticalDriveGetConfigCmd::getFeatureCDRead( THSSCSI_FeatureDescriptor_CDRead* pDesc ) const {
    return this->getGeneralFeatureDescriptor( pDesc, 0x1e );
}

bool CHSOpticalDriveGetConfigCmd::getFeatureRemovableMedium( THSSCSI_FeatureDescriptor_RemovableMedium* pDesc ) const {
    return this->getGeneralFeatureDescriptor( pDesc, 0x03 );
}

bool CHSOpticalDriveGetConfigCmd::getFeatureDriveSerialNumber( THSSCSI_FeatureDescriptor_DriveSerialNumber* pDesc ) const {
    if ( pDesc == nullptr ) return false;
    THSSCSI_FeatureInfo info;
    if ( this->execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE::Once, 0x108, &info ) == 0 )return false;
    
    memset( pDesc->SerialNumber, 0, sizeof( pDesc->SerialNumber ) );
    memcpy( &pDesc->header, info.Descriptors[0].pHeader, sizeof( THSSCSI_FeatureDescriptorHeader ) );
    
    
    uint8_t serialNumberLength = info.Descriptors[0].pHeader->AdditionalLength;

    for ( size_t i = 0; i < serialNumberLength; i++ ) {
        pDesc->SerialNumber[i] = *( info.Descriptors[0].pAdditionalData + i );
    }

    return true;
}

EHSSCSI_ConnectInterfaceName CHSOpticalDriveGetConfigCmd::getPhysicalInterfaceStandardName( void ) const {

    THSSCSI_FeatureDescriptor_Core core;
    if ( !this->getGeneralFeatureDescriptor( &core,0x001)  ) {
        return EHSSCSI_ConnectInterfaceName::Unknown;
    }


    core.PhysicalInterfaceStandard = HSSCSI_InverseEndian32( core.PhysicalInterfaceStandard );

    switch ( core.PhysicalInterfaceStandard ) {
        case 0x001:
            return EHSSCSI_ConnectInterfaceName::SCSI;
        case 0x002:
            return EHSSCSI_ConnectInterfaceName::ATAPI;
        case 0x003:
            return EHSSCSI_ConnectInterfaceName::IEEE1394_1995;
        case 0x004:
            return EHSSCSI_ConnectInterfaceName::IEEE1394A;
        case 0x005:
            return EHSSCSI_ConnectInterfaceName::Fibre_Channel;
        case 0x006:
            return EHSSCSI_ConnectInterfaceName::IEEE1394B;
        case 0x007:
            return EHSSCSI_ConnectInterfaceName::Serial_ATAPI;
        case 0x008:
            return EHSSCSI_ConnectInterfaceName::USB;
        default:
            return EHSSCSI_ConnectInterfaceName::Unknown;
    }
}

std::string CHSOpticalDriveGetConfigCmd::getPhysicalInterfaceStandardNameString( void ) const {
    return HSSCSI_GetConnectInterfaceNameStringByName(this->getPhysicalInterfaceStandardName());
}

