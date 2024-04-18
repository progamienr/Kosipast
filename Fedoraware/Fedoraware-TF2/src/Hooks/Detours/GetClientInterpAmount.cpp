#include "../Hooks.h"

MAKE_HOOK(GetClientInterpAmount, S::GetClientInterpAmount(), float, __cdecl,
	)
{
	return (/*Vars::Visuals::Removals::Interpolation.Value &&*/ !(Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())) ? 0.f : Hook.Original<FN>()();
}