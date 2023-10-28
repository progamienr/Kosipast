#pragma once
#include "../../Feature.h"

#include "../AimbotGlobal/AimbotGlobal.h"

class CAimbotHitscan
{
	std::vector<Target_t> GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);
	std::vector<Target_t> SortTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);

	bool IsHitboxValid(int nHitbox);
	int GetHitboxPriority(int nHitbox, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CBaseEntity* pTarget);
	bool CanHit(Target_t& target, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);

	void Aim(CUserCmd* pCmd, Vec3& vAngle);
	Vec3 Aim(Vec3 vCurAngle, Vec3 vToAngle);
	bool ShouldFire(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, const CUserCmd* pCmd, const Target_t& target);
	bool IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon);

	bool bLastTickHeld = false; // classic

public:
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);
};

ADD_FEATURE(CAimbotHitscan, AimbotHitscan)