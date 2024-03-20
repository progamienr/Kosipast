#include "../Hooks.h"

MAKE_HOOK(C_BaseEntity_InterpolateServerEntities, S::CBaseEntity_InterpolateServerEntities(), void, __fastcall,
	void* ecx, void* edx)
{
	if (G::Recharge)
		return;

	return Hook.Original<FN>()(ecx, edx);
}