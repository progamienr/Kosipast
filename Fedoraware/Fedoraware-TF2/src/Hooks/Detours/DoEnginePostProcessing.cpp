#include "../Hooks.h"

MAKE_HOOK(DoEnginePostProcessing, S::DoEnginePostProcessing(), void, __cdecl,
	int x, int y, int w, int h, bool bFlashlightIsOn, bool bPostVGui)
{
	if (Vars::Visuals::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
	{
		return Hook.Original<FN>()(x, y, w, h, bFlashlightIsOn, bPostVGui);
	}

	if (Vars::Visuals::DoPostProcessing.Value)
	{
		Hook.Original<FN>()(x, y, w, h, bFlashlightIsOn, bPostVGui);
	}
}