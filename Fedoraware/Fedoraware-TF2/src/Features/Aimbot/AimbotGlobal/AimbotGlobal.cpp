#include "AimbotGlobal.h"

#include "../../Vars.h"
#include "../../Menu/Playerlist/PlayerUtils.h"

bool CAimbotGlobal::IsKeyDown()
{
	return !Vars::Aimbot::Global::AimKey.Value ? true : F::KeyHandler.Down(Vars::Aimbot::Global::AimKey.Value);
}

void CAimbotGlobal::SortTargets(std::vector<Target_t>* targets, const ESortMethod& method)
{	// Sort by preference
	std::sort((*targets).begin(), (*targets).end(), [&](const Target_t& a, const Target_t& b) -> bool
			  {
				  switch (method)
				  {
					  case ESortMethod::FOV: return a.m_flFOVTo < b.m_flFOVTo;
					  case ESortMethod::DISTANCE: return a.m_flDistTo < b.m_flDistTo;
					  default: return false;
				  }
			  });
}

void CAimbotGlobal::SortPriority(std::vector<Target_t>* targets)
{	// Sort by priority
	std::sort((*targets).begin(), (*targets).end(), [&](const Target_t& a, const Target_t& b) -> bool
			  {
				  return a.m_nPriority > b.m_nPriority;
			  });
}

bool CAimbotGlobal::ShouldIgnore(CBaseEntity* pTarget, bool bMedigun)
{
	//if (*((BYTE*)I::TFGameRules + 1034))
	//{
	//	return true;
	//}

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	CBaseCombatWeapon* pWeapon = g_EntityCache.GetWeapon();

	PlayerInfo_t pi{};
	if (!pTarget || pTarget == pLocal || pTarget->GetDormant())
		return true;
	if (!I::EngineClient->GetPlayerInfo(pTarget->GetIndex(), &pi))
		return true;

	if (pLocal->m_iTeamNum() == pTarget->m_iTeamNum())
	{
		if (bMedigun)
			return pTarget->IsInvisible();
		return false;
	}

	if (Vars::Aimbot::Global::IgnoreOptions.Value & INVUL && pTarget->IsInvulnerable())
	{
		if (G::CurItemDefIndex != Heavy_t_TheHolidayPunch)
			return true;
	}
	if (Vars::Aimbot::Global::IgnoreOptions.Value & CLOAKED && pTarget->IsInvisible())
	{
		if (pTarget->GetInvisPercentage() >= Vars::Aimbot::Global::IgnoreCloakPercentage.Value)
			return true;
	}
	if (Vars::Aimbot::Global::IgnoreOptions.Value & DEADRINGER && pTarget->m_bFeignDeathReady())
		return true;
	if (Vars::Aimbot::Global::IgnoreOptions.Value & TAUNTING && pTarget->IsTaunting())
		return true;
	if (Vars::Aimbot::Global::IgnoreOptions.Value & VACCINATOR)
	{
		switch (G::CurWeaponType)
		{
		case EWeaponType::HITSCAN:
			if (pTarget->IsBulletResist() && G::CurItemDefIndex != Spy_m_TheEnforcer)
				return true;
			break;
		case EWeaponType::PROJECTILE:
			if (pTarget->IsFireResist() && (pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER || pWeapon->GetWeaponID() == TF_WEAPON_FLAREGUN))
				return true;
			else if (pTarget->IsBulletResist() && pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)
				return true;
			else if (pTarget->IsBlastResist())
				return true;
		}
	}
	if (Vars::Aimbot::Global::IgnoreOptions.Value & DISGUISED && pTarget->IsDisguised())
		return true;
	if (pow(pTarget->TickVelocity2D(), 2) > 4096.f && G::CurWeaponType != EWeaponType::PROJECTILE)
		return true;

	if (F::PlayerUtils.IsIgnored(pi.friendsID))
		return true;

	return false;
}

int CAimbotGlobal::GetPriority(int targetIdx)
{
	return F::PlayerUtils.GetPriority(targetIdx);
}

// will not predict for projectile weapons
bool CAimbotGlobal::ValidBomb(CBaseEntity* pBomb)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || G::CurWeaponType == EWeaponType::PROJECTILE)
		return false;

	const Vec3 vOrigin = pBomb->m_vecOrigin();

	CBaseEntity* pTarget;
	for (CEntitySphereQuery sphere(vOrigin, 300.f);
		(pTarget = sphere.GetCurrentEntity()) != nullptr;
		sphere.NextEntity())
	{
		if (!pTarget || !pTarget->IsAlive() || pTarget->IsPlayer() && pTarget->IsAGhost() || pTarget->m_iTeamNum() == pLocal->m_iTeamNum())
			continue;

		Vec3 vPos = {}; pTarget->GetCollision()->CalcNearestPoint(vOrigin, &vPos);
		if (vOrigin.DistTo(vPos) > 300.f)
			continue;

		const bool isPlayer = Vars::Aimbot::Global::AimAt.Value & PLAYER && pTarget->IsPlayer();
		const bool isSentry = Vars::Aimbot::Global::AimAt.Value & SENTRY && pTarget->IsSentrygun();
		const bool isDispenser = Vars::Aimbot::Global::AimAt.Value & DISPENSER && pTarget->IsDispenser();
		const bool isTeleporter = Vars::Aimbot::Global::AimAt.Value & TELEPORTER && pTarget->IsTeleporter();
		const bool isNPC = Vars::Aimbot::Global::AimAt.Value & NPC && pTarget->IsNPC();
		if (isPlayer || isSentry || isDispenser || isTeleporter || isNPC)
		{
			if (isPlayer && F::AimbotGlobal.ShouldIgnore(pTarget))
				continue;

			if (!Utils::VisPosProjectile(pBomb, pTarget, vOrigin, isPlayer ? pTarget->m_vecOrigin() + pTarget->GetViewOffset() : pTarget->GetWorldSpaceCenter(), MASK_SHOT))
				continue;

			return true;
		}
	}

	return false;
}

bool CAimbotGlobal::IsSandvich()
{
	return (G::CurItemDefIndex == Heavy_s_RoboSandvich ||
		G::CurItemDefIndex == Heavy_s_Sandvich ||
		G::CurItemDefIndex == Heavy_s_FestiveSandvich ||
		G::CurItemDefIndex == Heavy_s_Fishcake ||
		G::CurItemDefIndex == Heavy_s_TheDalokohsBar ||
		G::CurItemDefIndex == Heavy_s_SecondBanana);
}