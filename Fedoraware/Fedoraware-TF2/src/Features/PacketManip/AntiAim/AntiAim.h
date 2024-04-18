#pragma once
#include "../../Feature.h"

class CAntiAim
{
private:
	void FakeShotAngles(CUserCmd* pCmd);

	float EdgeDistance(CBaseEntity* pEntity, float flEdgeRayYaw, float flOffset);
	void RunOverlapping(CBaseEntity* pEntity, CUserCmd* pCmd, float& flRealYaw, bool bFake, float flEpsilon = 45.f);
	float GetYawOffset(CBaseEntity* pEntity, bool bFake);
	float GetBaseYaw(CBaseEntity* pLocal, CUserCmd* pCmd, bool bFake);
	float GetYaw(CBaseEntity* pLocal, CUserCmd* pCmd, bool bFake);

	float CalculateCustomRealPitch(float flWishPitch, bool bFakeDown);
	float GetPitch(float flCurPitch);

public:
	bool AntiAimOn();
	bool ShouldRun(CBaseEntity* pLocal);

	bool GetEdge(CBaseEntity* pEntity, float flEdgeOrigYaw, bool bUpPitch);
	void Run(CBaseEntity* pLocal, CUserCmd* pCmd, bool* pSendPacket);

	Vec2 vFakeAngles = {};
	Vec2 vRealAngles = {};
	std::vector<std::pair<Vec3, Vec3>> vEdgeTrace = {};
};

ADD_FEATURE(CAntiAim, AntiAim)