#include "../Hooks.h"

#include "../../SDK/Includes/icons.h"
#include "../../Features/Visuals/ESP/ESP.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Visuals/PlayerArrows/PlayerArrows.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/Visuals/SpectatorList/SpectatorList.h"
#include "../../Features/Visuals/Radar/Radar.h"
#include "../../Features/Menu/Menu.h"
#include "../../Features/Visuals/Notifications/Notifications.h"

void Paint()
{
	if (!g_ScreenSize.w || !g_ScreenSize.h)
		g_ScreenSize.Update();

	{	// HACK: for some reason we need to do this
		static bool bInitIcons = false;

		if (!bInitIcons)
		{
			bInitIcons = true;
			for (int nIndex = 0; nIndex < ICONS::TEXTURE_AMOUNT; nIndex++)
			{
				ICONS::ID[nIndex] = -1;
				g_Draw.Texture(-200, 0, 18, 18, nIndex);
			}
		}
	}

	I::MatSystemSurface->StartDrawing();
	{
		if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
			return I::MatSystemSurface->FinishDrawing();

		F::Notifications.Draw();

		auto pLocal = g_EntityCache.GetLocal();
		if (I::EngineVGui->IsGameUIVisible() || !pLocal)
			return I::MatSystemSurface->FinishDrawing();

		F::Visuals.DrawServerHitboxes(pLocal);
		F::Visuals.DrawAntiAim(pLocal);

		F::Visuals.PickupTimers();
		F::ESP.Run(pLocal);
		F::PlayerArrows.Run(pLocal);
		F::Radar.Run(pLocal);

		F::Visuals.DrawAimbotFOV(pLocal);
		F::Visuals.DrawSeedPrediction(pLocal);
		F::Visuals.DrawOnScreenConditions(pLocal);
		F::Visuals.DrawOnScreenPing(pLocal);
		F::SpectatorList.Run(pLocal);
		F::CritHack.Draw(pLocal);
		F::Visuals.DrawTickbaseText(pLocal);
		F::Visuals.DrawDebugInfo(pLocal);
	}
	I::MatSystemSurface->FinishDrawing();
}

MAKE_HOOK(EngineVGui_Paint, Utils::GetVFuncPtr(I::EngineVGui, 14), void, __fastcall,
	void* ecx, void* edx, int iMode)
{
	Hook.Original<FN>()(ecx, edx, iMode);

	if (iMode & PAINT_UIPANELS)
	{	
		CViewSetup viewSetup = {}; //Update W2S
		if (I::BaseClientDLL->GetPlayerView(viewSetup))
		{
			VMatrix worldToView = {}, viewToProjection = {}, worldToPixels = {};
			I::RenderView->GetMatricesForView(viewSetup, &worldToView, &viewToProjection, &G::WorldToProjection, &worldToPixels);
		}

		Paint();
	}
}