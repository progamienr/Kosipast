#include "../Hooks.h"

#include "../../Features/Visuals/Chams/Chams.h"
#include "../../Features/Visuals/Glow/Glow.h"
#include "../../Features/Visuals/Visuals.h"

MAKE_HOOK(ClientModeShared_DoPostScreenSpaceEffects, Utils::GetVFuncPtr(I::ClientModeShared, 39), bool, __fastcall,
	void* ecx, void* edx, const CViewSetup* pSetup)
{
	F::Chams.mEntities.clear();
	if (I::EngineVGui->IsGameUIVisible() || Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return Hook.Original<FN>()(ecx, edx, pSetup);

	F::Visuals.DrawBoxes();
	F::Visuals.DrawBulletLines();
	F::Visuals.DrawSimLines();
	F::Visuals.DrawSightlines();

	auto pLocal = g_EntityCache.GetLocal();
	auto pWeapon = g_EntityCache.GetWeapon();
	if (pLocal)
	{
		F::Chams.RenderMain(pLocal);
		F::Glow.RenderMain(pLocal);

		if (pWeapon)
			F::Visuals.ProjectileTrace(pLocal, pWeapon);
	}

	return Hook.Original<FN>()(ecx, edx, pSetup);
}