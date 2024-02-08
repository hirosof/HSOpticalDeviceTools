#include "CHSOpticalDiscReader.hpp"

CHSOpticalDiscReader::CHSOpticalDiscReader( ) {
	this->SetDrive( nullptr );
}

CHSOpticalDiscReader::CHSOpticalDiscReader( CHSOpticalDrive* pDrive ) {
	this->SetDrive( pDrive );
}

void CHSOpticalDiscReader::SetDrive( CHSOpticalDrive* pDrive ) {
	this->mp_Drive = pDrive;
	this->m_cmd.SetDrive( pDrive );
}
