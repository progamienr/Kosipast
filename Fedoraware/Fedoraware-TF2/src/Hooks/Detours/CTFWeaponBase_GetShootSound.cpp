#include "../Hooks.h"

MAKE_HOOK(CTFWeaponBase_GetShootSound, S::CTFWeaponBase_GetShootSound(), const char*, __fastcall,
	void* ecx, void* edx, int iIndex)
{
	if (Vars::Misc::Sound::GiantWeaponSounds.Value)
	{
		auto pWeapon = reinterpret_cast<CBaseCombatWeapon*>(ecx);
		auto pOwner = pWeapon ? I::ClientEntityList->GetClientEntityFromHandle(pWeapon->m_hOwnerEntity()) : nullptr;
		auto pLocal = g_EntityCache.GetLocal();
		if (pWeapon && pOwner == pLocal)
		{
			int nOldTeam = pWeapon->m_iTeamNum();
			pWeapon->m_iTeamNum() = 4;
			auto ret = Hook.Original<FN>()(ecx, edx, iIndex);
			pWeapon->m_iTeamNum() = nOldTeam;

			// credits: KGB

			if (FNV1A::Hash(ret) == FNV1A::HashConst("Weapon_FlameThrower.Fire"))
				return "MVM.GiantPyro_FlameStart";

			if (FNV1A::Hash(ret) == FNV1A::HashConst("Weapon_FlameThrower.FireLoop"))
				return "MVM.GiantPyro_FlameLoop";

			if (FNV1A::Hash(ret) == FNV1A::HashConst("Weapon_GrenadeLauncher.Single"))
				return "MVM.GiantDemoman_Grenadeshoot";

			return ret;
		}
	}

	return Hook.Original<FN>()(ecx, edx, iIndex);
}