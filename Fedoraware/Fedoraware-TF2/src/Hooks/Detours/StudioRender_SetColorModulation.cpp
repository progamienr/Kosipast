#include "../Hooks.h"

MAKE_HOOK(StudioRender_SetColorModulation, Utils::GetVFuncPtr(I::StudioRender, 27), void, __fastcall,
	void* ecx, void* edx, const float* pColor)
{
	const float flCustomBlend[3] = {
		Color::TOFLOAT(Vars::Colors::StaticPropModulation.Value.r),
		Color::TOFLOAT(Vars::Colors::StaticPropModulation.Value.g),
		Color::TOFLOAT(Vars::Colors::StaticPropModulation.Value.b)
	};

	const bool bScreenshot = I::EngineClient->IsTakingScreenshot() && Vars::Visuals::CleanScreenshots.Value;
	const bool bShouldUseCustomBlend = Vars::Visuals::World::PropModulation.Value && G::DrawingStaticProps && !bScreenshot;
	Hook.Original<FN>()(ecx, edx, bShouldUseCustomBlend ? flCustomBlend : pColor);
}