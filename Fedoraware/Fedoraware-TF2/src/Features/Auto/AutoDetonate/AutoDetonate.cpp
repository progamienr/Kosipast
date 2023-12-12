#include "AutoDetonate.h"

#include "../../Vars.h"

bool CAutoDetonate::CheckDetonation(CBaseEntity* pLocal, const std::vector<CBaseEntity*>& entityGroup, float radius, CUserCmd* pCmd)
{
	for (const auto& pExplosive : entityGroup)
	{
		if (pExplosive->GetPipebombType() == TYPE_STICKY)
		{
			if (!pExplosive->GetPipebombPulsed())
				continue;
		}

		// Iterate through entities in sphere radius
		CBaseEntity* pTarget;
		for (CEntitySphereQuery sphere(pExplosive->GetWorldSpaceCenter(), radius);
			 (pTarget = sphere.GetCurrentEntity()) != nullptr;
			 sphere.NextEntity())
		{
			if (!pTarget || pTarget == pLocal || !pTarget->IsAlive() || pTarget->IsPlayer() && pTarget->IsAGhost() || pTarget->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			const Vec3 vOrigin = pExplosive->GetWorldSpaceCenter();
			Vec3 vPos = {};
			pTarget->GetCollision()->CalcNearestPoint(vOrigin, &vPos);
			if (vOrigin.DistTo(vPos) > radius)
				continue;

			const bool isPlayer = Vars::Auto::Detonate::DetonateTargets.Value & PLAYER && pTarget->IsPlayer();
			const bool isSentry = Vars::Auto::Detonate::DetonateTargets.Value & SENTRY && pTarget->GetClassID() == ETFClassID::CObjectSentrygun;
			const bool isDispenser = Vars::Auto::Detonate::DetonateTargets.Value & DISPENSER && pTarget->GetClassID() == ETFClassID::CObjectDispenser;
			const bool isTeleporter = Vars::Auto::Detonate::DetonateTargets.Value & TELEPORTER && pTarget->GetClassID() == ETFClassID::CObjectTeleporter;
			const bool isNPC = Vars::Auto::Detonate::DetonateTargets.Value & NPC && pTarget->IsNPC();
			const bool isBomb = Vars::Auto::Detonate::DetonateTargets.Value & BOMB && pTarget->IsBomb();
			const bool isSticky = Vars::Auto::Detonate::DetonateTargets.Value & STICKY && pTarget->GetClassID() == ETFClassID::CTFGrenadePipebombProjectile && pTarget->GetPipebombType() == TYPE_STICKY && (G::CurItemDefIndex == Demoman_s_TheQuickiebombLauncher || G::CurItemDefIndex == Demoman_s_TheScottishResistance);

			if (isPlayer || isSentry || isDispenser || isTeleporter || isNPC || isBomb || isSticky)
			{
				if (isPlayer && F::AutoGlobal.ShouldIgnore(pTarget))
					continue;

				if (!Utils::VisPos(pExplosive, pTarget, vOrigin, isPlayer ? pTarget->m_vecOrigin() + pTarget->GetViewOffset() : pTarget->GetWorldSpaceCenter(), MASK_SOLID))
					continue;

				if (G::CurItemDefIndex == Demoman_s_TheScottishResistance)
				{	//	super fucking ghetto holy shit
					Vec3 vAngleTo = Math::CalcAngle(pLocal->GetWorldSpaceCenter(), vOrigin);
					Utils::FixMovement(pCmd, vAngleTo);
					pCmd->viewangles = vAngleTo;
					G::SilentTime = true;
				}
				return true;
			}
		}
	}

	return false;
}

void CAutoDetonate::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Auto::Detonate::Active.Value) { return; }

	float flStickyRadius = Utils::ATTRIB_HOOK_FLOAT(148.0f, "mult_explosion_radius", pWeapon);
	for (const auto& pExplosive : g_EntityCache.GetGroup(EGroupType::LOCAL_STICKIES))
	{
		float flRadiusMod = 1.0f;
		if (pExplosive->GetPipebombType() == TYPE_STICKY)
		{
			if (pExplosive->GetTouched() == false)
			{
				float flArmTime = I::Cvar->FindVar("tf_grenadelauncher_livetime")->GetFloat(); //0.8
				float RampTime = I::Cvar->FindVar("tf_sticky_radius_ramp_time")->GetFloat(); //2.0
				float AirDetRadius = I::Cvar->FindVar("tf_sticky_airdet_radius")->GetFloat(); //0.85
				flRadiusMod *= Math::RemapValClamped(I::GlobalVars->curtime - pExplosive->GetCreationTime(), flArmTime, flArmTime + RampTime, AirDetRadius, 1.0f);
			}
			flStickyRadius *= flRadiusMod;
		}
	}

	bool shouldDetonate = false;

	// Check sticky detonation
	if (Vars::Auto::Detonate::Stickies.Value
		&& CheckDetonation(pLocal, g_EntityCache.GetGroup(EGroupType::LOCAL_STICKIES), flStickyRadius * Vars::Auto::Detonate::RadiusScale.Value / 100, pCmd))
	{
		shouldDetonate = true;
	}

	// Check flare detonation
	if (Vars::Auto::Detonate::Flares.Value
		&& CheckDetonation(pLocal, g_EntityCache.GetGroup(EGroupType::LOCAL_FLARES), 110.0f * Vars::Auto::Detonate::RadiusScale.Value / 100, pCmd))
	{
		shouldDetonate = true;
	}

	if (!shouldDetonate) { return; }
	pCmd->buttons |= IN_ATTACK2;
}
