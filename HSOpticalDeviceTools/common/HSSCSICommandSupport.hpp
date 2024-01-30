#pragma once

#include "HSSCSICommandDefine.hpp"

EHSSCSIStatusCode  HSSCSIStatusToStatusCode(const UHSSCSIStatus status );
bool  HSSCSI_Alloc_CommandData( THSSCSI_CommandData *pData );
