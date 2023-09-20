#include "../Hooks.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/Commands/Commands.h"

MAKE_HOOK(C_TFWeaponBase_CalcIsAttackCritical, g_Pattern.Find(L"client.dll", L"53 57 6A 00 68 ? ? ? ? 68 ? ? ? ? 6A 00 8B F9 E8 ? ? ? ? 50 E8 ? ? ? ? 8B D8 83 C4 14 85 DB 0F 84"), void, __fastcall,
	void* ecx, void* edx)
{
	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();
	if (!pLocal || !pWeapon)
		return Hook.Original<FN>()(ecx, edx);

	if (!F::CritHack.CalcIsAttackCriticalHandler(pLocal, pWeapon))
		return;

	const auto nPreviousWeaponMode = pWeapon->m_iWeaponMode();
	pWeapon->m_iWeaponMode() = 0;

	Hook.Original<FN>()(ecx, edx);

	pWeapon->m_iWeaponMode() = nPreviousWeaponMode;
}
