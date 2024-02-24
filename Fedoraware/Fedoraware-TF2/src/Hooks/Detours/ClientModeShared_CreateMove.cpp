#include "../Hooks.h"

#include "../../Features/Vars.h"
#include "../../Features/EnginePrediction/EnginePrediction.h"
#include "../../Features/Aimbot/Aimbot.h"
#include "../../Features/Aimbot/AimbotProjectile/AimbotProjectile.h"
#include "../../Features/Auto/Auto.h"
#include "../../Features/Misc/Misc.h"
#include "../../Features/AntiHack/CheaterDetection.h"
#include "../../Features/TickHandler/TickHandler.h"
#include "../../Features/PacketManip/PacketManip.h"
#include "../../Features/Visuals/FakeAngle/FakeAngle.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/NoSpread/NoSpread.h"
#include "../../Features/Resolver/Resolver.h"
#include "../../Features/Visuals/Visuals.h"

void AttackingUpdate(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	if (!G::IsAttacking || !pLocal || !pWeapon)
		return;

	auto tfWeaponInfo = pWeapon->GetTFWeaponInfo();
	if (!tfWeaponInfo)
		return;

	const float flFireDelay = tfWeaponInfo->GetWeaponData(0).m_flTimeFireDelay; // pWeapon->GetWeaponData().m_flTimeFireDelay is wrong
	pWeapon->m_flNextPrimaryAttack() = TICKS_TO_TIME(pLocal->m_nTickBase() - G::ShiftedTicks) + flFireDelay;
	// this doesn't really work as intended since tickbase will be constant while shifting but it afaik it doesn't break anything
}

MAKE_HOOK(ClientModeShared_CreateMove, Utils::GetVFuncPtr(I::ClientModeShared, 21), bool, __fastcall,
	void* ecx, void* edx, float input_sample_frametime, CUserCmd* pCmd)
{
	G::PSilentAngles = false;
	G::SilentAngles = false;
	G::IsAttacking = false;

	if (!pCmd || !pCmd->command_number)
		return Hook.Original<FN>()(ecx, edx, input_sample_frametime, pCmd);

	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();

	// Get the pointer to pSendPacket
	uintptr_t _bp; __asm mov _bp, ebp;
	auto pSendPacket = reinterpret_cast<bool*>(***reinterpret_cast<uintptr_t***>(_bp) - 0x1);

	if (Vars::Misc::NetworkFix.Value || Vars::Misc::PredictionFix.Value) // fixes various issues mainly with low fps, however may cause a couple other issues (?)
		I::Prediction->Update(I::ClientState->m_nDeltaTick, I::ClientState->m_nDeltaTick > 0, I::ClientState->last_command_ack, I::ClientState->lastoutgoingcommand + I::ClientState->chokedcommands);

	G::Buttons = pCmd->buttons;
	G::CurrentUserCmd = pCmd;
	if (!G::LastUserCmd)
		G::LastUserCmd = pCmd;

	F::Backtrack.iTickCount = pCmd->tick_count;
	// correct tick_count for fakeinterp / nointerp
	pCmd->tick_count += TICKS_TO_TIME(F::Backtrack.flFakeInterp) - (Vars::Visuals::RemoveInterpolation.Value ? 0 : TICKS_TO_TIME(G::LerpTime));

	if (pLocal && pWeapon)
	{
		if (const int MaxSpeed = pLocal->m_flMaxspeed())
			G::Frozen = MaxSpeed == 1;

		// Update Global Info
		const int nItemDefIndex = pWeapon->m_iItemDefinitionIndex();

		if (G::CurItemDefIndex != nItemDefIndex || !pWeapon->m_iClip1() || !pLocal->IsAlive() || pLocal->IsTaunting() || pLocal->IsBonked() || pLocal->IsAGhost() || pLocal->IsInBumperKart())
			G::WaitForShift = 1; //Vars::CL_Move::DoubleTap::WaitReady.Value;

		G::CurItemDefIndex = nItemDefIndex;
		G::CanPrimaryAttack = pWeapon->CanPrimary(pLocal);
		G::CanSecondaryAttack = pWeapon->CanSecondary(pLocal);
		if (pWeapon->GetSlot() != SLOT_MELEE)
		{
			if (pWeapon->IsInReload())
				G::CanPrimaryAttack = pWeapon->HasPrimaryAmmoForShot();

			if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN &&
				pWeapon->GetMinigunState() != AC_STATE_FIRING && pWeapon->GetMinigunState() != AC_STATE_SPINNING)
			{
				G::CanPrimaryAttack = false;
			}

			if (G::CurItemDefIndex != Soldier_m_TheBeggarsBazooka && pWeapon->m_iClip1() == 0)
				G::CanPrimaryAttack = false;

			if (pLocal->InCond(TF_COND_GRAPPLINGHOOK))
				G::CanPrimaryAttack = false;
		}
		G::CanHeadShot = pWeapon->CanWeaponHeadShot();
		G::CurWeaponType = Utils::GetWeaponType(pWeapon);
		G::IsAttacking = Utils::IsAttacking(pCmd, pWeapon);
	}

	const bool bSkip = F::AimbotProjectile.bLastTickCancel;
	if (bSkip)
	{
		pCmd->weaponselect = F::AimbotProjectile.bLastTickCancel;
		F::AimbotProjectile.bLastTickCancel = 0;
	}

	// Run Features
	F::Misc.RunPre(pCmd);
	F::BadActors.OnTick();
	F::Backtrack.Run(pCmd);
	F::EnginePrediction.Start(pCmd);
	{
		const bool bAimRan = bSkip ? false : F::Aimbot.Run(pCmd);
		F::Auto.Run(pCmd);
		F::PacketManip.CreateMove(pCmd, pSendPacket);
		if (!bAimRan && G::CanPrimaryAttack && G::IsAttacking && !F::AimbotProjectile.bLastTickCancel)
			F::Visuals.ProjectileTrace(false);
	}
	F::EnginePrediction.End(pCmd);
	F::Ticks.MovePost(pCmd);
	F::CritHack.Run(pCmd);
	F::NoSpread.Run(pCmd);
	F::Misc.RunPost(pCmd, pSendPacket);
	F::Resolver.CreateMove();
	F::FakeAngle.ShouldRun = *pSendPacket;

	if (!G::DoubleTap)
	{
		if (G::PSilentAngles && G::ShiftedTicks == G::MaxShift)
			G::PSilentAngles = false, G::SilentAngles = true;

		static bool bWasSet = false;

		if (G::PSilentAngles && I::ClientState->chokedcommands < 21/*2*/) // failsafe
			*pSendPacket = false, bWasSet = true;
		else if (bWasSet)
			*pSendPacket = true, bWasSet = false;
	}
	else
		AttackingUpdate(pLocal, pWeapon);
	
	G::ViewAngles = pCmd->viewangles;
	G::LastUserCmd = pCmd;

	G::Choking = !*pSendPacket;
	G::ChokedAngles.push_back(pCmd->viewangles);

	F::EnginePrediction.Simulate(pCmd);

	const bool bShouldSkip = G::PSilentAngles || G::SilentAngles || G::AntiAim || G::AvoidingBackstab;
	return bShouldSkip ? false : Hook.Original<FN>()(ecx, edx, input_sample_frametime, pCmd);
}