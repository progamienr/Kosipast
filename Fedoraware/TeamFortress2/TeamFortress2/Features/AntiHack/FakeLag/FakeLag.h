#pragma once
#include "../../../SDK/SDK.h"

class CFakeLag
{
	enum FakelagModes // Update this enum if you're adding/removing modes!
	{
		FL_Plain, 
		FL_Random, 
		FL_Adaptive 
	};

	bool bPreservingBlast = false;
	bool bUnducking = false;
	std::pair<bool, int> pInAirTicks = {false, 0};	//	started on ground, ticks in air
	Vec3 vLastPosition;

	bool IsVisible(CBaseEntity* pLocal);
	bool IsAllowed(CBaseEntity* pLocal);
	void Prediction(const int nOldGroundInt, const int nOldFlags, CUserCmd* pCmd);	//	TODO: Create a struct for dealing with pre-prediction info.
	void PreserveBlastJump(const int nOldGroundInt, const int nOldFlags, CUserCmd* pCmd);
	void Unduck(const int nOldFlags);

public:
	void OnTick(CUserCmd* pCmd, bool* pSendPacket, const int nOldGround, const int nOldFlags);
};

ADD_FEATURE(CFakeLag, FakeLag)
