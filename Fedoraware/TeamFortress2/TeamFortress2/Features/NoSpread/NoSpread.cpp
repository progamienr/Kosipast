#include "NoSpread.h"
#include "../Vars.h"

bool CNoSpread::ShouldRun(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Aimbot::Projectile::NoSpread.Value)
		return false;

	if (G::CurWeaponType != EWeaponType::PROJECTILE)
		return false;

	if (!pLocal->IsAlive()
		|| pLocal->IsTaunting()
		|| pLocal->IsBonked()
		|| pLocal->GetFeignDeathReady()
		|| pLocal->IsCloaked()
		|| pLocal->IsInBumperKart()
		|| pLocal->IsAGhost())
	{
		return false;
	}

	switch (G::CurItemDefIndex)
	{
	case Soldier_m_RocketJumper:
	case Demoman_s_StickyJumper:
		return false;
	}

	return Utils::IsAttacking(pCmd, pWeapon);
}

void CNoSpread::Run(CUserCmd* pCmd)
{
	const auto pLocal = g_EntityCache.GetLocal();
	const auto pWeapon = g_EntityCache.GetWeapon();
	if (!pLocal || !pWeapon || !ShouldRun(pLocal, pWeapon, pCmd))
		return;

	Utils::SharedRandomInt(MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF, "SelectWeightedSequence", 0, 0, 0);
	for (int i = 0; i < 6; ++i)
		Utils::RandomFloat();

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		// don't let the _local_ syringes fool you
		pCmd->viewangles.x -= Utils::RandomFloat(-1.5f, 1.5f);
		pCmd->viewangles.y -= Utils::RandomFloat(-1.5f, 1.5f);
		G::SilentTime = true;
		return;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		//ShouldRun huntsman
		if (pWeapon->GetChargeBeginTime() > 0.f && I::GlobalVars->curtime - pWeapon->GetChargeBeginTime() <= 5.0f)
			return;

		float flRand = (float)Utils::RandomInt(0, 0x7fff) / 0x7fff;
		pCmd->viewangles.x -= -6 + flRand * 12.f;
		flRand = (float)Utils::RandomInt(0, 0x7fff) / 0x7fff;
		pCmd->viewangles.y -= -6 + flRand * 12.f;
		G::SilentTime = true;
		return;
	}
	default:
		Vec3 spread;
		pWeapon->GetSpreadAngles(spread);

		pCmd->viewangles -= spread - I::EngineClient->GetViewAngles();
		G::SilentTime = true;
		return;
	}
}