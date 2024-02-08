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

    if ( pDetailResult ) {
        *pDetailResult = params.result;
    }
#if 0
    printf( "MediaSerialNumber Length : 0x%02X%02X%02X%02X\n", msnHeader[0], msnHeader[1], msnHeader[2], msnHeader[3] );
#endif

    return true;
}
