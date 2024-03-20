#include "ProjectileSimulation.h"
#pragma warning (disable : 4018)
#pragma warning (disable : 4552)

bool CProjectileSimulation::GetInfoMain(CBaseEntity* pPlayer, CBaseCombatWeapon* pWeapon, const Vec3& vAngles, ProjectileInfo& out, bool bTrace, bool bQuick, float flCharge)
{
	if (!pPlayer || !pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->IsTaunting() || !pWeapon)
		return false;

	const bool bDucking = pPlayer->m_fFlags() & FL_DUCKING;
	const bool bFlipped = g_ConVars.cl_flipviewmodels->GetBool();
	const float flGravity = g_ConVars.sv_gravity->GetFloat();

	Vec3 pos, ang;

	if (Vars::Visuals::PTOverwrite.Value)
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { Vars::Visuals::PTOffX.Value, Vars::Visuals::PTOffY.Value, Vars::Visuals::PTOffZ.Value }, pos, ang, !bTrace ? true : Vars::Visuals::PTPipes.Value, bQuick);
		out = { TF_PROJECTILE_NONE, pos, ang, { Vars::Visuals::PTHull.Value, Vars::Visuals::PTHull.Value, Vars::Visuals::PTHull.Value }, Vars::Visuals::PTSpeed.Value, Vars::Visuals::PTGravity.Value, Vars::Visuals::PTNoSpin.Value, Vars::Visuals::PTLifeTime.Value };
		return true;
	}

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_ROCKETLAUNCHER:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, G::CurItemDefIndex == Soldier_m_TheOriginal ? 0.f : 12.f, bDucking ? 8.f : -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		const float speed = pPlayer->IsPrecisionRune() ? 3000.f : Utils::ATTRIB_HOOK_FLOAT(1100.f, "mult_projectile_speed", pWeapon);
		out = { TF_PROJECTILE_ROCKET, pos, ang, { 0.f, 0.f, 0.f }, bQuick ? 3783722.f : speed, 0.f, true };
		return true;
	}
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_RAYGUN:
	case TF_WEAPON_DRG_POMSON:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, bDucking ? 8.f : -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		if (pWeapon->GetWeaponID() == TF_WEAPON_DRG_POMSON)
			pos.z -= 13.f;
		const float speed = pWeapon->GetWeaponID() == TF_WEAPON_PARTICLE_CANNON ? 1100.f : 1200.f;
		out = { TF_PROJECTILE_ENERGY_RING, pos, ang, { 1.f, 1.f, 1.f }, bQuick ? 3783722.f : speed, 0.f, true };
		return true;
	}
	case TF_WEAPON_GRENADELAUNCHER: // vphysics projectiles affected by server start gravity
	case TF_WEAPON_CANNON:
	{
		const bool bCannon = pWeapon->GetWeaponID() == TF_WEAPON_CANNON;
		const float flMortar = bCannon ? Utils::ATTRIB_HOOK_FLOAT(0.f, "grenade_launcher_mortar_mode", pWeapon) : 0.f;

		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		const float speed = pPlayer->IsPrecisionRune() ? 3000.f : Utils::ATTRIB_HOOK_FLOAT(1200.f, "mult_projectile_speed", pWeapon);
		const float lifetime = flMortar
			? pWeapon->m_flDetonateTime() > 0.f ? pWeapon->m_flDetonateTime() - I::GlobalVars->curtime : flMortar
			: Utils::ATTRIB_HOOK_FLOAT(2.2f, "fuse_mult", pWeapon);
		out = { bCannon ? TF_PROJECTILE_CANNONBALL : TF_PROJECTILE_PIPEBOMB, pos, ang, { 5.f, 5.f, 5.f }, speed, 1.f, G::CurItemDefIndex == Demoman_m_TheLochnLoad, lifetime };
		return true;
	}
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		const float charge = flCharge > 0.f && G::CurItemDefIndex != Demoman_s_StickyJumper
			? Utils::ATTRIB_HOOK_FLOAT(4.f, "stickybomb_charge_rate", pWeapon) * flCharge
			: (pWeapon->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->m_flChargeBeginTime() : 0.f);
		const float speed = Math::RemapValClamped(charge, 0.f, Utils::ATTRIB_HOOK_FLOAT(4.f, "stickybomb_charge_rate", pWeapon), 900.f, 2400.f);
		out = { TF_PROJECTILE_PIPEBOMB_REMOTE, pos, ang, { 5.f, 5.f, 5.f }, speed, 1.f, false };
		return true;
	}
	case TF_WEAPON_FLAREGUN:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_FLARE, pos, ang, { 1.f, 1.f, 1.f }, Utils::ATTRIB_HOOK_FLOAT(2000.f, "mult_projectile_speed", pWeapon), 0.000375f * flGravity /*0.3*/, true};
		return true;
	}
	case TF_WEAPON_RAYGUN_REVENGE:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 12.f, bDucking ? 8.f : -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_FLARE, pos, ang, { 1.f, 1.f, 1.f }, 3000.f, 0.0005625f * flGravity /*0.45*/, true };
		return true;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		const float charge = pWeapon->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->m_flChargeBeginTime() : 0.f;
		const float speed = Math::RemapValClamped(charge, 0.f, 1.f, 1800.f, 2600.f);
		const float gravity = Math::RemapValClamped(charge, 0.f, 1.f, 0.000625f, 0.000125f) * flGravity /*0.5, 0.1*/;
		out = { TF_PROJECTILE_ARROW, pos, ang, { 1.f, 1.f, 1.f }, speed, gravity, true, 10.f /*arrows have some lifetime check for whatever reason*/ };
		return true;
	}
	case TF_WEAPON_CROSSBOW:
	case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 23.5f, 8.f, -3.f }, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_ARROW, pos, ang, pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW ? Vec3(3.f, 3.f, 3.f) : Vec3(1.f, 1.f, 1.f), 2400.f, 0.00025f * flGravity /*0.2*/, true, 10.f /*arrows have some lifetime check for whatever reason*/ };
		return true;
	}
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 6.f, -8.f }, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_SYRINGE, pos, ang, { 1.f, 1.f, 1.f }, 1000.f, 0.000375f * flGravity /*0.3*/, true };
		return true;
	}
	case TF_WEAPON_FLAMETHROWER: // this inherits player velocity, possibly account for
	{
		const float flHull = I::Cvar->FindVar("tf_flamethrower_boxsize")->GetFloat();

		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 40.f, bFlipped ? -5.f : 5.f /*doesn't flip*/, 0.f}, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_FLAME_ROCKET, pos, ang, { flHull, flHull, flHull }, 1000.f, 0.f, true, 0.33f };
		return true;
	}
	case TF_WEAPON_FLAME_BALL:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 70.f, bFlipped ? -7.f : 7.f /*doesn't flip*/, -9.f}, pos, ang, !bTrace ? true : false, bQuick);
		out = { TF_PROJECTILE_BALLOFFIRE, pos, ang, { 1.f, 1.f, 1.f /*damaging hull much bigger, shouldn't matter here*/ }, 3000.f, 0.f, true, 0.2f };
		return true;
	}
	case TF_WEAPON_CLEAVER:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_CLEAVER, pos, ang, { 1.f, 1.f, 10.f /*weird, probably still inaccurate*/ }, 3000.f, 2.f, !bTrace ? true : false, 1.55f };
		return true;
	}
	case TF_WEAPON_BAT_WOOD:
	case TF_WEAPON_BAT_GIFTWRAP:
	{
		auto pLocal = g_EntityCache.GetLocal();
		if (!pLocal)
			return false;

		const bool bWrapAssassin = pWeapon->GetWeaponID() == TF_WEAPON_BAT_GIFTWRAP;
		
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 0.f, 0.f, 0.f }, pos, ang, true, bQuick);
		Vec3 forward; Math::AngleVectors(ang, &forward);
		pos = (bQuick ? pLocal->GetAbsOrigin() : pLocal->m_vecOrigin()) + (Vec3(0, 0, 50) + forward * 32.f) * pLocal->m_flModelScale(); // why?
		out = { bWrapAssassin ? TF_PROJECTILE_FESTIVE_ARROW : TF_PROJECTILE_THROWABLE, pos, ang, { 3.f, 3.f, 3.f }, 2000.f, 1.f, false, bWrapAssassin ? 2.3f : 100.f };
		return true;
	}
	case TF_WEAPON_JAR:
	case TF_WEAPON_JAR_MILK:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_JAR, pos, ang, { 3.f, 3.f, 3.f }, 1000.f, 1.f, false, 2.2f };
		return true;
	}
	case TF_WEAPON_JAR_GAS:
	{
		Utils::GetProjectileFireSetup(pPlayer, vAngles, { 16.f, 8.f, -6.f }, pos, ang, true, bQuick);
		out = { TF_PROJECTILE_JAR_GAS, pos, ang, { 3.f, 3.f, 3.f }, 2000.f, 1.f, false, 2.2f };
		return true;
	}
	}

	switch (G::CurItemDefIndex)
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
		out = { TF_PROJECTILE_BREAD_MONSTER, pos, ang, bQuick ? Vec3( 4.f, 4.f, 4.f ) : Vec3( 17.f, 17.f, 17.f ), 500.f, 0.00125f * flGravity /*1*/, false};
		return true;
	}
	}

	return false;
}

bool CProjectileSimulation::GetInfo(CBaseEntity* pPlayer, CBaseCombatWeapon* pWeapon, const Vec3& vAngles, ProjectileInfo& out, bool bTrace, bool bQuick, float flCharge)
{
	const float flOldCurrentTime = I::GlobalVars->curtime;
	if (pPlayer && bQuick)
		I::GlobalVars->curtime = TICKS_TO_TIME(pPlayer->m_nTickBase());
	const bool bReturn = !GetInfoMain(pPlayer, pWeapon, vAngles, out, bTrace, bQuick, flCharge);
	I::GlobalVars->curtime = flOldCurrentTime;

	if (bReturn)
		return false;
	if (!bQuick)
		return true;

	// visual check
	const Vec3 vStart = bQuick ? pPlayer->GetEyePosition() : pPlayer->GetShootPos();
	const Vec3 vEnd = out.m_vPos;

	CGameTrace trace = {};
	CTraceFilterProjectile filter = {};
	filter.pSkip = pPlayer;
	Utils::TraceHull(vStart, vEnd, out.m_vHull * -1.f, out.m_vHull, MASK_SOLID, &filter, &trace);
	if (trace.DidHit())
		return false;

	return true;
}

bool CProjectileSimulation::Initialize(const ProjectileInfo& info, bool bSimulate)
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

		obj = env->CreatePolyObject(col, 0, info.m_vPos, info.m_vAng, &params);

		obj->Wake();
	}

	if (!env || !obj)
		return false;

	//set drag
	{
		float drag = 0.f;
		Vec3 drag_basis = {};
		Vec3 ang_drag_basis = {};

		//these values were dumped from the server by firing the projectiles with 0 0 0 angles
		//they are calculated in CPhysicsObject::RecomputeDragBases
		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			drag = Vars::Visuals::PTDrag.Value;
			drag_basis = { Vars::Visuals::PTDragBasisX.Value, Vars::Visuals::PTDragBasisY.Value, Vars::Visuals::PTDragBasisZ.Value };
			ang_drag_basis = { Vars::Visuals::PTAngDragBasisX.Value, Vars::Visuals::PTAngDragBasisY.Value, Vars::Visuals::PTAngDragBasisZ.Value };
			break;
		case TF_PROJECTILE_PIPEBOMB:
			drag = 1.f;
			drag_basis = { 0.003902f, 0.009962f, 0.009962f };
			ang_drag_basis = { 0.003618f, 0.001514f, 0.001514f };
			break;
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			drag = 1.f;
			drag_basis = { 0.007491f, 0.007491f, 0.007306f };
			ang_drag_basis = { 0.002777f, 0.002842f, 0.002812f };
			break;
		case TF_PROJECTILE_CANNONBALL:
			drag = 1.f;
			drag_basis = { 0.020971f, 0.019420f, 0.020971f };
			ang_drag_basis = { 0.012997f, 0.013496f, 0.013714f };
			break;
		case TF_PROJECTILE_CLEAVER: // guesstimate
			drag = 1.f;
			drag_basis = { 0.020971f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_THROWABLE: // guesstimate
			drag = 1.f;
			drag_basis = { 0.008500f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_FESTIVE_ARROW: // guesstimate
			drag = 1.f;
			drag_basis = { 0.015500f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_JAR: // guesstimate (there are different drags for different models, though shouldn't matter here)
			drag = 1.f;
			drag_basis = { 0.005500f, 0.005000f, 0.f };
			break;
		case TF_PROJECTILE_JAR_GAS: // who
			drag = 1.f;
			drag_basis = { 0.027101f, 0.067938f, 0.f };
		}

		obj->SetDragCoefficient(&drag, &drag);

		obj->m_dragBasis = drag_basis;
		obj->m_angDragBasis = ang_drag_basis;
	}

	//set position and velocity
	{
		Vec3 forward, up;

		Math::AngleVectors(info.m_vAng, &forward, nullptr, &up);

		Vec3 vel = { forward * info.m_flVelocity };
		Vec3 ang_vel;

		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			vel += up * Vars::Visuals::PTUpVelocity.Value;
			ang_vel = { Vars::Visuals::PTAngVelocityX.Value, Vars::Visuals::PTAngVelocityY.Value, Vars::Visuals::PTAngVelocityZ.Value };
			break;
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
			vel += up * 200.f;
			ang_vel = { 600.f, -1200.f, 0.f };
			break;
		case TF_PROJECTILE_CLEAVER:
			vel += up * 300.f;
			ang_vel = { 300.f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_THROWABLE:
		case TF_PROJECTILE_FESTIVE_ARROW:
			vel += up * 200.f;
			ang_vel = { 100.f, 0.f, 0.f };
			break;
		case TF_PROJECTILE_JAR:
		case TF_PROJECTILE_JAR_GAS:
			vel += up * 200.f;
			ang_vel = { 300.f, 0.f, 0.f };
		}

		if (info.m_bNoSpin)
			ang_vel.Zero();

		if (bSimulate && obj->m_dragBasis.IsZero()) // don't include vphysics projectiles
			vel.z += 400.f * info.m_flGravity * TICK_INTERVAL; // i don't know why this makes it more accurate but it does

		obj->SetPosition(info.m_vPos, info.m_vAng, true);
		obj->SetVelocity(&vel, &ang_vel);
	}

	//set env params
	{
		float max_vel = 1000000.f;
		float max_ang_vel = 1000000.f;

		//only pipes need k_flMaxVelocity and k_flMaxAngularVelocity
		switch (info.m_iType)
		{
		case TF_PROJECTILE_NONE:
			if (Vars::Visuals::PTMaxVelocity.Value)
				max_vel = Vars::Visuals::PTMaxVelocity.Value;
			if (Vars::Visuals::PTMaxAngularVelocity.Value)
				max_ang_vel = Vars::Visuals::PTMaxAngularVelocity.Value;
			break;
		case TF_PROJECTILE_PIPEBOMB:
		case TF_PROJECTILE_PIPEBOMB_REMOTE:
		case TF_PROJECTILE_PIPEBOMB_PRACTICE:
		case TF_PROJECTILE_CANNONBALL:
			max_vel = k_flMaxVelocity;
			max_ang_vel = k_flMaxAngularVelocity;
		}

		physics_performanceparams_t params{};
		params.Defaults();

		params.maxVelocity = max_vel;
		params.maxAngularVelocity = max_ang_vel;

		env->SetPerformanceSettings(&params);
		env->SetAirDensity(2.f);
		env->SetGravity({ 0.f, 0.f, -(800.f * info.m_flGravity) });

		env->ResetSimulationClock(); // not needed?
	}

	return true;
}

void CProjectileSimulation::RunTick(ProjectileInfo& info)
{
	if (!env)
		return;

	info.PredictionLines.push_back({ GetOrigin(), Math::GetRotatedPosition(GetOrigin(), Math::VelocityToAngles(GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value) });

	env->Simulate(TICK_INTERVAL);

	/* // params.maxVelocity limits velocity uniformly
	Vec3 vVelocity, vAngular;
	obj->GetVelocity(&vVelocity, &vAngular);
	const float flMaxVel = I::Cvar->FindVar("sv_maxvelocity")->GetFloat();
	vVelocity = { std::clamp(vVelocity.x, -flMaxVel, flMaxVel), std::clamp(vVelocity.y, -flMaxVel, flMaxVel), std::clamp(vVelocity.z, -flMaxVel, flMaxVel) };
	obj->SetVelocity(&vVelocity, &vAngular);
	*/
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