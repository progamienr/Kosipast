#include "../Hooks.h"

MAKE_HOOK(StudioRender_SetColorModulation, Utils::GetVFuncPtr(I::StudioRender, 27), void, __fastcall,
		  void* ecx, void* edx, const float* pColor)
{
	const float flCustomBlend[3] = {
		Color::TOFLOAT(Colors::StaticPropModulation.r),
		Color::TOFLOAT(Colors::StaticPropModulation.g),
		Color::TOFLOAT(Colors::StaticPropModulation.b)
	};

	const bool bScreenshot = I::EngineClient->IsTakingScreenshot() && Vars::Visuals::CleanScreenshots.Value;
	const bool bShouldUseCustomBlend = Vars::Visuals::PropModulation.Value && G::DrawingStaticProps && !bScreenshot;
	Hook.Original<FN>()(ecx, edx, bShouldUseCustomBlend ? flCustomBlend : pColor);
}