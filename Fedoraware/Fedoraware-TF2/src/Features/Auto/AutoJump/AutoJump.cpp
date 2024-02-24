#include "AutoJump.h"

#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"

void CAutoJump::ManageAngle(CBaseCombatWeapon* pWeapon, CUserCmd* pCmd, Vec3& viewAngles)
{
	Vec3 wishVel = { pCmd->forwardmove, pCmd->sidemove, 0.f }, wishAng;
	Math::VectorAngles(wishVel, wishAng);

	const bool bMoving = wishVel.Length2D() > 200.f;

	float v_x = 0.f;
	float v_y = bMoving ? viewAngles.y - wishAng.y : viewAngles.y;
	if (pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheOriginal)
	{
		v_x = bMoving ? 70.f : 89.f;
		v_y -= 180.f;
	}
	else
	{
		v_x = bMoving ? 75.f : 89.f;
		v_y -= bMoving ? 133.f : 81.5f;
	}
	viewAngles = { v_x, v_y, 0 };
}

void CAutoJump::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	const bool bCurrGrounded = pLocal->OnSolid();

	bool bValidWeapon = false;
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_PARTICLE_CANNON: bValidWeapon = true;
		}
		if (bValidWeapon && pWeapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka)
			bValidWeapon = G::IsAttacking;
	}
	if (bValidWeapon && (Vars::Auto::Jump::JumpKey.Value == VK_RBUTTON || Vars::Auto::Jump::CTapKey.Value == VK_RBUTTON))
		pCmd->buttons &= ~IN_ATTACK2; // fix for retarded issue

	// doesn't seem 100% consistent, unsure if it's fps related, user error, or what
	if (pLocal->IsAlive() && !pLocal->IsAGhost() && !I::EngineVGui->IsGameUIVisible() && !I::VGuiSurface->IsCursorVisible() && bValidWeapon)
	{
		if (iFrame == -1 && G::CanPrimaryAttack && pWeapon->m_iClip1() > 0)
		{
			const bool bJumpKey = F::KeyHandler.Down(Vars::Auto::Jump::JumpKey.Value);
			const bool bCTapKey = F::KeyHandler.Down(Vars::Auto::Jump::CTapKey.Value);
			bool bWillHit = false;
			const bool bReloading = pWeapon->IsInReload();

			Vec3 viewAngles = pCmd->viewangles;
			if (bJumpKey)
				ManageAngle(pWeapon, pCmd, viewAngles);
			if (bJumpKey || bCTapKey)
			{
				ProjectileInfo projInfo = {};
				if (F::ProjSim.GetInfo(pLocal, pWeapon, viewAngles, projInfo) && F::ProjSim.Initialize(projInfo))
				{
					for (int n = 0; n < 10; n++)
					{
						Vec3 Old = F::ProjSim.GetOrigin();
						F::ProjSim.RunTick(projInfo);
						Vec3 New = F::ProjSim.GetOrigin();

						CGameTrace trace = {};
						CTraceFilterProjectile filter = {};
						filter.pSkip = pLocal;
						Utils::Trace(Old, New, MASK_SOLID, &filter, &trace);
						if (trace.DidHit())
						{
							auto VisPos = [](CBaseEntity* pSkip, const CBaseEntity* pEntity, const Vec3& from, const Vec3& to)
								{
									CGameTrace trace = {};
									CTraceFilterProjectile filter = {};
									filter.pSkip = pSkip;
									Utils::Trace(from, to, MASK_SOLID, &filter, &trace);
									if (trace.DidHit())
										return trace.entity && trace.entity == pEntity;
									return true;
								};

							// distance of 150 seems to be ideal if we predict player movement
							if (!n || trace.vEndPos.DistTo(pLocal->GetShootPos()) < 140.f && VisPos(pLocal, pLocal, trace.vEndPos, pLocal->GetShootPos()))
							{	// this might be ever so slightly slow due to how jank rockets are, might cause occasional issues
								iDelay = std::max(n + (n > Vars::Auto::Jump::ApplyAbove.Value ? Vars::Auto::Jump::TimingOffset.Value : 0), 0);
								bWillHit = true;

								Utils::ConLog("Auto jump", std::format("Ticks to hit: {}", iDelay).c_str(), { 255, 0, 0, 255 }, Vars::Debug::Logging.Value);
								if (Vars::Debug::Info.Value && !bReloading)
								{
									G::LinesStorage.push_back({ {{ pLocal->GetShootPos(), {} }, { trace.vEndPos, {} }}, I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value, true });
									Vec3 angles; Math::VectorAngles(trace.Plane.normal, angles);
									G::BoxesStorage.push_back({ trace.vEndPos, { -1.f, -1.f, -1.f }, { 1.f, 1.f, 1.f }, angles, I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value, {}, true });
								}
							}

							break;
						}
					}
				}
			}

			if (bWillHit)
			{
				if (bCurrGrounded && bCurrGrounded == bLastGrounded && !pLocal->IsDucking() && !bReloading)
				{
					if (bJumpKey)
					{
						iFrame = 0;
						bFull = true;
					}
					else if (bCTapKey)
						iFrame = 0;
				}
				else if (!bCurrGrounded && pCmd->buttons & IN_DUCK || bReloading)
				{
					pCmd->buttons |= IN_ATTACK;
					if (bJumpKey)
					{
						G::SilentAngles = true; // would use G::PSilentAngles but that would mess with timing
						pCmd->viewangles = viewAngles;
					}
				}
			}

			if (iFrame == -1 && pWeapon->GetWeaponID() == TF_WEAPON_PARTICLE_CANNON && G::Buttons & IN_ATTACK2)
				pCmd->buttons |= IN_ATTACK2;
		}
	}
	else
		iFrame = -1;

	if (iFrame != -1)
	{
		iFrame++;

		// even if we aren't attacking, prevent other stuff from messing with timing, e.g. antiaim
		G::IsAttacking = true;

		if (iFrame == 1)
		{
			pCmd->buttons |= IN_ATTACK;
			if (bFull)
			{
				G::SilentAngles = true; // would use G::PSilentAngles but that would mess with timing
				ManageAngle(pWeapon, pCmd, pCmd->viewangles);
			}
		}

		if (iDelay > 1)
		{
			switch (iFrame - iDelay + 2)
			{
			case 1:
				pCmd->buttons |= IN_DUCK;
				break;
			case 2:
				pCmd->buttons |= IN_JUMP;
			}
		}
		else // won't ctap in time
			pCmd->buttons |= IN_DUCK | IN_JUMP;

		if (iFrame == iDelay + (iDelay > 1 ? 1 : 3))
		{
			iFrame = -1;
			bFull = false;
		}
	}

	bLastGrounded = bCurrGrounded;
}