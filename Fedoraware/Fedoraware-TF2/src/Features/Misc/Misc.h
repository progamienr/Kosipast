#pragma once
#include "../Feature.h"

#ifdef DEBUG
#include <iostream>
#include <fstream>
#endif

class CMisc
{
	void AutoJump(CBaseEntity* pLocal, CUserCmd* pCmd);
	void AutoJumpbug(CBaseEntity* pLocal, CUserCmd* pCmd);
	void AutoStrafe(CBaseEntity* pLocal, CUserCmd* pCmd);
	void AntiBackstab(CBaseEntity* pLocal, CUserCmd* pCmd);
	void AutoPeek(CBaseEntity* pLocal, CUserCmd* pCmd);
	void AntiAFK(CBaseEntity* pLocal, CUserCmd* pCmd);
	void InstantRespawnMVM(CBaseEntity* pLocal);

	void CheatsBypass();
	int iLastCmdrate = -1;
	void PingReducer();
	void WeaponSway();

	void TauntKartControl(CBaseEntity* pLocal, CUserCmd* pCmd);
	void FastMovement(CBaseEntity* pLocal, CUserCmd* pCmd);
	void AntiWarp(CBaseEntity* pLocal, CUserCmd* pCmd);
	void LegJitter(CBaseEntity* pLocal, CUserCmd* pCmd, bool pSendPacket);

	bool bSteamCleared = false;
	Vec3 vPeekReturnPos = {};

public:
	void RunPre(CBaseEntity* pLocal, CUserCmd* pCmd);
	void RunPost(CBaseEntity* pLocal, CUserCmd* pCmd, bool pSendPacket);
	void Event(CGameEvent* pEvent, FNV1A_t uNameHash);
	void DoubletapPacket(CUserCmd* pCmd, bool* pSendPacket);
	void DetectChoke();

	void UnlockAchievements();
	void LockAchievements();
	void SteamRPC();

#ifdef DEBUG
	void DumpClassIDS();
#endif
};

ADD_FEATURE(CMisc, Misc)
