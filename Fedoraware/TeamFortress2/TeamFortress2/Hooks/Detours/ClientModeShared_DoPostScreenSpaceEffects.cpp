#include "../Hooks.h"

#include "../../Features/Chams/Chams.h"
#include "../../Features/Glow/Glow.h"
#include "../../Features/Visuals/Visuals.h"

MAKE_HOOK(ClientModeShared_DoPostScreenSpaceEffects, Utils::GetVFuncPtr(I::ClientModeShared, 39), bool, __fastcall,
		  void* ecx, void* edx, const CViewSetup* pSetup)
{
	if (I::EngineClient->IsTakingScreenshot() && Vars::Visuals::CleanScreenshots.Value) { return Hook.Original<FN>()(ecx, edx, pSetup); }

	F::Chams.Render();
	F::Glow.Render();

	if (Vars::Visuals::ProjectileTrajectory.Value)
	{
		F::Visuals.ProjectileTrace();
	}
	F::Visuals.DrawBulletLines();
	F::Visuals.DrawSimLines();
	F::Visuals.DrawBoxes();

	F::Visuals.DrawSightlines();

	F::Visuals.DrawBulletTracers();

	return Hook.Original<FN>()(ecx, edx, pSetup);
}