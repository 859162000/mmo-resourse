#include "stdAfx.h"
#include "WGCityError.h"

uint WGCityErrorHandler::Execute( WGCityError* pPacket, Player* pPlayer )
{
__ENTER_FUNCTION
	return PACKET_EXE_CONTINUE;

__LEAVE_FUNCTION

	return PACKET_EXE_ERROR;
}
