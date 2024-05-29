#include "CHSCompactDiscReader.hpp"


const size_t CHSCompactDiscReader::NormalCDDATrackSectorSize = 2352U;

const UINT16  CHSCompactDiscReader::CRCTABLE[256] = {
	0x0000 ,0x1021 ,0x2042 ,0x3063 ,0x4084 ,0x50A5 ,0x60C6 ,0x70E7 ,
	0x8108 ,0x9129 ,0xA14A ,0xB16B ,0xC18C ,0xD1AD ,0xE1CE ,0xF1EF ,
	0x1231 ,0x0210 ,0x3273 ,0x2252 ,0x52B5 ,0x4294 ,0x72F7 ,0x62D6 ,
	0x9339 ,0x8318 ,0xB37B ,0xA35A ,0xD3BD ,0xC39C ,0xF3FF ,0xE3DE ,
	0x2462 ,0x3443 ,0x0420 ,0x1401 ,0x64E6 ,0x74C7 ,0x44A4 ,0x5485 ,
	0xA56A ,0xB54B ,0x8528 ,0x9509 ,0xE5EE ,0xF5CF ,0xC5AC ,0xD58D ,
	0x3653 ,0x2672 ,0x1611 ,0x0630 ,0x76D7 ,0x66F6 ,0x5695 ,0x46B4 ,
	0xB75B ,0xA77A ,0x9719 ,0x8738 ,0xF7DF ,0xE7FE ,0xD79D ,0xC7BC ,
	0x48C4 ,0x58E5 ,0x6886 ,0x78A7 ,0x0840 ,0x1861 ,0x2802 ,0x3823 ,
	0xC9CC ,0xD9ED ,0xE98E ,0xF9AF ,0x8948 ,0x9969 ,0xA90A ,0xB92B ,
	0x5AF5 ,0x4AD4 ,0x7AB7 ,0x6A96 ,0x1A71 ,0x0A50 ,0x3A33 ,0x2A12 ,
	0xDBFD ,0xCBDC ,0xFBBF ,0xEB9E ,0x9B79 ,0x8B58 ,0xBB3B ,0xAB1A ,
	0x6CA6 ,0x7C87 ,0x4CE4 ,0x5CC5 ,0x2C22 ,0x3C03 ,0x0C60 ,0x1C41 ,
	0xEDAE ,0xFD8F ,0xCDEC ,0xDDCD ,0xAD2A ,0xBD0B ,0x8D68 ,0x9D49 ,
	0x7E97 ,0x6EB6 ,0x5ED5 ,0x4EF4 ,0x3E13 ,0x2E32 ,0x1E51 ,0x0E70 ,
	0xFF9F ,0xEFBE ,0xDFDD ,0xCFFC ,0xBF1B ,0xAF3A ,0x9F59 ,0x8F78 ,
	0x9188 ,0x81A9 ,0xB1CA ,0xA1EB ,0xD10C ,0xC12D ,0xF14E ,0xE16F ,
	0x1080 ,0x00A1 ,0x30C2 ,0x20E3 ,0x5004 ,0x4025 ,0x7046 ,0x6067 ,
	0x83B9 ,0x9398 ,0xA3FB ,0xB3DA ,0xC33D ,0xD31C ,0xE37F ,0xF35E ,
	0x02B1 ,0x1290 ,0x22F3 ,0x32D2 ,0x4235 ,0x5214 ,0x6277 ,0x7256 ,
	0xB5EA ,0xA5CB ,0x95A8 ,0x8589 ,0xF56E ,0xE54F ,0xD52C ,0xC50D ,
	0x34E2 ,0x24C3 ,0x14A0 ,0x0481 ,0x7466 ,0x6447 ,0x5424 ,0x4405 ,
	0xA7DB ,0xB7FA ,0x8799 ,0x97B8 ,0xE75F ,0xF77E ,0xC71D ,0xD73C ,
	0x26D3 ,0x36F2 ,0x0691 ,0x16B0 ,0x6657 ,0x7676 ,0x4615 ,0x5634 ,
	0xD94C ,0xC96D ,0xF90E ,0xE92F ,0x99C8 ,0x89E9 ,0xB98A ,0xA9AB ,
	0x5844 ,0x4865 ,0x7806 ,0x6827 ,0x18C0 ,0x08E1 ,0x3882 ,0x28A3 ,
	0xCB7D ,0xDB5C ,0xEB3F ,0xFB1E ,0x8BF9 ,0x9BD8 ,0xABBB ,0xBB9A ,
	0x4A75 ,0x5A54 ,0x6A37 ,0x7A16 ,0x0AF1 ,0x1AD0 ,0x2AB3 ,0x3A92 ,
	0xFD2E ,0xED0F ,0xDD6C ,0xCD4D ,0xBDAA ,0xAD8B ,0x9DE8 ,0x8DC9 ,
	0x7C26 ,0x6C07 ,0x5C64 ,0x4C45 ,0x3CA2 ,0x2C83 ,0x1CE0 ,0x0CC1 ,
	0xEF1F ,0xFF3E ,0xCF5D ,0xDF7C ,0xAF9B ,0xBFBA ,0x8FD9 ,0x9FF8 ,
	0x6E17 ,0x7E36 ,0x4E55 ,0x5E74 ,0x2E93 ,0x3EB2 ,0x0ED1 ,0x1EF0
};


const uint8_t CHSCompactDiscReader::NumberOfCDTextReadTry = 2;


bool CHSCompactDiscReader::Crc16( void* lpData, size_t size, UINT16* lpCRC16 ) {
	if ( lpData == nullptr ) return false;
	if ( lpCRC16 == nullptr ) return false;
	char* lpByteData = reinterpret_cast<char*>( lpData );
	uint16_t crc = 0;
	size_t tableidx;
	for ( size_t i = 0; i < size; i++ ) {
		tableidx = ( crc >> 8 ) ^ ( *( lpByteData + i ) );
		tableidx &= 0xFF;
		crc = ( crc << 8 ) ^ CHSCompactDiscReader::CRCTABLE[tableidx];
	}
	*lpCRC16 = crc ^ 0xFFFF;
	return true;
}

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

UHSSCSI_AddressData32 CHSCompactDiscReader::MakeAddressData32( uint32_t value ) {
	UHSSCSI_AddressData32 data;
	data.u32Value = value;
	return data;
}

UHSSCSI_AddressData32 CHSCompactDiscReader::MakeAddressData32( uint8_t m, uint8_t s, uint8_t f ) {
	UHSSCSI_AddressData32 data;
	data.urawValues[0] = 0;
	data.urawValues[1] = m;
	data.urawValues[2] = s;
	data.urawValues[3] = f;
	return data;
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
	return this->m_cmd.getCurrentProfileFamilyName( ) == EHSSCSI_ProfileFamilyName::CD;
}

bool CHSCompactDiscReader::isSupportedCDText( void ) const {

	THSSCSI_FeatureDescriptor_CDRead cdread;

	if ( this->m_cmd.getCDReadFeatureDescriptor( &cdread ) ) {
		return cdread.CDText;
	}

	return false;
}

bool CHSCompactDiscReader::readFormmatedTOC( THSSCSI_FormattedTOC* pInfo, EHSSCSI_AddressFormType addressType ) const{

	if ( pInfo == nullptr ) return false;
	if ( this->mp_Drive == nullptr ) return false;
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


	if ( !this->executeRawCommand( &cmd ) )return false;
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


	if ( !this->executeRawCommand( &cmd ) )return false;
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

bool CHSCompactDiscReader::readRawTOC( THSSCSI_RawTOC* pInfo, EHSSCSI_AddressFormType addressType )  const{
	if ( pInfo == nullptr ) return false;
	if ( this->mp_Drive == nullptr ) return false;
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


	if ( !this->executeRawCommand( &cmd ) )return false;
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


	if ( !this->executeRawCommand( &cmd ) )return false;
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

EHSSCSI_CDText_ReadResult CHSCompactDiscReader::readCDText( THSSCSI_CDTEXT_Information* pInfo ) const {

	if ( pInfo == nullptr ) return EHSSCSI_CDText_ReadResult::InvalidParameter;
	if ( this->mp_Drive == nullptr ) return EHSSCSI_CDText_ReadResult::InvalidParameter;
	if ( this->mp_Drive->isReady( ) == false ) return EHSSCSI_CDText_ReadResult::NotReady;
	if ( this->isCDMediaPresent( ) == false ) return EHSSCSI_CDText_ReadResult::NotReady;
	if ( this->isSupportedCDText( ) == false ) return EHSSCSI_CDText_ReadResult::NotSupported;

	THSSCSI_CommandData cmd;
	THSSCSI_TOC_PMA_ATIP_ResponseHeader resHeaderOnly;
	size_t responseHeaderSize = sizeof( THSSCSI_TOC_PMA_ATIP_ResponseHeader );
	if ( !HSSCSI_InitializeCommandData( &cmd ) )return EHSSCSI_CDText_ReadResult::Failed;

	cmd.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_IN;
	cmd.pSPTDStruct->DataBuffer = &resHeaderOnly;
	cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( responseHeaderSize );

	cmd.pSPTDStruct->CdbLength = 10;
	cmd.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_READ_TOC_PMA_ATIP;

	cmd.pSPTDStruct->Cdb[1] = 0;
	cmd.pSPTDStruct->Cdb[2] = 5;
	cmd.pSPTDStruct->Cdb[7] = ( responseHeaderSize & 0xFF00 ) >> 8;
	cmd.pSPTDStruct->Cdb[8] = ( responseHeaderSize & 0x00FF );
	cmd.pSPTDStruct->TimeOutValue = 5;

	pInfo->validHeader = false;
	pInfo->hasItems = false;
	pInfo->NumberOfBlocks = 0;

	if ( !this->executeRawCommand( &cmd ) ) {
		return EHSSCSI_CDText_ReadResult::Failed;
	}

	if ( cmd.result.DeviceIOControlResult == FALSE ) {
		if ( cmd.result.DeviceIOControlLastError == ERROR_SEM_TIMEOUT ) {
			return EHSSCSI_CDText_ReadResult::TimeOut;
		}
		return EHSSCSI_CDText_ReadResult::Failed;
	}

	if ( HSSCSIStatusToStatusCode( cmd.result.scsiStatus ) != EHSSCSIStatusCode::Good ) {
		return EHSSCSI_CDText_ReadResult::Failed;
	}

	size_t responseAllSize = HSSCSI_InverseEndian16( resHeaderOnly.DataLength ) + 2;

	if ( responseAllSize == 4 ) {
		//全レスポンスデータサイズが4の場合はヘッダーのみの為、CD-TEXT本体の情報なし
		pInfo->header = resHeaderOnly;
		pInfo->validHeader = true;
		return EHSSCSI_CDText_ReadResult::Success;
	}

	size_t restOf8 = responseAllSize % 8;
	size_t paddingBufferSize = 0;
	if ( restOf8 != 0 ) paddingBufferSize = 8 - restOf8;
	responseAllSize += paddingBufferSize;

	std::shared_ptr<uint8_t> resBuffer( new uint8_t[responseAllSize] );
	THSSCSI_TOC_PMA_ATIP_ResponseHeader* pHeader = reinterpret_cast<THSSCSI_TOC_PMA_ATIP_ResponseHeader*>( resBuffer.get( ) );
	THSSCSI_CDTEXT_PackData* pTopItem = reinterpret_cast<THSSCSI_CDTEXT_PackData*>( resBuffer.get( ) + responseHeaderSize );
	size_t NumberOfDataElements = ( responseAllSize - responseHeaderSize - paddingBufferSize ) / sizeof( THSSCSI_CDTEXT_PackData );
	THSSCSI_CDTEXT_PackData* pCurrent;

	bool isCrcCheckPass = true;
	UINT16 real_crc;

	for ( uint16_t readTrayCount = 0; readTrayCount < NumberOfCDTextReadTry; readTrayCount++ ) {

		cmd.pSPTDStruct->DataBuffer = resBuffer.get( );
		cmd.pSPTDStruct->DataTransferLength = static_cast<ULONG>( responseAllSize );

		cmd.pSPTDStruct->Cdb[7] = ( responseAllSize & 0xFF00 ) >> 8;
		cmd.pSPTDStruct->Cdb[8] = ( responseAllSize & 0x00FF );

		if ( !this->executeRawCommand( &cmd ) ) {
			return EHSSCSI_CDText_ReadResult::Failed;
		}

		if ( cmd.result.DeviceIOControlResult == FALSE ) {
			if ( cmd.result.DeviceIOControlLastError == ERROR_SEM_TIMEOUT ) {
				return EHSSCSI_CDText_ReadResult::TimeOut;
			}
			return EHSSCSI_CDText_ReadResult::Failed;
		}

		if ( HSSCSIStatusToStatusCode( cmd.result.scsiStatus ) != EHSSCSIStatusCode::Good ) {
			return EHSSCSI_CDText_ReadResult::Failed;
		}

		isCrcCheckPass = true;

		for ( size_t i = 0; i < NumberOfDataElements; i++ ) {
			pCurrent = pTopItem + i;
			pCurrent->CRC_OR_Reserved = HSSCSI_InverseEndian16( pCurrent->CRC_OR_Reserved );
			if ( pCurrent->CRC_OR_Reserved == 0 ) continue;
			if ( this->Crc16( pCurrent, 16, &real_crc ) ) {
				if ( real_crc != pCurrent->CRC_OR_Reserved ) {
					isCrcCheckPass = false;
					break;
				}
			}
		}

		if ( isCrcCheckPass ) {
			break;
		}
	}

	if ( !isCrcCheckPass ) {
		//CRCチェックに失敗したら読み込み失敗としてfalseを返す
		return EHSSCSI_CDText_ReadResult::Failed;
	}

	pInfo->header = *pHeader;
	pInfo->hasItems = true;
	pInfo->rawItems.clear( );

	pInfo->parsedItems.clear( );
	pInfo->NumberOfBlocks = 0;

	THSSCSI_CDTEXT_ParsedItemNames * pItemNames;

	std::string * pTargetStrData = nullptr;
	uint8_t rest_offset , current_Track ;
	bool has_restData;

	for ( size_t i = 0; i < NumberOfDataElements; i++ ) {
		pCurrent = pTopItem + i;

		pInfo->rawItems.push_back( *pCurrent );
		pInfo->NumberOfBlocks = max( pInfo->NumberOfBlocks, pCurrent->BlockNumber );

		THSSCSI_CDTEXT_ParsedNames& names = pInfo->parsedItems[pCurrent->BlockNumber];
		names.isDoubleByteCharatorCode = pCurrent->isDoubleByteCharacterCode;

#ifdef _DEBUG

		printf( "[CD-TEXT Pack Data %zu]\n", i );
		printf( "\tIndicator1:%02X (%d)\n", pCurrent->PackIndicator1, pCurrent->PackIndicator1 );
		printf( "\tIndicator2:%02X (%d)\n", pCurrent->PackIndicator2, pCurrent->PackIndicator2 );
		printf( "\tIndicator3:%02X (%d)\n", pCurrent->PackIndicator3, pCurrent->PackIndicator3 );
		printf( "\tCharacterPosition:%u\n", pCurrent->CharacterPosition );
		printf( "\tBlockNumber:%u\n", pCurrent->BlockNumber );
		printf( "\tisDoubleByteCharatorCode:%u\n", pCurrent->isDoubleByteCharacterCode );
		printf( "\tText Field Data (Hex) : " );
		for ( size_t z = 0; z < 12; z++ ) {
			printf( "%02X ", pCurrent->TextDataField.single[z] );
		}

		printf( "\n\tText Field Data (Spaced ASCII) : " );
		for ( size_t z = 0; z < 12; z++ ) {
			printf( "%c ", pCurrent->TextDataField.single[z] );
		}

		printf( "\n\tText Field Data (ASCII) : " );
		for ( size_t z = 0; z < 12; z++ ) {
			printf( "%c", pCurrent->TextDataField.single[z] );
		}

		printf( "\n\n" );
		
#endif

		if ( ( pCurrent->PackIndicator1 >= 0x80 ) && ( pCurrent->PackIndicator1 <= 0x84 ) ) {

			pItemNames = nullptr;
			current_Track = pCurrent->PackIndicator2;
			rest_offset = 0;

			do{
				pItemNames = nullptr;
				if ( current_Track == 0 ) {
					pItemNames = &names.album;
				} else if ( ( current_Track >= 0x01 ) && ( current_Track <= 0x63 ) ) {
					pItemNames = &names.trackTitles[current_Track];
				}

				if ( pItemNames == nullptr ) break;

				switch ( pCurrent->PackIndicator1 ) {
					case 0x80:
						pTargetStrData = &pItemNames->Name;
						break;
					case 0x81:
						pTargetStrData = &pItemNames->PerformerName;
						break;
					case 0x82:
						pTargetStrData = &pItemNames->SongWriterName;
						break;
					case 0x83:
						pTargetStrData = &pItemNames->ComposerName;
						break;
					case 0x84:
						pTargetStrData = &pItemNames->ArrangerName;
						break;
				}

				if ( pCurrent->isDoubleByteCharacterCode == false ) {
					for ( uint8_t idx = rest_offset; idx < 12; idx++ ) {
						rest_offset = idx + 1;
						if ( pCurrent->TextDataField.single[idx] == 0 ) {
							break;
						}
						pTargetStrData->push_back( pCurrent->TextDataField.single[idx] );
					}
				} else {
					for ( uint8_t idx = rest_offset / 2; idx < 6; idx++ ) {
						rest_offset = ( idx + 1 ) * 2;
						if ( pCurrent->TextDataField.dual[idx] == 0 ) {
							break;
						}
						pTargetStrData->push_back( pCurrent->TextDataField.single[idx * 2] );
						pTargetStrData->push_back( pCurrent->TextDataField.single[idx * 2 + 1] );
					}
				}

				has_restData = false;
				for ( size_t idx = rest_offset; idx < 12; idx++ ) {
					if ( pCurrent->TextDataField.single[idx] != 0 ) {
						has_restData = true;
						break;
					}
				}

				if ( !has_restData ) {
					break;
				}

				current_Track++;

			}while( rest_offset != 12 );

		}
	}

	pInfo->NumberOfBlocks++;

	return EHSSCSI_CDText_ReadResult::Success;
}

size_t CHSCompactDiscReader::readStereoAudioTrack( CHSSCSIGeneralBuffer* pBuffer, uint8_t track_number, 
	UHSSCSI_AddressData32 offset, EHSSCSI_AddressFormType offsetAddressType,
	UHSSCSI_AddressData32 readSize, EHSSCSI_AddressFormType readSizeAddressType ) const{

	if ( pBuffer == nullptr ) return 0;
	if ( this->mp_Drive == nullptr )return 0;
	size_t read_offset_LBA = 0;
	switch ( offsetAddressType ) {
		case EHSSCSI_AddressFormType::SplittedMSF:
			read_offset_LBA = MergeMSF( offset ).u32Value;
			break;
		case EHSSCSI_AddressFormType::MergedMSF:
			read_offset_LBA = offset.u32Value;
			break;
		case EHSSCSI_AddressFormType::LBA:
			read_offset_LBA = offset.u32Value;
			break;
	}
	
	size_t read_size_LBA = 0;
	switch ( readSizeAddressType ) {
		case EHSSCSI_AddressFormType::SplittedMSF:
			read_size_LBA = MergeMSF( readSize ).u32Value;
			break;
		case EHSSCSI_AddressFormType::MergedMSF:
			read_size_LBA = readSize.u32Value;
			break;
		case EHSSCSI_AddressFormType::LBA:
			read_size_LBA = readSize.u32Value;
			break;
	}

	THSSCSI_RawTOC toc;
	if ( this->readRawTOC( &toc, EHSSCSI_AddressFormType::LBA ) == false ) {
		return  0;
	}

	auto it = toc.trackItems.find( track_number );
	if ( it == toc.trackItems.end( ) ) {
		return 0;
	}

	if ( it->second.TrackType != EHSSCSI_TrackType::Audio2Channel ) {
		return 0;
	}

	if ( read_offset_LBA >= it->second.TrackLength.u32Value ) {
		return 0;
	}


	size_t  real_start_offset = read_offset_LBA + it->second.TrackStartAddress.u32Value;
	size_t real_end_offset = min( it->second.TrackEndAddress.u32Value, real_start_offset + read_size_LBA - 1 );
	size_t real_read_size = real_end_offset - real_start_offset + 1;

	DWORD transferMaxBytesSize;
	size_t  read_unit_size = 16;
	if ( this->mp_Drive->getMaxTransferLength( &transferMaxBytesSize ) ) {
		read_unit_size = transferMaxBytesSize / NormalCDDATrackSectorSize;
	}

	size_t  NumberOfReadUnit = real_read_size / read_unit_size;
	size_t  RestOfReadUnit = real_read_size % read_unit_size;
	if ( RestOfReadUnit != 0 ) NumberOfReadUnit++;

	if ( pBuffer->prepare( real_read_size * NormalCDDATrackSectorSize ) == false ) {
		return 0;
	}

	uint8_t *pTop = pBuffer->getBufferType<uint8_t*>( );
	uint8_t *pCurrent;
	DWORD read_result_size;
	uint32_t  read_result_all_size = 0;
	BOOL bret;
	RAW_READ_INFO info;
	info.TrackMode = CDDA;
	for ( size_t i = 0; i < NumberOfReadUnit; i++ ) {
		info.DiskOffset.QuadPart = ( real_start_offset + i* read_unit_size) *2048;
		info.SectorCount = static_cast<DWORD>( read_unit_size);
		if ( ( i + 1 ) == NumberOfReadUnit ) {
			if ( RestOfReadUnit != 0 ) {
				info.SectorCount = static_cast<DWORD>( RestOfReadUnit);
			}
		}
		pCurrent = pTop + ( NormalCDDATrackSectorSize * i * read_unit_size );

		bret = DeviceIoControl( this->mp_Drive->getHandle( ),
			IOCTL_CDROM_RAW_READ,
			&info,
			static_cast<DWORD>( sizeof( RAW_READ_INFO ) ),
			pCurrent,
			static_cast<DWORD>( info.SectorCount * NormalCDDATrackSectorSize),
			&read_result_size,
			nullptr );
		
		if ( bret == FALSE ) {
			break;
		}

		read_result_all_size += read_result_size;
		
		if ( read_result_size != ( info.SectorCount * NormalCDDATrackSectorSize ) ) {
			break;
		}
	}

	return read_result_all_size / NormalCDDATrackSectorSize;
}

std::string CHSCompactDiscReader::getTOCString( const char joinChar ) const {
	THSSCSI_RawTOC raw;
	if ( this->readRawTOC( &raw, EHSSCSI_AddressFormType::MergedMSF ) == false ) {
		return std::string();
	}

	return this->GetTOCStringStatic( &raw, joinChar );

}

std::string CHSCompactDiscReader::GetTOCStringStatic( const THSSCSI_RawTOC* pToc, const char joinChar ) {
	if(pToc == nullptr ) 	return std::string( );

	THSSCSI_RawTOC raw = *pToc;
	THSSCSI_RawTOCSessionItem sessionItem = raw.sessionItems[raw.FirstSessionNumber];

	std::string s;
	char buf[16];

	sprintf_s( buf, "%u", sessionItem.FirstTrackNumber );
	s.append( buf );

	sprintf_s( buf, "%u", sessionItem.LastTrackNumber );
	s.push_back( joinChar );
	s.append( buf );


	switch ( raw.AddressType ) {
		case EHSSCSI_AddressFormType::SplittedMSF:
			sprintf_s( buf, "%u", MergeMSF( sessionItem.PointOfLeadOutAreaStart ).u32Value);
			break;
		case EHSSCSI_AddressFormType::MergedMSF:
			sprintf_s( buf, "%u", sessionItem.PointOfLeadOutAreaStart.u32Value );
			break;
		case EHSSCSI_AddressFormType::LBA:
			sprintf_s( buf, "%u", LBA_to_MergedMSF(sessionItem.PointOfLeadOutAreaStart ).u32Value);
			break;
	}

	s.push_back( joinChar );
	s.append( buf );

	for ( uint8_t i = sessionItem.FirstTrackNumber; i <= sessionItem.LastTrackNumber; i++ ) {
		switch ( raw.AddressType ) {
			case EHSSCSI_AddressFormType::SplittedMSF:
				sprintf_s( buf, "%u", MergeMSF( raw.trackItems[i].TrackStartAddress ).u32Value );
				break;
			case EHSSCSI_AddressFormType::MergedMSF:
				sprintf_s( buf, "%u", raw.trackItems[i].TrackStartAddress.u32Value );
				break;
			case EHSSCSI_AddressFormType::LBA:
				sprintf_s( buf, "%u", LBA_to_MergedMSF( raw.trackItems[i].TrackStartAddress ).u32Value );
				break;
		}

		s.push_back( joinChar );
		s.append( buf );
	}

	return s;
}

std::string CHSCompactDiscReader::getMusicBrainzDiscIDSource( void ) const {
	THSSCSI_RawTOC raw;
	if ( this->readRawTOC( &raw, EHSSCSI_AddressFormType::MergedMSF ) == false ) {
		return std::string();
	}
	
	return this->GetMusicBrainzDiscIDSourceStatic(&raw);
}

std::string CHSCompactDiscReader::GetMusicBrainzDiscIDSourceStatic( const THSSCSI_RawTOC* pToc ) {

	if ( pToc == nullptr ) 	return std::string( );

	THSSCSI_RawTOC raw = *pToc;
	THSSCSI_RawTOCSessionItem sessionItem = raw.sessionItems[raw.FirstSessionNumber];

	std::string s;
	char buf[16];

	sprintf_s( buf, "%02X", sessionItem.FirstTrackNumber );
	s.append( buf );

	sprintf_s( buf, "%02X", sessionItem.LastTrackNumber );
	s.append( buf );

	switch ( raw.AddressType ) {
		case EHSSCSI_AddressFormType::SplittedMSF:
			sprintf_s( buf, "%08X", MergeMSF( sessionItem.PointOfLeadOutAreaStart ).u32Value );
			break;
		case EHSSCSI_AddressFormType::MergedMSF:
			sprintf_s( buf, "%08X", sessionItem.PointOfLeadOutAreaStart.u32Value );
			break;
		case EHSSCSI_AddressFormType::LBA:
			sprintf_s( buf, "%08X", LBA_to_MergedMSF( sessionItem.PointOfLeadOutAreaStart ).u32Value );
			break;
	}

	s.append( buf );


	for ( uint8_t i = 1; i <= 99; i++ ) {

		if ( ( i >= sessionItem.FirstTrackNumber ) && ( i <= sessionItem.LastTrackNumber ) ) {

			switch ( raw.AddressType ) {
				case EHSSCSI_AddressFormType::SplittedMSF:
					sprintf_s( buf, "%08X", MergeMSF( raw.trackItems[i].TrackStartAddress ).u32Value );
					break;
				case EHSSCSI_AddressFormType::MergedMSF:
					sprintf_s( buf, "%08X", raw.trackItems[i].TrackStartAddress.u32Value );
					break;
				case EHSSCSI_AddressFormType::LBA:
					sprintf_s( buf, "%08X", LBA_to_MergedMSF( raw.trackItems[i].TrackStartAddress ).u32Value );
					break;
			}

		} else {
			sprintf_s( buf, "%08X", 0 );

		}
		s.append( buf );
	}
	return s;
}


bool CHSCompactDiscReader::setSpeedMax( HSSCSI_SPTD_RESULT* pResult ) const {
	THSSCSI_CommandData params;

	HSSCSI_InitializeCommandData( &params );

	params.pSPTDStruct->DataIn = SCSI_IOCTL_DATA_UNSPECIFIED;
	params.pSPTDStruct->CdbLength = 12;
	params.pSPTDStruct->Cdb[0] = HSSCSI_CDB_OC_SET_CD_SPEED;
	params.pSPTDStruct->Cdb[1] = 0;
	params.pSPTDStruct->Cdb[2] = 0xFF;
	params.pSPTDStruct->Cdb[3] = 0xFF;
	params.pSPTDStruct->Cdb[4] = 0xFF;
	params.pSPTDStruct->Cdb[5] = 0xFF;

	if ( this->executeRawCommand( &params ) == false ) {
		return false;
	}

	if ( pResult != nullptr ) {
		*pResult = params.result;
		pResult->resultSize = 0;
	}

	if ( params.result.DeviceIOControlResult == FALSE ) return false;

	return HSSCSIStatusToStatusCode( params.result.scsiStatus ) == EHSSCSIStatusCode::Good;
}