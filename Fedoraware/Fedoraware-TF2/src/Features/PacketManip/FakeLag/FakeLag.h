#pragma once
#include "../../Feature.h"

class CFakeLag
{
	Vec3 vLastPosition;
	bool bPreservingBlast = false;
	bool bUnducking = false;

	bool IsAllowed(CBaseEntity* pLocal);
	void Prediction(CBaseEntity* pLocal, CUserCmd* pCmd);
	void PreserveBlastJump(CBaseEntity* pLocal);
	void Unduck(CBaseEntity* pLocal, CUserCmd* pCmd);

public:
	void Run(CBaseEntity* pLocal, CUserCmd* pCmd, bool* pSendPacket);
};

ADD_FEATURE(CFakeLag, FakeLag)
