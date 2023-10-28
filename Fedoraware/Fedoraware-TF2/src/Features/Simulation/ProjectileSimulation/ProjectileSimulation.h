#pragma once

#include "../../Feature.h"

struct ProjectileInfo
{
	ETFProjectileType m_type{};

	Vec3 m_pos{};
	Vec3 m_ang{};
	Vec3 m_hull{};

	float m_velocity{};
	float m_gravity{};

	bool no_spin{};

	float m_lifetime = 10.f;

	std::deque<std::pair<Vec3, Vec3>> PredictionLines;
};

class CProjectileSimulation
{
	bool GetInfoMain(CBaseEntity* pPlayer, CBaseCombatWeapon* pWeapon, const Vec3& vAngles, ProjectileInfo& out, bool bQuick, float flCharge);

	const objectparams_t g_PhysDefaultObjectParams =
	{
		NULL,
		1.0, //mass
		1.0, // inertia
		0.1f, // damping
		0.1f, // rotdamping
		0.05f, // rotIntertiaLimit
		"DEFAULT",
		NULL,// game data
		0.f, // volume (leave 0 if you don't have one or call physcollision->CollideVolume() to compute it)
		1.0f, // drag coefficient
		true,// enable collisions?
	};

public:
	bool GetInfo(CBaseEntity* pPlayer, CBaseCombatWeapon* pWeapon, const Vec3& vAngles, ProjectileInfo& out, bool bQuick = false, float flCharge = -1.f);
	bool Initialize(const ProjectileInfo& info);
	void RunTick(ProjectileInfo& info);
	Vec3 GetOrigin();
	Vec3 GetVelocity();
};

ADD_FEATURE(CProjectileSimulation, ProjSim)