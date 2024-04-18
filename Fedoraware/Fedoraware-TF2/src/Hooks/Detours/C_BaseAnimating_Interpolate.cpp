#include "../Hooks.h"

MAKE_HOOK(C_BaseAnimating_Interpolate, S::CBaseAnimating_Interpolate(), bool, __fastcall,
	void* ecx, void* edx, float currentTime)
{
	if (ecx == g_EntityCache.GetLocal() ? G::Recharge : Vars::Visuals::Removals::Interpolation.Value)
		return true;

	return Hook.Original<FN>()(ecx, edx, currentTime);
}