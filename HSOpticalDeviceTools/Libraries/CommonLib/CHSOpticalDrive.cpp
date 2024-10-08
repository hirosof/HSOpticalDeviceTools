#include "CHSOpticalDrive.hpp"

HANDLE CHSOpticalDrive::OpenDrive( char driveLetter ) {

    bool Check = ( driveLetter >= 'A' ) && ( driveLetter <= 'Z' );
    Check |= ( driveLetter >= 'a' ) && ( driveLetter <= 'z' );

    if ( Check ) {
        char FileName[8];
        wsprintfA( FileName, "\\\\.\\%c:", driveLetter );

        return CreateFileA( FileName, 
            GENERIC_READ | GENERIC_WRITE, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, 
            NULL,
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL,
            NULL );
    }

    return INVALID_HANDLE_VALUE;
}

bool CHSOpticalDrive::IsOpticalDrive( const char driveLetter ) {
    bool Check = ( driveLetter >= 'A' ) && ( driveLetter <= 'Z' );
    Check |= ( driveLetter >= 'a' ) && ( driveLetter <= 'z' );
    if ( Check ) {
        char DL[4];
        wsprintfA( DL, "%c:\\", driveLetter );
        return ( GetDriveTypeA( DL ) == DRIVE_CDROM );
    }
    return false;
}

bool CHSOpticalDrive::EnumOpticalDrive( THSEnumrateOpticalDriveInfo* pInfo ) {

    if ( pInfo == nullptr ) return false;

    memset( pInfo, 0, sizeof( THSEnumrateOpticalDriveInfo ) );

    bool bRet;
    pInfo->uOpticalDriveCount = 0;
    for ( char c = 'A'; c <= 'Z'; c++ ) {
        if ( CHSOpticalDrive::IsOpticalDrive( c ) ) {
            bRet = CHSOpticalDrive::GetDeviceInfo( c, &pInfo->Drives[pInfo->uOpticalDriveCount].Info );
            pInfo->Drives[pInfo->uOpticalDriveCount].Letter = c;
            pInfo->Drives[pInfo->uOpticalDriveCount].bIncludedInfo = bRet;
            pInfo->uOpticalDriveCount++;
        }
    }
    return true;

}

bool CHSOpticalDrive::GetFirstOpticalDriveInfo( THSOpticalDriveInfo* pInfo ) {
    if ( pInfo == nullptr ) return false;

    for ( char c = 'A'; c <= 'Z'; c++ ) {
        if ( CHSOpticalDrive::IsOpticalDrive( c ) ) {
            memset( pInfo, 0, sizeof( THSOpticalDriveInfo ) );
            pInfo->Letter = c;
            pInfo->bIncludedInfo = CHSOpticalDrive::GetDeviceInfo( c, &pInfo->Info );
            return true;
        }
    }
    return false;
}

bool CHSOpticalDrive::GetDeviceInfo( const char opticalDriveLetter, THSOpticalDriveDeviceInfo* pInfo ) {

    if ( pInfo == nullptr ) return false;
    if ( CHSOpticalDrive::IsOpticalDrive( opticalDriveLetter ) ) {
        HANDLE hDrive = CHSOpticalDrive::OpenDrive( opticalDriveLetter );

        if ( hDrive != INVALID_HANDLE_VALUE ) {
            bool bRet = CHSOpticalDrive::GetDeviceInfoFromHandle( hDrive, pInfo );
            CloseHandle( hDrive );
            return bRet;
        }
    }
    return false;
}

bool CHSOpticalDrive::GetDeviceInfoFromHandle( HANDLE hOpticalDrive, THSOpticalDriveDeviceInfo* pInfo ) {
    if ( hOpticalDrive == INVALID_HANDLE_VALUE ) return false;
    if ( hOpticalDrive == NULL ) return false;
    if ( pInfo ) {

        INQUIRYDATA data = { 0 };
        DWORD returnSize;
        const char space[2] = { ' ' , '\0' };

        BOOL bRet = DeviceIoControl( hOpticalDrive, IOCTL_CDROM_GET_INQUIRY_DATA, 0, 0,
            &data, sizeof( INQUIRYDATA ), &returnSize, NULL );
        if ( bRet ) {

            memset( pInfo, 0, sizeof( THSOpticalDriveDeviceInfo ) );
            pInfo->raw = data;
            memcpy( pInfo->VendorID, data.VendorId , DHSOpticalDriveVendorIDLength );
            memcpy( pInfo->ProductID, data.ProductId, DHSOpticalDriveProductIDLength );
            memcpy( pInfo->ProductRevisionLevel, data.ProductRevisionLevel, DHSOpticalDriveProductRevisionLevelLength);

            pInfo->DisplayName[0] = '\0';

            lstrcatA( pInfo->DisplayName, pInfo->VendorID );
            if ( pInfo->DisplayName[0] != '\0' ) lstrcatA( pInfo->DisplayName, space );
            lstrcatA( pInfo->DisplayName, pInfo->ProductID );
            if ( pInfo->DisplayName[0] != '\0' ) lstrcatA( pInfo->DisplayName, space );
            lstrcatA( pInfo->DisplayName, pInfo->ProductRevisionLevel );


            return true;
        }
    }
    return false;
}

uint8_t CHSOpticalDrive::CountOfOpticalDrive( void ) {
    uint8_t count = 0;
    for ( char c = 'A'; c <= 'Z'; c++ ) {
        if ( CHSOpticalDrive::IsOpticalDrive( c ) ) {
            count++;
        }
    }
    return count;
}

CHSOpticalDrive::CHSOpticalDrive( void ) {
    this->hDrive = NULL;
    this->cOpenedDriveLetter = '\0';
}

CHSOpticalDrive::~CHSOpticalDrive( ) {
    this->close( );
}

bool CHSOpticalDrive::open( const char opticalDriveLetter ) {
    if ( this->hDrive ) return false;

    if ( CHSOpticalDrive::IsOpticalDrive( opticalDriveLetter ) ) {
        this->hDrive = CHSOpticalDrive::OpenDrive( opticalDriveLetter );
        if ( this->hDrive == INVALID_HANDLE_VALUE ) {
            this->hDrive = NULL;
            return false;
        } else {
            cOpenedDriveLetter = opticalDriveLetter;
        }
        return true;
    }
    return false;
}

bool CHSOpticalDrive::getCurrentDeviceInfo( THSOpticalDriveDeviceInfo* pInfo ) const {
    return CHSOpticalDrive::GetDeviceInfoFromHandle(this->hDrive , pInfo);
}

char CHSOpticalDrive::getCurrentDriveLetter( void ) const{
    return this->cOpenedDriveLetter;
}

bool CHSOpticalDrive::close( void ) {
    if ( this->hDrive ) {
        CloseHandle( this->hDrive );
        this->hDrive = NULL;
        this->cOpenedDriveLetter = '\0';
        return true;
    }
    return false;
}

HANDLE CHSOpticalDrive::getHandle( void ) const {
    return this->hDrive;
}

bool CHSOpticalDrive::isOpened( void ) const {
    return ( this->hDrive != NULL );
}

bool CHSOpticalDrive::executeCommand( THSSCSI_CommandData* pData) const {

    if ( this->isOpened( ) == false ) return false;
    if ( pData == nullptr ) return false;
    if ( pData->raw.get( ) == nullptr ) return false;

    DWORD returnSize;
    DWORD lastError = 0;

    BOOL bret = DeviceIoControl( this->getHandle( ),
        IOCTL_SCSI_PASS_THROUGH_DIRECT,
        pData->pSPTDStruct, static_cast<DWORD>( pData->allocatedRawSize ),
        pData->pSPTDStruct, static_cast<DWORD>( pData->allocatedRawSize ),
        &returnSize, NULL );

    if ( bret == FALSE ) lastError = GetLastError( );

    memset( &pData->result, 0, sizeof( HSSCSI_SPTD_RESULT ) );

    HSSCSI_SPTD_RESULT* pResult = &pData->result;
  
    pResult->DeviceIOControlResult = bret;
    pResult->DeviceIOControlLastError = lastError;
    pResult->executedOperationCode = pData->pSPTDStruct->Cdb[0];
    pResult->isValidSCSIResultFields = false;
    if ( bret ) {
        pResult->resultSize = returnSize;
        pResult->scsiStatus.rawValue = pData->pSPTDStruct->ScsiStatus;

        pResult->scsiSK = 0;
        pResult->scsiASC = 0;
        pResult->scsiASCQ = 0;

        pResult->isValidSCSIResultFields = true;

        if ( pData->senseData.pRaw != nullptr ) {
            uint8_t scene_rc = pData->senseData.pResponseCodeOnly->responseCode;
            if ( ( scene_rc == 0x72 ) || ( scene_rc == 0x73 ) ) {
                pResult->scsiSK = pData->senseData.pDesc->senseKey;
                pResult->scsiASC = pData->senseData.pDesc->additionalSenseCode;
                pResult->scsiASCQ = pData->senseData.pDesc->additionalSenseCodeQualifier;
            } else if ( ( scene_rc == 0x70 ) || ( scene_rc == 0x71 ) ) {
                pResult->scsiSK = pData->senseData.pFixed->senseKey;
                pResult->scsiASC = pData->senseData.pFixed->additionalSenseCode;
                pResult->scsiASCQ = pData->senseData.pFixed->additionalSenseCodeQualifier;
            } 
        }


    }

    return true;
}

bool CHSOpticalDrive::isReady( void )   const {
    return this->checkReady(nullptr) == EHSSCSI_ReadyStatus::Ready;
}

EHSSCSI_ReadyStatus CHSOpticalDrive::checkReady( HSSCSI_SPTD_RESULT* pDetailResult )  const {

    THSSCSI_CommandData params;
    
    HSSCSI_InitializeCommandData( &params );

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;

    params.pSPTDStruct->CdbLength = 6;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_TEST_UNIT_READY;

    if ( this->executeCommand( &params ) == false ) {
        return EHSSCSI_ReadyStatus::FailedGotStatus;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) {
        return EHSSCSI_ReadyStatus::FailedGotStatus;
    }

    if ( HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good ) {
        return EHSSCSI_ReadyStatus::Ready;
    } else if ( HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::CheckCondition ) {
        if ( params.result.scsiSK == static_cast<uint8_t>( EHSSCSI_SenseKey::NOT_READY ) ) {
            if ( params.result.scsiASC == 0x3A ) {
                return EHSSCSI_ReadyStatus::MediumNotPresent;
            }
        }
    }
    return EHSSCSI_ReadyStatus::NotReady;
}

bool CHSOpticalDrive::isMediaPresent( void ) const {
    THSSCSI_MediaEventStatus mes;

    if ( this->getMediaEventStatus( &mes, nullptr ) ) {
        return  mes.MediaPresent;
    }

    return false;
}

bool CHSOpticalDrive::ejectMedia( bool asyncWork ) const {
    return this->trayOpen(nullptr , asyncWork);
}

bool CHSOpticalDrive::loadMedia( bool asyncWork ) const {
    return this->trayClose( nullptr, asyncWork );
}

bool CHSOpticalDrive::trayOpen( HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {

    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
    params.pSPTDStruct->TimeOutValue = 60;

    params.pSPTDStruct->CdbLength = 6;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_START_STOP_UNIT;
    params.pSPTDStruct->Cdb[1] = ( asyncWork ) ? 0x1 : 0x0;
    params.pSPTDStruct->Cdb[4] = 0x02;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) return false;

    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

bool CHSOpticalDrive::trayClose( HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {
    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
    params.pSPTDStruct->TimeOutValue = 60;
    params.pSPTDStruct->CdbLength = 6;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_START_STOP_UNIT;
    params.pSPTDStruct->Cdb[1] = ( asyncWork ) ? 0x1 : 0x0;
    params.pSPTDStruct->Cdb[4] = 0x03;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) return false;

    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

bool CHSOpticalDrive::trayLock( HSSCSI_SPTD_RESULT* pDetailResult ) const {
    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
    params.pSPTDStruct->CdbLength = 6;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_PREVENT_ALLOW_MEDIUM_REMOVAL;
    params.pSPTDStruct->Cdb[4] = 1;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) return false;

    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

bool CHSOpticalDrive::trayUnlock( HSSCSI_SPTD_RESULT* pDetailResult ) const {
    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
    params.pSPTDStruct->CdbLength = 6;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_PREVENT_ALLOW_MEDIUM_REMOVAL;
    params.pSPTDStruct->Cdb[4] = 0;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) return false;

    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}


bool CHSOpticalDrive::isTrayOpened( void ) const {
    return this->checkTrayState() == EHSOD_TrayState::Opened;
}


EHSOD_TrayState CHSOpticalDrive::checkTrayState( void ) const {

    THSSCSI_MechanismStatus ms;
    if ( this->getMechanismStatus( &ms, nullptr ) ) {
        return ( ms.DoorOpen ) ? EHSOD_TrayState::Opened : EHSOD_TrayState::Closed;
    }

    return EHSOD_TrayState::FailedGotStatus;
}

bool CHSOpticalDrive::spinUp( HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {
    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
    
    params.pSPTDStruct->CdbLength = 6;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_START_STOP_UNIT;
    params.pSPTDStruct->Cdb[1] = ( asyncWork ) ? 0x1 : 0x0;
    params.pSPTDStruct->Cdb[4] = 0x01;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) return false;

    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

bool CHSOpticalDrive::spinDown( HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {
    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;

    params.pSPTDStruct->CdbLength = 6;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_START_STOP_UNIT;
    params.pSPTDStruct->Cdb[1] = ( asyncWork ) ? 0x1 : 0x0;
    params.pSPTDStruct->Cdb[4] = 0x00;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) return false;

    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

bool CHSOpticalDrive::setPowerState( uint8_t condition, HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {
    THSSCSI_CommandData params;


    HSSCSI_InitializeCommandData( &params );

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;

    params.pSPTDStruct->CdbLength = 6;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_START_STOP_UNIT;
    params.pSPTDStruct->Cdb[1] = ( asyncWork ) ? 0x1 : 0x0;
    params.pSPTDStruct->Cdb[4] = (condition & 0xF) << 4;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) return false;

    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

EHSOD_AlimentMaskType CHSOpticalDrive::getAlimentMask( ULONG* pRawAlimentMask ) const {

    if ( this->isOpened( ) == false ) return EHSOD_AlimentMaskType::FailedGotAliment;

    STORAGE_ADAPTER_DESCRIPTOR sad;
    STORAGE_PROPERTY_QUERY query;

    memset( &query, 0, sizeof( STORAGE_PROPERTY_QUERY ) );
    memset( &sad, 0, sizeof( STORAGE_ADAPTER_DESCRIPTOR ) );

    query.PropertyId = StorageAdapterProperty;
    query.QueryType = PropertyStandardQuery;

    DWORD returnSize;
    BOOL bRet = DeviceIoControl( this->getHandle( ), IOCTL_STORAGE_QUERY_PROPERTY,
        &query, static_cast<DWORD>( sizeof( STORAGE_PROPERTY_QUERY ) ),
        &sad, static_cast<DWORD>( sizeof( STORAGE_ADAPTER_DESCRIPTOR ) ),
        &returnSize, nullptr );
    if ( bRet ) {
        if ( pRawAlimentMask ) *pRawAlimentMask = sad.AlignmentMask;
        switch ( sad.AlignmentMask) {
            case 0:
                return EHSOD_AlimentMaskType::ByteAliment;
            case 1:
                return EHSOD_AlimentMaskType::WordAliment;
            case 3:
                return EHSOD_AlimentMaskType::DwordAliment;
            case 7:
                return EHSOD_AlimentMaskType::DoubleDwordAliment;
        }
        return EHSOD_AlimentMaskType::UnknownAliment;
    }
    return EHSOD_AlimentMaskType::FailedGotAliment;
}

bool CHSOpticalDrive::getMaxTransferLength( DWORD* pMaxTransferLength ) const {
    if ( this->isOpened( ) == false ) return false;
    if ( pMaxTransferLength == nullptr ) return false;

    STORAGE_ADAPTER_DESCRIPTOR sad;
    STORAGE_PROPERTY_QUERY query;

    memset( &query, 0, sizeof( STORAGE_PROPERTY_QUERY ) );
    memset( &sad, 0, sizeof( STORAGE_ADAPTER_DESCRIPTOR ) );

    query.PropertyId = StorageAdapterProperty;
    query.QueryType = PropertyStandardQuery;

    DWORD returnSize;
    BOOL bRet = DeviceIoControl( this->getHandle( ), IOCTL_STORAGE_QUERY_PROPERTY,
        &query, static_cast<DWORD>( sizeof( STORAGE_PROPERTY_QUERY ) ),
        &sad, static_cast<DWORD>( sizeof( STORAGE_ADAPTER_DESCRIPTOR ) ),
        &returnSize, nullptr );


    if ( bRet ) {
        *pMaxTransferLength = sad.MaximumTransferLength;
       
        return true;
    }

    return false;
}

EHSSCSI_ConnectInterfaceName CHSOpticalDrive::getBusType( STORAGE_BUS_TYPE* pRawBusType ) const {

    if ( this->isOpened( ) == false ) return EHSSCSI_ConnectInterfaceName::Unknown;

    STORAGE_ADAPTER_DESCRIPTOR sad;
    STORAGE_PROPERTY_QUERY query;

    memset( &query, 0, sizeof( STORAGE_PROPERTY_QUERY ) );
    memset( &sad, 0, sizeof( STORAGE_ADAPTER_DESCRIPTOR ) );

    query.PropertyId = StorageAdapterProperty;
    query.QueryType = PropertyStandardQuery;

    DWORD returnSize;
    BOOL bRet = DeviceIoControl( this->getHandle( ), IOCTL_STORAGE_QUERY_PROPERTY,
        &query, static_cast<DWORD>( sizeof( STORAGE_PROPERTY_QUERY ) ),
        &sad, static_cast<DWORD>( sizeof( STORAGE_ADAPTER_DESCRIPTOR ) ),
        &returnSize, nullptr );


    if ( bRet ) {

        STORAGE_BUS_TYPE busType =  static_cast<STORAGE_BUS_TYPE >(sad.BusType);
        if ( pRawBusType ) *pRawBusType = busType;

        switch ( busType ) {
            case BusTypeScsi:
                return EHSSCSI_ConnectInterfaceName::SCSI;
            case BusTypeAtapi:
                return EHSSCSI_ConnectInterfaceName::ATAPI;
            case BusType1394:
                return EHSSCSI_ConnectInterfaceName::IEEE1394_1995;
            case BusTypeFibre:
                return EHSSCSI_ConnectInterfaceName::Fibre_Channel;
            case BusTypeSata:
                return EHSSCSI_ConnectInterfaceName::Serial_ATAPI;
            case BusTypeUsb:
                return EHSSCSI_ConnectInterfaceName::USB;
            default:
                return EHSSCSI_ConnectInterfaceName::Unknown;
        }

    }

    return EHSSCSI_ConnectInterfaceName::Unknown;
}

bool CHSOpticalDrive::getMechanismStatus( THSSCSI_MechanismStatus* pStatus, HSSCSI_SPTD_RESULT* pDetailResult ) const{

    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    THSSCSI_MechanismStatus  ms;

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
    params.pSPTDStruct->DataBuffer = &ms;
    params.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( ms ) );

    params.pSPTDStruct->CdbLength = 12;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_MECHANISM_STATUS;

    params.pSPTDStruct->Cdb[8] = ( sizeof( ms )  & 0xFF00 ) >> 8;
    params.pSPTDStruct->Cdb[9] = sizeof( ms ) & 0x00FF;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult ) {
        *pDetailResult = params.result;
         pDetailResult->resultSize = 0;
   }

    if ( HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good ) {
        *pStatus = ms;
    }

    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

bool CHSOpticalDrive::getMediaEventStatus( THSSCSI_MediaEventStatus* pStatus, HSSCSI_SPTD_RESULT* pDetailResult ) const{
    if ( pStatus == nullptr )return false;

    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    THSSCSI_MediaEventStatus  mes = { 0 };

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
    params.pSPTDStruct->DataBuffer = &mes;
    params.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( mes ) );

    params.pSPTDStruct->CdbLength = 10;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_GET_EVENT_STATUS_NOTIFICATION;
    params.pSPTDStruct->Cdb[1] = 1;
    params.pSPTDStruct->Cdb[4] = 0x10;
    params.pSPTDStruct->Cdb[7] = ( sizeof( mes ) & 0xFF00 ) >> 8;
    params.pSPTDStruct->Cdb[8] = sizeof( mes ) & 0x00FF;

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( pDetailResult ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good ) {
        *pStatus = mes;
    }


    return HSSCSIStatusToSCSIEnumStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}


