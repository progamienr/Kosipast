#include "../Hooks.h"

MAKE_HOOK(CViewRender_DrawUnderwaterOverlay, S::CViewRender_DrawUnderwaterOverlay(), void, __fastcall,
	void* eax, void* edi)
{
	if (Vars::Visuals::RemoveScreenOverlays.Value && !(Vars::Visuals::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()))
	{
		return;
	}
	return Hook.Original<FN>()(eax, edi);
}