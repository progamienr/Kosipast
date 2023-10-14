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

	G::Recharge = (F::KeyHandler.Down(Vars::CL_Move::DoubleTap::RechargeKey.Value) || bPassive) && !G::DoubleTap && !G::Teleport && G::ShiftedTicks < G::MaxShift && !bSpeedhack;
		
	if (G::Recharge && bGoalReached)
		G::ShiftedGoal = G::ShiftedTicks + 1;
}

void CTickshiftHandler::Teleport(CUserCmd* pCmd)
{
	G::Teleport = false;

	if (!G::ShiftedTicks || G::DoubleTap || G::Recharge || bSpeedhack)
		return;

	G::Teleport = F::KeyHandler.Down(Vars::CL_Move::DoubleTap::TeleportKey.Value);

	if (G::Teleport && bGoalReached)
		G::ShiftedGoal = std::max(G::ShiftedTicks - Vars::CL_Move::DoubleTap::WarpRate.Value, 0);
}

void CTickshiftHandler::Doubletap(const CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!Vars::CL_Move::DoubleTap::Enabled.Value || G::ShiftedTicks < std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift))
		return;

	if (!pCmd || !G::ShiftedTicks || G::Teleport || G::Recharge || bSpeedhack)
		return;
	if (G::WaitForShift && Vars::CL_Move::DoubleTap::WaitReady.Value)
		return;

	if (Vars::CL_Move::DoubleTap::Mode.Value == 1 && !F::KeyHandler.Down(Vars::CL_Move::DoubleTap::DoubletapKey.Value))
		return;

	if (G::WeaponCanAttack && (G::IsAttacking || G::CurWeaponType == EWeaponType::MELEE && pCmd->buttons & IN_ATTACK) &&
		(Vars::CL_Move::DoubleTap::NotInAir.Value ? pLocal->OnSolid() : true))
		G::DoubleTap = true;

	if (G::DoubleTap && bGoalReached)
		G::ShiftedGoal = G::ShiftedTicks - std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift);
}

int CTickshiftHandler::GetTicks(CBaseEntity* pLocal)
{
	if (G::ShiftedGoal < G::ShiftedTicks)
		return G::ShiftedTicks - G::ShiftedGoal;

	if (!Vars::CL_Move::DoubleTap::Enabled.Value || G::ShiftedTicks < std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift))
		return 0;

	if (G::DoubleTap || G::Teleport || G::Recharge || bSpeedhack)
		return 0;
	if (G::WaitForShift && Vars::CL_Move::DoubleTap::WaitReady.Value)
		return 0;

	if (Vars::CL_Move::DoubleTap::Mode.Value == 1 && !F::KeyHandler.Down(Vars::CL_Move::DoubleTap::DoubletapKey.Value))
		return 0;

	if (Vars::CL_Move::DoubleTap::NotInAir.Value ? pLocal->OnSolid() : true)
		return std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift);

	return 0;
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
		//if (G::DoubleTap && Vars::CL_Move::DoubleTap::AntiWarp.Value)
		//	G::AntiWarp = true;
		const int iGoal = G::ShiftedGoal;
		while (G::ShiftedTicks > iGoal)
		{
			if (G::DoubleTap && Vars::CL_Move::DoubleTap::AntiWarp.Value)
				G::ShouldStop = true; // doesn't seem to work on all weapons?? (sodapopper, pretty boy's, winger)
			CLMoveFunc(accumulated_extra_samples, G::ShiftedTicks - 1 == iGoal);
		}
		//G::AntiWarp = false;
		G::ShouldStop = false;
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
	if (Vars::CL_Move::DoubleTap::Mode.Value == 2 && F::KeyHandler.Pressed(Vars::CL_Move::DoubleTap::DoubletapKey.Value))
		Vars::CL_Move::DoubleTap::Enabled.Value = !Vars::CL_Move::DoubleTap::Enabled.Value;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal)
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