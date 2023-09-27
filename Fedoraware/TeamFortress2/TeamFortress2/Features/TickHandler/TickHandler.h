#pragma once
#include "../../SDK/SDK.h"

class CTickshiftHandler
{
	// logic
	void Speedhack(CUserCmd* pCmd);
	void Recharge(CUserCmd* pCmd, CBaseEntity* pLocal);
	void Teleport(CUserCmd* pCmd);
	void Doubletap(const CUserCmd* pCmd, CBaseEntity* pLocal);

	// utils
	void CLMoveFunc(float accumulated_extra_samples, bool bFinalTick);

public:
	bool MeleeDoubletapCheck(CBaseEntity* pLocal);	//	checks if we WILL doubletap, used by melee aimbot from AimbotMelee.cpp
	void CLMove(float accumulated_extra_samples, bool bFinalTick);	//	to be run from CL_Move.cpp
	void CreateMove(CUserCmd* pCmd);								//	to be run from ClientModeShared_CreateMove.cpp
	void Reset();

	int iDeficit = 0;
	bool bSpeedhack = false;
	int iNextPassiveTick = 0;
	int iTickRate = 0;
};

ADD_FEATURE(CTickshiftHandler, Ticks)