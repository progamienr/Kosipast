#pragma once
#include "../Feature.h"

#ifdef DEBUG
#include <iostream>
#include <fstream>
#endif

class CMisc
{
	void AutoJump(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AutoJumpbug(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AutoStrafe(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AntiBackstab(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AutoPeek(CUserCmd* pCmd, CBaseEntity* pLocal);
	void AntiAFK(CUserCmd* pCmd, CBaseEntity* pLocal);
	void InstantRespawnMVM(CBaseEntity* pLocal);

	void CheatsBypass();
	int iLastCmdrate = -1;
	void PingReducer();
	void DetectChoke();
	void WeaponSway();

	void TauntKartControl(CUserCmd* pCmd, CBaseEntity* pLocal);
	void FastStop(CUserCmd* pCmd, CBaseEntity* pLocal);
	void FastAccel(CUserCmd* pCmd, CBaseEntity* pLocal);
	void FastStrafe(CUserCmd* pCmd, CBaseEntity* pLocal);
	void InstaStop(CUserCmd* pCmd);
	void StopMovement(CUserCmd* pCmd, CBaseEntity* pLocal);
	void LegJitter(CUserCmd* pCmd, bool pSendPacket, CBaseEntity* pLocal);

	bool bSteamCleared = false;
	Vec3 vPeekReturnPos = {};

public:
	void RunPre(CUserCmd* pCmd);
	void RunPost(CUserCmd* pCmd, bool pSendPacket);
	void Event(CGameEvent* pEvent, FNV1A_t uNameHash);

	void UnlockAchievements();
	void LockAchievements();
	void SteamRPC();

	void DoubletapPacket(CUserCmd* pCmd, bool* pSendPacket);

#ifdef DEBUG
	void DumpClassIDS();
#endif
};

ADD_FEATURE(CMisc, Misc)
