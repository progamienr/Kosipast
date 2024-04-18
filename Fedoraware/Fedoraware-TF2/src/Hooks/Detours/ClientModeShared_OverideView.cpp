#include "../Hooks.h"

#include "../../Features/Visuals/Visuals.h"

MAKE_HOOK(ClientModeShared_OverrrideView, Utils::GetVFuncPtr(I::ClientModeShared, 16), void, __fastcall,
	void* ecx, void* edx, CViewSetup* pView)
{
	Hook.Original<FN>()(ecx, edx, pView);
	if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return;

	auto pLocal = g_EntityCache.GetLocal();
	if (pLocal && pView)
	{
		F::Visuals.FOV(pLocal, pView);
		F::Visuals.ThirdPerson(pLocal, pView);
	}
}