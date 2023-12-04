#include "../Hooks.h"

#include "../../Features/Resolver/Resolver.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Menu/Playerlist/Playerlist.h"
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
			G::PunchAngles = pLocal->GetPunchAngles(); // use in aimbot 
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
		for (auto pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
		{
			if (!pEntity || pEntity == g_EntityCache.GetLocal() || pEntity->IsTaunting())
				continue; // local player managed in CPrediction_RunCommand

			if (auto nDifference = std::clamp(TIME_TO_TICKS(pEntity->m_flSimulationTime() - pEntity->m_flOldSimulationTime()), 0, 22))
			{
				float flOldFrameTime = I::GlobalVars->frametime;

				I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;

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
		for (auto& Line : G::LinesStorage)
		{
			if (Line.m_flTime < 0.f)
			{
				if (Line.m_line.empty())
					continue;

				if (Line.m_flTime < -1.f)
				{
					Line.m_flTime = std::min(-1.f, Line.m_flTime + I::GlobalVars->interval_per_tick);
					continue;
				}

				Line.m_line.pop_front();
			}
		}
		for (int i = 0; i < I::EngineClient->GetMaxClients(); i++)
		{
			if (const auto& player = I::ClientEntityList->GetClientEntity(i))
			{
				const VelFixRecord record = { player->m_vecOrigin(), player->m_fFlags(), player->m_flSimulationTime() };
				G::VelFixRecords[player] = record;
			}
		}
		F::PlayerList.UpdatePlayers();

		break;
	case EClientFrameStage::FRAME_RENDER_START:
		if (!G::UnloadWndProcHook)
		{
			if (G::ShouldUpdateMaterialCache)
			{
				F::Visuals.ClearMaterialHandles();
				F::Visuals.StoreMaterialHandles();
				G::ShouldUpdateMaterialCache = false;
			}

			//if (Vars::Visuals::SkyModulation.Value || Vars::Visuals::WorldModulation.Value)
				F::Visuals.ModulateWorld();
		}

		break;
	}
}