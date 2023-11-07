#include "../Hooks.h"

#include "../../Features/Prediction/Prediction.h"
#include "../../Features/Aimbot/Aimbot.h"
#include "../../Features/Aimbot/AimbotProjectile/AimbotProjectile.h"
#include "../../Features/Auto/Auto.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/PacketManip/PacketManip.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/Visuals/FakeAngleManager/FakeAng.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/NoSpread/NoSpread.h"
#include "../../Features/Resolver/Resolver.h"
#include "../../Features/AntiHack/CheaterDetection.h"
#include "../../Features/Vars.h"
#include "../../Features/Visuals/Chams/DMEChams.h"
#include "../../Features/Visuals/Glow/Glow.h"
#include "../../Features/Menu/MaterialEditor/MaterialEditor.h"
#include "../../Features/TickHandler/TickHandler.h"
#include "../../Features/Backtrack/Backtrack.h"

void AttackingUpdate(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	if (!G::IsAttacking || !pLocal || !pWeapon)
		return;

	auto tfWeaponInfo = pWeapon->GetTFWeaponInfo();
	if (!tfWeaponInfo)
		return;

	const float flFireDelay = tfWeaponInfo->GetWeaponData(0).m_flTimeFireDelay; // pWeapon->GetWeaponData().m_flTimeFireDelay is wrong
	pWeapon->m_flNextPrimaryAttack() = TICKS_TO_TIME(pLocal->GetTickBase() - G::ShiftedTicks) + flFireDelay;
	// this doesn't really work as intended since tickbase will be constant while shifting but it afaik it doesn't break anything
}

MAKE_HOOK(ClientModeShared_CreateMove, Utils::GetVFuncPtr(I::ClientModeShared, 21), bool, __fastcall,
	void* ecx, void* edx, float input_sample_frametime, CUserCmd* pCmd)
{
	G::UpdateView = true;
	G::SilentTime = false;
	G::IsAttacking = false;

	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();

	if (!pCmd || !pCmd->command_number)
		return Hook.Original<FN>()(ecx, edx, input_sample_frametime, pCmd);

	G::Buttons = pCmd->buttons;
	if (Hook.Original<FN>()(ecx, edx, input_sample_frametime, pCmd))
		I::Prediction->SetLocalViewAngles(pCmd->viewangles);

	// Get the pointer to pSendPacket
	uintptr_t _bp;
	__asm mov _bp, ebp;
	auto pSendPacket = reinterpret_cast<bool*>(***reinterpret_cast<uintptr_t***>(_bp) - 0x1);

	//	save old info
	static Vec3 vOldAngles = pCmd->viewangles;
	static float fOldSide = pCmd->sidemove;
	static float fOldForward = pCmd->forwardmove;

	G::CurrentUserCmd = pCmd;
	if (!G::LastUserCmd)
		G::LastUserCmd = pCmd;

	//if (!G::DoubleTap)
		F::Backtrack.iTickCount = pCmd->tick_count;
	// correct tick_count for fakeinterp / nointerp
	pCmd->tick_count += TICKS_TO_TIME(F::Backtrack.flFakeInterp) - (Vars::Misc::DisableInterpolation.Value ? 0 : TICKS_TO_TIME(G::LerpTime));

	//if (!G::DoubleTap)
	if (pLocal && pWeapon)
	{
		if (const int MaxSpeed = pLocal->GetMaxSpeed())
			G::Frozen = MaxSpeed == 1;

		// Update Global Info
		const int nItemDefIndex = pWeapon->GetItemDefIndex();

		if (G::CurItemDefIndex != nItemDefIndex || !pWeapon->GetClip1() || !pLocal->IsAlive() || pLocal->IsTaunting() || pLocal->IsBonked() || pLocal->IsAGhost() || pLocal->IsInBumperKart())
			G::WaitForShift = 1; //Vars::CL_Move::DoubleTap::WaitReady.Value;

		G::CurItemDefIndex = nItemDefIndex;
		G::WeaponCanHeadShot = pWeapon->CanWeaponHeadShot();
		G::WeaponCanAttack = pWeapon->CanShoot(pLocal);
		G::WeaponCanSecondaryAttack = pWeapon->CanSecondaryAttack(pLocal);
		if (pWeapon->GetSlot() != SLOT_MELEE)
		{
			if (pWeapon->IsInReload())
				G::WeaponCanAttack = true;

			if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN &&
				(pWeapon->GetMinigunState() == AC_STATE_IDLE || pWeapon->GetMinigunState() == AC_STATE_STARTFIRING))
			{
				G::WeaponCanAttack = false;
			}

			if (G::CurItemDefIndex != Soldier_m_TheBeggarsBazooka && pWeapon->GetClip1() == 0)
				G::WeaponCanAttack = false;

			if (pLocal->InCond(TF_COND_GRAPPLINGHOOK))
				G::WeaponCanAttack = false;
		}
		G::CurWeaponType = Utils::GetWeaponType(pWeapon);
		G::IsAttacking = Utils::IsAttacking(pCmd, pWeapon);

		if (F::AimbotProjectile.bLastTickCancel)
		{
			pCmd->weaponselect = pLocal->GetWeaponFromSlot(SLOT_SECONDARY)->GetIndex();
			F::AimbotProjectile.bLastTickCancel = false;
		}
	}
	/*
	else
	{
		//const int nOldTickBase = pLocal->GetTickBase();
		//pLocal->GetTickBase() -= G::ShiftedTicks + 1; // silly
		G::WeaponCanAttack = pWeapon->CanShoot(pLocal);
		G::IsAttacking = Utils::IsAttacking(pCmd, pWeapon);
		//pLocal->GetTickBase() = nOldTickBase;
	}
	*/

	// Run Features
	F::Misc.RunPre(pCmd, pSendPacket);
	F::BadActors.OnTick();
	F::Backtrack.Run(pCmd);
	F::EnginePrediction.Start(pCmd);
	{
		F::Aimbot.Run(pCmd);
		F::Auto.Run(pCmd);
		F::PacketManip.CreateMove(pCmd, pSendPacket);
	}
	F::EnginePrediction.End(pCmd);
	F::Ticks.MovePost(pCmd);
	F::CritHack.Run(pCmd);
	F::NoSpread.Run(pCmd);
	F::Misc.RunPost(pCmd, pSendPacket);
	F::Resolver.CreateMove();

	if (*pSendPacket)
	{
		F::FakeAng.Run(pCmd);
		F::FakeAng.DrawChams = Vars::AntiHack::AntiAim::Active.Value || Vars::CL_Move::FakeLag::Enabled.Value;
	}

	if (!G::DoubleTap)
	{
		static bool bWasSet = false;
		if (G::SilentTime && !bWasSet) // failsafe
			*pSendPacket = false, bWasSet = true;
		else if(bWasSet)
			*pSendPacket = true, bWasSet = false;
	}
	else
		AttackingUpdate(pLocal, pWeapon);

	// do this at the end just in case aimbot / triggerbot fired.
	if (pCmd->buttons & IN_ATTACK && (Vars::CL_Move::DoubleTap::SafeTick.Value || Vars::CL_Move::DoubleTap::SafeTickAirOverride.Value && pLocal && !pLocal->OnSolid()))
	{
		if (G::NextSafeTick > I::GlobalVars->tickcount && G::DoubleTap && G::ShiftedTicks)
			pCmd->buttons &= ~IN_ATTACK;
		else
			G::NextSafeTick = I::GlobalVars->tickcount + g_ConVars.sv_maxusrcmdprocessticks_holdaim->GetInt() + 1;
	}
	
	G::ViewAngles = pCmd->viewangles;
	G::LastUserCmd = pCmd;

	const bool bShouldSkip = (G::SilentTime || G::AntiAim || G::AvoidingBackstab || !G::UpdateView || !F::Misc.TauntControl(pCmd));
	return bShouldSkip ? false : Hook.Original<FN>()(ecx, edx, input_sample_frametime, pCmd);
}