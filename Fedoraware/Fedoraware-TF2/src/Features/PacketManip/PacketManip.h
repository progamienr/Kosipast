#pragma once
#include "../Feature.h"
#include "FakeLag/FakeLag.h"
#include "AntiAim/AntiAim.h"

class CPacketManip
{
	bool WillTimeOut();
	bool AntiAimCheck(CBaseEntity* pLocal);

public:
	void Run(CBaseEntity* pLocal, CUserCmd* pCmd, bool* pSendPacket);
};

ADD_FEATURE(CPacketManip, PacketManip)
// (Baan)