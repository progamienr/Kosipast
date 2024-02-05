#include "TickHandler.h"

#include "../../Hooks/HookManager.h"
#include "../../Hooks/Hooks.h"
#include "../NetworkFix/NetworkFix.h"

void CTickshiftHandler::Reset()
{
	bSpeedhack = G::DoubleTap = G::Recharge = G::Teleport = false;
	G::ShiftedTicks = G::ShiftedGoal = 0;
	iNextPassiveTick = 0;
	iTickRate = round(1.f / I::GlobalVars->interval_per_tick);
}

void CTickshiftHandler::Recharge(CUserCmd* pCmd, CBaseEntity* pLocal) // occasionally breaks ??
{
	G::Recharge = false;

	bool bPassive = false;
	if (Vars::CL_Move::DoubleTap::Options.Value & (1 << 4) && !pLocal->IsAlive())
		bPassive = true;

	if (Vars::CL_Move::DoubleTap::Options.Value & (1 << 3) && !G::DoubleTap && !G::Teleport && G::ShiftedTicks < G::MaxShift && pLocal->m_vecVelocity().Length2D() < 5.0f && !(pCmd->buttons))
		bPassive = true;

	if (I::GlobalVars->tickcount >= iNextPassiveTick && Vars::CL_Move::DoubleTap::PassiveRecharge.Value)
	{
		bPassive = true;
		iNextPassiveTick = I::GlobalVars->tickcount + (iTickRate / Vars::CL_Move::DoubleTap::PassiveRecharge.Value);
	}

	if (iDeficit && G::ShiftedTicks < G::MaxShift && Vars::CL_Move::DoubleTap::Options.Value & (1 << 2))
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
	if (G::WaitForShift)
		return;

	if (Vars::CL_Move::DoubleTap::Mode.Value == 1 && !F::KeyHandler.Down(Vars::CL_Move::DoubleTap::DoubletapKey.Value))
		return;

	if (G::WeaponCanAttack && (G::IsAttacking || G::CurWeaponType == EWeaponType::MELEE && pCmd->buttons & IN_ATTACK) &&
		(Vars::CL_Move::DoubleTap::Options.Value & (1 << 1) ? pLocal->OnSolid() : true))
	{
		G::DoubleTap = true;
		if (Vars::CL_Move::DoubleTap::Options.Value & (1 << 0))
			G::AntiWarp = true;
	}

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
	if (G::WaitForShift)
		return 0;

	if (Vars::CL_Move::DoubleTap::Mode.Value == 1 && !F::KeyHandler.Down(Vars::CL_Move::DoubleTap::DoubletapKey.Value))
		return 0;

	if (Vars::CL_Move::DoubleTap::Options.Value & (1 << 1) ? pLocal->OnSolid() : true)
		return std::min(Vars::CL_Move::DoubleTap::TickLimit.Value, G::MaxShift);

	return 0;
}

bool CTickshiftHandler::ValidWeapon(CBaseCombatWeapon* pWeapon)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_LUNCHBOX:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_BUFF_ITEM:
	case TF_WEAPON_JAR_GAS:
	case TF_WEAPON_ROCKETPACK:
	case TF_WEAPON_LASER_POINTER:
	case TF_WEAPON_MEDIGUN:
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_JAR:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_BUILDER:
		return false;
	}

	return true;
}

void CTickshiftHandler::Speedhack(CUserCmd* pCmd)
{
	bSpeedhack = Vars::CL_Move::SpeedEnabled.Value;
	if (!bSpeedhack)
		return;

	G::DoubleTap = G::Teleport = G::Recharge = false;
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

void CTickshiftHandler::MoveMain(float accumulated_extra_samples, bool bFinalTick)
{
	if (auto pWeapon = g_EntityCache.GetWeapon())
	{
		const int iWeaponID = pWeapon->GetWeaponID();
		if (iWeaponID != TF_WEAPON_PIPEBOMBLAUNCHER && iWeaponID != TF_WEAPON_CANNON)
		{
			if (!ValidWeapon(pWeapon))
				G::WaitForShift = 2;
			else if (G::IsAttacking || !G::WeaponCanAttack || pWeapon->IsInReload())
				G::WaitForShift = Vars::CL_Move::DoubleTap::TickLimit.Value;
		}
	}
	else
		G::WaitForShift = 2;
	
	G::MaxShift = g_ConVars.sv_maxusrcmdprocessticks ? g_ConVars.sv_maxusrcmdprocessticks->GetInt() : 24;
	if (Vars::AntiHack::AntiAim::Active.Value)
		G::MaxShift -= 3;

	while (G::ShiftedTicks > G::MaxShift)
		CLMoveFunc(accumulated_extra_samples, false); // skim any excess ticks

	G::ShiftedTicks++; // since we now have full control over CL_Move, increment.
	if (G::ShiftedTicks <= 0)
	{
		G::ShiftedTicks = 0;
		return;
	}

	if (bSpeedhack)
	{
		for (int i = 0; i < Vars::CL_Move::SpeedFactor.Value; i++)
			CLMoveFunc(accumulated_extra_samples, i == Vars::CL_Move::SpeedFactor.Value);
		return;
	}

	G::ShiftedGoal = std::clamp(G::ShiftedGoal, 0, G::MaxShift);
	if (G::ShiftedTicks > G::ShiftedGoal) // normal use/doubletap/teleport
	{
		while (G::ShiftedTicks > G::ShiftedGoal)
			CLMoveFunc(accumulated_extra_samples, bFinalTick);
		G::AntiWarp = false;
		if (G::Teleport)
			iDeficit = 0;

		G::Teleport = G::DoubleTap = false;
		return;
	}
	else if (G::ShiftedTicks < G::ShiftedGoal) // recharge
	{
		CBaseEntity* pLocal = g_EntityCache.GetLocal();
		if (pLocal)
			Recharge(G::LastUserCmd, pLocal);
		return;
	}
}

void CTickshiftHandler::MovePre()
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	CUserCmd* pCmd = G::CurrentUserCmd;
	if (!pLocal || !pCmd)
		return;

	Recharge(pCmd, pLocal);
	Teleport(pCmd);
	Speedhack(pCmd);
}

void CTickshiftHandler::MovePost(CUserCmd* pCmd)
{
	if (Vars::CL_Move::DoubleTap::Mode.Value == 2 && F::KeyHandler.Pressed(Vars::CL_Move::DoubleTap::DoubletapKey.Value))
		Vars::CL_Move::DoubleTap::Enabled.Value = !Vars::CL_Move::DoubleTap::Enabled.Value;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal)
		return;

	Doubletap(pCmd, pLocal);
}

void CTickshiftHandler::CLMove(float accumulated_extra_samples, bool bFinalTick)
{
	F::NetworkFix.FixInputDelay(bFinalTick);

	MovePre();
	MoveMain(accumulated_extra_samples, bFinalTick);
}