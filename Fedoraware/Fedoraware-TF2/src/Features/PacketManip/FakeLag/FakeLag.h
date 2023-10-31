#pragma once
#include "../../Feature.h"

class CFakeLag
{
	enum FakelagModes // Update this enum if you're adding/removing modes!
	{
		FL_Plain, 
		FL_Random, 
		FL_Adaptive 
	};

	int iAirTicks = 0; // if this goes above 14 we need to stop fakelag from going further or else the player will rubberband badly, this can be fixed i just don't know how lol
	Vec3 vLastPosition;
	bool bPreservingBlast = false;
	bool bUnducking = false;

	bool IsAllowed(CBaseEntity* pLocal);
	void Prediction(CUserCmd* pCmd);
	void PreserveBlastJump();
	void Unduck(CUserCmd* pCmd);

public:
	void Run(CUserCmd* pCmd, bool* pSendPacket);
};

ADD_FEATURE(CFakeLag, FakeLag)
