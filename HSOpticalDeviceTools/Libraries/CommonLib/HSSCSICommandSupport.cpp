#include "HSSCSICommandSupport.hpp"

EHSSCSIStatusCode HSSCSIStatusToSCSIEnumStatusCode(const UHSSCSIStatus status ) {
	switch ( status.rawValue ) {
		case 0x0:
			return EHSSCSIStatusCode::Good;
		case 0x2:
			return EHSSCSIStatusCode::CheckCondition;
		case 0x4:
			return EHSSCSIStatusCode::ConditionMet;
		case 0x8:
			return EHSSCSIStatusCode::Busy;
		case 0x18:
			return EHSSCSIStatusCode::ReservationConflict;
		case 0x28:
			return EHSSCSIStatusCode::TaskSetFull;
		case 0x30:
			return EHSSCSIStatusCode::ACAActive;
		case 0x40:
			return EHSSCSIStatusCode::TaskAborted;
		default:
			return EHSSCSIStatusCode::Unknown;
	}
}

std::string HSSCSIStatusToString( const UHSSCSIStatus status ) {
	return HSSCSIEnumStatusCodeToString( HSSCSIStatusToSCSIEnumStatusCode( status ) );
}

std::string HSSCSIEnumStatusCodeToString( EHSSCSIStatusCode code ) {

	switch ( code ) {
		case EHSSCSIStatusCode::Good:
			return std::string( "GOOD" );
		case EHSSCSIStatusCode::CheckCondition:
			return std::string( "CHECK CONDITION" );
		case EHSSCSIStatusCode::ConditionMet:
			return std::string( "CONDITION MET" );
		case EHSSCSIStatusCode::Busy:
			return std::string( "BUSY" );
		case EHSSCSIStatusCode::ReservationConflict:
			return std::string( "RESERVATION CONFLICT" );
		case EHSSCSIStatusCode::TaskSetFull:
			return std::string( "TASK SET FULL" );
		case EHSSCSIStatusCode::ACAActive:
			return std::string( "ACA ACTIVE" );
		case EHSSCSIStatusCode::TaskAborted:
			return std::string( "TASK ABORTED" );
	}

	return std::string("UNKNOWN" );
}

bool HSSCSI_InitializeCommandData( THSSCSI_CommandData* pData ) {
	if ( pData == nullptr ) {
		return false;
	}
	pData->allocatedRawSize = sizeof( SCSI_PASS_THROUGH_DIRECT ) + HSSCSI_SENSEDATA_MAX_SIZE;
	pData->raw = std::shared_ptr<uint8_t>(new uint8_t[pData->allocatedRawSize]);
	memset( pData->raw.get( ), 0, pData->allocatedRawSize );

	pData->pSPTDStruct = reinterpret_cast<SCSI_PASS_THROUGH_DIRECT*>( pData->raw.get( ) );
	pData->senseData.pRaw = pData->raw.get( ) + sizeof( SCSI_PASS_THROUGH_DIRECT );

	pData->pSPTDStruct->Length = static_cast<USHORT>( sizeof( SCSI_PASS_THROUGH_DIRECT ) );
	pData->pSPTDStruct->SenseInfoLength = static_cast<UCHAR>( HSSCSI_SENSEDATA_MAX_SIZE );
	pData->pSPTDStruct->SenseInfoOffset = static_cast<ULONG>( sizeof( SCSI_PASS_THROUGH_DIRECT ) );
	pData->pSPTDStruct->TimeOutValue = HSSCSI_SPTD_DEFAULT_TIMEOUT;

	memset( &pData->result, 0, sizeof( HSSCSI_SPTD_RESULT ) );

	return true;
}

std::string HSSCSI_GetConnectInterfaceNameStringByName( const EHSSCSI_ConnectInterfaceName name ) {
	switch ( name ) {
		case EHSSCSI_ConnectInterfaceName::SCSI:
			return std::string( "SCSI" );
		case EHSSCSI_ConnectInterfaceName::ATAPI:
			return std::string( "ATAPI" );
		case EHSSCSI_ConnectInterfaceName::IEEE1394_1995:
			return std::string( "IEEE1394-1995" );
		case EHSSCSI_ConnectInterfaceName::IEEE1394A:
			return std::string( "IEEE1394A" );
		case EHSSCSI_ConnectInterfaceName::Fibre_Channel:
			return std::string( "Fibre Channel" );
		case EHSSCSI_ConnectInterfaceName::IEEE1394B:
			return std::string( "IEEE1394B" );
		case EHSSCSI_ConnectInterfaceName::Serial_ATAPI:
			return std::string( "Serial ATA" );
		case EHSSCSI_ConnectInterfaceName::USB:
			return std::string( "USB" );
		default:
			return std::string( "Unknown" );
	}
}
