#pragma once

#define _NTSCSI_USER_MODE_
#include <cstdint>
#include <Windows.h>
#include <ntddscsi.h>

#include <scsi.h>
#include <memory>
#include <string>
#include <sstream>
#include <vector>


#pragma pack(push , 1)

union UHSSCSIStatus {
	struct {
		uint8_t reserved1 : 1;
		uint8_t statusByteCode : 5;
		uint8_t reserved2 : 2;
	};
	uint8_t rawValue;
};

enum struct EHSSCSIStatusCode {
	Good = 0,
	CheckCondition,
	ConditionMetGood,
	Busy,
	IntermediateGood,
	IntermediateConditionMet,
	ReservationConflict,
	CommandTerminated,
	QueueFullOrTaskSetFull,
	ACAActive,
	Unknown
};


enum struct EHSSCSI_SenseKey : uint8_t {
	NO_SENSE = 0,
	RECOVERED_ERROR,
	NOT_READY,
	MEDIUM_ERROR,
	HARDWARE_ERROR,
	ILLEGAL_REQUEST,
	UNIT_ATTENTION,
	DATA_PROTECT,
	BLANK_CHECK,
	VENDER_SPECIFIC,
	COPY_ABORTED,
	ABORTED_COMMAND,
	RESERVED,
	VOLUME_OVERFLOW,
	MISCOMPARE,
	COMPLETED
};

const size_t HSSCSI_SENSEDATA_MAX_SIZE = 252;

struct THSSCSISenseDataResponseCodeOnly {
	uint8_t responseCode : 7;
};

struct THSSCSISenseDataResponseDescriptorFormat {
	uint8_t responseCode : 7; // 0x72 or 0x73
	uint8_t reserved1 : 1;
	uint8_t senseKey : 4;
	uint8_t reserved2 : 4;
	uint8_t additionalSenseCode;
	uint8_t additionalSenseCodeQualifier;
	uint8_t reserved3[3];
	uint8_t additionalSenseLength;
	uint8_t additionalDescriptorData[244];
};

struct THSSCSISenseDataResponseFixedFormat  {
	uint8_t responseCode : 7; // 0x70 or 0x71
	uint8_t valid : 1;
	uint8_t obsolete;
	uint8_t senseKey : 4;
	uint8_t reserved : 1;
	uint8_t ILI : 1;
	uint8_t EOM : 1;
	uint8_t fileMark: 1;
	uint8_t information[4];
	uint8_t additionalSenseLength;
	uint8_t commandSpecificInformation[4];
	uint8_t additionalSenseCode;
	uint8_t additionalSenseCodeQualifier;
	uint8_t fieldReplaceableUnitCode;
	uint8_t senseKeySpecific1 : 7;
	uint8_t SKSV : 1;
	uint8_t senseKeySpecific2[2];
	uint8_t additionalSenseData[234];
};



struct HSSCSI_SPTD_RESULT {
	BOOL DeviceIOControlResult;
	DWORD DeviceIOControlLastError;
	DWORD resultSize;
	uint8_t executedOperationCode;
	UHSSCSIStatus scsiStatus;
	uint8_t scsiSK;
	uint8_t scsiASC;
	uint8_t scsiASCQ;
};

const ULONG HSSCSI_SPTD_DEFAULT_TIMEOUT = 10;

#pragma warning( push )
#pragma warning( disable:26495 )
struct THSSCSI_CommandData {
	std::shared_ptr<uint8_t> raw;
	size_t allocatedRawSize;
	SCSI_PASS_THROUGH_DIRECT* pSPTDStruct;
	union {
		uint8_t* pRaw;
		THSSCSISenseDataResponseCodeOnly* pResponseCodeOnly;
		THSSCSISenseDataResponseDescriptorFormat* pDesc;
		THSSCSISenseDataResponseFixedFormat* pFixed;
	}senseData;

	HSSCSI_SPTD_RESULT result;
};
#pragma warning(pop )


enum struct EHSSCSI_ReadyStatus {
	Ready = 0,
	NotReady,
	FailedGotStatus,
	MediumNotPresent
};




using HSSCSI_SPTD_ResponseRawDataType = uint8_t;
using HSSCSI_SPTD_ResponseRawData = std::shared_ptr<HSSCSI_SPTD_ResponseRawDataType[]>;




/*
	Command Descriptor Block - Operation Code
*/

const uint8_t HSSCSI_CDB_OC_TEST_UNIT_READY = 0x00;
const uint8_t HSSCSI_CDB_OC_START_STOP_UNIT = 0x1B;
const uint8_t HSSCSI_CDB_OC_MECHANISM_STATUS = 0xBD;
const uint8_t HSSCSI_CDB_OC_GET_CONFIGURATION = 0x46;
const uint8_t HSSCSI_CDB_OC_READ_DISC_INFORMATION = 0x51;
const uint8_t HSSCSI_CDB_OC_READ_CAPACITY = 0x25;
const uint8_t HSSCSI_CDB_OC_READ_FORMAT_CAPACITIES = 0x23;
const uint8_t HSSCSI_CDB_OC_READ_TOC_PMA_ATIP = 0x43;
const uint8_t HSSCSI_CDB_OC_GET_EVENT_STATUS_NOTIFICATION = 0x4A;
const uint8_t HSSCSI_CDB_OC_GET_PERFORMANCE = 0xAC;
const uint8_t HSSCSI_CDB_OC_SET_CD_SPEED = 0xBB;


#pragma pack(pop )