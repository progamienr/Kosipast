#include "Auto.h"

#include "../Vars.h"

#include "AutoDetonate/AutoDetonate.h"
#include "AutoBlast/AutoBlast.h"
#include "AutoUber/AutoUber.h"
#include "AutoJump/AutoJump.h"

bool CAuto::ShouldRun(CBaseEntity* pLocal)
{
	/*
	if (!Vars::Auto::Global::Active.m_Var || !F::AutoGlobal.IsKeyDown()) // this is bad because i say so
		return false;
	*/

	// if triggerbot is active and we havent set a key its clear we want to trigger all the time, forcing keybinds is madness (especially when it's not done @ AimbotGlobal.cpp)
	if (!Vars::Auto::Global::Active.Value || (!F::AutoGlobal.IsKeyDown() && Vars::Auto::Global::AutoKey.Value))
		return false;

	if (I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible())
		return false;

	if (G::DoubleTap)
		return false;

	return true;
}

void CAuto::Run(CUserCmd* pCmd)
{
	/*
	if (Vars::Auto::Stab::Disguise.Value && F::AutoStab.m_bShouldDisguise)
		I::EngineClient->ClientCmd_Unrestricted("lastdisguise");
	*/

	//F::AutoStab.m_bShouldDisguise = false;

	const auto pLocal = g_EntityCache.GetLocal();
	const auto pWeapon = g_EntityCache.GetWeapon();

	if (pLocal && pWeapon && ShouldRun(pLocal))
	{
		F::AutoDetonate.Run(pLocal, pWeapon, pCmd);
		F::AutoAirblast.Run(pLocal, pWeapon, pCmd);
		F::AutoUber.Run(pLocal, pWeapon, pCmd);
	}
	F::AutoJump.Run(pLocal, pWeapon, pCmd);
}