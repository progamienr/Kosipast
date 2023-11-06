#include "../Hooks.h"

//	xref C_BaseEntity_BaseInterpolatePart1
MAKE_HOOK(C_BaseAnimating_Interpolate, S::CBaseAnimating_Interpolate(), bool, __fastcall,
	void* ecx, void* edx, float currentTime)
{
	if (Vars::Misc::DisableInterpolation.Value && ecx != g_EntityCache.GetLocal() ||
		G::Recharge && ecx == g_EntityCache.GetLocal())
		return true;
	return Hook.Original<FN>()(ecx, edx, currentTime);
}