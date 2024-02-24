#include "../Hooks.h"

#include "../../Features/Resolver/Resolver.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Visuals/FakeAngle/FakeAngle.h"
#include "../../Features/Menu/Playerlist/PlayerUtils.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/Simulation/MovementSimulation/MovementSimulation.h"

MAKE_HOOK(BaseClientDLL_FrameStageNotify, Utils::GetVFuncPtr(I::BaseClientDLL, 35), void, __fastcall,
	void* ecx, void* edx, EClientFrameStage curStage)
{
	switch (curStage)
	{
	case EClientFrameStage::FRAME_RENDER_START:
		G::PunchAngles = Vec3();
		if (const auto& pLocal = g_EntityCache.GetLocal())
		{
			// Remove punch effect
			G::PunchAngles = pLocal->PunchAngles(); // use in aimbot 
			if (Vars::Visuals::RemovePunch.Value)
				pLocal->ClearPunchAngle(); // visual no-recoil
			F::Resolver.FrameStageNotify(pLocal);
		}
		F::Visuals.SkyboxChanger();

		break;
	}

	Hook.Original<FN>()(ecx, edx, curStage);

	switch (curStage)
	{
	case EClientFrameStage::FRAME_NET_UPDATE_START:
		g_EntityCache.Clear();

		break;
	case EClientFrameStage::FRAME_NET_UPDATE_END:
		g_EntityCache.Fill();
		F::PlayerUtils.UpdatePlayers();
		for (auto pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
		{
			if (!pEntity || pEntity == g_EntityCache.GetLocal() || pEntity->IsTaunting() && !Vars::Visuals::RemoveInterpolation.Value)
				continue; // local player managed in CPrediction_RunCommand

			if (auto nDifference = std::clamp(TIME_TO_TICKS(pEntity->m_flSimulationTime() - pEntity->m_flOldSimulationTime()), 0, 22))
			{
				float flOldFrameTime = I::GlobalVars->frametime;

				I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;

				for (int n = 0; n < nDifference; n++)
				{
					G::UpdatingAnims = true;
					pEntity->UpdateClientSideAnimation();
					G::UpdatingAnims = false;
				}

				I::GlobalVars->frametime = flOldFrameTime;
			}
		}

		F::Backtrack.FrameStageNotify();
		F::MoveSim.FillVelocities();
		F::Visuals.FillSightlines();
		for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
		{
			if (const auto& player = I::ClientEntityList->GetClientEntity(n))
			{
				const VelFixRecord record = { player->m_vecOrigin(), player->m_fFlags(), player->m_flSimulationTime() };
				G::VelFixRecords[player] = record;
			}
		}

		F::FakeAngle.Run();

		break;
	case EClientFrameStage::FRAME_RENDER_START:
		if (!G::UnloadWndProcHook)
		{
			F::Visuals.ModulateWorld();
		}

		break;
	}
}