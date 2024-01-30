#include "CHSOpticalDriveGetConfigCmd.hpp"

bool CHSOpticalDriveGetConfigCmd::executeRawGeneralCommand( THSSCSI_CommandData* pData ) const {
    if ( this->mpDrive ) return this->mpDrive->executeCommand( pData );
    return false;
}


size_t CHSOpticalDriveGetConfigCmd::executeRawGetConfigCmd( EHSSCSI_GET_CONFIGURATION_RT_TYPE type,
    uint16_t startFeatureNumber, HSSCSI_SPTD_ResponseRawData* pRawResponseData, HSSCSI_SPTD_RESULT* pDetailResult ) {

    if ( pRawResponseData == nullptr ) return 0;

    THSSCSI_CommandData data;
    if ( !HSSCSI_Alloc_CommandData( &data ) )return 0;

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

    data.pSPTDStruct->Cdb[7] = ( size % 0xFF00 ) >> 8;
    data.pSPTDStruct->Cdb[8] = size % 0xFF;

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
	this->SetDrive( nullptr );

}

CHSOpticalDriveGetConfigCmd::CHSOpticalDriveGetConfigCmd( CHSOpticalDrive* pDrive ) {
	this->SetDrive( pDrive );
}

void CHSOpticalDriveGetConfigCmd::SetDrive( CHSOpticalDrive* pDrive ) {
	this->mpDrive = pDrive;
}


size_t CHSOpticalDriveGetConfigCmd::execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE type, 
	uint16_t startFeatureNumber, THSSCSI_FeatureInfo* pInfo, HSSCSI_SPTD_RESULT* pDetailResult ) {


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

bool CHSOpticalDriveGetConfigCmd::getCurrentProfileNumber( uint16_t* pCurrentProfile ) {
    if ( pCurrentProfile == nullptr ) return false;

    THSSCSI_CommandData data;
    if ( !HSSCSI_Alloc_CommandData( &data ) )return false;

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
        return 0;
    }

    if ( data.result.DeviceIOControlResult == FALSE ) {
        return 0;
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

bool CHSOpticalDriveGetConfigCmd::getSupportProfileNumbers( HSSCSI_ProfilesNumberVector* pProfiles ) {
    if ( pProfiles == nullptr ) return false;

    THSSCSI_FeatureInfo info;
    size_t size = this->execute( EHSSCSI_GET_CONFIGURATION_RT_TYPE::Once, 0, &info );

    if ( size > 0 ) {
        pProfiles->clear( );

        THSSCSI_ProfileDescriptor* pProf;
        size_t  profDescNums;
        for ( auto desc : info.Descriptors ) {

            if ( desc.pHeader->FeatureCode[0] != 0 )continue;
            if ( desc.pHeader->FeatureCode[1] != 0 )continue;

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

bool CHSOpticalDriveGetConfigCmd::getSupportProfileNames( HSSCSI_ProfilesNameVector* pProfiles ) {
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

bool CHSOpticalDriveGetConfigCmd::getSupportProfiles( HSSCSI_Profiles* pProfiles, bool bIncludeUnknown ) {
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

std::string CHSOpticalDriveGetConfigCmd::GetProfileNameString( uint16_t profileNumber ) {
    EHSSCSI_ProfileName pn = CHSOpticalDriveGetConfigCmd::GetProfileName( profileNumber );
    return CHSOpticalDriveGetConfigCmd::GetProfileNameString( pn );
}

std::string CHSOpticalDriveGetConfigCmd::GetProfileNameString( EHSSCSI_ProfileName profileName ) {
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
            return std::string( "DVD-R DL [Jump]" );
        case EHSSCSI_ProfileName::DVD_R_DL_Sequential:
            return std::string( "DVD-R DL [Sequential]" );
        case EHSSCSI_ProfileName::DVD_RW_Restricted:
            return std::string( "DVD-RW [Restricted]" );
        case EHSSCSI_ProfileName::DVD_RW_Sequential:
            return std::string( "DVD-RW [Sequential]" );
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
            return std::string( "BD-R [SRM]" );
        case EHSSCSI_ProfileName::BD_R_RRM:
            return std::string( "BD-R [RRM]" );
        case EHSSCSI_ProfileName::BD_RE:
            return std::string( "BD-RE" );

    }
    return std::string( "Unknown" );
}
