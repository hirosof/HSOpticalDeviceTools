#pragma once
#include "CHSOpticalDrive.hpp"
#include "CHSOpticalDriveGetConfigCmd.hpp"



class  CHSOpticalDiscReader {
private:
	CHSOpticalDrive* mp_Drive;
	CHSOpticalDriveGetConfigCmd m_cmd;

public:

	CHSOpticalDiscReader( );
	CHSOpticalDiscReader( CHSOpticalDrive* pDrive );
	void SetDrive( CHSOpticalDrive* pDrive );


};