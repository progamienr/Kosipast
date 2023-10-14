#include "../Hooks.h"
#include "../../Features/CritHack/CritHack.h"

MAKE_HOOK(C_BaseCombatWeapon_AddToCritBucket, g_Pattern.Find(L"client.dll", L"55 8B EC A1 ? ? ? ? F3 0F 10 81 ? ? ? ? F3 0F 10 48 ? 0F 2F C8 76 1D F3 0F 58 45 ? 0F 2F C8 F3 0F 11 81 ? ? ? ? 77 03 0F 28 C1 F3 0F 11 81 ? ? ? ? 5D C2 04 00"), void, __fastcall,
	void* ecx, void* edx, float flDamage)
{
	if (Vars::Debug::DebugInfo.Value)
		I::Cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "Called AddToCritBucket.\n");

	//const auto& pWeapon = g_EntityCache.GetWeapon();
	//if (pWeapon)
	//{
	//	if (F::CritHack.Storage[pWeapon->GetSlot()].BaseDamage == 0)
	//		F::CritHack.Storage[pWeapon->GetSlot()].BaseDamage = flDamage;
	//}

	if (F::CritHack.ProtectData)
		return;

	return Hook.Original<FN>()(ecx, edx, flDamage);
}