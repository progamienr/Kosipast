#pragma once
#include "../AimbotGlobal/AimbotGlobal.h"

class CAimbotProjectile
{
	/*
	struct ProjectileInfo_t
	{
		float m_flVelocity = 0.0f;
		float m_flGravity = 0.0f;
		float m_flMaxTime = 0.0f;
	};
	*/

	struct Solution_t
	{
		float m_flPitch = 0.0f;
		float m_flYaw = 0.0f;
		float m_flTime = 0.0f;
	};

	// new funcs
	std::vector<Target_t> GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);
	std::vector<Target_t> SortTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);

	int GetHitboxPriority(int nHitbox, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target);
	bool CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, const float flVelocity, const float flGravity, Solution_t& out);
	bool TestAngle(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target, const Vec3& vOriginal, const Vec3& vPredict, const Vec3& vAngles, const int& iSimTime);
	bool CanHit(Target_t& target, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon);
	bool RunMain(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);

	//bool GetSplashTarget(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Target_t& outTarget); implement splash

	void Aim(CUserCmd* pCmd, Vec3& vAngle);
	Vec3 Aim(Vec3 vCurAngle, Vec3 vToAngle);

	bool bLastTickHeld = false;
	bool bFlameThrower = false;

public:
	void Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd);

	bool bLastTickCancel = false;
};

ADD_FEATURE(CAimbotProjectile, AimbotProjectile)