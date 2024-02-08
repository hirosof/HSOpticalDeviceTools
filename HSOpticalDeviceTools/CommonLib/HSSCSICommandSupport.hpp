#pragma once

#include "HSSCSICommandDefine.hpp"

EHSSCSIStatusCode  HSSCSIStatusToStatusCode(const UHSSCSIStatus status );
bool  HSSCSI_InitializeCommandData( THSSCSI_CommandData *pData );
