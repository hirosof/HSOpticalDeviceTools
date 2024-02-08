#include "HSSCSICommandSupport.hpp"

EHSSCSIStatusCode HSSCSIStatusToStatusCode(const UHSSCSIStatus status ) {
	switch ( status.statusByteCode ) {
		case 0x0:
			return EHSSCSIStatusCode::Good;
		case 0x1:
			return EHSSCSIStatusCode::CheckCondition;
		case 0x2:
			return EHSSCSIStatusCode::ConditionMetGood;
		case 0x4:
			return EHSSCSIStatusCode::Busy;
		case 0x8:
			return EHSSCSIStatusCode::IntermediateGood;
		case 0xA:
			return EHSSCSIStatusCode::IntermediateConditionMet;
		case 0xC:
			return EHSSCSIStatusCode::ReservationConflict;
		case 0x11:
			return EHSSCSIStatusCode::CommandTerminated;
		case 0x14:
			return EHSSCSIStatusCode::QueueFullOrTaskSetFull;
		case 0x18:
			return EHSSCSIStatusCode::ACAActive;
		default:
			return EHSSCSIStatusCode::Unknown;
	}
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
