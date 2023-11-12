#include "AimbotProjectile.h"

#include "../../Vars.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"
#include "../../Simulation/ProjectileSimulation/ProjectileSimulation.h"
#include "../../Backtrack/Backtrack.h"
#include "../../Visuals/Visuals.h"

/* Returns all valid targets */
std::vector<Target_t> CAimbotProjectile::GetTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	std::vector<Target_t> validTargets;
	const auto sortMethod = static_cast<ESortMethod>(Vars::Aimbot::Projectile::SortMethod.Value);

	const Vec3 vLocalPos = pLocal->GetShootPos();
	const Vec3 vLocalAngles = I::EngineClient->GetViewAngles();

	// Players
	if (Vars::Aimbot::Global::AimAt.Value & (ToAimAt::PLAYER))
	{
		EGroupType groupType = EGroupType::PLAYERS_ENEMIES;
		if (pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW || SandvichAimbot::bIsSandvich)
			groupType = EGroupType::PLAYERS_ALL;
		else if (SandvichAimbot::bIsSandvich)
			groupType = EGroupType::PLAYERS_TEAMMATES;

		for (const auto& pTarget : g_EntityCache.GetGroup(groupType))
		{
			if (!pTarget->IsAlive() || pTarget->IsAGhost() || pTarget == pLocal)
				continue;

			// Check if weapon should shoot at friendly players
			if ((groupType == EGroupType::PLAYERS_ALL || groupType == EGroupType::PLAYERS_TEAMMATES) &&
				pTarget->m_iTeamNum() == pLocal->m_iTeamNum())
			{
				if (pTarget->m_iHealth() >= pTarget->GetMaxHealth())
					continue;
			}

			if (pTarget->m_iTeamNum() != pLocal->m_iTeamNum())
			{
				if (F::AimbotGlobal.ShouldIgnore(pTarget))
					continue;
			}

			Vec3 vPos = pTarget->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
				continue;

			const float flDistTo = (sortMethod == ESortMethod::DISTANCE) ? vLocalPos.DistTo(vPos) : 0.0f;
			const auto& priority = F::AimbotGlobal.GetPriority(pTarget->GetIndex());
			validTargets.push_back({ pTarget, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, -1, false, priority });
		}
	}

	// Buildings
	const bool bIsRescueRanger = pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE;

	for (const auto& pBuilding : g_EntityCache.GetGroup(bIsRescueRanger ? EGroupType::BUILDINGS_ALL : EGroupType::BUILDINGS_ENEMIES))
	{
		bool isSentry = pBuilding->GetClassID() == ETFClassID::CObjectSentrygun;
		bool isDispenser = pBuilding->GetClassID() == ETFClassID::CObjectDispenser;
		bool isTeleporter = pBuilding->GetClassID() == ETFClassID::CObjectTeleporter;

		if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::SENTRY)) && isSentry)
			continue;
		if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::DISPENSER)) && isDispenser)
			continue;
		if (!(Vars::Aimbot::Global::AimAt.Value & (ToAimAt::TELEPORTER)) && isTeleporter)
			continue;

		const auto& Building = reinterpret_cast<CBaseObject*>(pBuilding);

		if (!pBuilding->IsAlive()) { continue; }

		// Check if the Rescue Ranger should shoot at friendly buildings
		if (bIsRescueRanger && (pBuilding->m_iTeamNum() == pLocal->m_iTeamNum()))
		{
			if (Building->GetHealth() >= Building->GetMaxHealth())
				continue;
		}

		Vec3 vPos = pBuilding->GetWorldSpaceCenter();
		Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
		const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

		if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
			continue;
		const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;
		validTargets.push_back({ pBuilding, isSentry ? ETargetType::SENTRY : (isDispenser ? ETargetType::DISPENSER : ETargetType::TELEPORTER), vPos, vAngleTo, flFOVTo, flDistTo });
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

			if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
				continue;

			validTargets.push_back({ pNPC, ETargetType::NPC, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	//Bombs
	if (Vars::Aimbot::Global::AimAt.Value & (ToAimAt::BOMB))
	{
		//This is pretty bad with projectiles
		for (const auto& pBomb : g_EntityCache.GetGroup(EGroupType::WORLD_BOMBS))
		{
			Vec3 vPos = pBomb->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);

			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);
			const float flDistTo = sortMethod == ESortMethod::DISTANCE ? vLocalPos.DistTo(vPos) : 0.0f;

			if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
				continue;

			if (!F::AimbotGlobal.ValidBomb(pBomb))
				continue;

			validTargets.push_back({ pBomb, ETargetType::BOMBS, vPos, vAngleTo, flFOVTo, flDistTo });
		}
	}

	return validTargets;
}

std::vector<Target_t> CAimbotProjectile::SortTargets(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	auto validTargets = GetTargets(pLocal, pWeapon);

	const auto& sortMethod = static_cast<ESortMethod>(Vars::Aimbot::Projectile::SortMethod.Value);
	F::AimbotGlobal.SortTargets(&validTargets, sortMethod);
	F::AimbotGlobal.SortPriority(&validTargets, sortMethod);

	std::vector<Target_t> sortedTargets = {};
	int i = 0; for (auto& target : validTargets)
	{
		i++; if (i > Vars::Aimbot::Global::MaxTargets.Value) break;

		sortedTargets.push_back(target);
	}

	return sortedTargets;
}



bool CanSplash(CBaseCombatWeapon* pWeapon)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_PARTICLE_CANNON:
	case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
	case TF_WEAPON_ROCKETLAUNCHER:
	case TF_WEAPON_PIPEBOMBLAUNCHER: return true;
	}

	return false;
}

bool IsPipe(CBaseCombatWeapon* pWeapon)
{
	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_GRENADELAUNCHER:
	case TF_WEAPON_CANNON:
	case TF_WEAPON_PIPEBOMBLAUNCHER:
	case TF_WEAPON_STICKBOMB:
	case TF_WEAPON_STICKY_BALL_LAUNCHER: return true;
	}

	return false;
}

float PrimeTime(CBaseCombatWeapon* pWeapon)
{
	switch (pWeapon->GetItemDefIndex())
	{
	case Demoman_s_StickybombLauncher:
	case Demoman_s_StickybombLauncherR:
	case Demoman_s_FestiveStickybombLauncher: return 0.8f;
	case Demoman_s_TheQuickiebombLauncher: return 0.6f;
	case Demoman_s_TheScottishResistance: return 1.6f;
	}

	return 0.f;
}

float SolveProjectileSpeed(ProjectileInfo projInfo, CBaseCombatWeapon* pWeapon, const Vec3& vLocalPos, const Vec3& vTargetPos)
{
	const float flVelocity = projInfo.m_velocity;
	const float flGrav = 800.0f * projInfo.m_gravity;
	if (!IsPipe(pWeapon) || !flGrav)
		return flVelocity;

	const Vec3 vDelta = vTargetPos - vLocalPos;
	const float flDist = vDelta.Length2D();

	const float flRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flVelocity, 2));
	if (flRoot < 0.f)
		return 0.f;

	const float flPitch = atan((pow(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
	const float flTime = flDist / (cos(flPitch) * flVelocity);

	float flDrag = 1.f;
	switch (pWeapon->GetItemDefIndex())
	{
	case Demoman_m_GrenadeLauncher:
	case Demoman_m_GrenadeLauncherR:
	case Demoman_m_FestiveGrenadeLauncher:
	case Demoman_m_TheIronBomber:
	case Demoman_m_Autumn:
	case Demoman_m_MacabreWeb:
	case Demoman_m_Rainbow:
	case Demoman_m_SweetDreams:
	case Demoman_m_CoffinNail:
	case Demoman_m_TopShelf:
	case Demoman_m_Warhawk:
	case Demoman_m_ButcherBird: flDrag = 0.12f; break;
	case Demoman_m_TheLochnLoad: flDrag = 0.07f;  break;
	case Demoman_m_TheLooseCannon: flDrag = 0.49f; break;
	case Demoman_s_StickybombLauncher:
	case Demoman_s_StickybombLauncherR:
	case Demoman_s_FestiveStickybombLauncher:
	case Demoman_s_TheQuickiebombLauncher:
	case Demoman_s_TheScottishResistance: flDrag = 0.2f; break;
	}

	return flVelocity - (flVelocity * flTime) * flDrag;
}

int CAimbotProjectile::GetHitboxPriority(int nHitbox, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target)
{
	bool bHeadshot = target.m_TargetType == ETargetType::PLAYER && pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW;
	if (Vars::Aimbot::Global::BAimLethal.Value && bHeadshot)
	{
		float charge = I::GlobalVars->curtime - pWeapon->GetChargeBeginTime();
		float damage = Math::RemapValClamped(charge, 0.f, 1.f, 50.f, 120.f);
		if (pLocal->IsMiniCritBoosted())
			damage *= 1.36f;
		if (damage >= target.m_pEntity->m_iHealth())
			bHeadshot = false;

		if (pLocal->IsCritBoosted()) // for reliability
			bHeadshot = false;
	}
	const bool bLower = target.m_pEntity->m_fFlags() & FL_ONGROUND && CanSplash(pWeapon);

	if (bHeadshot)
		target.m_nAimedHitbox = HITBOX_HEAD;

	switch (nHitbox)
	{
	case 0: return bHeadshot ? 0 : 2; // head
	case 1: return bHeadshot ? 3 : (bLower ? 1 : 0); // body
	case 2: return bHeadshot ? 3 : (bLower ? 0 : 1); // feet
	}

	return 3;
};

bool CAimbotProjectile::CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, const float flVelocity, const float flGravity, Solution_t& out)
{
	const Vec3 vDelta = vTargetPos - vLocalPos;
	const float flDist = vDelta.Length2D();
	const float flGrav = g_ConVars.sv_gravity->GetFloat() * flGravity;

	if (!flGrav)
	{
		const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTargetPos);
		out.m_flPitch = -DEG2RAD(vAngleTo.x);
		out.m_flYaw = DEG2RAD(vAngleTo.y);
	}
	else
	{ // arch
		const float flRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flVelocity, 2));
		if (flRoot < 0.f)
			return false;
		out.m_flPitch = atan((pow(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
		out.m_flYaw = atan2(vDelta.y, vDelta.x);
	}
	out.m_flTime = flDist / (cos(out.m_flPitch) * flVelocity);

	return true;
}

bool CAimbotProjectile::TestAngle(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target, const Vec3& vOriginal, const Vec3& vPredict, const Vec3& vAngles, const int& iSimTime)
{
	target.m_pEntity->SetAbsOrigin(vOriginal);

	ProjectileInfo projInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, vAngles, projInfo))
		return false;

	if (!F::ProjSim.Initialize(projInfo))
		return false;

	target.m_pEntity->SetAbsOrigin(vPredict);

	for (int n = 0; n < iSimTime; n++) {
		Vec3 Old = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(projInfo);
		Vec3 New = F::ProjSim.GetOrigin();

		CGameTrace trace = {};
		CTraceFilterProjectile filter = {};
		filter.pSkip = pLocal;
		Utils::TraceHull(Old, New, projInfo.m_hull * -1.f, projInfo.m_hull, MASK_SOLID, &filter, &trace);

		if (trace.DidHit())
		{
			target.m_pEntity->SetAbsOrigin(vOriginal);

			if (trace.entity && trace.entity == target.m_pEntity)
			{
				G::ProjLines = projInfo.PredictionLines;
				G::ProjLines.push_back({ trace.vEndPos, Math::GetRotatedPosition(trace.vEndPos, Math::VelocityToAngles(F::ProjSim.GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value) });

				// attempted to have a headshot check though this seems more detrimental than useful outside of smooth aimbot
				if (Vars::Aimbot::Projectile::AimMethod.Value == 1 && target.m_nAimedHitbox == HITBOX_HEAD)
				{ // i think this is accurate ?
					const Vec3 vOffset = (trace.vEndPos - New) + (vOriginal - vPredict);

					Vec3 vOld = F::ProjSim.GetOrigin() + vOffset;
					F::ProjSim.RunTick(projInfo);
					Vec3 vNew = F::ProjSim.GetOrigin() + vOffset;

					CGameTrace cTrace = {};
					Utils::Trace(vOld, vNew, MASK_SHOT, &filter, &cTrace);
					cTrace.vEndPos -= vOffset;

					if (cTrace.DidHit() && (!cTrace.entity || cTrace.entity != target.m_pEntity || cTrace.hitbox != HITBOX_HEAD))
						return false;
					
					if (!cTrace.DidHit()) // loop and see if closest hitbox is head
					{
						const model_t* pModel = target.m_pEntity->GetModel();
						if (!pModel) return false;
						const studiohdr_t* pHDR = I::ModelInfoClient->GetStudioModel(pModel);
						if (!pHDR) return false;
						const mstudiohitboxset_t* pSet = pHDR->GetHitboxSet(target.m_pEntity->m_nHitboxSet());
						if (!pSet) return false;

						matrix3x4 BoneMatrix[128];
						if (!target.m_pEntity->SetupBones(BoneMatrix, 128, BONE_USED_BY_ANYTHING, target.m_pEntity->m_flSimulationTime()))
							return false;

						QAngle direction; Vector forward;
						Math::VectorAngles(Old - New, direction);
						Math::AngleVectors(direction, &forward);
						const Vec3 vPos = trace.vEndPos + forward * 16 + vOriginal - vPredict;

						//F::Visuals.ClearBulletLines();
						//G::BulletsStorage.push_back({ {pLocal->GetShootPos(), vPos}, I::GlobalVars->curtime + 5.f, Vars::Aimbot::Projectile::PredictionColor });

						float closestDist; int closestId = -1;
						for (int i = 0; i < pSet->numhitboxes; ++i)
						{
							const mstudiobbox_t* bbox = pSet->hitbox(i);
							if (!bbox)
								continue;

							matrix3x4 rotMatrix;
							Math::AngleMatrix(bbox->angle, rotMatrix);
							matrix3x4 matrix;
							Math::ConcatTransforms(BoneMatrix[bbox->bone], rotMatrix, matrix);
							Vec3 mOrigin;
							Math::GetMatrixOrigin(matrix, mOrigin);

							const float flDist = vPos.DistTo(mOrigin);
							if (closestId != -1 && flDist < closestDist || closestId == -1)
							{
								closestDist = flDist;
								closestId = i;
							}
						}
						return closestId == 0;
					}
				}

				return true;
			}
			return false;
		}
	}

	return false;
}

bool CAimbotProjectile::CanHit(Target_t& target, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	if (Vars::Aimbot::Global::IgnoreOptions.Value & (1 << 6) && G::ChokeMap[target.m_pEntity->GetIndex()] > Vars::Aimbot::Global::TickTolerance.Value)
		return false;

	const INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan)
		return false;

	ProjectileInfo projInfo = {};

	const bool bPlayer = target.m_TargetType == ETargetType::PLAYER;
	const Vec3 vEyePos = pLocal->GetShootPos();
	const Vec3 vOriginalPos = target.m_pEntity->GetAbsOrigin();
	Vec3 vTargetPos = vOriginalPos, vOffset = {};
	float flMaxTime = Vars::Aimbot::Projectile::PredictionTime.Value;
	{
		if (!F::ProjSim.GetInfo(pLocal, pWeapon, {}, projInfo))
			return false;

		if (flMaxTime > projInfo.m_lifetime)
			flMaxTime = projInfo.m_lifetime;
	}
	//const int iLatency = TIME_TO_TICKS(F::Backtrack.GetReal());
	//const float flLatency = F::Backtrack.GetReal()/*iNetChan->GetLatency(FLOW_OUTGOING)*/ + I::GlobalVars->interval_per_tick;
	const float flLatency = F::Backtrack.GetReal()/*iNetChan->GetLatency(FLOW_OUTGOING)*/;
	const float latOff = I::GlobalVars->interval_per_tick * Vars::Aimbot::Projectile::LatOff.Value;
	const float phyOff = I::GlobalVars->interval_per_tick * Vars::Aimbot::Projectile::PhyOff.Value;

	std::vector<Vec3> vPoints = {};
	{
		const bool bIsDucking = (target.m_pEntity->m_fFlags() & FL_DUCKING);
		const Vec3 vMins = I::GameMovement->GetPlayerMins(bIsDucking), vMaxs = I::GameMovement->GetPlayerMaxs(bIsDucking);

		for (int i = 0; i < 3; i++) {
			for (int n = 0; n < 3; n++) {
				if (GetHitboxPriority(n, pLocal, pWeapon, target) == i)
				{
					switch (n)
					{
					case 0:
						/*
						vPoints.push_back(bPlayer && target.m_nAimedHitbox == HITBOX_HEAD ?
										  Utils::GetHeadOffset(target.m_pEntity, { 0, -6.f, 0 }) : // i think this is the right axis
										  Vec3(0, 0, vMaxs.z - 10.f)); break;
						*/
						if (bPlayer && target.m_nAimedHitbox == HITBOX_HEAD)
						{
							switch (Vars::Aimbot::Projectile::HuntermanMode.Value)
							{
							case 0: vPoints.push_back(Utils::GetHeadOffset(target.m_pEntity)); break;
							case 1: vPoints.push_back(Utils::GetHeadOffset(target.m_pEntity, { 0, -Vars::Aimbot::Projectile::HuntermanShift.Value, 0 })); break;
							case 2: { const Vec3 vOff = Utils::GetHeadOffset(target.m_pEntity);
									  vPoints.push_back(Vec3(vOff.x, vOff.y, std::min(vOff.z + Vars::Aimbot::Projectile::HuntermanShift.Value, vMaxs.z))); break; }
							case 3: { const Vec3 vOff = Utils::GetHeadOffset(target.m_pEntity);
									  vPoints.push_back(Vec3(vOff.x, vOff.y, std::max(vMaxs.z - Vars::Aimbot::Projectile::VerticalShift.Value, vOff.z))); break; }
							case 4: { const Vec3 vOff = Utils::GetHeadOffset(target.m_pEntity);
									  vPoints.push_back(Vec3(vOff.x, vOff.y, vOff.z + (vMaxs.z - vOff.z) * (Vars::Aimbot::Projectile::HuntermanLerp.Value / 100.f))); break; }
							}
						}
						else
							vPoints.push_back(Vec3(0, 0, vMaxs.z - Vars::Aimbot::Projectile::VerticalShift.Value));
						break;
					case 1: vPoints.push_back(Vec3(0, 0, (vMins.z + vMaxs.z) / 2)); break;
					case 2: vPoints.push_back(Vec3(0, 0, vMins.z + Vars::Aimbot::Projectile::VerticalShift.Value)); break;
					}
				}
			}
		}
	}

	PlayerStorage storage;
	F::MoveSim.Initialize(target.m_pEntity, storage);

	Vec3 vAngleTo;
	int i = 0, iLowestPriority = 3, iEndTick = 0; // time to point valid, end in n ticks
	for (;i < TIME_TO_TICKS(flMaxTime); i++)
	{
		if (!storage.m_bFailed)
		{
			F::MoveSim.RunTick(storage);
			vTargetPos = storage.m_MoveData.m_vecAbsOrigin;
		}

		// actually test points
		Solution_t solution;

		int iPriority = -1;
		for (const auto& vPoint : vPoints) // get most ideal point
		{
			iPriority++;
			if (iPriority >= iLowestPriority || !iLowestPriority)
				break;

			if (!CalculateAngle(vEyePos, vTargetPos + vOffset + vPoint, SolveProjectileSpeed(projInfo, pWeapon, vEyePos, vTargetPos), projInfo.m_gravity, solution))
				continue;

			if (!iEndTick)
			{
				if (solution.m_flTime + flLatency + latOff > TICKS_TO_TIME(i) || PrimeTime(pWeapon) + flLatency + latOff > TICKS_TO_TIME(i)) // TICKS_TO_TIME(flLatency)
					continue;
				iEndTick = i + vPoints.size() - 1;
			}

			Vec3 vAngles = Aim(G::CurrentUserCmd->viewangles, { -RAD2DEG(solution.m_flPitch), RAD2DEG(solution.m_flYaw), 0.0f });
			if (IsPipe(pWeapon)) // demoman offset fix
			{
				Vec3 forward = {}, up = {};
				Math::AngleVectors(Vec3(vAngles.x, vAngles.y, 0.f), &forward, nullptr, &up);

				Vec3 velocity = ((forward * projInfo.m_velocity) - (up * 200.f)), angle = {};
				Math::VectorAngles(velocity, angle);

				vAngles.x = angle.x;
			}

			if (TestAngle(pLocal, pWeapon, target, vOriginalPos, vTargetPos, vAngles, i - TIME_TO_TICKS(flLatency + phyOff)))
			{
				iLowestPriority = iPriority;
				vAngleTo = vAngles;
			}
		}

		if (iEndTick && iEndTick == i || !iLowestPriority)
			break;
	}

	G::MoveLines = storage.PredictionLines;

	F::MoveSim.Restore(storage);

	if (iLowestPriority != 3 &&
		(target.m_TargetType == ETargetType::PLAYER ? !storage.m_bFailed : true)) // don't attempt to aim at players when movesim fails
	{
		target.m_vAngleTo = vAngleTo;
		target.m_vPos = vTargetPos;
		if (Vars::Aimbot::Global::ShowHitboxes.Value)
		{
			F::Visuals.DrawHitbox(target.m_pEntity, vTargetPos, I::GlobalVars->curtime + (Vars::Visuals::TimedLines.Value ? TICKS_TO_TIME(i) : 5.f));

			if (target.m_nAimedHitbox == HITBOX_HEAD) // huntsman head
			{
				const Vec3 vOriginOffset = target.m_pEntity->m_vecOrigin() - vTargetPos;

				const model_t* pModel = target.m_pEntity->GetModel();
				if (!pModel) return true;
				const studiohdr_t* pHDR = I::ModelInfoClient->GetStudioModel(pModel);
				if (!pHDR) return true;
				const mstudiohitboxset_t* pSet = pHDR->GetHitboxSet(target.m_pEntity->m_nHitboxSet());
				if (!pSet) return true;

				matrix3x4 BoneMatrix[128];
				if (!target.m_pEntity->SetupBones(BoneMatrix, 128, BONE_USED_BY_ANYTHING, target.m_pEntity->m_flSimulationTime()))
					return true;

				const mstudiobbox_t* bbox = pSet->hitbox(HITBOX_HEAD);
				if (!bbox)
					return true;

				matrix3x4 rotMatrix;
				Math::AngleMatrix(bbox->angle, rotMatrix);

				matrix3x4 matrix;
				Math::ConcatTransforms(BoneMatrix[bbox->bone], rotMatrix, matrix);

				Vec3 bboxAngle;
				Math::MatrixAngles(matrix, bboxAngle);

				Vec3 matrixOrigin;
				Math::GetMatrixOrigin(matrix, matrixOrigin);

				G::BoxesStorage.push_back({ matrixOrigin - vOriginOffset, bbox->bbmin, bbox->bbmax, bboxAngle, I::GlobalVars->curtime + (Vars::Visuals::TimedLines.Value ? TICKS_TO_TIME(i) : 5.f), Vars::Colors::HitboxEdge.Value, Vars::Colors::HitboxFace.Value });
			}
		}
		return true;
	}

	return false;
}



// assume angle calculated outside with other overload
void CAimbotProjectile::Aim(CUserCmd* pCmd, Vec3& vAngle)
{
	if (Vars::Aimbot::Projectile::AimMethod.Value != 2)
	{
		pCmd->viewangles = vAngle;
		I::EngineClient->SetViewAngles(pCmd->viewangles);
	}
	else if (G::IsAttacking)
	{
		Utils::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
		//if (bFlameThrower)
		//	G::UpdateView = false;
		//else
			G::SilentTime = true;
	}
}

Vec3 CAimbotProjectile::Aim(Vec3 vCurAngle, Vec3 vToAngle)
{
	Vec3 vReturn = {};

	vToAngle -= G::PunchAngles;
	Math::ClampAngles(vToAngle);

	switch (Vars::Aimbot::Projectile::AimMethod.Value)
	{
	case 0: // Plain
		vReturn = vToAngle;
		break;

	case 1: //Smooth
		if (Vars::Aimbot::Projectile::SmoothingAmount.Value == 0)
		{ // plain aim at 0 smoothing factor
			vReturn = vToAngle;
			break;
		}
		//a + (b - a) * t [lerp]
		vReturn = vCurAngle + (vToAngle - vCurAngle) * (1.f - (float)Vars::Aimbot::Projectile::SmoothingAmount.Value / 100.f);
		break;

	case 2: // Silent
		vReturn = vToAngle;
		break;

	default: break;
	}

	return vReturn;
}

bool CAimbotProjectile::RunMain(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	const int nWeaponID = pWeapon->GetWeaponID();
	const bool bAutomatic = pWeapon->IsStreamingWeapon(), bKeepFiring = bAutomatic && G::LastUserCmd->buttons & IN_ATTACK;

	if (bKeepFiring && !G::WeaponCanAttack && F::AimbotGlobal.IsKeyDown())
		pCmd->buttons |= IN_ATTACK;

	if (!Vars::Aimbot::Global::Active.Value || !Vars::Aimbot::Projectile::Active.Value ||
		!G::WeaponCanAttack && Vars::Aimbot::Projectile::AimMethod.Value == 2 /*&& !G::DoubleTap*/ && nWeaponID != TF_WEAPON_PIPEBOMBLAUNCHER && nWeaponID != TF_WEAPON_CANNON)
		return true;

	const bool bShouldAim = Vars::Aimbot::Global::AimKey.Value == VK_LBUTTON ? (pCmd->buttons & IN_ATTACK) : F::AimbotGlobal.IsKeyDown();
	if (!bShouldAim &&
		(Vars::Aimbot::Global::AutoShoot.Value
		? true
		: !bLastTickHeld))
	{
		return true;
	}

	auto targets = SortTargets(pLocal, pWeapon);
	if (targets.empty())
		return true;

	if (bShouldAim && (nWeaponID == TF_WEAPON_COMPOUND_BOW ||
		nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER || nWeaponID == TF_WEAPON_CANNON))
	{
		pCmd->buttons |= IN_ATTACK;
		if (!G::WeaponCanAttack)
			return true;
	}

	for (auto& target : targets)
	{
		if (SandvichAimbot::bIsSandvich)
		{
			SandvichAimbot::RunSandvichAimbot(pLocal, pWeapon, pCmd, target.m_pEntity);
			return false;
		}

		if (!CanHit(target, pLocal, pWeapon)) continue;

		G::CurrentTargetIdx = target.m_pEntity->GetIndex();
		if (Vars::Aimbot::Projectile::AimMethod.Value == 2)
			G::AimPos = target.m_vPos;

		if (Vars::Aimbot::Global::AutoShoot.Value)
		{
			pCmd->buttons |= IN_ATTACK;

			if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
			{
				if (pWeapon->GetClip1() > 0)
					pCmd->buttons &= ~IN_ATTACK;
			}
			else
			{
				if ((nWeaponID == TF_WEAPON_COMPOUND_BOW || nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER)
					&& pWeapon->GetChargeBeginTime() > 0.0f)
				{
					pCmd->buttons &= ~IN_ATTACK;
				}
				else if (nWeaponID == TF_WEAPON_CANNON && pWeapon->GetDetonateTime() > 0.0f)
				{
					const Vec3 vEyePos = pLocal->GetShootPos();
					const float bestCharge = vEyePos.DistTo(target.m_vPos) / 1454.0f;

					if (Vars::Aimbot::Projectile::ChargeLooseCannon.Value)
					{
						if (target.m_TargetType == ETargetType::SENTRY ||
							target.m_TargetType == ETargetType::DISPENSER ||
							target.m_TargetType == ETargetType::TELEPORTER ||
							target.m_TargetType == ETargetType::BOMBS) // please DONT double donk buildings or bombs
						{
							pCmd->buttons &= ~IN_ATTACK;
						}

						if (target.m_pEntity->m_iHealth() > 50) // check if we even need to double donk to kill first
						{
							if (pWeapon->GetDetonateTime() - I::GlobalVars->curtime <= bestCharge)
								pCmd->buttons &= ~IN_ATTACK;
						}
						else
							pCmd->buttons &= ~IN_ATTACK;
					}
					else
						pCmd->buttons &= ~IN_ATTACK;
				}
			}
		}

		G::IsAttacking = Utils::IsAttacking(pCmd, pWeapon);

		if ((G::IsAttacking || !Vars::Aimbot::Global::AutoShoot.Value) && Vars::Visuals::SimLines.Value)
		{
			F::Visuals.ClearBulletLines();
			G::LinesStorage.clear();

			G::LinesStorage.push_back({ G::MoveLines, Vars::Visuals::TimedLines.Value ? -1.f : I::GlobalVars->curtime + 5.f, Vars::Colors::PredictionColor.Value });
			if (G::IsAttacking)
				G::LinesStorage.push_back({ G::ProjLines, Vars::Visuals::TimedLines.Value ? -1.f - F::Backtrack.GetReal() : I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value });
		}

		Aim(pCmd, target.m_vAngleTo);

		break;
	}

	return false;
}

void CAimbotProjectile::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	const bool bEarly = RunMain(pLocal, pWeapon, pCmd);
	const bool bHeld = Vars::Aimbot::Global::AimKey.Value == VK_LBUTTON ? (pCmd->buttons & IN_ATTACK) : F::AimbotGlobal.IsKeyDown();

	const float charge = pWeapon->GetChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->GetChargeBeginTime() : 0.f;
	const float amount = Math::RemapValClamped(charge, 0.f, Utils::ATTRIB_HOOK_FLOAT(4.0f, "stickybomb_charge_rate", pWeapon), 0.f, 1.f);

	const bool bAutoRelease = Vars::Aimbot::Projectile::AutoRelease.Value && amount > Vars::Aimbot::Projectile::AutoReleaseAt.Value;
	const bool bCancel = amount > 0.95f && pWeapon->GetWeaponID() != TF_WEAPON_COMPOUND_BOW;

	// add user toggle to control whether to cancel or not
	if ((bCancel || bEarly && (!(G::Buttons & IN_ATTACK) || bAutoRelease)) && G::LastUserCmd->buttons & IN_ATTACK && bLastTickHeld)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_COMPOUND_BOW:
			pCmd->buttons |= IN_ATTACK2;
			pCmd->buttons &= ~IN_ATTACK;
			break;
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_CANNON:
			pCmd->weaponselect = pLocal->GetWeaponFromSlot(SLOT_MELEE)->GetIndex();
			bLastTickCancel = true;
		}
	}
	else if (bAutoRelease && pWeapon->GetWeaponID() != TF_WEAPON_COMPOUND_BOW &&
		!pWeapon->IsInReload() && !bLastTickReload && G::LastUserCmd->buttons & IN_ATTACK && !bLastTickHeld)
	{
		pCmd->buttons &= ~IN_ATTACK;
	}

	bLastTickHeld = bHeld && !bEarly, bLastTickReload = pWeapon->IsInReload();
}