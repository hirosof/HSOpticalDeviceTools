#include "../CommonLib/CHSOpticalDrive.hpp"


#pragma pack(push , 1)




#pragma pack(pop)


class CHSOpticalDriveExperiment : public CHSOpticalDrive {
public:

	bool __notsupport_readMediaSerialNumber( std::vector<uint8_t>* pSerialNumber, HSSCSI_SPTD_RESULT* pDetailResult );




};