#pragma once
#include "../Feature.h"
#include "FakeLag/FakeLag.h"
#include "AntiAim/AntiAim.h"

class CPacketManip {
private:
	bool WillTimeOut();
	bool AACheck(CUserCmd* pCmd);
public:
	void CreateMove(CUserCmd* pCmd, bool* pSendPacket);
};

ADD_FEATURE(CPacketManip, PacketManip)
// (Baan)