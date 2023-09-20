#pragma once
#include "../../SDK/SDK.h"

class CNoSpread
{
private:
	bool IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon);
	bool ShouldRun(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);

public:
	void Run(CUserCmd* pCmd);
};

ADD_FEATURE(CNoSpread, NoSpread)