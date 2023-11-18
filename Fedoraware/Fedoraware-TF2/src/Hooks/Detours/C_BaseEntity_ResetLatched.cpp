#include "../Hooks.h"

MAKE_HOOK(C_BaseEntity_ResetLatched, S::CBaseEntity_ResetLatched(), void, __fastcall,
	void* ecx, void* edx)
{
	if (Vars::Misc::PredictionErrorJitterFix.Value)
		return;

	Hook.Original<FN>()(ecx, edx);
}