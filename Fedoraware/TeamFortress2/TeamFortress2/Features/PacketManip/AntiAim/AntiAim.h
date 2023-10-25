#pragma once
#include "../../SDK/SDK.h"

class CAntiAim
{
private:
	// utils
	void FixMovement(CUserCmd* pCmd, const Vec3& vOldAngles, float fOldSideMove, float fOldForwardMove);
	void FakeShotAngles(CUserCmd* pCmd);

	// logic
	float EdgeDistance(float flEdgeRayYaw, CBaseEntity* pEntity);
	bool IsOverlapping(float epsilon);

	// angles
	float GetYawOffset(const bool bFake);
	float GetBaseYaw(CBaseEntity* pLocal, CUserCmd* pCmd, const bool bFake);
	float CalculateCustomRealPitch(float WishPitch, bool FakeDown);
	float GetPitch(const float flCurPitch);

	float flBaseYaw = 0.f;

public:
	Vec2 vFakeAngles{};
	Vec2 vRealAngles{};
	bool bSendingReal = false; // for leg jitter

	bool GetEdge(const float flEdgeOrigYaw = I::EngineClient->GetViewAngles().y, CBaseEntity* pEntity = g_EntityCache.GetLocal());
	void Run(CUserCmd* pCmd, bool* pSendPacket);
	bool ShouldAntiAim(CBaseEntity* pLocal);
};

ADD_FEATURE(CAntiAim, AntiAim)