#include "AimbotMelee.h"

#include "../../Vars.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../TickHandler/TickHandler.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Visuals/Visuals.h"

std::vector<Target_t> CAimbotMelee::GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	std::vector<Target_t> validTargets;

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();
	if (lockedTarget.m_pEntity)
	{
		if (!lockedTarget.m_pEntity->IsAlive() || lockedTarget.m_pEntity->IsAGhost())
			lockedTarget.m_pEntity = nullptr;
		else
		{
			Vec3 vPos = lockedTarget.m_pEntity->GetHitboxPos(HITBOX_PELVIS);
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flDistTo = vLocalPos.DistTo(vPos);
			validTargets.push_back({ lockedTarget.m_pEntity, lockedTarget.m_TargetType, vPos, vAngleTo, lockedTarget.m_flFOVTo, flDistTo, lockedTarget.m_nPriority });

			return validTargets;
		}
	}

	const auto sortMethod = ESortMethod::DISTANCE; //static_cast<ESortMethod>(Vars::Aimbot::Melee::SortMethod.Value);

	// Players
	if (Vars::Aimbot::Global::AimAt.Value & ToAimAt::PLAYER)
	{
		const bool bDisciplinary = Vars::Aimbot::Melee::WhipTeam.Value && pWeapon->m_iItemDefinitionIndex() == Soldier_t_TheDisciplinaryAction;
		for (const auto& pTarget : g_EntityCache.GetGroup(bDisciplinary ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			// Is the target valid and alive?
			if (!pTarget->IsAlive() || pTarget->IsAGhost() || pTarget == pLocal)
				continue;

			if (F::AimbotGlobal.ShouldIgnore(pTarget))
				continue;

			Vec3 vPos = pTarget->GetHitboxPos(HITBOX_PELVIS);
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::Melee::AimFOV.Value)
				continue;

			const float flDistTo = vLocalPos.DistTo(vPos);
			const int priority = F::AimbotGlobal.GetPriority(pTarget->GetIndex());
			validTargets.push_back({ pTarget, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, priority });
		}
	}

	// Buildings
	if (Vars::Aimbot::Global::AimAt.Value)
	{
		const bool hasWrench = (pWeapon->GetWeaponID() == TF_WEAPON_WRENCH);
		const bool canDestroySapper = (G::CurItemDefIndex == Pyro_t_Homewrecker ||
									   G::CurItemDefIndex == Pyro_t_TheMaul ||
									   G::CurItemDefIndex == Pyro_t_NeonAnnihilator ||
									   G::CurItemDefIndex == Pyro_t_NeonAnnihilatorG);

		for (const auto& pBuilding : g_EntityCache.GetGroup(hasWrench || canDestroySapper ? EGroupType::BUILDINGS_ALL : EGroupType::BUILDINGS_ENEMIES))
		{
			if (!pBuilding->IsAlive())
				continue;

			bool isSentry = pBuilding->GetClassID() == ETFClassID::CObjectSentrygun;
			bool isDispenser = pBuilding->GetClassID() == ETFClassID::CObjectDispenser;
			bool isTeleporter = pBuilding->GetClassID() == ETFClassID::CObjectTeleporter;

			if (!(Vars::Aimbot::Global::AimAt.Value & ToAimAt::SENTRY) && isSentry)
				continue;
			if (!(Vars::Aimbot::Global::AimAt.Value & ToAimAt::DISPENSER) && isDispenser)
				continue;
			if (!(Vars::Aimbot::Global::AimAt.Value & ToAimAt::TELEPORTER) && isTeleporter)
				continue;

			if (pBuilding->m_iTeamNum() == pLocal->m_iTeamNum())
			{
				if (hasWrench)
				{
					if (!AimFriendlyBuilding(pBuilding))
						continue;
				}

				if (canDestroySapper)
				{
					if (!pBuilding->m_bHasSapper())
						continue;
				}
			}

			Vec3 vPos = pBuilding->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = vLocalPos.DistTo(vPos);

			if (flFOVTo > Vars::Aimbot::Melee::AimFOV.Value)
				continue;

			validTargets.push_back({ pBuilding, isSentry ? ETargetType::SENTRY : (isDispenser ? ETargetType::DISPENSER : ETargetType::TELEPORTER), vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	// NPCs
	if (Vars::Aimbot::Global::AimAt.Value & ToAimAt::NPC)
	{
		for (const auto& pNPC : g_EntityCache.GetGroup(EGroupType::WORLD_NPC))
		{
			Vec3 vPos = pNPC->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;

			if (flFOVTo > Vars::Aimbot::Melee::AimFOV.Value)
				continue;

			validTargets.push_back({ pNPC, ETargetType::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return validTargets;
}

bool CAimbotMelee::AimFriendlyBuilding(CBaseEntity* pBuilding)
{
	if (pBuilding->m_iUpgradeLevel() != 3 || pBuilding->m_bHasSapper() || pBuilding->m_iBOHealth() < pBuilding->m_iMaxHealth())
		return true;

	if (pBuilding->IsSentrygun())
	{
		int iShells, iMaxShells, iRockets, iMaxRockets;

		pBuilding->GetAmmoCount(iShells, iMaxShells, iRockets, iMaxRockets);

		if (iShells < iMaxShells || iRockets < iMaxRockets)
			return true;
	}

	return false;
}

std::vector<Target_t> CAimbotMelee::SortTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	if (lockedTarget.m_pEntity)
		return { lockedTarget };

	auto validTargets = GetTargets(pLocal, pWeapon);

	const auto& sortMethod = ESortMethod::DISTANCE; //static_cast<ESortMethod>(Vars::Aimbot::Melee::SortMethod.Value);
	F::AimbotGlobal.SortTargets(&validTargets, sortMethod);

	std::vector<Target_t> sortedTargets = {};
	int i = 0; for (auto& target : validTargets)
	{
		i++; if (i > Vars::Aimbot::Global::MaxTargets.Value) break;

		sortedTargets.push_back(target);
	}

	F::AimbotGlobal.SortPriority(&sortedTargets, sortMethod);

	return sortedTargets;
}



int CAimbotMelee::GetSwingTime(CBaseCombatWeapon* pWeapon)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
		return 0;
	return Vars::Aimbot::Melee::SwingTicks.Value;
}

void CAimbotMelee::SimulatePlayers(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, std::vector<Target_t> targets, 
								   Vec3& vEyePos, std::unordered_map<CBaseEntity*, std::deque<TickRecord>>& pRecordMap,
								   std::unordered_map<CBaseEntity*, std::deque<std::pair<Vec3, Vec3>>>& simLines)
{
	for (auto& target : targets)
		pRecordMap[target.m_pEntity] = {};

	if (lockedTarget.m_pEntity)
		return;

	// swing prediction / auto warp
	const int iSwingTicks = GetSwingTime(pWeapon);
	int iMax = (iDoubletapTicks && Vars::CL_Move::DoubleTap::Options.Value & (1 << 0) && pLocal->OnSolid())
		? std::max(iSwingTicks - Vars::CL_Move::DoubleTap::TickLimit.Value, 0)
		: std::max(iSwingTicks, iDoubletapTicks);

	if ((Vars::Aimbot::Melee::SwingPrediction.Value || iDoubletapTicks) && pWeapon->m_flSmackTime() < 0.f && iMax)
	{
		PlayerStorage localStorage;
		std::unordered_map<CBaseEntity*, PlayerStorage> targetStorage;

		F::MoveSim.Initialize(pLocal, localStorage, false, iDoubletapTicks);
		for (auto& target : targets)
		{
			targetStorage[target.m_pEntity] = {};
			F::MoveSim.Initialize(target.m_pEntity, targetStorage[target.m_pEntity], false);
		}

		for (int i = 0; i < iMax; i++) // intended for plocal to collide with targets
		{
			if (i < iMax/* - 1*/)
			{
				if (pLocal->IsCharging() && iMax - i <= GetSwingTime(pWeapon)) // demo charge fix for swing pred
				{
					localStorage.m_MoveData.m_flMaxSpeed = pLocal->TeamFortress_CalculateMaxSpeed(true);
					localStorage.m_MoveData.m_flClientMaxSpeed = localStorage.m_MoveData.m_flMaxSpeed;
				}
				F::MoveSim.RunTick(localStorage);
			}
			if (i < iSwingTicks - iDoubletapTicks)
			{
				for (auto& target : targets)
				{
					F::MoveSim.RunTick(targetStorage[target.m_pEntity]);
					if (!targetStorage[target.m_pEntity].m_bFailed)
						pRecordMap[target.m_pEntity].push_front({
							target.m_pEntity->m_flSimulationTime() + TICKS_TO_TIME(i + 1),
							I::GlobalVars->curtime + TICKS_TO_TIME(i + 1),
							I::GlobalVars->tickcount + i + 1,
							false,
							BoneMatrixes{},
							targetStorage[target.m_pEntity].m_MoveData.m_vecOrigin
						});
				}
			}
		}
		vEyePos = localStorage.m_MoveData.m_vecOrigin + pLocal->m_vecViewOffset();

		if (Vars::Visuals::SwingLines.Value)
		{
			const bool bAlwaysDraw = !Vars::Aimbot::Global::AutoShoot.Value || Vars::Debug::Info.Value;
			if (!bAlwaysDraw)
			{
				G::ProjLines = localStorage.PredictionLines;
				for (auto& target : targets)
					simLines[target.m_pEntity] = targetStorage[target.m_pEntity].PredictionLines;
			}
			else
			{
				G::LinesStorage.clear();
				G::LinesStorage.push_back({ localStorage.PredictionLines, I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value });
				for (auto& target : targets)
					G::LinesStorage.push_back({ targetStorage[target.m_pEntity].PredictionLines, I::GlobalVars->curtime + 5.f, Vars::Colors::PredictionColor.Value });
			}
		}

		F::MoveSim.Restore(localStorage);
		for (auto& target : targets)
			F::MoveSim.Restore(targetStorage[target.m_pEntity]);
	}
}

bool CAimbotMelee::CanBackstab(CBaseEntity* pTarget, CBaseEntity* pLocal, Vec3 eyeAngles)
{
	if (!pLocal || !pTarget)
		return false;

	Vector vecToTarget;
	vecToTarget = pTarget->GetAbsOrigin() - pLocal->m_vecOrigin();
	vecToTarget.z = 0.0f;
	float vecDist = vecToTarget.Length();
	vecToTarget.Normalize();

	Vector vecOwnerForward;
	Math::AngleVectors(eyeAngles, &vecOwnerForward);
	vecOwnerForward.z = 0.0f;
	vecOwnerForward.Normalize();

	Vector vecTargetForward;
	Math::AngleVectors(F::Backtrack.noInterpEyeAngles[pTarget->GetIndex()], &vecTargetForward);
	vecTargetForward.z = 0.0f;
	vecTargetForward.Normalize();

	float flPosVsTargetViewDot = vecToTarget.Dot(vecTargetForward); // Behind?
	float flPosVsOwnerViewDot = vecToTarget.Dot(vecOwnerForward); // Facing?
	float flViewAnglesDot = vecTargetForward.Dot(vecOwnerForward); // Facestab?

	if (Vars::Aimbot::Melee::IgnoreRazorback.Value && pTarget->m_iClass() == CLASS_SNIPER)
	{
		auto pWeapon = pTarget->GetWeaponFromSlot(SLOT_SECONDARY);
		if (pWeapon && pWeapon->m_iItemDefinitionIndex() == Sniper_s_TheRazorback)
			return false;
	}

	return (vecDist >= 1.f &&
		flPosVsTargetViewDot > 0.f && flPosVsOwnerViewDot > 0.5 && flViewAnglesDot > -0.3f);
}

int CAimbotMelee::CanHit(Target_t& target, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Vec3 vEyePos, std::deque<TickRecord> newRecords)
{
	const float flHull = Utils::ATTRIB_HOOK_FLOAT(18, "melee_bounds_multiplier", pWeapon);
	const float flRange = pWeapon->GetSwingRange(pLocal) * Utils::ATTRIB_HOOK_FLOAT(1, "melee_range_multiplier", pWeapon);
	if (flRange <= 0.f)
		return false;

	static Vec3 vecSwingMins = { -flHull, -flHull, -flHull };
	static Vec3 vecSwingMaxs = { flHull, flHull, flHull };

	CGameTrace trace;
	CTraceFilterHitscan filter;
	filter.pSkip = pLocal;

	matrix3x4 bones[128];
	target.m_pEntity->SetupBones(bones, 128, BONE_USED_BY_ANYTHING, target.m_pEntity->m_flSimulationTime());

	std::deque<TickRecord> pRecords;
	{
		const auto& records = F::Backtrack.GetRecords(target.m_pEntity);
		if (!records || !Vars::Backtrack::Enabled.Value || target.m_TargetType != ETargetType::PLAYER)
		{
			pRecords.push_front({
				target.m_pEntity->m_flSimulationTime(),
				I::GlobalVars->curtime,
				I::GlobalVars->tickcount,
				false,
				*reinterpret_cast<BoneMatrixes*>(&bones),
				target.m_pEntity->m_vecOrigin()
			});
		}
		else
			pRecords = *records;
	}

	if (!newRecords.empty())
	{
		for (TickRecord& pTick : newRecords)
		{
			pRecords.pop_back(); pRecords.push_front({ pTick.flSimTime, pTick.flCreateTime, pTick.iTickCount, false, *reinterpret_cast<BoneMatrixes*>(&bones), pTick.vOrigin });
		}
		for (TickRecord& pTick : pRecords)
		{
			pTick.flSimTime -= TICKS_TO_TIME(newRecords.size());
			pTick.flCreateTime -= TICKS_TO_TIME(newRecords.size());
			pTick.iTickCount -= newRecords.size();
		}
	}

	// this might be retarded
	const float flTargetPos = (target.m_pEntity->m_vecMaxs().z - target.m_pEntity->m_vecMins().z) * 65.f / 82.f;
	const float flLocalPos = (pLocal->m_vecMaxs().z - pLocal->m_vecMins().z) * 65.f / 82.f;
	const Vec3 vecDiff = { 0, 0, std::min(flTargetPos, flLocalPos) };

	std::deque<TickRecord> validRecords = target.m_TargetType == ETargetType::PLAYER
		? F::Backtrack.GetValidRecords(target.m_pEntity, pRecords, (BacktrackMode)Vars::Backtrack::Method.Value, pLocal)
		: pRecords;
	for (auto& pTick : validRecords)
	{
		const Vec3 vRestore = target.m_pEntity->GetAbsOrigin();
		target.m_pEntity->SetAbsOrigin(pTick.vOrigin);

		target.m_vPos = pTick.vOrigin + vecDiff;
		target.m_vAngleTo = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(vEyePos, target.m_vPos));

		Vec3 vecForward = Vec3();
		Math::AngleVectors(target.m_vAngleTo, &vecForward);
		Vec3 vecTraceEnd = vEyePos + (vecForward * flRange);

		Utils::Trace(vEyePos, vecTraceEnd, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
		bool bReturn = trace.entity && trace.entity == target.m_pEntity;
		if (!bReturn)
		{
			Utils::TraceHull(vEyePos, vecTraceEnd, vecSwingMins, vecSwingMaxs, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
			bReturn = trace.entity && trace.entity == target.m_pEntity;
		}
		/* does not respect aimbot viewangles, not using for now but seems promising ?
		bool bReturn = false;
		{
			const Vec3 vPos = pLocal->GetAbsOrigin();
			const Vec3 vAng = pLocal->GetEyeAngles();

			pLocal->SetAbsOrigin(vEyePos - pLocal->m_vecViewOffset());
			pLocal->GetEyeAngles() = vAngleTo;

			CGameTrace trace;
			bReturn = pWeapon->DoSwingTraceInternal(trace);
			if (bReturn)
				bReturn = trace.entity == target.m_pEntity;

			pLocal->SetAbsOrigin(vPos);
			pLocal->GetEyeAngles() = vAng;
		}
		*/

		if (bReturn && Vars::Aimbot::Melee::AutoBackstab.Value && pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
		{
			if (target.m_TargetType == ETargetType::PLAYER)
				bReturn = CanBackstab(target.m_pEntity, pLocal, target.m_vAngleTo);
			else
				bReturn = false;
		}

		target.m_pEntity->SetAbsOrigin(vRestore);

		if (bReturn)
		{
			if (target.m_TargetType == ETargetType::PLAYER)
			{
				target.pTick = &pTick;
				if (Vars::Backtrack::Enabled.Value)
					target.ShouldBacktrack = true;
			}

			return true;
		}
		else if (Vars::Aimbot::Melee::AimMethod.Value == 1)
		{
			auto vAngle = Math::CalcAngle(vEyePos, target.m_vPos);

			Vec3 vecForward = Vec3();
			Math::AngleVectors(vAngle, &vecForward);
			Vec3 vecTraceEnd = vEyePos + (vecForward * flRange);

			Utils::Trace(vEyePos, vecTraceEnd, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
			if (trace.entity && trace.entity == target.m_pEntity)
				return 2;
		}
	}

	return false;
}



bool CAimbotMelee::IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
		return pCmd->buttons & IN_ATTACK;

	return TIME_TO_TICKS(pWeapon->m_flSmackTime()) == I::GlobalVars->tickcount - 1; // seems to work most (?) of the time
}

// assume angle calculated outside with other overload
void CAimbotMelee::Aim(CUserCmd* pCmd, Vec3& vAngle)
{
	if (Vars::Aimbot::Melee::AimMethod.Value != 2)
	{
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
	}
	else if (G::IsAttacking)
	{
		Utils::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
		G::PSilentAngles = true;
	}
}

Vec3 CAimbotMelee::Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod)
{
	Vec3 vReturn = {};

	vToAngle -= G::PunchAngles;
	Math::ClampAngles(vToAngle);

	switch (iMethod)
	{
	case 1: // Smooth
	{
		auto shortDist = [](const float flAngleA, const float flAngleB)
			{
				const float flDelta = fmodf((flAngleA - flAngleB), 360.f);
				return fmodf(2 * flDelta, 360.f) - flDelta;
			};
		const float t = 1.f - (float)Vars::Aimbot::Melee::SmoothingAmount.Value / 100.f;
		vReturn.x = vCurAngle.x - shortDist(vCurAngle.x, vToAngle.x) * t;
		vReturn.y = vCurAngle.y - shortDist(vCurAngle.y, vToAngle.y) * t;
		break;
	}
	case 0: // Plain
	case 2: // Silent
		vReturn = vToAngle;
		break;

	default: break;
	}

	return vReturn;
}

void CAimbotMelee::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (lockedTarget.m_pEntity && pWeapon->m_flSmackTime() < 0.f)
		lockedTarget.m_pEntity = nullptr;
	if ((!Vars::Aimbot::Global::Active.Value || !Vars::Aimbot::Melee::Active.Value) && !lockedTarget.m_pEntity || (!G::WeaponCanAttack || !Vars::Aimbot::Global::AutoShoot.Value) && pWeapon->m_flSmackTime() < 0.f)
		return;

	const bool bShouldAim = (Vars::Aimbot::Global::AimKey.Value == VK_LBUTTON ? (pCmd->buttons & IN_ATTACK) : F::AimbotGlobal.IsKeyDown());
	if (!bShouldAim && !lockedTarget.m_pEntity)
		return;

	auto targets = SortTargets(pLocal, pWeapon);
	if (targets.empty())
		return;

	iDoubletapTicks = F::Ticks.GetTicks(pLocal);
	const bool bShouldSwing = iDoubletapTicks <= (GetSwingTime(pWeapon) ? 14 : 0) || Vars::CL_Move::DoubleTap::Options.Value & (1 << 0) && pLocal->OnSolid();

	Vec3 vEyePos = pLocal->GetShootPos();
	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> pRecordMap;
	std::unordered_map<CBaseEntity*, std::deque<std::pair<Vec3, Vec3>>> simLines;
	SimulatePlayers(pLocal, pWeapon, targets, vEyePos, pRecordMap, simLines);

	for (auto& target : targets)
	{
		const auto iResult = CanHit(target, pLocal, pWeapon, vEyePos, pRecordMap[target.m_pEntity]);
		if (!iResult) continue;
		if (iResult == 2)
		{
			Aim(pCmd, target.m_vAngleTo);
			break;
		}

		G::CurrentTarget = { target.m_pEntity->GetIndex(), I::GlobalVars->tickcount };
		if (!pRecordMap[target.m_pEntity].empty() && !lockedTarget.m_pEntity)
			lockedTarget = target;

		if (Vars::Aimbot::Melee::AimMethod.Value == 2)
			G::AimPos = target.m_vPos;

		if (Vars::Aimbot::Global::AutoShoot.Value && pWeapon->m_flSmackTime() < 0.f)
		{
			if (bShouldSwing)
				pCmd->buttons |= IN_ATTACK;
			if (iDoubletapTicks)
				G::DoubleTap = true;
		}
		// game will not manage this while shifting, do it manually
		static float flSmackTime = -1.f;
		if (G::DoubleTap && pCmd->buttons & IN_ATTACK && pWeapon->GetWeaponID() != TF_WEAPON_KNIFE)
		{
			flSmackTime = TICKS_TO_TIME(I::GlobalVars->tickcount + 13 - (Vars::CL_Move::DoubleTap::Options.Value & (1 << 0) && pLocal->OnSolid() ? 1 : 0));
			pWeapon->m_flSmackTime() = flSmackTime;
		}
		else if (pWeapon->m_flSmackTime() > 0.f && flSmackTime > 0.f)
			pWeapon->m_flSmackTime() = flSmackTime;
		else if (pWeapon->m_flSmackTime() < 0.f)
			flSmackTime = -1.f;

		const bool bAttacking = IsAttacking(pCmd, pWeapon);
		G::IsAttacking = bAttacking || bShouldSwing && G::DoubleTap; // dumb but works

		if (G::IsAttacking && target.pTick)
		{
			if (target.ShouldBacktrack)
				pCmd->tick_count = TIME_TO_TICKS((*target.pTick).flSimTime) + TIME_TO_TICKS(F::Backtrack.flFakeInterp) + Vars::Backtrack::TicksetOffset.Value + G::AnticipatedChoke * Vars::Backtrack::ChokeSetMod.Value;
			// bug: older (maybe only fast) records seem to be progressively more unreliable ?
			// ^ this might be something wonky with the tr_walkway ramp because rijin misses on this too lmao

			if (Vars::Visuals::BulletTracer.Value)
			{
				F::Visuals.ClearBulletLines();
				G::BulletsStorage.push_back({ {vEyePos, target.m_vPos}, I::GlobalVars->curtime + 5.f, Vars::Colors::BulletTracer.Value, true });
			}
			if (Vars::Visuals::ShowHitboxes.Value)
				F::Visuals.DrawHitbox((matrix3x4*)(&(*target.pTick).BoneMatrix.BoneMatrix), target.m_pEntity);
		}
		if (Vars::Visuals::SwingLines.Value && target.pTick && G::IsAttacking)
		{
			const bool bAlwaysDraw = !Vars::Aimbot::Global::AutoShoot.Value || Vars::Debug::Info.Value;
			if (!bAlwaysDraw)
			{
				G::LinesStorage.clear();
				G::LinesStorage.push_back({ G::ProjLines, I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value });
				G::LinesStorage.push_back({ simLines[target.m_pEntity], I::GlobalVars->curtime + 5.f, Vars::Colors::PredictionColor.Value }); // not working for whatever reason
			}
		}

		Aim(pCmd, target.m_vAngleTo);
		G::IsAttacking = bAttacking;
		break;
	}
}
