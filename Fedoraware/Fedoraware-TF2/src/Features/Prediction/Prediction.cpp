#include "Prediction.h"

#include <iostream>

#include "../TickHandler/TickHandler.h"

namespace S
{
	MAKE_SIGNATURE(ResetInstanceCounters, CLIENT_DLL, "68 ? ? ? ? 6A ? 68 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? E8 ? ? ? ? 83 C4 ? C3", 0x0);
}

int CEnginePrediction::GetTickbase(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	static int nTick = 0;
	static CUserCmd* pLastCmd = nullptr;

	if (pCmd)
	{
		if (!pLastCmd || pLastCmd->hasbeenpredicted)
			nTick = pLocal->GetTickBase();
		else
			nTick++;
		pLastCmd = pCmd;
	}

	return nTick;
}

void CEnginePrediction::Start(CUserCmd* pCmd)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || !I::MoveHelper)
		return;

	static auto fnResetInstanceCounters = S::ResetInstanceCounters.As<void(__cdecl*)()>();
	fnResetInstanceCounters();

	pLocal->SetCurrentCmd(pCmd);

	oldrandomseed = *I::RandomSeed;
	*I::RandomSeed = MD5_PseudoRandom(pCmd->command_number) & std::numeric_limits<int>::max();

	m_nOldTickCount = I::GlobalVars->tickcount;
	m_fOldCurrentTime = I::GlobalVars->curtime;
	m_fOldFrameTime = I::GlobalVars->frametime;

	I::GlobalVars->tickcount = GetTickbase(pCmd, pLocal);
	I::GlobalVars->curtime = TICKS_TO_TIME(I::GlobalVars->tickcount);
	I::GlobalVars->frametime = (I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL);
	G::TickBase = I::GlobalVars->tickcount;

	// don't mess up eye pos with antiwarp
	if (G::DoubleTap && Vars::CL_Move::DoubleTap::AntiWarp.Value && pLocal->OnSolid())
		return;

	const int nOldTickBase = pLocal->GetTickBase();
	const bool bOldIsFirstPrediction = I::Prediction->m_bFirstTimePredicted;
	const bool bOldInPrediction = I::Prediction->m_bInPrediction;

	I::Prediction->m_bFirstTimePredicted = false;
	I::Prediction->m_bInPrediction = true;

	I::GameMovement->StartTrackPredictionErrors(pLocal);

	pLocal->UpdateButtonState(pCmd->buttons);

	I::Prediction->SetLocalViewAngles(pCmd->viewangles);
		
	const int iThinkTick = pLocal->m_nNextThinkTick();

	if (pLocal->PhysicsRunThink(0))
		pLocal->PreThink();

	if (iThinkTick > 0 && iThinkTick <= I::GlobalVars->tickcount)
	{
		pLocal->SetNextThink(-1, NULL);
		pLocal->Think();
	}

	I::MoveHelper->SetHost(pLocal);

	I::Prediction->SetupMove(pLocal, pCmd, I::MoveHelper, &m_MoveData);
	// demo charge fix pt 2 (still currently iffy with antiwarp occasionally)
	if (G::DoubleTap && G::CurWeaponType == EWeaponType::MELEE && pLocal->IsCharging() && F::Ticks.GetTicks(pLocal) <= 14)
		m_MoveData.m_flMaxSpeed = m_MoveData.m_flClientMaxSpeed = pLocal->TeamFortress_CalculateMaxSpeed(true);
	I::GameMovement->ProcessMovement(pLocal, &m_MoveData);
	// I::Prediction->FinishMove occasionally fucks with pCmd, might only be when respawning within a tick
	I::Prediction->FinishMove(pLocal, pCmd, &m_MoveData); // CRASH: read access violation. pCmd was 0x... (after call)

	pLocal->PostThink();
	I::GameMovement->FinishTrackPredictionErrors(pLocal);
	pLocal->SetTickBase(nOldTickBase);

	I::Prediction->m_bFirstTimePredicted = bOldIsFirstPrediction;
	I::Prediction->m_bInPrediction = bOldInPrediction;
}

void CEnginePrediction::End(CUserCmd* pCmd)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || !I::MoveHelper)
		return;

	I::MoveHelper->SetHost(nullptr);

	I::GlobalVars->tickcount = m_nOldTickCount;
	I::GlobalVars->curtime = m_fOldCurrentTime;
	I::GlobalVars->frametime = m_fOldFrameTime;

	pLocal->SetCurrentCmd(nullptr);

	*I::RandomSeed = -1;
}