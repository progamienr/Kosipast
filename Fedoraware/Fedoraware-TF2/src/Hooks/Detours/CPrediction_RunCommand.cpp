#include "../Hooks.h"

#include "../../Features/PacketManip/AntiAim/AntiAim.h"

//std::vector<Vec3> chokedAngles;

MAKE_HOOK(CPrediction_RunCommand, Utils::GetVFuncPtr(I::Prediction, 17), void, __fastcall,
	void* ecx, void* edx, CBaseEntity* pPlayer, CUserCmd* pCmd, CMoveHelper* moveHelper)
{
	if (G::Recharge && pPlayer == g_EntityCache.GetLocal())
		return;

	Hook.Original<FN>()(ecx, edx, pPlayer, pCmd, moveHelper);

	// credits: KGB
	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!G::Recharge && !pCmd->hasbeenpredicted && pPlayer == g_EntityCache.GetLocal() && !pPlayer->IsInBumperKart())
	{
		if (const auto& pAnimState = pPlayer->GetAnimState())
		{
			//chokedAngles.push_back(G::ViewAngles);
			if (G::Choking)
				return;

			float flOldFrameTime = I::GlobalVars->frametime;
			I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;
			for (auto& vAngle : G::ChokedAngles)
			{
				pAnimState->Update(vAngle.y, vAngle.x);
				pPlayer->FrameAdvance(I::GlobalVars->frametime);
			}
			I::GlobalVars->frametime = flOldFrameTime;

			G::ChokedAngles.clear();
		}
	}
}