#include "../CommonLib/CHSOpticalDrive.hpp"


#pragma pack(push , 1)

struct THSSCSI_PerformanceHeader {
	uint32_t DataLength;
	bool Except : 1;
	bool Write : 1;
	uint8_t reserved1 : 6;
	uint8_t reserved2[3];
};

struct THSSCSI_NormalPerformance {
	uint32_t StartLBA;
	uint32_t StartPerformance;
	uint32_t EndLBA;
	uint32_t EndPerformance;
};

struct THSSCSI_NormalPerformanceWithHeader {
	THSSCSI_PerformanceHeader header;
	uint32_t StartLBA;
	uint32_t StartPerformance;
	uint32_t EndLBA;
	uint32_t EndPerformance;
};
using CHSSCSI_NormalPerformanceStdVector = std::vector< THSSCSI_NormalPerformance>;



#pragma pack(pop)


class CHSOpticalDriveExperiment : public CHSOpticalDrive {
public:

	bool __notsupport_readMediaSerialNumber( std::vector<uint8_t>* pSerialNumber, HSSCSI_SPTD_RESULT* pDetailResult );

	bool __notsupport_getReadPerformance( CHSSCSI_NormalPerformanceStdVector* pPerformance, HSSCSI_SPTD_RESULT* pDetailResult )const;



};