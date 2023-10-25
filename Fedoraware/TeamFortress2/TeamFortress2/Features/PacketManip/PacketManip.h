#pragma once
#include "../../SDK/SDK.h"
#include "FakeLag/FakeLag.h"
#include "AntiAim/AntiAim.h"

class CPacketManip {
private:
	bool WillTimeOut();
	bool AACheck(CUserCmd* pCmd);
public:
	void CreateMove(CUserCmd* pCmd, bool* pSendPacket, const int nOldGroundInt, const int nOldFlags);
};

ADD_FEATURE(CPacketManip, PacketManip)
// (Baan)