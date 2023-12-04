#pragma once
#include "../../Feature.h"

class CAutoJump
{
	void ManageAngle(CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Vec3& viewAngles);

	bool bLastGrounded = false;

	int iFrame = -1;
	bool bFull = false;
	int iDelay = 0;

public:
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoJump, AutoJump)