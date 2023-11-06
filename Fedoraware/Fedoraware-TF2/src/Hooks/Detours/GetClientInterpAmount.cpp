#include "../Hooks.h"

MAKE_HOOK(GetClientInterpAmount, S::GetClientInterpAmount(), float, __cdecl,
	)
{
	return 0.f;

	/*
	return (Vars::Misc::DisableInterpolation.Value && !(Vars::Visuals::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot()))
		? 0.f
		: Hook.Original<FN>()();
	*/
}