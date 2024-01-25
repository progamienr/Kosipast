#include "../Hooks.h"

MAKE_HOOK(CBaseAnimating_UpdateClientSideAnimation, S::CBaseAnimating_UpdateClientSideAnimation(), void, __fastcall,
	void* ecx, void* edx)
{
	if (!G::UpdatingAnims && Vars::Visuals::RemoveInterpolation.Value)
		return;

	auto pEntity = reinterpret_cast<CBaseEntity*>(ecx);
	if (!pEntity || pEntity->m_flSimulationTime() == pEntity->m_flOldSimulationTime() && !pEntity->IsTaunting() || pEntity == g_EntityCache.GetLocal() && !pEntity->IsInBumperKart())
		return;

	G::AnimateKart = true;
	Hook.Original<FN>()(ecx, edx);
	G::AnimateKart = false;
}