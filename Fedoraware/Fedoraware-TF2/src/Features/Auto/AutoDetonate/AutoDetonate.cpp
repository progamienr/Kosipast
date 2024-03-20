#include "AutoDetonate.h"

#include "../../Vars.h"

bool CAutoDetonate::CheckDetonation(CBaseEntity* pLocal, EGroupType entityGroup, float flRadiusScale, CUserCmd* pCmd)
{
	for (const auto& pExplosive : g_EntityCache.GetGroup(entityGroup))
	{
		if (pExplosive->m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE || !pExplosive->m_bPulsed())
			continue;

		const Vec3 vOrigin = pExplosive->GetWorldSpaceCenter();
		if (entityGroup == EGroupType::LOCAL_STICKIES)
		{
			if (pExplosive->m_iType() == TF_GL_MODE_REMOTE_DETONATE && !pExplosive->m_bTouched())
			{
				float tf_grenadelauncher_livetime = I::Cvar->FindVar("tf_grenadelauncher_livetime")->GetFloat();
				float tf_sticky_radius_ramp_time = I::Cvar->FindVar("tf_sticky_radius_ramp_time")->GetFloat();
				float tf_sticky_airdet_radius = I::Cvar->FindVar("tf_sticky_airdet_radius")->GetFloat();
				flRadiusScale *= Math::RemapValClamped(I::GlobalVars->curtime - pExplosive->m_flCreationTime(), tf_grenadelauncher_livetime, tf_grenadelauncher_livetime + tf_sticky_radius_ramp_time, tf_sticky_airdet_radius, 1.f);
			}
		}
		float flRadius = (entityGroup == EGroupType::LOCAL_STICKIES ? 146.f : 110.f) * flRadiusScale;

		// Iterate through entities in sphere radius
		CBaseEntity* pTarget;
		for (CEntitySphereQuery sphere(vOrigin, flRadius);
			(pTarget = sphere.GetCurrentEntity()) != nullptr;
			sphere.NextEntity())
		{
			if (!pTarget || pTarget == pLocal || !pTarget->IsAlive() || pTarget->IsPlayer() && pTarget->IsAGhost() || pTarget->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			// CEntitySphereQuery actually does a box test so we need to make sure the distance is less than the radius first
			Vec3 vPos = {}; pTarget->GetCollision()->CalcNearestPoint(vOrigin, &vPos);
			if (vOrigin.DistTo(vPos) > flRadius)
				continue;

			const bool isPlayer = Vars::Auto::Detonate::DetonateTargets.Value & PLAYER && pTarget->IsPlayer();
			const bool isSentry = Vars::Auto::Detonate::DetonateTargets.Value & SENTRY && pTarget->IsSentrygun();
			const bool isDispenser = Vars::Auto::Detonate::DetonateTargets.Value & DISPENSER && pTarget->IsDispenser();
			const bool isTeleporter = Vars::Auto::Detonate::DetonateTargets.Value & TELEPORTER && pTarget->IsTeleporter();
			const bool isNPC = Vars::Auto::Detonate::DetonateTargets.Value & NPC && pTarget->IsNPC();
			const bool isBomb = Vars::Auto::Detonate::DetonateTargets.Value & BOMB && pTarget->IsBomb();
			const bool isSticky = Vars::Auto::Detonate::DetonateTargets.Value & STICKY && pTarget->GetClassID() == ETFClassID::CTFGrenadePipebombProjectile && pTarget->m_iType() == TF_GL_MODE_REMOTE_DETONATE && (G::CurItemDefIndex == Demoman_s_TheQuickiebombLauncher || G::CurItemDefIndex == Demoman_s_TheScottishResistance);
			if (isPlayer || isSentry || isDispenser || isTeleporter || isNPC || isBomb || isSticky)
			{
				if (isPlayer && F::AutoGlobal.ShouldIgnore(pTarget))
					continue;

				if (!Utils::VisPosProjectile(pExplosive, pTarget, vOrigin, isPlayer ? pTarget->m_vecOrigin() + pTarget->GetViewOffset() : pTarget->GetWorldSpaceCenter(), MASK_SHOT))
					continue;

				if (G::CurItemDefIndex == Demoman_s_TheScottishResistance)
				{
					Vec3 vAngleTo = Math::CalcAngle(pLocal->GetShootPos(), vOrigin);
					Utils::FixMovement(pCmd, vAngleTo);
					pCmd->viewangles = vAngleTo;
					G::PSilentAngles = true;
				}
				return true;
			}
		}
	}

	return false;
}

void CAutoDetonate::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Auto::Detonate::Active.Value)
		return;

	// Check sticky detonation
	if (Vars::Auto::Detonate::Stickies.Value && CheckDetonation(pLocal, EGroupType::LOCAL_STICKIES, Vars::Auto::Detonate::RadiusScale.Value / 100, pCmd))
		pCmd->buttons |= IN_ATTACK2;

	// Check flare detonation
	if (Vars::Auto::Detonate::Flares.Value && CheckDetonation(pLocal, EGroupType::LOCAL_FLARES, Vars::Auto::Detonate::RadiusScale.Value / 100, pCmd))
		pCmd->buttons |= IN_ATTACK2;
}
