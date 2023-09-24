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
		{
			lockedTarget.m_pEntity = nullptr;
		}
		else
		{
			Vec3 vPos = lockedTarget.m_pEntity->GetHitboxPos(HITBOX_PELVIS);
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flDistTo = vLocalPos.DistTo(vPos);
			validTargets.push_back({ lockedTarget.m_pEntity, lockedTarget.m_TargetType, vPos, vAngleTo, lockedTarget.m_flFOVTo, flDistTo, -1, false, lockedTarget.n_Priority });

			return validTargets;
		}
	}

	const auto sortMethod = static_cast<ESortMethod>(Vars::Aimbot::Melee::SortMethod.Value);

	// Players
	if (Vars::Aimbot::Global::AimAt.Value & (ToAimAt::PLAYER))
	{
		const bool bDisciplinary = Vars::Aimbot::Melee::WhipTeam.Value && pWeapon->GetItemDefIndex() == Soldier_t_TheDisciplinaryAction;

		for (const auto& pTarget : g_EntityCache.GetGroup(bDisciplinary ? EGroupType::PLAYERS_ALL : EGroupType::PLAYERS_ENEMIES))
		{
			// Is the target valid and alive?
			if (!pTarget->IsAlive() || pTarget->IsAGhost() || pTarget == pLocal)
			{
				continue;
			}

			if (F::AimbotGlobal.ShouldIgnore(pTarget, true)) { continue; }

			Vec3 vPos = pTarget->GetHitboxPos(HITBOX_PELVIS);
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::Melee::AimFOV.Value)
			{
				continue;
			}

			const auto& priority = F::AimbotGlobal.GetPriority(pTarget->GetIndex());
			const float flDistTo = vLocalPos.DistTo(vPos);
			validTargets.push_back({ pTarget, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, -1, false, priority });
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

		for (const auto& pObject : g_EntityCache.GetGroup(hasWrench || canDestroySapper ? EGroupType::BUILDINGS_ALL : EGroupType::BUILDINGS_ENEMIES))
		{
			if (!pObject)
				continue;

			bool isSentry = pObject->GetClassID() == ETFClassID::CObjectSentrygun;
			bool isDispenser = pObject->GetClassID() == ETFClassID::CObjectDispenser;
			bool isTeleporter = pObject->GetClassID() == ETFClassID::CObjectTeleporter;

			if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::SENTRY)) && isSentry) continue;
			if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::DISPENSER)) && isDispenser) continue;
			if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::TELEPORTER)) && isTeleporter) continue;

			const auto& pBuilding = reinterpret_cast<CBaseObject*>(pObject);
			if (!pBuilding || !pObject->IsAlive())
				continue;

			if (pBuilding->GetTeamNum() == pLocal->GetTeamNum())
			{
				if (hasWrench)
				{
					if (!AimFriendlyBuilding(pBuilding))
						continue;
				}

				if (canDestroySapper)
				{
					if (!pBuilding->GetSapped())
						continue;
				}
			}

			Vec3 vPos = pObject->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::Melee::AimFOV.Value)
				continue;

			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;
			validTargets.push_back({ pObject, isSentry ? ETargetType::SENTRY : (isDispenser ? ETargetType::DISPENSER : ETargetType::TELEPORTER), vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	// NPCs
	if (Vars::Aimbot::Global::AimAt.Value & (ToAimAt::NPC))
	{
		for (const auto& pNPC : g_EntityCache.GetGroup(EGroupType::WORLD_NPC))
		{
			Vec3 vPos = pNPC->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;

			if (flFOVTo > Vars::Aimbot::Melee::AimFOV.Value)
			{
				continue;
			}

			validTargets.push_back({ pNPC, ETargetType::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return validTargets;
}

bool CAimbotMelee::AimFriendlyBuilding(CBaseObject* pBuilding)
{
	if (pBuilding->GetLevel() != 3 || pBuilding->GetSapped() || pBuilding->GetHealth() < pBuilding->GetMaxHealth())
	{
		return true;
	}

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
	if (lockedTarget.m_pEntity) return { lockedTarget };

	auto validTargets = GetTargets(pLocal, pWeapon);

	const auto& sortMethod = static_cast<ESortMethod>(1/*Vars::Aimbot::Melee::SortMethod.Value*/);
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
	if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE) return 0;

	return 13;
}

// to do: warp to
void CAimbotMelee::SimulatePlayers(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, std::vector<Target_t> targets, 
								   Vec3& vEyePos, std::unordered_map<CBaseEntity*, std::deque<TickRecord>>& pRecordMap,
								   std::unordered_map<CBaseEntity*, std::deque<std::pair<Vec3, Vec3>>>& simLines)
{
	for (auto& target : targets)
		pRecordMap[target.m_pEntity] = {};

	// swing prediction
	int iTicks = GetSwingTime(pWeapon);
	if (Vars::Aimbot::Melee::PredictSwing.Value && pWeapon->GetSmackTime() < 0.f && iTicks)
	{
		PlayerStorage localStorage;
		std::unordered_map<CBaseEntity*, PlayerStorage> targetStorage;

		bool bFail = false;

		if (!F::MoveSim.Initialize(g_EntityCache.GetLocal(), localStorage))
			bFail = true;
		for (auto& target : targets)
		{
			targetStorage[target.m_pEntity] = {};
			if (!F::MoveSim.Initialize(target.m_pEntity, targetStorage[target.m_pEntity]))
				bFail = true;
		}

		if (!bFail)
		{
			for (int i = 0; i < iTicks; i++) // intended for plocal to collide with targets but doesn't seem to always work
			{
				F::MoveSim.RunTick(localStorage);
				for (auto& target : targets)
				{
					F::MoveSim.RunTick(targetStorage[target.m_pEntity]);
					target.m_pEntity->SetAbsOrigin(targetStorage[target.m_pEntity].m_MoveData.m_vecAbsOrigin);

					pRecordMap[target.m_pEntity].push_front({
						target.m_pEntity->GetSimulationTime() + TICKS_TO_TIME(i + 1),
						I::GlobalVars->curtime + TICKS_TO_TIME(i + 1),
						I::GlobalVars->tickcount + i + 1,
						false,
						BoneMatrixes{},
						target.m_pEntity->GetAbsOrigin()
					});
				}
			}
			vEyePos = localStorage.m_MoveData.m_vecAbsOrigin + pLocal->GetViewOffset();

			G::ProjLines = localStorage.PredictionLines;
			for (auto& target : targets)
				simLines[target.m_pEntity] = targetStorage[target.m_pEntity].PredictionLines;
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
	vecToTarget = pTarget->GetAbsOrigin() - pLocal->GetAbsOrigin();
	vecToTarget.z = 0.0f;
	float vecDist = vecToTarget.Length();
	vecToTarget.NormalizeInPlace();

	Vector vecOwnerForward;
	Math::AngleVectors(eyeAngles, &vecOwnerForward);
	vecOwnerForward.z = 0.0f;
	vecOwnerForward.NormalizeInPlace();

	Vector vecTargetForward;
	Math::AngleVectors(F::Backtrack.noInterpEyeAngles[pTarget->GetIndex()], &vecTargetForward);
	vecTargetForward.z = 0.0f;
	vecTargetForward.NormalizeInPlace();

	float flPosVsTargetViewDot = vecToTarget.Dot(vecTargetForward); // Behind?
	float flPosVsOwnerViewDot = vecToTarget.Dot(vecOwnerForward); // Facing?
	float flViewAnglesDot = vecTargetForward.Dot(vecOwnerForward); // Facestab?

	if (Vars::Aimbot::Melee::IgnoreRazorback.Value && pTarget->GetClassNum() == CLASS_SNIPER)
	{
		auto pWeapon = pTarget->GetWeaponFromSlot(SLOT_SECONDARY);
		if (pWeapon && pWeapon->GetItemDefIndex() == Sniper_s_TheRazorback)
			return false;
	}

	return (vecDist >= 1.f &&
		flPosVsTargetViewDot > 0.f && flPosVsOwnerViewDot > 0.5 && flViewAnglesDot > -0.3f);
}

bool CAimbotMelee::CanHit(Target_t& target, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Vec3 vEyePos, std::deque<TickRecord> newRecords)
{
	static Vec3 vecSwingMins = { -18.0f, -18.0f, -18.0f };
	static Vec3 vecSwingMaxs = { 18.0f, 18.0f, 18.0f };
	
	const float flRange = pWeapon->GetSwingRange(pLocal) - 0.1f;
	if (flRange <= 0.0f)
		return false;

	CTraceFilterHitscan filter;
	filter.pSkip = pLocal;
	CGameTrace trace;

	matrix3x4 bones[128];
	target.m_pEntity->SetupBones(bones, 128, BONE_USED_BY_ANYTHING, target.m_pEntity->GetSimulationTime());

	std::deque<TickRecord> pRecords;
	{
		const auto& records = F::Backtrack.GetRecords(target.m_pEntity);
		if (!records || !Vars::Backtrack::Enabled.Value || target.m_TargetType != ETargetType::PLAYER)
		{
			pRecords.push_front({
				target.m_pEntity->GetSimulationTime(),
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
	Vec3 vecDiff = { 0, 0, (pLocal->IsDucking() && pLocal->OnSolid() || target.m_pEntity->m_fFlags() & FL_DUCKING) ? 45.f : 65.f };

	std::deque<TickRecord> validRecords = target.m_TargetType == ETargetType::PLAYER ? F::Backtrack.GetValidRecords(target.m_pEntity, pRecords, (BacktrackMode)Vars::Backtrack::Method.Value) : pRecords;
	for (auto& pTick : validRecords)
	{
		const Vec3 vRestore = target.m_pEntity->GetAbsOrigin();
		target.m_pEntity->SetAbsOrigin(pTick.vOrigin);

		target.m_vPos = pTick.vOrigin + vecDiff;
		target.m_vAngleTo = Aim(G::CurrentUserCmd->viewangles, Math::CalcAngle(vEyePos, target.m_vPos));

		Vec3 vecForward = Vec3();
		Math::AngleVectors(target.m_vAngleTo, &vecForward);
		Vec3 vecTraceEnd = vEyePos + (vecForward * flRange);

		Utils::Trace(vEyePos, vecTraceEnd, MASK_SHOT, &filter, &trace);
		bool bReturn = (trace.entity && trace.entity == target.m_pEntity);
		if (!bReturn)
		{
			Utils::TraceHull(vEyePos, vecTraceEnd, vecSwingMins, vecSwingMaxs, MASK_SHOT, &filter, &trace);
			bReturn = (trace.entity && trace.entity == target.m_pEntity);
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
				{
					target.ShouldBacktrack = true;
				}
			}

			return true;
		}
	}

	return false;
}



bool CAimbotMelee::IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	if (pWeapon->GetWeaponID() == TF_WEAPON_KNIFE)
	{
		return pCmd->buttons & IN_ATTACK;
	}

	return TIME_TO_TICKS(pWeapon->GetSmackTime()) + 2 == I::GlobalVars->tickcount; // seems to work most (?) of the time, would like to not have arbitrary number
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
		pCmd->viewangles = vAngle;
		Utils::FixMovement(pCmd, pCmd->viewangles);
		G::SilentTime = true;
	}
}

Vec3 CAimbotMelee::Aim(Vec3 vCurAngle, Vec3 vToAngle)
{
	Vec3 vReturn = {};

	vToAngle -= G::PunchAngles;
	Math::ClampAngles(vToAngle);

	switch (Vars::Aimbot::Melee::AimMethod.Value)
	{
	case 0: // Plain
		vReturn = vToAngle;
		break;

	case 1: //Smooth
		if (Vars::Aimbot::Melee::SmoothingAmount.Value == 0)
		{ // plain aim at 0 smoothing factor
			vReturn = vToAngle;
			break;
		}
		//a + (b - a) * t [lerp]
		vReturn = vCurAngle + (vToAngle - vCurAngle) * (1.f - (float)Vars::Aimbot::Melee::SmoothingAmount.Value / 100.f);
		break;

	case 2: // Silent
		vReturn = vToAngle;
		break;

	default: break;
	}

	return vReturn;
}

void CAimbotMelee::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if ((!Vars::Aimbot::Global::Active.Value || !Vars::Aimbot::Melee::Active.Value) && !lockedTarget.m_pEntity || !G::WeaponCanAttack && pWeapon->GetSmackTime() < 0.f)
	{
		lockedTarget.m_pEntity = nullptr;
		return;
	}

	const bool bShouldAim = (Vars::Aimbot::Global::AimKey.Value == VK_LBUTTON ? (pCmd->buttons & IN_ATTACK) : F::AimbotGlobal.IsKeyDown());
	if (!bShouldAim && !lockedTarget.m_pEntity)
		return;

	auto targets = SortTargets(pLocal, pWeapon);
	if (targets.empty())
		return;

	Vec3 vEyePos = pLocal->GetShootPos();
	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> pRecordMap;
	std::unordered_map<CBaseEntity*, std::deque<std::pair<Vec3, Vec3>>> simLines;
	SimulatePlayers(pLocal, pWeapon, targets, vEyePos, pRecordMap, simLines);

	for (auto& target : targets)
	{
		if (!CanHit(target, pLocal, pWeapon, vEyePos, pRecordMap[target.m_pEntity])) continue;

		G::CurrentTargetIdx = target.m_pEntity->GetIndex();
		if (Vars::Aimbot::Melee::AimMethod.Value == 2)
		{
			G::AimPos = target.m_vPos;
		}

		if (Vars::Aimbot::Global::AutoShoot.Value)
		{
			pCmd->buttons |= IN_ATTACK;
		}

		G::IsAttacking = IsAttacking(pCmd, pWeapon);

		if (G::IsAttacking && target.pTick)
		{
			if (!pRecordMap[target.m_pEntity].empty() && !lockedTarget.m_pEntity)
				lockedTarget = target;

			if (target.ShouldBacktrack)
				pCmd->tick_count = TIME_TO_TICKS((*target.pTick).flSimTime) + TIME_TO_TICKS(F::Backtrack.flFakeInterp);

			if (Vars::Visuals::BulletTracer.Value)
			{
				F::Visuals.ClearBulletLines();
				G::BulletsStorage.push_back({ {pLocal->GetShootPos(), target.m_vPos}, I::GlobalVars->curtime + 5.f, Colors::BulletTracer });
			}
			if (Vars::Aimbot::Global::ShowHitboxes.Value)
				F::Visuals.DrawHitbox((matrix3x4*)(&(*target.pTick).BoneMatrix.BoneMatrix), target.m_pEntity);
		}
		if ((G::IsAttacking || !Vars::Aimbot::Global::AutoShoot.Value) && target.pTick && Vars::Visuals::SwingLines.Value)
		{
			G::LinesStorage.clear();
			G::LinesStorage.push_back({ G::ProjLines, I::GlobalVars->curtime + 5.f, Vars::Aimbot::Projectile::ProjectileColor });
			G::LinesStorage.push_back({ simLines[target.m_pEntity], I::GlobalVars->curtime + 5.f, Vars::Aimbot::Projectile::PredictionColor }); // not working for whatever reason
		}

		Aim(pCmd, target.m_vAngleTo);

		break;
	}
}
