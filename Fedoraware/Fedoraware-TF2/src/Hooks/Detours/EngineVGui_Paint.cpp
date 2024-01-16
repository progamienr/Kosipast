#include "../Hooks.h"

#include "../../SDK/Includes/icons.h"
#include "../../Features/Visuals/ESP/ESP.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Visuals/PlayerArrows/PlayerArrows.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/Menu/SpectatorList/SpectatorList.h"
#include "../../Features/Visuals/Radar/Radar.h"
#include "../../Features/Menu/Menu.h"
#include "../../Features/Misc/Notifications/Notifications.h"
#include "../../Features/AutoQueue/AutoQueue.h"
#include "../../Features/Menu/Playerlist/Playerlist.h"

namespace S
{
	MAKE_SIGNATURE(StartDrawing, MATSURFACE_DLL, "55 8B EC 64 A1 ? ? ? ? 6A FF 68 ? ? ? ? 50 64 89 25 ? ? ? ? 83 EC 14", 0x0);
	MAKE_SIGNATURE(FinishDrawing, MATSURFACE_DLL, "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 51 56 6A 00", 0x0);
}

MAKE_HOOK(EngineVGui_Paint, Utils::GetVFuncPtr(I::EngineVGui, 14), void, __fastcall,
	void* ecx, void* edx, int iMode)
{
	static auto StartDrawing = reinterpret_cast<void(__thiscall*)(void*)>(S::StartDrawing());
	static auto FinishDrawing = reinterpret_cast<void(__thiscall*)(void*)>(S::FinishDrawing());

	if (!g_ScreenSize.w || !g_ScreenSize.h)
	{
		g_ScreenSize.Update();
	}

	//HACK: for some reason we need to do this
	{
		static bool bInitIcons = false;

		if (!bInitIcons)
		{
			for (int nIndex = 0; nIndex < ICONS::TEXTURE_AMOUNT; nIndex++)
			{
				ICONS::ID[nIndex] = -1;
				g_Draw.Texture(-200, 0, 18, 18, { 255, 255, 255, 255 }, nIndex);
			}

			bInitIcons = true;
		}
	}

	Hook.Original<FN>()(ecx, edx, iMode);

	if (iMode & PAINT_UIPANELS)
	{
		//Update W2S
		{
			CViewSetup viewSetup = {};

			if (I::BaseClientDLL->GetPlayerView(viewSetup))
			{
				VMatrix worldToView = {}, viewToProjection = {}, worldToPixels = {};
				I::RenderView->GetMatricesForView(viewSetup, &worldToView, &viewToProjection, &G::WorldToProjection, &worldToPixels);
			}
		}

		StartDrawing(I::VGuiSurface);
		{
			if (Vars::Visuals::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
				return FinishDrawing(I::VGuiSurface);

			F::AutoQueue.Run();
			F::Notifications.Draw();

			if (I::EngineVGui->IsGameUIVisible())
				return FinishDrawing(I::VGuiSurface);

			//static int nAvatar = 0;
			//static int nAvatarID = 0;
			//static uint32 w, h;
			//static byte removebytes[16384];
			//if (!nAvatar)
			//{
			//	nAvatar = g_SteamInterfaces.Friends015->GetMediumFriendAvatar(CSteamID(g_SteamInterfaces.User->GetSteamID()));

			//	if (g_SteamInterfaces.Utils007->GetImageSize(nAvatar, &w, &h))
			//	{
			//		const int nSize = static_cast<int>(4 * w * h * sizeof(uint8));

			//		if (g_SteamInterfaces.Utils007->GetImageRGBA(nAvatar, removebytes, nSize))
			//		{
			//			for (int i = 0; i <= 16384; i += 4)
			//			{
			//				// Do not convert these to hex >:)
			//				if (rawData[i] == 105 &&
			//					rawData[i + 1] == 20 &&
			//					rawData[i + 2] == 136 &&
			//					rawData[i + 3] == 0x01)
			//				{
			//					removebytes[i] = 0x00;
			//					removebytes[i + 1] = 0x00;
			//					removebytes[i + 2] = 0x00;
			//					removebytes[i + 3] = 0x00;
			//				}
			//			}
			//			nAvatarID = g_Draw.CreateTextureFromArray(removebytes, w, h);
			//		}
			//	}
			//}
			//if (nAvatarID)
			//{
			//	I::Surface->DrawSetTexture(nAvatarID);
			//	I::Surface->DrawTexturedRect(100, 300, w, h);
			//	g_Draw.LineCircle(100 + (w / 2), 300 + (h / 2), w / 2, 300, Utils::Rainbow());
			//}

			F::ESP.Run();
			F::Visuals.PickupTimers();
			F::PlayerArrows.Run();
			F::SpectatorList.Run();
			F::CritHack.Draw();
			F::Radar.Run();
			F::PlayerList.Run();

			if (CBaseEntity* pLocal = g_EntityCache.GetLocal())
			{
				F::Visuals.DrawAntiAim(pLocal);
				F::Visuals.DrawTickbaseText();
				F::Visuals.DrawAimbotFOV(pLocal);
				F::Visuals.DrawDebugInfo(pLocal);
				F::Visuals.DrawOnScreenConditions(pLocal);
				F::Visuals.DrawOnScreenPing(pLocal);
				F::Visuals.DrawServerHitboxes();
			}
		}
		FinishDrawing(I::VGuiSurface);
	}
}