#pragma once
#include "../../Feature.h"

class CNoSpreadProjectile
{
private:
	bool ShouldRun(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);

public:
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CNoSpreadProjectile, NoSpreadProjectile)