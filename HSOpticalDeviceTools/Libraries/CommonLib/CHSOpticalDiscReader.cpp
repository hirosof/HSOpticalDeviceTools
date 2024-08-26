#include "CHSOpticalDiscReader.hpp"

bool CHSOpticalDiscReader::executeRawCommand( THSSCSI_CommandData* pData ) const {
	return ( this->mp_Drive != nullptr ) ? this->mp_Drive->executeCommand( pData ) : false;

}

CHSOpticalDiscReader::CHSOpticalDiscReader( ) {
	this->setDrive( nullptr );
}

CHSOpticalDiscReader::CHSOpticalDiscReader( CHSOpticalDrive* pDrive ) {
	this->setDrive( pDrive );
}

void CHSOpticalDiscReader::setDrive( CHSOpticalDrive* pDrive ) {
	this->mp_Drive = pDrive;
	this->m_cmd.setDrive( pDrive );
}

bool CHSOpticalDiscReader::readCapacity( THSSCSI_ReadCapacityResponse* pres ) {


	if ( pres == nullptr )return false;

	THSSCSI_CommandData cmd;
	if ( !HSSCSI_InitializeCommandData( &cmd ) )return false;


	THSSCSI_ReadCapacityResponse rawResp;

	cmd.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
	cmd.pSPTDStruct->DataBuffer = &rawResp;
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof(THSSCSI_ReadCapacityResponse) );

	cmd.pSPTDStruct->CdbLength = 10;
	cmd.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_READ_CAPACITY;


	if ( this->executeRawCommand( &cmd ) == false ) {
		return false;
	}


	if ( cmd.result.DeviceIOControlResult ) {
		if ( HSSCSIStatusToSCSIEnumStatusCode( cmd.result.scsiStatus ) == EHSSCSIStatusCode::Good ) {

			pres->BlockLengthInBytes = HSSCSI_InverseEndian32( rawResp.BlockLengthInBytes );
			pres->LogicalBlockAddress = HSSCSI_InverseEndian32( rawResp.LogicalBlockAddress );

			return true;
		}
	}


	return false;
}

bool CHSOpticalDiscReader::readDiscInformation( THSSCSI_DiscInformation* pInfo, THSSCSI_InterpretedDiscInformation* pInterpretedInfo ) {

	if ( pInfo == nullptr )return false;

	THSSCSI_CommandData cmd;
	if ( !HSSCSI_InitializeCommandData( &cmd ) )return false;

	THSSCSI_DiscInformation raw;

	uint16_t rawSize = static_cast<uint16_t>( sizeof( raw ) );

	cmd.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
	cmd.pSPTDStruct->DataBuffer = &raw;
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( raw ) );

	cmd.pSPTDStruct->CdbLength = 10;
	cmd.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_READ_DISC_INFORMATION;
	cmd.pSPTDStruct->Cdb[1] = 0;
	cmd.pSPTDStruct->Cdb[7] = ( rawSize & 0xFF00 ) >> 8;
	cmd.pSPTDStruct->Cdb[8] = ( rawSize & 0x00FF );
	

	if ( this->executeRawCommand( &cmd ) == false ) {
		return false;
	}


	if ( cmd.result.DeviceIOControlResult == false ) {
		return false;

	}

	if ( HSSCSIStatusToSCSIEnumStatusCode( cmd.result.scsiStatus ) != EHSSCSIStatusCode::Good ) {
		return false;
	}

	if ( raw.DiscInformationDataType != 0 ) {
		return false;
	}

	*pInfo = raw;

	if ( pInterpretedInfo != nullptr ) {
		pInterpretedInfo->DiscIdentification = HSSCSI_InverseEndian32( raw.DiscIdentification );
		pInterpretedInfo->DiscInformationLength = HSSCSI_InverseEndian16( raw.DiscInformationLength );
		pInterpretedInfo->FirstTrackNumberInLastSession = ( raw.FirstTrackNumberInLastSessionMSB << 8 ) | ( raw.FirstTrackNumberInLastSessionLSB );
		pInterpretedInfo->LastPossibleLeadOutStartAddress =  HSSCSI_InverseEndian32( raw.LastPossibleLeadOutStartAddress );
		pInterpretedInfo->LastSessionLeadInStartAddress = HSSCSI_InverseEndian32( raw.LastSessionLeadInStartAddress );
		pInterpretedInfo->LastTrackNumberInLastSession = ( raw.LastTrackNumberInLastSessionMSB << 8 ) | ( raw.LastTrackNumberInLastSessionLSB );
		pInterpretedInfo->NumberOfSessions = ( raw.NumberOfSessionsMSB << 8 ) | ( raw.NumberOfSessionsLSB );
	}

	return true;
}

bool CHSOpticalDiscReader::readTrackResourcesInformation( THSSCSI_TrackResourcesInformation* pInfo ) {
	if ( pInfo == nullptr )return false;

	THSSCSI_CommandData cmd;
	if ( !HSSCSI_InitializeCommandData( &cmd ) )return false;

	THSSCSI_TrackResourcesInformation raw;
	memset( &raw, 0, sizeof( raw ) );

	uint16_t rawSize = static_cast<uint16_t>( sizeof( raw ) );

	cmd.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
	cmd.pSPTDStruct->DataBuffer = &raw;
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( raw ) );

	cmd.pSPTDStruct->CdbLength = 10;
	cmd.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_READ_DISC_INFORMATION;
	cmd.pSPTDStruct->Cdb[1] = 1;
	cmd.pSPTDStruct->Cdb[7] = ( rawSize & 0xFF00 ) >> 8;
	cmd.pSPTDStruct->Cdb[8] =( rawSize & 0x00FF );


	if ( this->executeRawCommand( &cmd ) == false ) {
		return false;
	}


	if ( cmd.result.DeviceIOControlResult ) {
		if ( HSSCSIStatusToSCSIEnumStatusCode( cmd.result.scsiStatus ) == EHSSCSIStatusCode::Good ) {

			if ( raw.DiscInformationDataType != 1 ) return false;

			*pInfo = raw;

			pInfo->DiscInformationLength = HSSCSI_InverseEndian16( pInfo->DiscInformationLength );
			pInfo->MaxPossibleNumberOfTracksOnDisc = HSSCSI_InverseEndian16( pInfo->MaxPossibleNumberOfTracksOnDisc );
			pInfo->MaxPossibleNumberOfAppendableTracksOnDisc = HSSCSI_InverseEndian16( pInfo->MaxPossibleNumberOfAppendableTracksOnDisc );
			pInfo->NumberOfAssignedTracksOnDisc = HSSCSI_InverseEndian16( pInfo->NumberOfAssignedTracksOnDisc );
			pInfo->CurrentNumberOfAppendableTracksOnDisc = HSSCSI_InverseEndian16( pInfo->CurrentNumberOfAppendableTracksOnDisc );
			
			return true;
		}
	}
	return false;
}

bool CHSOpticalDiscReader::readPOWResourcesDiscInformation( THSSCSI_POWResourcesDiscInformation* pInfo ) {
	if ( pInfo == nullptr )return false;

	THSSCSI_CommandData cmd;
	if ( !HSSCSI_InitializeCommandData( &cmd ) )return false;

	THSSCSI_POWResourcesDiscInformation raw;
	memset( &raw, 0, sizeof( raw ) );

	uint16_t rawSize = static_cast<uint16_t>( sizeof( raw ) );

	cmd.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
	cmd.pSPTDStruct->DataBuffer = &raw;
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( sizeof( raw ) );

	cmd.pSPTDStruct->CdbLength = 10;
	cmd.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_READ_DISC_INFORMATION;
	cmd.pSPTDStruct->Cdb[1] = 2;
	cmd.pSPTDStruct->Cdb[7] = ( rawSize & 0xFF00 ) >> 8;
	cmd.pSPTDStruct->Cdb[8] = ( rawSize & 0x00FF );


	if ( this->executeRawCommand( &cmd ) == false ) {
		return false;
	}


	if ( cmd.result.DeviceIOControlResult ) {
		if ( HSSCSIStatusToSCSIEnumStatusCode( cmd.result.scsiStatus ) == EHSSCSIStatusCode::Good ) {
			if ( raw.DiscInformationDataType != 2 ) return false;
			*pInfo = raw;
			pInfo->DiscInformationLength = HSSCSI_InverseEndian16( pInfo->DiscInformationLength );
			pInfo->NumberOfRemainingPOWUpdates = HSSCSI_InverseEndian32( pInfo->NumberOfRemainingPOWUpdates );
			pInfo->RemainingPOWReplacements = HSSCSI_InverseEndian32( pInfo->RemainingPOWReplacements );
			pInfo->RemainingPOWReallocationMapEntries = HSSCSI_InverseEndian32( pInfo->RemainingPOWReallocationMapEntries );
			return true;
		}
	}
	return false;
}
