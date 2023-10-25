#include "AutoJump.h"

void CAutoJump::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	const Vec3 vCurrPos = pLocal->GetVecOrigin();
	const bool bCurrGrounded = pLocal->OnSolid();

	bool bValidWeapon = false;
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON: bValidWeapon = true;
		}
		if (bValidWeapon && pWeapon->GetItemDefIndex() == Soldier_m_TheBeggarsBazooka)
			bValidWeapon = G::IsAttacking;
	}
	if (bValidWeapon)
		pCmd->buttons &= ~IN_ATTACK2; // fix for retarded issue

	// barebones (doesn't seem 100% consistent, unsure if it's user error or what)
	if (bValidWeapon && iJumpFrame == -1 && bCurrGrounded && bCurrGrounded == bLastGrounded &&
		G::WeaponCanAttack && !pLocal->IsDucking() && pWeapon->GetClip1() > 0)
	{
		if (F::KeyHandler.Down(Vars::Triggerbot::Jump::JumpKey.Value))
		{
			iJumpFrame = 0;
			bFire = true;
		}

		if (F::KeyHandler.Down(Vars::Triggerbot::Jump::CTapKey.Value))
			iJumpFrame = 0;
	}

	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost() || I::EngineVGui->IsGameUIVisible())
		iJumpFrame = -1;

	if (iJumpFrame != -1)
	{
		// don't do a ctap if moving up a slope -/ not looking down far enough-
		if (iJumpFrame == 0)
			bCTap = vCurrPos.z - vLastPos.z < 0.5f/* && pCmd->viewangles.x > 30.f*/;

		iJumpFrame++;

		switch (iJumpFrame)
		{
		case 1:
			if (bCTap && pWeapon->GetWeaponID() != TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT) // need to be quicker with direct hit, skip
			{
				pCmd->buttons |= IN_ATTACK;
				if (pWeapon->IsInReload()) // repeat, takes 1 tick to get out of reload to actually fire
					iJumpFrame--;
				break;
			}
			iJumpFrame++;
			[[fallthrough]];
		case 2:
		{
			bool bBreak = bCTap && pWeapon->GetWeaponID() != TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT;
			if ((!bCTap || pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT) && pWeapon->IsInReload())
			{
				pCmd->buttons |= IN_ATTACK;
				bBreak = true;
			}
			if (bCTap)
				pCmd->buttons |= IN_DUCK;
			else
				pCmd->buttons |= IN_JUMP;
			if (bBreak)
				break;
			iJumpFrame++;
			[[fallthrough]];
		}
		case 3:
			if (!bCTap || pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT)
				pCmd->buttons |= IN_ATTACK;
			if (!(G::LastUserCmd->buttons & IN_DUCK))
				pCmd->buttons |= IN_DUCK;
			if (!(G::LastUserCmd->buttons & IN_JUMP))
				pCmd->buttons |= IN_JUMP;
		}

		if (bFire && pCmd->buttons & IN_ATTACK)
		{
			G::UpdateView = false; // would use G::SilentTime but that would mess with timing

			const bool bOriginal = pWeapon->GetItemDefIndex() == Soldier_m_TheOriginal;
			const bool bMoving = pLocal->m_vecVelocity().Length2D() > 200.f;

			float v_x = 0.f;
			float v_y = bMoving ? Math::VelocityToAngles(pLocal->m_vecVelocity()).y : pCmd->viewangles.y;
			if (bOriginal)
			{
				v_x = bMoving ? 70.f : 89.f;
				v_y -= 180.f;
			}
			else
			{
				v_x = bMoving ? 75.f : 89.f;
				v_y -= bMoving ? 133.f : 81.5f;
			}
			pCmd->viewangles = { v_x, v_y, 0 };
		}

		if (iJumpFrame == 4)
		{
			iJumpFrame = -1;
			bFire = false, bCTap = false;
		}
	}

	vLastPos = vCurrPos;
	bLastGrounded = bCurrGrounded;
}