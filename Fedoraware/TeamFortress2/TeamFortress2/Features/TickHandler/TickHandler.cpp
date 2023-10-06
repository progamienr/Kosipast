#include "TickHandler.h"
#include "../../Hooks/HookManager.h"
#include "../../Hooks/Hooks.h"

void CTickshiftHandler::Speedhack(CUserCmd* pCmd)
{
	bSpeedhack = Vars::CL_Move::SpeedEnabled.Value;
	if (!bSpeedhack)
		return;

	G::DoubleTap = G::Teleport = G::Recharge = false;
}

void CTickshiftHandler::Recharge(CUserCmd* pCmd, CBaseEntity* pLocal) // occasionally breaks ??
{
	G::Recharge = false;

	bool bPassive = false;
	if (Vars::CL_Move::DoubleTap::RechargeWhileDead.Value && !pLocal->IsAlive())
		bPassive = true;

	if (Vars::CL_Move::DoubleTap::AutoRecharge.Value && !G::DoubleTap && !G::Teleport && G::ShiftedTicks < G::MaxShift && pLocal->GetVecVelocity().Length2D() < 5.0f && !(pCmd->buttons))
		bPassive = true;

	if (I::GlobalVars->tickcount >= iNextPassiveTick && Vars::CL_Move::DoubleTap::PassiveRecharge.Value)
	{
		bPassive = true;
		iNextPassiveTick = I::GlobalVars->tickcount + (iTickRate / Vars::CL_Move::DoubleTap::PassiveRecharge.Value);
	}

	if (iDeficit && G::ShiftedTicks < G::MaxShift && Vars::CL_Move::DoubleTap::AutoRetain.Value)
	{
		bPassive = true;
		iDeficit--;
	}
	else if (iDeficit)
		iDeficit = 0;

	static KeyHelper rechargeKey{ &Vars::CL_Move::DoubleTap::RechargeKey.Value };
	G::Recharge = (rechargeKey.Down() || bPassive) && !G::DoubleTap && !G::Teleport && G::ShiftedTicks < G::MaxShift && !bSpeedhack;
		
	if (G::Recharge && bGoalReached)
		G::ShiftedGoal = G::ShiftedTicks + 1;
}

void CTickshiftHandler::Teleport(CUserCmd* pCmd)
{
	G::Teleport = false;

	if (!G::ShiftedTicks || G::DoubleTap || G::Recharge || bSpeedhack)
		return;

	static KeyHelper teleportKey{ &Vars::CL_Move::DoubleTap::TeleportKey.Value };
	G::Teleport = teleportKey.Down();

	if (G::Teleport && bGoalReached)
		G::ShiftedGoal = std::max(G::ShiftedTicks - Vars::CL_Move::DoubleTap::WarpRate.Value, 0);
}

void CTickshiftHandler::Doubletap(const CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!Vars::CL_Move::DoubleTap::Enabled.Value || G::ShiftedTicks < std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift))
		return;

	if (!pCmd || !G::ShiftedTicks || G::Teleport || G::Recharge || bSpeedhack/*|| (G::ShiftedTicks < Vars::CL_Move::DTTicks.Value)*/)
		return;
	if (G::WaitForShift && Vars::CL_Move::DoubleTap::WaitReady.Value)
		return;

	static KeyHelper doubletapKey{ &Vars::CL_Move::DoubleTap::DoubletapKey.Value };
	if (!doubletapKey.Down() && Vars::CL_Move::DoubleTap::Mode.Value == 1)
		return;

	if (G::WeaponCanAttack && (G::IsAttacking || G::CurWeaponType == EWeaponType::MELEE && pCmd->buttons & IN_ATTACK) &&
		(!Vars::CL_Move::DoubleTap::NotInAir.Value || Vars::CL_Move::DoubleTap::NotInAir.Value && pLocal->OnSolid()))
		G::DoubleTap = true;

	if (G::DoubleTap && bGoalReached)
		G::ShiftedGoal = G::ShiftedTicks - std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift);
}

bool CTickshiftHandler::MeleeDoubletapCheck(CBaseEntity* pLocal) // this is dumb
{
	if (!Vars::CL_Move::DoubleTap::Enabled.Value || G::ShiftedTicks < std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift))
		return false;

	if (G::DoubleTap || G::Teleport || G::Recharge || bSpeedhack/*|| (G::ShiftedTicks < Vars::CL_Move::DTTicks.Value)*/)
		return false;
	if (G::WaitForShift && Vars::CL_Move::DoubleTap::WaitReady.Value)
		return false;

	static KeyHelper doubletapKey{ &Vars::CL_Move::DoubleTap::DoubletapKey.Value };
	if (!doubletapKey.Down() && Vars::CL_Move::DoubleTap::Mode.Value == 1)
		return false;

	return Vars::CL_Move::DoubleTap::NotInAir.Value ? pLocal->OnSolid() : true;
}

void CTickshiftHandler::CLMoveFunc(float accumulated_extra_samples, bool bFinalTick)
{
	static auto CL_Move = g_HookManager.GetMapHooks()["CL_Move"];
	if (!CL_Move)
	{
		CL_Move = g_HookManager.GetMapHooks()["CL_Move"];
		return;
	}

	G::ShiftedTicks--;
	if (G::ShiftedTicks < 0)
		return;
	if (G::WaitForShift > 0)
		G::WaitForShift--;

	if (G::ShiftedTicks < std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift))
		G::WaitForShift = 1;

	bGoalReached = bFinalTick && G::ShiftedTicks == G::ShiftedGoal;

	CL_Move->Original<void(__cdecl*)(float, bool)>()(accumulated_extra_samples, bFinalTick);
}

void CTickshiftHandler::CLMove(float accumulated_extra_samples, bool bFinalTick)
{
	{
		CBaseCombatWeapon* pWeapon = g_EntityCache.GetWeapon();
		if (pWeapon)
		{
			if (G::IsAttacking || !G::WeaponCanAttack || pWeapon->IsInReload())
				G::WaitForShift = G::ShiftedTicks;
		}
	}

	G::MaxShift = g_ConVars.sv_maxusrcmdprocessticks ? g_ConVars.sv_maxusrcmdprocessticks->GetInt() : 24;
	if (G::AntiAim.first || G::AntiAim.second)
		G::MaxShift -= 1;

	while (G::ShiftedTicks > G::MaxShift)
		CLMoveFunc(accumulated_extra_samples, false); // skim any excess ticks

	G::ShiftedTicks++; // since we now have full control over CL_Move, increment.
	if (G::ShiftedTicks <= 0)
	{
		G::ShiftedTicks = 0;
		return;
	} // ruhroh

	/* disabling as users can now toggle at will
	if (!Vars::CL_Move::DoubleTap::Enabled.Value || I::EngineClient->IsPlayingTimeDemo())
	{
		while (G::ShiftedTicks > 1)
			CLMoveFunc(accumulated_extra_samples, false);
		return CLMoveFunc(accumulated_extra_samples, true);
	}
	*/

	// shiftedticks might not be fully accurate immediately
	G::ShiftedGoal = std::clamp(G::ShiftedGoal, 0, G::MaxShift);
	if (G::ShiftedTicks - 1 > G::ShiftedGoal) // doubletap/teleport
	{
		if (G::DoubleTap && Vars::CL_Move::DoubleTap::AntiWarp.Value)
			G::AntiWarp = true;
		const int iGoal = G::ShiftedGoal;
		while (G::ShiftedTicks > iGoal)
			CLMoveFunc(accumulated_extra_samples, G::ShiftedTicks - 1 == iGoal);
		G::AntiWarp = false;
		if (G::Teleport) // low values won't midigate this
			iDeficit = 0;

		G::Teleport = G::DoubleTap = false;
		return;
	}
	else if (G::ShiftedTicks - 1 < G::ShiftedGoal) // recharge
	{
		CBaseEntity* pLocal = g_EntityCache.GetLocal();
		if (pLocal)
			Recharge(G::LastUserCmd, pLocal);
		return;
	}

	if (bSpeedhack)
	{
		for (int i = 0; i < Vars::CL_Move::SpeedFactor.Value; i++)
			CLMoveFunc(accumulated_extra_samples, i == Vars::CL_Move::SpeedFactor.Value);
		return;
	}

	return CLMoveFunc(accumulated_extra_samples, true);
}

void CTickshiftHandler::CreateMove(CUserCmd* pCmd)
{
	static KeyHelper doubletapKey{ &Vars::CL_Move::DoubleTap::DoubletapKey.Value };
	if (doubletapKey.Pressed() && Vars::CL_Move::DoubleTap::Mode.Value == 2)
		Vars::CL_Move::DoubleTap::Enabled.Value = !Vars::CL_Move::DoubleTap::Enabled.Value;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal)
		return;

	CBaseCombatWeapon* pWeapon = g_EntityCache.GetWeapon();
	if (pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_GRAPPLINGHOOK) // crash fix, don't know why this is necessary
		return;

	Recharge(pCmd, pLocal);
	Teleport(pCmd);
	Doubletap(pCmd, pLocal);
	Speedhack(pCmd);
}

void CTickshiftHandler::Reset()
{
	bSpeedhack = G::DoubleTap = G::Recharge = G::Teleport = false;
	G::ShiftedTicks = G::ShiftedGoal = 0;
	iNextPassiveTick = 0;
	iTickRate = round(1.f / I::GlobalVars->interval_per_tick);
}