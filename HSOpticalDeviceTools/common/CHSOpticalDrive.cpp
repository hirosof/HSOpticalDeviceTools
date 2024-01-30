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
            memcpy( pInfo->VendorID, data.VendorId , DHSOpticalDriveVendorIDLength );
            memcpy( pInfo->ProductID, data.ProductId, DHSOpticalDriveProductIDLength );
            memcpy( pInfo->ProductRevisionLevel, data.ProductRevisionLevel, DHSOpticalDriveProductRevisionLevelLength);


            pInfo->DeviceName[0] = '\0';

            lstrcatA( pInfo->DeviceName, pInfo->VendorID );
            if ( pInfo->DeviceName[0] != '\0' ) lstrcatA( pInfo->DeviceName, space );
            lstrcatA( pInfo->DeviceName, pInfo->ProductID );
            if ( pInfo->DeviceName[0] != '\0' ) lstrcatA( pInfo->DeviceName, space );
            lstrcatA( pInfo->DeviceName, pInfo->ProductRevisionLevel );


            return true;
        }
    }
    return false;
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

    if ( bret ) {
        pResult->resultSize = returnSize;
        pResult->scsiStatus.rawValue = pData->pSPTDStruct->ScsiStatus;

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
            } else {
                pResult->scsiSK = 0;
                pResult->scsiASC = 0;
                pResult->scsiASCQ = 0;
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
    
    HSSCSI_Alloc_CommandData( &params );

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

    if ( HSSCSIStatusToStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good ) {
        return EHSSCSI_ReadyStatus::Ready;
    } else if ( HSSCSIStatusToStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::CheckCondition ) {
        if ( params.result.scsiSK == static_cast<uint8_t>( EHSSCSI_SenseKey::NOT_READY ) ) {
            if ( params.result.scsiASC == 0x3A ) {
                return EHSSCSI_ReadyStatus::MediumNotPresent;
            }
        }
    }
    return EHSSCSI_ReadyStatus::NotReady;
}

bool CHSOpticalDrive::ejectMedia( bool asyncWork ) const {
    return this->trayOpen(nullptr , asyncWork);
}

bool CHSOpticalDrive::loadMedia( bool asyncWork ) const {
    return this->trayClose( nullptr, asyncWork );
}

bool CHSOpticalDrive::trayOpen( HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {

    THSSCSI_CommandData params;

    HSSCSI_Alloc_CommandData( &params );

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

    if ( params.result.DeviceIOControlResult == FALSE ) return FALSE;

    return HSSCSIStatusToStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

bool CHSOpticalDrive::trayClose( HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {
    THSSCSI_CommandData params;

    HSSCSI_Alloc_CommandData( &params );

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

    if ( params.result.DeviceIOControlResult == FALSE ) return FALSE;

    return HSSCSIStatusToStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}



bool CHSOpticalDrive::isTrayOpened( void ) const {
    return this->checkTrayState( nullptr ) == EHSOD_TrayState::Opened;
}

EHSOD_TrayState CHSOpticalDrive::checkTrayState( HSSCSI_SPTD_RESULT* pDetailResult ) const {
    THSSCSI_CommandData params;

    HSSCSI_Alloc_CommandData( &params );

    uint8_t  msHeader[8] = { 0 }; //MECHANISM STATUS Header

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
    params.pSPTDStruct->DataBuffer = msHeader;
    params.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( msHeader ) );
    
    params.pSPTDStruct->CdbLength = 12;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_MECHANISM_STATUS;
    params.pSPTDStruct->Cdb[9] = static_cast<uint8_t>( sizeof( msHeader ) );

    if ( this->executeCommand( &params ) == false ) {
        return EHSOD_TrayState::FailedGotStatus;
    }

    if ( pDetailResult != nullptr ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult ) {
        if ( HSSCSIStatusToStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good ) {            
            return ( ( msHeader[1] & 0x10 ) == 0x10 ) ? EHSOD_TrayState::Opened : EHSOD_TrayState::Closed;
        }
    }

    return EHSOD_TrayState::FailedGotStatus;
}

bool CHSOpticalDrive::spinUp( HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {
    THSSCSI_CommandData params;


    HSSCSI_Alloc_CommandData( &params );

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

    if ( params.result.DeviceIOControlResult == FALSE ) return FALSE;

    return HSSCSIStatusToStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}

bool CHSOpticalDrive::spinDown( HSSCSI_SPTD_RESULT* pDetailResult, bool asyncWork ) const {
    THSSCSI_CommandData params;

    HSSCSI_Alloc_CommandData( &params );

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

    if ( params.result.DeviceIOControlResult == FALSE ) return FALSE;

    return HSSCSIStatusToStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}
