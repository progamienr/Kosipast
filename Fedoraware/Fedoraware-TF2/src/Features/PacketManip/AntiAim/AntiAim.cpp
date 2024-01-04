#include "AntiAim.h"
#include "../../Misc/Misc.h"

void CAntiAim::FakeShotAngles(CUserCmd* pCmd)
{
	if (!Vars::AntiHack::AntiAim::InvalidShootPitch.Value || !G::IsAttacking || G::CurWeaponType != EWeaponType::HITSCAN)
		return;

	G::SilentAngles = true;
	pCmd->viewangles.x = CalculateCustomRealPitch(-pCmd->viewangles.x, false) + 180;
	pCmd->viewangles.y += 180;
}

float CAntiAim::EdgeDistance(float flEdgeRayYaw, CBaseEntity* pEntity)
{
	// Main ray tracing area
	CGameTrace trace;
	Ray_t ray;
	Vector forward;
	const float sy = sinf(DEG2RAD(flEdgeRayYaw)); // yaw
	const float cy = cosf(DEG2RAD(flEdgeRayYaw));
	constexpr float sp = 0.f; // pitch: sinf(DEG2RAD(0))
	constexpr float cp = 1.f; // cosf(DEG2RAD(0))
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
	forward = forward * 300.0f + pEntity->GetEyePosition();
	ray.Init(pEntity->GetEyePosition(), forward);
	// trace::g_pFilterNoPlayer to only focus on the enviroment
	CTraceFilterWorldAndPropsOnly Filter = {};
	I::EngineTrace->TraceRay(ray, 0x4200400B, &Filter, &trace);

	const float edgeDistance = (trace.vStartPos - trace.vEndPos).Length2D();
	return edgeDistance;
}

bool CAntiAim::GetEdge(const float flEdgeOrigYaw, CBaseEntity* pEntity)
{
	// distance two vectors and report their combined distances
	float flEdgeLeftDist = EdgeDistance(flEdgeOrigYaw - 21, pEntity) + EdgeDistance(flEdgeOrigYaw - 27, pEntity);
	float flEdgeRightDist = EdgeDistance(flEdgeOrigYaw + 21, pEntity) + EdgeDistance(flEdgeOrigYaw + 27, pEntity);

	// If the distance is too far, then set the distance to max so the angle
	// isnt used
	if (flEdgeLeftDist >= 260) { flEdgeLeftDist = 999999999.f; }
	if (flEdgeRightDist >= 260) { flEdgeRightDist = 999999999.f; }

	// Depending on the edge, choose a direction to face
	return flEdgeRightDist < flEdgeLeftDist;
}

bool CAntiAim::IsOverlapping(float epsilon = 45.f)
{
	if (!Vars::AntiHack::AntiAim::AntiOverlap.Value)
		return false;
	return std::abs(GetYawOffset(false) - GetYawOffset(true)) < epsilon;
}

float CAntiAim::GetYawOffset(const bool bFake)
{
	const int iMode = bFake ? Vars::AntiHack::AntiAim::YawFake.Value : Vars::AntiHack::AntiAim::YawReal.Value;
	switch (iMode)
	{
		case 1: return 0.f;
		case 2: return 90.f;
		case 3: return -90.f;
		case 4: return 180.f;
		case 5: return fmod(I::GlobalVars->tickcount * Vars::AntiHack::AntiAim::SpinSpeed.Value + 180.0f, 360.0f) - 180.0f;
		case 6: return (GetEdge() ? 1.f : -1.f) * (bFake ? -90.f : 90.f);
	}
	return 0.f;
}

float CAntiAim::GetBaseYaw(CBaseEntity* pLocal, CUserCmd* pCmd, const bool bFake)
{
	const int iMode = bFake ? Vars::AntiHack::AntiAim::FakeYawMode.Value : Vars::AntiHack::AntiAim::RealYawMode.Value;
	const float flOffset = bFake ? Vars::AntiHack::AntiAim::FakeYawOffset.Value : Vars::AntiHack::AntiAim::RealYawOffset.Value;
	//	0 offset, 1 at player, 2 at player + offset
	switch (iMode)
	{
	case 0: return pCmd->viewangles.y + flOffset;
	case 1:
	case 2:
	{
		float flSmallestAngleTo = 0.f; float flSmallestFovTo = 360.f;
		for (CBaseEntity* pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			if (!pEnemy || !pEnemy->IsAlive() || pEnemy->GetDormant()) // is enemy valid
				continue;

			PlayerInfo_t pInfo{ };
			if (I::EngineClient->GetPlayerInfo(pEnemy->GetIndex(), &pInfo))
			{
				if (G::IsIgnored(pInfo.friendsID))
					continue;
			}
			const Vec3 vAngleTo = Math::CalcAngle(pLocal->GetAbsOrigin(), pEnemy->GetAbsOrigin());
			const float flFOVTo = Math::CalcFov(I::EngineClient->GetViewAngles(), vAngleTo);

			if (flFOVTo < flSmallestFovTo) { flSmallestAngleTo = vAngleTo.y; flSmallestFovTo = flFOVTo; }
		}
		return (flSmallestFovTo == 360.f ? pCmd->viewangles.y + (iMode == 2 ? flOffset : 0) : flSmallestAngleTo + (iMode == 2 ? flOffset : 0));
	}
	}
	return pCmd->viewangles.y;
}

float CAntiAim::CalculateCustomRealPitch(float WishPitch, bool FakeDown)
{
	return FakeDown ? 720 + WishPitch : -720 + WishPitch;
}

float CAntiAim::GetPitch(const float flCurPitch)
{
	const int iFake = Vars::AntiHack::AntiAim::PitchFake.Value, iReal = Vars::AntiHack::AntiAim::PitchReal.Value;
	switch (iReal)
	{
		case 1: return iFake ? CalculateCustomRealPitch(-89.f, iFake - 1) : -89.f;
		case 2: return iFake ? CalculateCustomRealPitch(89.f, iFake - 1) : 89.f;
		case 3: return iFake ? CalculateCustomRealPitch(0.f, iFake - 1) : 0.f;
	}

	return iFake ? -89.f + (89.f * (iFake - 1)) : flCurPitch;
}

void CAntiAim::Run(CUserCmd* pCmd, bool* pSendPacket)
{
	if (F::KeyHandler.Pressed(Vars::AntiHack::AntiAim::ToggleKey.Value))
		Vars::AntiHack::AntiAim::Active.Value = !Vars::AntiHack::AntiAim::Active.Value;

	FakeShotAngles(pCmd);
	G::AntiAim = false;
	bSendingReal = !*pSendPacket;

	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!iNetChan || !pLocal)
		return;

	Vec2& vAngles = bSendingReal ? vRealAngles : vFakeAngles;
	vAngles = { pCmd->viewangles.x, pCmd->viewangles.y };
	
	if (!ShouldAntiAim(pLocal))
	{
		vRealAngles = { pCmd->viewangles.x, pCmd->viewangles.y };
		vFakeAngles = { pCmd->viewangles.x, pCmd->viewangles.y };
		return;
	}

	G::AntiAim = true;
	G::SilentAngles = true;

	if (!iNetChan->m_nChokedPackets) // get base yaw on the first choked tick.
		flBaseYaw = GetBaseYaw(pLocal, pCmd, false);
	vAngles = {
		GetPitch(pCmd->viewangles.x),
		(bSendingReal ? flBaseYaw : GetBaseYaw(pLocal, pCmd, true)) + GetYawOffset(!bSendingReal)
	};
	Utils::FixMovement(pCmd, vAngles);
	pCmd->viewangles.x = vAngles.x;
	pCmd->viewangles.y = vAngles.y;
}

bool CAntiAim::ShouldAntiAim(CBaseEntity* pLocal)
{
	const bool bPlayerReady = pLocal->IsAlive() && !pLocal->IsTaunting() && !pLocal->IsInBumperKart() && !pLocal->IsAGhost() && !G::IsAttacking;
	const bool bMovementReady = pLocal->GetMoveType() <= 5 && !pLocal->IsCharging() && !F::Misc.bMovementStopped && !F::Misc.bFastAccel;
	const bool bNotBusy = !G::AvoidingBackstab;
	const bool bEnabled = Vars::AntiHack::AntiAim::Active.Value;

	return bPlayerReady && bMovementReady && bNotBusy && bEnabled;
}