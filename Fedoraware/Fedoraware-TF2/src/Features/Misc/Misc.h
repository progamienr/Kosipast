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
	void AntiBackstab(CBaseEntity* pLocal, CUserCmd* pCmd);
	void AutoPeek(CUserCmd* pCmd, CBaseEntity* pLocal);

	void AntiAFK(CBaseEntity* pLocal, CUserCmd* pCmd);
	void CheatsBypass();
	int iLastCmdrate = -1;
	void PingReducer();
	void DetectChoke();
	void WeaponSway();

	void TauntKartControl(CUserCmd* pCmd, bool* pSendPacket);
	void FastStop(CUserCmd* pCmd, CBaseEntity* pLocal);
	void FastAccel(CUserCmd* pCmd, CBaseEntity* pLocal, bool* pSendPacket);
	void FastStrafe(CUserCmd* pCmd, bool* pSendPacket);
	void InstaStop(CUserCmd* pCmd, bool* pSendPacket);
	void StopMovement(CUserCmd* pCmd, CBaseEntity* pLocal);
	void LegJitter(CUserCmd* pCmd, CBaseEntity* pLocal);
	void DoubletapPacket(CUserCmd* pCmd, bool* pSendPacket);

	bool SteamCleared = false;
	bool bMovementScuffed = false;

	void PrintProjAngles(CBaseEntity* pLocal);
public:
#ifdef DEBUG
	void DumpClassIDS();
#endif

	void RunPre(CUserCmd* pCmd);
	void RunPost(CUserCmd* pCmd, bool* pSendPacket);
	void Event(CGameEvent* pEvent, FNV1A_t uNameHash);

	void UnlockAchievements();
	void LockAchievements();
	void SteamRPC();

	bool bAntiWarp = false;
	bool bFastAccel = false;
	bool bMovementStopped = false;

	Vec3 PeekReturnPos;
};

ADD_FEATURE(CMisc, Misc)
