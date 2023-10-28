#pragma once
#include "../../Feature.h"

class CAutoJump
{
	Vec3 vLastPos = {};
	bool bLastGrounded = false;

	int iJumpFrame = -1;

	bool bFire = false;
	bool bCTap = false;

public:
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAutoJump, AutoJump)