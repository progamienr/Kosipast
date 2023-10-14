#pragma once

#include "../../../SDK/SDK.h"

class CAutoJump
{
	Vec3 vLastPos = {};
	bool bLastGrounded = false;

	int iJumpFrame = -1;
	int iRocketFrame = -1;

	bool bCTap = false;

public:
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoJump, AutoJump)