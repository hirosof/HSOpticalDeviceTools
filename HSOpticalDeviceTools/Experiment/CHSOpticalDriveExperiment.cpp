#include "CHSOpticalDriveExperiment.hpp"

bool CHSOpticalDriveExperiment::__notsupport_readMediaSerialNumber( std::vector<uint8_t>* pSerialNumber, HSSCSI_SPTD_RESULT* pDetailResult ) {


    /*
        確認する限り以下が返ってきたので未サポートである (INVALID COMMAND OPERATION CODE)
        ScsiStatus：0x01
        SK：0x05
        ASC：0x20
        ASCQ：0x00
    */


    if ( pSerialNumber == nullptr )return false;

    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );

    uint8_t  msnHeader[4] = { 0 };

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
    params.pSPTDStruct->DataBuffer = msnHeader;
    params.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( msnHeader ) );
    params.pSPTDStruct->TimeOutValue = 120;
    params.pSPTDStruct->CdbLength = 12;
    params.pSPTDStruct->Cdb[0] = 0xab;
    params.pSPTDStruct->Cdb[1] = 0x01;

    params.pSPTDStruct->Cdb[9] = static_cast<uint8_t>( sizeof( msnHeader ) );

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) {
        return false;
    }


    if ( pDetailResult ) {
        *pDetailResult = params.result;
    }
#if 0
    printf( "MediaSerialNumber Length : 0x%02X%02X%02X%02X\n", msnHeader[0], msnHeader[1], msnHeader[2], msnHeader[3] );
#endif

    return true;
}

bool CHSOpticalDriveExperiment::__notsupport_getReadPerformance( CHSSCSI_NormalPerformanceStdVector* pPerformance, HSSCSI_SPTD_RESULT* pDetailResult ) const {

    if ( pPerformance == nullptr )return false;

    THSSCSI_CommandData params;

    HSSCSI_InitializeCommandData( &params );


    THSSCSI_PerformanceHeader headerOnly = { 0 };
    //  uint8_t headerOnly[64];

    params.pSPTDStruct->TimeOutValue = 60;

    params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
    params.pSPTDStruct->DataBuffer = &headerOnly;
    params.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( headerOnly ) );

    params.pSPTDStruct->CdbLength = 12;
    params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_GET_PERFORMANCE;

    //Data Type=0 (Tolerance=00b , Write=0b , Except=00b)
    params.pSPTDStruct->Cdb[1] = 0;

    //Starting LBA = 0
    params.pSPTDStruct->Cdb[2] = 0;
    params.pSPTDStruct->Cdb[3] = 0;
    params.pSPTDStruct->Cdb[4] = 0;
    params.pSPTDStruct->Cdb[5] = 0;


    //Maximum Number of Descriptors = 0 (header only)
    params.pSPTDStruct->Cdb[8] = 0;
    params.pSPTDStruct->Cdb[9] = 0;

    //Type=0 (Performance data)
    params.pSPTDStruct->Cdb[10] = 0;

    printf( "0\n" );

    if ( this->executeCommand( &params ) == false ) {
        return false;
    }

    printf( "1\n" );

    if ( pDetailResult ) {
        *pDetailResult = params.result;
        pDetailResult->resultSize = 0;
    }

    if ( params.result.DeviceIOControlResult == FALSE ) {
        return false;
    }

    printf( "2\n" );

    //    if ( HSSCSIStatusToStatusCode( params.result.scsiStatus ) != EHSSCSIStatusCode::Good ) {
      //      return false;
        //}

    printf( "3\n" );




    //printf( " length = %u\n", HSSCSI_InverseEndian32( headerOnly.DataLength ) );

    
    return true;
}


