#include "../Hooks.h"
#include "../../Features/TickHandler/TickHandler.h"
#include "../../Features/Menu/Conditions/Conditions.h"
#include "../../Features/CheaterDetection/CheaterDetection.h"
#include "../../Features/Menu/Playerlist/PlayerCore.h"
#include "../../Features/AutoQueue/AutoQueue.h"
#include "../../Features/Backtrack/Backtrack.h"

MAKE_HOOK(CL_Move, S::CL_Move(), void, __cdecl,
	float accumulated_extra_samples, bool bFinalTick)
{
	if (G::UnloadWndProcHook)
		return Hook.Original<FN>()(accumulated_extra_samples, bFinalTick);

	auto pLocal = g_EntityCache.GetLocal();
	auto pWeapon = g_EntityCache.GetWeapon();

	F::Backtrack.iTickCount = I::GlobalVars->tickcount;

	F::Conditions.Run(pLocal, pWeapon);
	F::CheaterDetection.Run();
	F::Ticks.Run(accumulated_extra_samples, bFinalTick, pLocal);
	F::PlayerCore.Run();
	F::AutoQueue.Run();
	I::EngineClient->FireEvents();

	for (auto& Line : G::LinesStorage)
	{
		if (Line.m_flTime < 0.f)
			Line.m_flTime = std::min(Line.m_flTime + 1.f, 0.f);
	}
}