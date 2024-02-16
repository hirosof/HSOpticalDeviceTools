#include "CHSCompactDiscReader.hpp"


size_t CHSCompactDiscReader::NormalCDDATrackSectorSize = 2352U;

CHSCompactDiscReader::CHSCompactDiscReader( ) : CHSOpticalDiscReader(){
}

CHSCompactDiscReader::CHSCompactDiscReader( CHSOpticalDrive* pDrive ) : CHSOpticalDiscReader(pDrive) {
}

UHSSCSI_AddressData32 CHSCompactDiscReader::MergeMSF( UHSSCSI_AddressData32 address ) {
	UHSSCSI_AddressData32 ad32;
	ad32.i32Value = ( address.irawValues[1] * 60 + address.irawValues[2] ) * 75 + address.irawValues[3];
	return ad32;
}

UHSSCSI_AddressData32 CHSCompactDiscReader::InverseEndianAddressData32( UHSSCSI_AddressData32 address ) {
	UHSSCSI_AddressData32 ad32;
	ad32.u32Value = HSSCSI_InverseEndian32( address.u32Value );
	return ad32;
}

UHSSCSI_AddressData32 CHSCompactDiscReader::SplitMSF( UHSSCSI_AddressData32 address ) {
	UHSSCSI_AddressData32 ad32;

	ad32.urawValues[0] = 0;
	ad32.irawValues[3] = address.i32Value % 75;
	ad32.irawValues[2] = ( address.i32Value / 75 ) % 60;
	ad32.irawValues[1] = address.i32Value / (75 * 60);
	return ad32;
}

UHSSCSI_AddressData32 CHSCompactDiscReader::SplittedMSF_to_LBA( UHSSCSI_AddressData32 address ) {
	return MergedMSF_to_LBA( MergeMSF( address ) );
}

UHSSCSI_AddressData32 CHSCompactDiscReader::LBA_to_SplittedMSF( UHSSCSI_AddressData32 address ) {
	return SplitMSF( LBA_to_MergedMSF( address ) );
}

UHSSCSI_AddressData32 CHSCompactDiscReader::MergedMSF_to_LBA( UHSSCSI_AddressData32 address ) {

	UHSSCSI_AddressData32 msf90sp = { 0,90,0,0 };
	UHSSCSI_AddressData32 msf90 = MergeMSF( msf90sp );
	UHSSCSI_AddressData32 ad32;

	if ( address.u32Value < msf90.u32Value ) {
		ad32.i32Value = address.i32Value - 150;
	} else {
		ad32.i32Value = address.i32Value - 450150;
	}
	return ad32;
}

UHSSCSI_AddressData32 CHSCompactDiscReader::LBA_to_MergedMSF( UHSSCSI_AddressData32 address ) {

	UHSSCSI_AddressData32 ad32 = { 0,0,0,0 };


	if ( ( address.i32Value >= -150 ) && ( address.i32Value <= 404849 ) ) {
		ad32.i32Value = address.i32Value + 150;
	} else if ( ( address.i32Value >= -45150 ) && ( address.i32Value <= -151 ) ) {
		ad32.i32Value = address.i32Value + 450150;
	}

	return ad32;
}

EHSSCSI_TrackType CHSCompactDiscReader::GetTrackTypeFromControl( uint8_t control, bool* pPermittedDigitalCopy ) {

	if ( pPermittedDigitalCopy ) {
		*pPermittedDigitalCopy = ( control & 2 ) ? true : false;
	}

	uint8_t judgeTargetValue = control & 0xD;

	switch ( judgeTargetValue ) {
		case 0:
			return EHSSCSI_TrackType::Audio2Channel;
		case 1:
			return EHSSCSI_TrackType::Audio2ChannelWithPreEmphasis;
		case 8:
			return EHSSCSI_TrackType::Audio4Channel;
		case 9:
			return EHSSCSI_TrackType::Audio4ChannelWithPreEmphasis;
		case 4:
			return EHSSCSI_TrackType::DataUninterrupted;
		case 5:
			return EHSSCSI_TrackType::DataIncrement;
		default:
			return EHSSCSI_TrackType::Unknown;
	}
}

bool CHSCompactDiscReader::isCDMediaPresent( void ) const {
	return this->m_cmd.getCurrentProfileRoughType( ) == EHSSCSI_ProfileRoughType::CD;
}

bool CHSCompactDiscReader::isSupportedCDText( void ) const {

	THSSCSI_FeatureDescriptor_CDRead cdread;

	if ( this->m_cmd.getCDReadFeatureDescriptor( &cdread ) ) {
		return cdread.CDText;
	}

	return false;
}

bool CHSCompactDiscReader::readFormmatedTOC( THSSCSI_FormattedTOC* pInfo, EHSSCSI_AddressFormType addressType ) {

	if ( pInfo == nullptr ) return false;
	if ( this->mp_Drive->isReady( ) == false ) return false;
	if ( this->isCDMediaPresent( ) == false ) return false;


	THSSCSI_CommandData cmd;
	THSSCSI_TOC_PMA_ATIP_ResponseHeader resHeaderOnly;
	size_t responseHeaderSize = sizeof( THSSCSI_TOC_PMA_ATIP_ResponseHeader );
	if ( !HSSCSI_InitializeCommandData( &cmd ) )return false;

	cmd.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
	cmd.pSPTDStruct->DataBuffer = &resHeaderOnly;
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( responseHeaderSize );

	cmd.pSPTDStruct->CdbLength = 10;
	cmd.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_READ_TOC_PMA_ATIP;

	cmd.pSPTDStruct->Cdb[1] = ( addressType == EHSSCSI_AddressFormType::LBA ) ? 0x0 : 0x2;
	cmd.pSPTDStruct->Cdb[2] = 0;
	cmd.pSPTDStruct->Cdb[7] = ( responseHeaderSize & 0xFF00 ) >> 8;
	cmd.pSPTDStruct->Cdb[8] = ( responseHeaderSize & 0x00FF );


	if ( !this->mp_Drive->executeCommand( &cmd ) )return false;
	if ( cmd.result.DeviceIOControlResult == FALSE )  return false;
	if ( HSSCSIStatusToStatusCode( cmd.result.scsiStatus ) != EHSSCSIStatusCode::Good )  return false;


	size_t responseAllSize = HSSCSI_InverseEndian16( resHeaderOnly.DataLength ) + 2;

	size_t restOf8 = responseAllSize % 8;
	size_t paddingBufferSize = 0;
	if ( restOf8 != 0 ) paddingBufferSize = 8 - restOf8;
	responseAllSize += paddingBufferSize;

	std::shared_ptr<uint8_t> resBuffer( new uint8_t[responseAllSize] );
	cmd.pSPTDStruct->DataBuffer = resBuffer.get( );
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( responseAllSize );

	cmd.pSPTDStruct->Cdb[7] = ( responseAllSize & 0xFF00 ) >> 8;
	cmd.pSPTDStruct->Cdb[8] = ( responseAllSize & 0x00FF );


	if ( !this->mp_Drive->executeCommand( &cmd ) )return false;
	if ( cmd.result.DeviceIOControlResult == FALSE )  return false;

	if ( HSSCSIStatusToStatusCode( cmd.result.scsiStatus ) != EHSSCSIStatusCode::Good )  return false;


	THSSCSI_TOC_PMA_ATIP_ResponseHeader* pHeader = reinterpret_cast<THSSCSI_TOC_PMA_ATIP_ResponseHeader*>( resBuffer.get( ) );
	THSSCSI_FormattedTOCRawItem* pTopItem = reinterpret_cast<THSSCSI_FormattedTOCRawItem*>( resBuffer.get( ) + responseHeaderSize );
	size_t NumberOfDataElements = ( responseAllSize - responseHeaderSize - paddingBufferSize ) / sizeof( THSSCSI_FormattedTOCRawItem );

	THSSCSI_FormattedTOCRawItem* pCurrent;
	THSSCSI_FormattedTOCInterpretedItem interItem;

	pInfo->AddressType = addressType;
	pInfo->header = *pHeader;
	pInfo->StartTrackNumber = pHeader->FirstNumberField;
	pInfo->EndTrackNumber = pHeader->LastNumberField;
	pInfo->CountOfTracks = pHeader->LastNumberField - pHeader->FirstNumberField + 1;
	pInfo->rawItems.clear( );
	pInfo->items.clear( );

	for ( size_t i = 0; i < NumberOfDataElements; i++ ) {
		pCurrent = pTopItem + i;

		pInfo->rawItems.push_back( *pCurrent );

		interItem.ADR = pCurrent->ADR;
		interItem.Control = pCurrent->Control;
		interItem.TrackNumber = pCurrent->TrackNumber;

		switch ( addressType ) {
			case EHSSCSI_AddressFormType::SplittedMSF:
				interItem.TrackStartAddress = pCurrent->TrackStartAddress;
				break;
			case EHSSCSI_AddressFormType::MergedMSF:
				interItem.TrackStartAddress = MergeMSF( pCurrent->TrackStartAddress );
				break;
			case EHSSCSI_AddressFormType::LBA:
				interItem.TrackStartAddress = InverseEndianAddressData32( pCurrent->TrackStartAddress );
				break;
		}

		interItem.TrackType = GetTrackTypeFromControl( interItem.Control, &interItem.PermittedDigitalCopy );


		/*
		*  pCurrent->TrackNumber の通知が0xAAの場合、
		* リードアウト領域の開始地点の通知(最終トラックの終端+1)なので
		*  それを除いて、itemの要素として追加する
		*/
		if ( pCurrent->TrackNumber != 0xAA ) {
			pInfo->items.insert(std::make_pair(pCurrent->TrackNumber ,  interItem ));
		} else {
			pInfo->PointOfLeadOutAreaStart= interItem.TrackStartAddress;
		}
	}

	//各トラックの長さと終端を計算する
	UHSSCSI_AddressData32 nextPoint;
	for ( auto it = pInfo->items.begin( ); it != pInfo->items.end( ); it++ ) {

		if ( it->first == pHeader->LastNumberField ) {
			nextPoint = pInfo->PointOfLeadOutAreaStart;
		} else {
			nextPoint = pInfo->items.at( it->first + 1 ).TrackStartAddress;
		}

		switch ( addressType ) {
			case EHSSCSI_AddressFormType::SplittedMSF:
				it->second.TrackLength.u32Value = MergeMSF( nextPoint ).u32Value - MergeMSF( it->second.TrackStartAddress ).u32Value;
				it->second.TrackLength = SplitMSF( it->second.TrackLength );

				it->second.TrackEndAddress.u32Value = MergeMSF( nextPoint ).u32Value - 1;
				it->second.TrackEndAddress = SplitMSF( it->second.TrackEndAddress );
				break;
			case EHSSCSI_AddressFormType::MergedMSF:
				it->second.TrackLength.u32Value = nextPoint.u32Value - it->second.TrackStartAddress.u32Value;
				it->second.TrackEndAddress.u32Value = nextPoint.u32Value - 1;
				break;
			case EHSSCSI_AddressFormType::LBA:
				it->second.TrackLength.i32Value = nextPoint.i32Value - it->second.TrackStartAddress.i32Value;
				it->second.TrackEndAddress.i32Value = nextPoint.i32Value - 1;
				break;
		}
	}

	return true;
}

bool CHSCompactDiscReader::readRawTOC( THSSCSI_RawTOC* pInfo, EHSSCSI_AddressFormType addressType ) {
	if ( pInfo == nullptr ) return false;
	if ( this->mp_Drive->isReady( ) == false ) return false;
	if ( this->isCDMediaPresent( ) == false ) return false;


	THSSCSI_CommandData cmd;
	THSSCSI_TOC_PMA_ATIP_ResponseHeader resHeaderOnly;
	size_t responseHeaderSize = sizeof( THSSCSI_TOC_PMA_ATIP_ResponseHeader );
	if ( !HSSCSI_InitializeCommandData( &cmd ) )return false;

	cmd.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
	cmd.pSPTDStruct->DataBuffer = &resHeaderOnly;
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( responseHeaderSize );

	cmd.pSPTDStruct->CdbLength = 10;
	cmd.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_READ_TOC_PMA_ATIP;

	cmd.pSPTDStruct->Cdb[1] = 0;
	cmd.pSPTDStruct->Cdb[2] = 2;
	cmd.pSPTDStruct->Cdb[7] = ( responseHeaderSize & 0xFF00 ) >> 8;
	cmd.pSPTDStruct->Cdb[8] = ( responseHeaderSize & 0x00FF );


	if ( !this->mp_Drive->executeCommand( &cmd ) )return false;
	if ( cmd.result.DeviceIOControlResult == FALSE )  return false;
	if ( HSSCSIStatusToStatusCode( cmd.result.scsiStatus ) != EHSSCSIStatusCode::Good )  return false;


	size_t responseAllSize = HSSCSI_InverseEndian16( resHeaderOnly.DataLength ) + 2;
	
	size_t restOf8 = responseAllSize % 8;
	size_t paddingBufferSize = 0;
	if ( restOf8 != 0 ) paddingBufferSize = 8 - restOf8;
	responseAllSize += paddingBufferSize;

	std::shared_ptr<uint8_t> resBuffer( new uint8_t[responseAllSize] );
	cmd.pSPTDStruct->DataBuffer = resBuffer.get( );
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( responseAllSize );

	cmd.pSPTDStruct->Cdb[7] = ( responseAllSize & 0xFF00 ) >> 8;
	cmd.pSPTDStruct->Cdb[8] = ( responseAllSize & 0x00FF );


	if ( !this->mp_Drive->executeCommand( &cmd ) )return false;
	if ( cmd.result.DeviceIOControlResult == FALSE )  return false;

	if ( HSSCSIStatusToStatusCode( cmd.result.scsiStatus ) != EHSSCSIStatusCode::Good )  return false;


	THSSCSI_TOC_PMA_ATIP_ResponseHeader* pHeader = reinterpret_cast<THSSCSI_TOC_PMA_ATIP_ResponseHeader*>( resBuffer.get( ) );
	THSSCSI_RawTOCRawItem* pTopItem = reinterpret_cast<THSSCSI_RawTOCRawItem*>( resBuffer.get( ) + responseHeaderSize );
	size_t NumberOfDataElements = ( responseAllSize - responseHeaderSize - paddingBufferSize ) / sizeof( THSSCSI_RawTOCRawItem );

	pInfo->AddressType = addressType;
	pInfo->header = *pHeader;
	pInfo->FirstSessionNumber = pHeader->FirstNumberField;
	pInfo->LastSessionNumber = pHeader->LastNumberField;
	pInfo->CountOfSessions = pHeader->LastNumberField - pHeader->FirstNumberField + 1;
	pInfo->rawItems.clear( );
	pInfo->sessionItems.clear( );
	pInfo->trackItems.clear( );

	THSSCSI_RawTOCRawItem* pCurrent;

	pInfo->FirstTrackNumber = 0xFF;
	pInfo->LastTrackNumber = 0;


	for ( size_t i = 0; i < NumberOfDataElements; i++ ) {
		pCurrent = pTopItem + i;
		pInfo->rawItems.push_back( *pCurrent );

		THSSCSI_RawTOCSessionItem& currentSessionData = pInfo->sessionItems[pCurrent->SessionNumber];

		currentSessionData.SessionNumber = pCurrent->SessionNumber;

		if ( pCurrent->ADR == 1 ) {

			if ( pCurrent->POINT == 0xA0 ) {

				currentSessionData.FirstTrackNumber = pCurrent->PMIN;
				pInfo->FirstTrackNumber = min( pInfo->FirstTrackNumber, currentSessionData.FirstTrackNumber );


				currentSessionData.DiscType = EHSSCSI_DiscType::Mode1;

				if ( pCurrent->PSEC == 0x10 ) {
					currentSessionData.DiscType = EHSSCSI_DiscType::CD_I;
				} else if ( pCurrent->PSEC == 0x20 ) {
					currentSessionData.DiscType = EHSSCSI_DiscType::Mode2;
				}

			}else if ( pCurrent->POINT == 0xA1 ) {
				currentSessionData.LastTrackNumber = pCurrent->PMIN;
				pInfo->LastTrackNumber = max( pInfo->LastTrackNumber, currentSessionData.LastTrackNumber );
	
			}else if ( pCurrent->POINT == 0xA2 ) {
				currentSessionData.PointOfLeadOutAreaStart.urawValues[0] = 0;
				currentSessionData.PointOfLeadOutAreaStart.urawValues[1] = pCurrent->PMIN;
				currentSessionData.PointOfLeadOutAreaStart.urawValues[2] = pCurrent->PSEC;
				currentSessionData.PointOfLeadOutAreaStart.urawValues[3] = pCurrent->PFRAME;
				currentSessionData.PointOfLeadOutAreaStart= MergeMSF(currentSessionData.PointOfLeadOutAreaStart );

			} else if ( ( pCurrent->POINT >= 0x01 ) && ( pCurrent->POINT <= 0x63 ) ) {

				THSSCSI_RawTOCTrackItem& currentTrackData = pInfo->trackItems[pCurrent->POINT];

				currentTrackData.TrackStartAddress.urawValues[0] = 0;
				currentTrackData.TrackStartAddress.urawValues[1] = pCurrent->PMIN;
				currentTrackData.TrackStartAddress.urawValues[2] = pCurrent->PSEC;
				currentTrackData.TrackStartAddress.urawValues[3] = pCurrent->PFRAME;
				currentTrackData.TrackStartAddress = MergeMSF( currentTrackData.TrackStartAddress );

				currentTrackData.ADR = pCurrent->ADR;
				currentTrackData.Control = pCurrent->Control;
				currentTrackData.SessionNumber = pCurrent->SessionNumber;
				currentTrackData.TrackType = GetTrackTypeFromControl( currentTrackData.Control, &currentTrackData.PermittedDigitalCopy );
				currentTrackData.TrackNumber = pCurrent->POINT;
			}
		}
	}

	pInfo->CountOfTracks = pInfo->LastTrackNumber - pInfo->FirstTrackNumber + 1;

	for ( uint8_t trackIndex = pInfo->FirstTrackNumber; trackIndex <= pInfo->LastTrackNumber; trackIndex++ ) {

		THSSCSI_RawTOCTrackItem& currentTrackData = pInfo->trackItems[trackIndex];
		THSSCSI_RawTOCSessionItem& currentSessionData = pInfo->sessionItems[currentTrackData.SessionNumber];

		if ( trackIndex == currentSessionData.LastTrackNumber ) {
			currentTrackData.TrackEndAddress.u32Value = currentSessionData.PointOfLeadOutAreaStart.u32Value - 1;
		} else {
			currentTrackData.TrackEndAddress.u32Value = pInfo->trackItems[trackIndex + 1].TrackStartAddress.u32Value - 1;
		}
		currentTrackData.TrackLength.u32Value = currentTrackData.TrackEndAddress.u32Value - currentTrackData.TrackStartAddress.u32Value + 1;

		switch ( addressType ) {

			case EHSSCSI_AddressFormType::SplittedMSF:
				currentTrackData.TrackStartAddress = SplitMSF( currentTrackData.TrackStartAddress );
				currentTrackData.TrackEndAddress = SplitMSF( currentTrackData.TrackEndAddress );
				currentTrackData.TrackLength = SplitMSF( currentTrackData.TrackLength );
				break;
			case EHSSCSI_AddressFormType::LBA:
				currentTrackData.TrackStartAddress = MergedMSF_to_LBA( currentTrackData.TrackStartAddress );
				currentTrackData.TrackEndAddress = MergedMSF_to_LBA( currentTrackData.TrackEndAddress );
				break;
		}
	}


	for ( uint8_t sessionIndex = pInfo->FirstSessionNumber; sessionIndex <= pInfo->LastSessionNumber; sessionIndex++ ) {
		THSSCSI_RawTOCSessionItem& currentSessionData = pInfo->sessionItems[sessionIndex];
		switch ( addressType ) {
			case EHSSCSI_AddressFormType::SplittedMSF:
				currentSessionData.PointOfLeadOutAreaStart = SplitMSF( currentSessionData.PointOfLeadOutAreaStart );
				break;
			case EHSSCSI_AddressFormType::LBA:
				currentSessionData.PointOfLeadOutAreaStart = MergedMSF_to_LBA( currentSessionData.PointOfLeadOutAreaStart );
				break;
		}
	}
	return true;
}

