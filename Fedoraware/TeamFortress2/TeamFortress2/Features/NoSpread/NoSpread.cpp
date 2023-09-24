#include "NoSpread.h"
#include "../Vars.h"

bool CNoSpread::IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
	{
		static bool bLoading = false, bFiring = false;

		if (pWeapon->GetClip1() == 0)
			bLoading = false,
			bFiring = false;
		else if (!bFiring)
			bLoading = true;

		if ((bFiring || bLoading && !(pCmd->buttons & IN_ATTACK)) && G::WeaponCanAttack)
		{
			bFiring = true;
			bLoading = false;
			return true;
		}
	}
	else
	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW || pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
		{
			static bool bCharging = false;

			if (pWeapon->GetChargeBeginTime() > 0.0f)
			{
				bCharging = true;
			}

			if (!(pCmd->buttons & IN_ATTACK) && bCharging)
			{
				bCharging = false;
				return true;
			}
		}
		else if (pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
		{
			static bool Charging = false;

			if (pWeapon->GetDetonateTime() > 0.0f)
			{
				Charging = true;
			}

			if (!(pCmd->buttons & IN_ATTACK) && Charging)
			{
				Charging = false;
				return true;
			}
		}

		//pssst..
		//Dragon's Fury has a gauge (seen on the weapon model) maybe it would help for pSilent hmm..
		/*
		if (pWeapon->GetWeaponID() == 109) {
		}*/

		else
		{
			if ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack)
			{
				return true;
			}
		}
	}

	return false;
}

bool CNoSpread::ShouldRun(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Aimbot::Projectile::NoSpread.Value)
	{
		return false;
	}

	if (G::CurWeaponType != EWeaponType::PROJECTILE)
	{
		return false;
	}

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
	{
		return false;
	}
	default: break;
	}

	return IsAttacking(pCmd, pWeapon);
}

void CNoSpread::Run(CUserCmd* pCmd)
{
	const auto pLocal = g_EntityCache.GetLocal();
	const auto pWeapon = g_EntityCache.GetWeapon();
	if (!pLocal || !pWeapon) { return; }
	if (!ShouldRun(pLocal, pWeapon, pCmd)) { return; }

	Utils::SharedRandomInt(MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF, "SelectWeightedSequence", 0, 0, 0);
	for (int i = 0; i < 6; ++i)
		Utils::RandomFloat();

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_SYRINGEGUN_MEDIC:
	{
		//don't let the _local_ syringes fool you
		pCmd->viewangles.x -= Utils::RandomFloat(-1.5f, 1.5f);
		pCmd->viewangles.y -= Utils::RandomFloat(-1.5f, 1.5f);
		G::SilentTime = true;
		return;
	}
	case TF_WEAPON_COMPOUND_BOW:
	{
		//ShouldRun huntsman
		if (pWeapon->GetChargeBeginTime() > 0.f && I::GlobalVars->curtime - pWeapon->GetChargeBeginTime() <= 5.0f)
		{
			return;
		}

		float frand = (float)Utils::RandomInt(0, 0x7fff) / 0x7fff;
		pCmd->viewangles.x -= -6 + frand * 12.f;
		frand = (float)Utils::RandomInt(0, 0x7fff) / 0x7fff;
		pCmd->viewangles.y -= -6 + frand * 12.f;
		G::SilentTime = true;
		return;
	}
	default:
		Vec3 spread;
		pWeapon->GetSpreadAngles(spread);

		spread -= pLocal->GetAbsAngles();
		pCmd->viewangles -= spread;
		G::SilentTime = true;
		return;
	}
}
