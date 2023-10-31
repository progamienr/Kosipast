#include "ProjectileSimulation.h"
#pragma warning (disable : 4018)
#pragma warning (disable : 4552)

IPhysicsEnvironment* env = nullptr;
IPhysicsObject* obj = nullptr;

bool CProjectileSimulation::GetInfoMain(CBaseEntity* pPlayer, CBaseCombatWeapon* pWeapon, const Vec3& vAngles, ProjectileInfo& out, bool bQuick, float flCharge) // possibly refine values and magic numbers
{
	if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsTaunting() || !pWeapon)
		return false;

	ConVar* cl_flipviewmodels = g_ConVars.cl_flipviewmodels;
	if (!cl_flipviewmodels)
		return false;

	const bool bDucking = pPlayer->m_fFlags() & FL_DUCKING;
	const bool bFlipped = cl_flipviewmodels->GetBool();

	Vec3 pos, ang;

	if (Vars::Visuals::PTOverwrite.Value)
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { Vars::Visuals::PTOffX.Value, Vars::Visuals::PTOffY.Value, Vars::Visuals::PTOffZ.Value }, pos, ang, Vars::Visuals::PTPipes.Value, bQuick);
		out = { static_cast<ETFProjectileType>(Vars::Visuals::PTType.Value), pos, ang, { Vars::Visuals::PTHull.Value, Vars::Visuals::PTHull.Value, Vars::Visuals::PTHull.Value }, Vars::Visuals::PTSpeed.Value, Vars::Visuals::PTGravity.Value, Vars::Visuals::PTNoSpin.Value, Vars::Visuals::PTLifeTime.Value };
		return true;
	}

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_ROCKETLAUNCHER:
	{
		if (pWeapon->GetItemDefIndex() == Soldier_m_TheOriginal)
			Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 0.f, bDucking ? 8.f : -3.f }, pos, ang, false, bQuick);
		else
			Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, pos, ang, false, bQuick);
		out = { TF_PROJECTILE_ROCKET, pos, ang, { 1.f, 1.f, 1.f /*0.f, 0.f, 0.f i think is real size*/ }, bQuick ? 1081344.f : Utils::ATTRIB_HOOK_FLOAT(1100.f, "mult_projectile_speed", pWeapon), 0.f, true };
		return true;
	}
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_RAYGUN:
	case TF_WEAPON_DRG_POMSON:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, bDucking ? 8.f : -3.f }, pos, ang, false, bQuick);
		float speed = 1200.f;
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_PARTICLE_CANNON: speed = Utils::ATTRIB_HOOK_FLOAT(1100.f, "mult_projectile_speed", pWeapon); break;
		case TF_WEAPON_DRG_POMSON: pos.z -= 13.f; break;
		}
		out = { TF_PROJECTILE_ENERGY_RING, pos, ang, { 1.f, 1.f, 1.f }, bQuick ? 1081344.f : speed, 0.f, true };
		return true;
	}
	case TF_WEAPON_GRENADELAUNCHER:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		const bool is_lochnload = G::CurItemDefIndex == Demoman_m_TheLochnLoad;
		float speed = is_lochnload ? 1490.f : 1200.f;
		if (pPlayer->IsPrecisionRune())
			speed *= 2.5f;
		float lifetime = 2.2f; // estimates
		if (G::CurItemDefIndex == Demoman_m_TheIronBomber)
			lifetime *= 0.7f;
		out = { TF_PROJECTILE_PIPEBOMB, pos, ang, { 4.f, 4.f, 4.f }, speed, 1.f, is_lochnload, lifetime };
		return true;
	}
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		const float charge = flCharge > 0.f
			? Utils::ATTRIB_HOOK_FLOAT(4.f, "stickybomb_charge_rate", pWeapon) * flCharge
			: (pWeapon->GetChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->GetChargeBeginTime() : 0.f);
		const float speed = Math::RemapValClamped(charge, 0.f, Utils::ATTRIB_HOOK_FLOAT(4.f, "stickybomb_charge_rate", pWeapon), 900.f, 2400.f);
		out = { TF_PROJECTILE_PIPEBOMB_REMOTE, pos, ang, { 4.f, 4.f, 4.f }, speed, 1.f, false };
		return true;
	}
	case TF_WEAPON_CANNON:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		float speed = 1454.f;
		if (pPlayer->IsPrecisionRune())
			speed *= 2.5f;
		float lifetime = pWeapon->GetDetonateTime() - I::GlobalVars->curtime;
		if (pWeapon->GetDetonateTime() <= 0.f)
			lifetime = 1.06f;
		out = { TF_PROJECTILE_CANNONBALL, pos, ang, { 4.f, 4.f, 4.f }, speed, 1.f, false, lifetime };
		return true;
	}
	case TF_WEAPON_FLAREGUN:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, pos, ang, false, bQuick);
		out = { TF_PROJECTILE_FLARE, pos, ang, { 1.f, 1.f, 1.f }, 2000.f, 0.3f, true };
		return true;
	}
	case TF_WEAPON_RAYGUN_REVENGE:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, pos, ang, false, bQuick);
		out = { TF_PROJECTILE_FLARE, pos, ang, { 1.f, 1.f, 1.f }, 3000.f, 0.45f, true };
		return true;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, pos, ang, false, bQuick);
		const float charge = pWeapon->GetChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->GetChargeBeginTime() : 0.f;
		const float speed = Math::RemapValClamped(charge, 0.f, 1.f, 1800.f, 2600.f);
		const float gravity = Math::RemapValClamped(charge, 0.f, 1.f, 0.5f, 0.1f);
		out = { TF_PROJECTILE_ARROW, pos, ang, { 1.f, 1.f, 1.f }, speed, gravity, true };
		return true;
	}
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, pos, ang, false, bQuick);
		out = { TF_PROJECTILE_ARROW, pos, ang, { 3.f, 3.f, 3.f }, 2400.f, 0.2f, true };
		return true;
	}
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 6.f, -8.f }, pos, ang, false, bQuick);
		out = { TF_PROJECTILE_SYRINGE, pos, ang, { 1.f, 1.f, 1.f }, 1000.f, 0.3f, true };
		return true;
	}
	case TF_WEAPON_FLAME_BALL:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 70.f, bFlipped ? -7.f : 7.f /*doesn't flip*/, -9.f}, pos, ang, false, bQuick);
		out = { TF_PROJECTILE_BALLOFFIRE, pos, ang, { 1.f, 1.f, 1.f }, 3000.f, 0.f, true, 0.2f };
		// unrelated note to self: flamethrower offset { 40, 5, 0 }
		return true;
	}
	case TF_WEAPON_CLEAVER:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_CLEAVER, pos, ang, { 1.5f, 1.5f, 1.5f }, 3000.f, 2.f, false };
		return true;
	}
	case TF_WEAPON_BAT_WOOD:
	case TF_WEAPON_BAT_GIFTWRAP:
	{
		auto pLocal = g_EntityCache.GetLocal();
		if (!pLocal)
			return false;

		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 0.f, 0.f, 0.f }, pos, ang, true, bQuick);
		Vec3 forward; Math::AngleVectors(ang, &forward);
		pos = (bQuick ? pLocal->GetAbsOrigin() : pLocal->GetVecOrigin()) + (Vec3(0, 0, 50) + forward * 32.f) * pLocal->m_flModelScale(); // why?
		out = { TF_PROJECTILE_THROWABLE, pos, ang, { 3.f, 3.f, 3.f }, 2000.f, 1.f, false };
		return true;
	}
	case TF_WEAPON_JAR:
	case TF_WEAPON_JAR_MILK:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_JAR, pos, ang, { 1.5f, 1.5f, 1.5f }, 1000.f, 1.f, false, 2.2f };
		return true;
	}
	case TF_WEAPON_JAR_GAS:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_JAR_GAS, pos, ang, { 1.5f, 1.5f, 1.5f }, 2000.f, 1.f, false, 2.2f };
		return true;
	}
	}

	switch (pWeapon->GetItemDefIndex())
	{
	case Heavy_s_RoboSandvich:
	case Heavy_s_Sandvich:
	case Heavy_s_FestiveSandvich:
	case Heavy_s_Fishcake:
	case Heavy_s_TheDalokohsBar:
	case Heavy_s_SecondBanana:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 0.f, 0.f, -8.f }, pos, ang, true, bQuick);
		ang -= Vec3(10, 0, 0);
		out = { TF_PROJECTILE_NONE, pos, ang, bQuick ? Vec3( 4.f, 4.f, 4.f ) : Vec3( 17.f, 17.f, 17.f ), 500.f, 1.f, false, 10.f };
		return true;
	}
	}

	return false;
}

bool CProjectileSimulation::GetInfo(CBaseEntity* pPlayer, CBaseCombatWeapon* pWeapon, const Vec3& vAngles, ProjectileInfo& out, bool bQuick, float flCharge)
{
	if (!GetInfoMain(pPlayer, pWeapon, vAngles, out, bQuick, flCharge))
		return false;

	const Vec3 vStart = bQuick ? pPlayer->GetEyePosition() : pPlayer->GetShootPos();
	const Vec3 vEnd = out.m_pos;

	CGameTrace trace = {};
	CTraceFilterProjectile filter = {};
	filter.pSkip = pPlayer;
	Utils::TraceHull(vStart, vEnd, out.m_hull * -1.f, out.m_hull, MASK_SOLID, &filter, &trace);
	if (trace.DidHit())
		return false;

	return true;
}

bool CProjectileSimulation::Initialize(const ProjectileInfo& info)
{
	if (!env)
		env = I::Physics->CreateEnvironment();

	if (!obj)
	{
		//it doesn't matter what the size is for non drag affected projectiles
		//pipes use the size below so it works out just fine
		auto col{ I::PhysicsCollision->BBoxToCollide({ -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }) };

		auto params{ g_PhysDefaultObjectParams };

		params.damping = 0.f;
		params.rotdamping = 0.f;
		params.inertia = 0.f;
		params.rotInertiaLimit = 0.f;
		params.enableCollisions = false;

		obj = env->CreatePolyObject(col, 0, info.m_pos, info.m_ang, &params);

		obj->Wake();
	}

	if (!env || !obj)
		return false;

	//set position and velocity
	{
		Vec3 forward, up;

		Math::AngleVectors(info.m_ang, &forward, nullptr, &up);

		Vec3 vel = { forward * info.m_velocity };
		Vec3 ang_vel;

		if (Vars::Visuals::PTOverwrite.Value)
		{
			vel += up * Vars::Visuals::PTUpVelocity.Value;
			ang_vel = { Vars::Visuals::PTAngVelocityX.Value, Vars::Visuals::PTAngVelocityY.Value, Vars::Visuals::PTAngVelocityZ.Value };
		}
		else
		{
			switch (info.m_type)
			{
			case TF_PROJECTILE_PIPEBOMB:
			case TF_PROJECTILE_PIPEBOMB_REMOTE:
			case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			case TF_PROJECTILE_CANNONBALL:
			{
				// CTFWeaponBaseGun::FirePipeBomb
				// pick your poison
				vel += up * 200.f;
				ang_vel = { 600.f, -1200.f, 0.f };

				break;
			}
			case TF_PROJECTILE_CLEAVER:
			{
				// CTFCleaver::GetVelocityVector
				vel += up * 300.f;
				// CTFProjectile_Throwable::GetAngularImpulse
				ang_vel = { 300.f, 0.f, 0.f };

				break;
			}
			case TF_PROJECTILE_THROWABLE:
			{
				// CTFBat_Wood::GetBallDynamics
				vel += up * 200.f;
				ang_vel = { 100.f, 0.f, 0.f };

				break;
			}
			case TF_PROJECTILE_JAR:
			case TF_PROJECTILE_JAR_GAS:
			{
				vel += up * 200.f;
				ang_vel = { 300.f, 0.f, 0.f };

				break;
			}
			default: break;
			}
		}

		if (info.no_spin)
			ang_vel.Zero();

		obj->SetPosition(info.m_pos, info.m_ang, true);
		obj->SetVelocity(&vel, &ang_vel);
	}

	//set drag
	{
		float drag;
		Vec3 drag_basis;
		Vec3 ang_drag_basis;

		if (Vars::Visuals::PTOverwrite.Value)
		{
			drag = Vars::Visuals::PTDrag.Value;
			drag_basis = { Vars::Visuals::PTDragBasisX.Value, Vars::Visuals::PTDragBasisY.Value, Vars::Visuals::PTDragBasisZ.Value };
			ang_drag_basis = { Vars::Visuals::PTAngDragBasisX.Value, Vars::Visuals::PTAngDragBasisY.Value, Vars::Visuals::PTAngDragBasisZ.Value };
		}
		else
		{
			//these values were dumped from the server by firing the projectiles with 0 0 0 angles
			//they are calculated in CPhysicsObject::RecomputeDragBases
			switch (info.m_type)
			{
			case TF_PROJECTILE_PIPEBOMB:
			{
				drag = 1.f;
				drag_basis = { 0.003902f, 0.009962f, 0.009962f };
				ang_drag_basis = { 0.003618f, 0.001514f, 0.001514f };

				break;
			}
			case TF_PROJECTILE_PIPEBOMB_REMOTE:
			case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			{
				drag = 1.f;
				drag_basis = { 0.007491f, 0.007491f, 0.007306f };
				ang_drag_basis = { 0.002777f, 0.002842f, 0.002812f };

				break;
			}
			case TF_PROJECTILE_CANNONBALL:
			{
				drag = 1.f;
				drag_basis = { 0.020971f, 0.019420f, 0.020971f };
				ang_drag_basis = { 0.012997f, 0.013496f, 0.013714f };

				break;
			}
			case TF_PROJECTILE_CLEAVER:
			{
				// guesstimate
				drag = 1.f;
				drag_basis = { 0.020971f, 0.f, 0.f };

				break;
			}
			case TF_PROJECTILE_THROWABLE:
			{
				// guesstimate
				drag = 1.f;
				drag_basis = { 0.010500f, 0.f, 0.f };

				break;
			}
			case TF_PROJECTILE_JAR:
			{
				// guesstimate (there are different drags for different models, though shouldn't matter here)
				drag = 1.f;
				drag_basis = { 0.003902f, 0.f, 0.f };

				break;
			}
			case TF_PROJECTILE_JAR_GAS:
			{
				// who
				drag = 1.f;
				drag_basis = { 0.027101f, 0.067938f, 0.f };

				break;
			}
			default: break;
			}
		}

		obj->SetDragCoefficient(&drag, &drag);

		obj->m_dragBasis = drag_basis;
		obj->m_angDragBasis = ang_drag_basis;
	}

	//set env params
	{
		float max_vel = 1000000.f;
		float max_ang_vel = 1000000.f;

		//only pipes need k_flMaxVelocity and k_flMaxAngularVelocity
		switch (info.m_type)
		{
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
		{
			max_vel = k_flMaxVelocity;
			max_ang_vel = k_flMaxAngularVelocity;

			break;
		}
		default: break;
		}

		physics_performanceparams_t params{};
		params.Defaults();

		params.maxVelocity = max_vel;
		params.maxAngularVelocity = max_ang_vel;

		env->SetPerformanceSettings(&params);
		env->SetAirDensity(2.f);
		env->SetGravity({ 0.f, 0.f, -(800.f * info.m_gravity) });

		env->ResetSimulationClock(); //not needed?
	}

	return true;
}

void CProjectileSimulation::RunTick(ProjectileInfo& info)
{
	if (!env)
		return;

	info.PredictionLines.push_back({ GetOrigin(), Math::GetRotatedPosition(GetOrigin(), Math::VelocityToAngles(GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value) });

	env->Simulate(TICK_INTERVAL);
}

Vec3 CProjectileSimulation::GetOrigin()
{
	if (!obj)
		return {};

	Vec3 out;

	obj->GetPosition(&out, nullptr);

	return out;
}

Vec3 CProjectileSimulation::GetVelocity()
{
	if (!obj)
		return {};

	Vec3 out;

	obj->GetVelocity(&out, nullptr);

	return out;
}