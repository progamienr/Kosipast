#include "../Hooks.h"

#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/Simulation/MovementSimulation/MovementSimulation.h"
#include "../../Features/Resolver/Resolver.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/CheaterDetection/CheaterDetection.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/Menu/Playerlist/PlayerUtils.h"

MAKE_HOOK(BaseClientDLL_FrameStageNotify, Utils::GetVFuncPtr(I::BaseClientDLL, 35), void, __fastcall,
	void* ecx, void* edx, EClientFrameStage curStage)
{
	switch (curStage)
	{
	case EClientFrameStage::FRAME_RENDER_START:
		G::PunchAngles = Vec3();
		if (auto pLocal = g_EntityCache.GetLocal())
		{
			G::PunchAngles = pLocal->m_vecPunchAngle(); // use in aimbot 
			if (Vars::Visuals::Removals::ViewPunch.Value)
				pLocal->ClearPunchAngle(); // visual no-recoil
			F::Resolver.FrameStageNotify(pLocal);
		}
	}

	Hook.Original<FN>()(ecx, edx, curStage);

	switch (curStage)
	{
	case EClientFrameStage::FRAME_NET_UPDATE_START:
		g_EntityCache.Clear();

		break;
	case EClientFrameStage::FRAME_NET_UPDATE_END:
		g_EntityCache.Fill();
		for (auto& pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
		{
			G::VelFixRecords[pEntity] = { pEntity->m_vecOrigin(), pEntity->m_vecMaxs().z - pEntity->m_vecMins().z, pEntity->m_flSimulationTime() };

			if (pEntity->GetIndex() == I::EngineClient->GetLocalPlayer())
				continue; // local player managed in CPrediction_RunCommand

			static auto sv_maxusrcmdprocessticks = g_ConVars.FindVar("sv_maxusrcmdprocessticks");
			const int iTicks = sv_maxusrcmdprocessticks ? sv_maxusrcmdprocessticks->GetInt() : 24;
			if (auto iDifference = std::min(TIME_TO_TICKS(pEntity->m_flSimulationTime() - pEntity->m_flOldSimulationTime()), iTicks))
			{
				float flOldFrameTime = I::GlobalVars->frametime;
				I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;
				for (int i = 0; i < iDifference; i++)
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
		F::CritHack.Fill();
		F::CheaterDetection.Fill();
		F::Visuals.FillSightlines();
		F::Misc.DetectChoke();
		F::PlayerUtils.UpdatePlayers();

		break;
	case EClientFrameStage::FRAME_RENDER_START:
		if (!G::UnloadWndProcHook)
		{
			F::Visuals.SkyboxChanger();
			F::Visuals.Modulate();
		}
	}
}