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
	if (Vars::Aimbot::Global::AimAt.Value & ToAimAt::PLAYER)
	{
		EGroupType groupType = EGroupType::PLAYERS_ENEMIES;
		if (pWeapon->GetWeaponID() == TF_WEAPON_CROSSBOW)
			groupType = EGroupType::PLAYERS_ALL;
		else if (F::AimbotGlobal.IsSandvich())
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

			if (F::AimbotGlobal.ShouldIgnore(pTarget))
				continue;

			Vec3 vPos = pTarget->GetWorldSpaceCenter();
			Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vPos);
			const float flFOVTo = Math::CalcFov(vLocalAngles, vAngleTo);

			if (flFOVTo > Vars::Aimbot::Projectile::AimFOV.Value)
				continue;

			const float flDistTo = (sortMethod == ESortMethod::DISTANCE) ? vLocalPos.DistTo(vPos) : 0.0f;
			const int priority = F::AimbotGlobal.GetPriority(pTarget->GetIndex());
			validTargets.push_back({ pTarget, ETargetType::PLAYER, vPos, vAngleTo, flFOVTo, flDistTo, priority });
		}
	}

	// Buildings
	const bool bIsRescueRanger = pWeapon->GetWeaponID() == TF_WEAPON_SHOTGUN_BUILDING_RESCUE;

	for (const auto& pBuilding : g_EntityCache.GetGroup(bIsRescueRanger ? EGroupType::BUILDINGS_ALL : EGroupType::BUILDINGS_ENEMIES))
	{
		if (!pBuilding->IsAlive())
			continue;

		bool isSentry = pBuilding->IsSentrygun(), isDispenser = pBuilding->IsDispenser(), isTeleporter = pBuilding->IsTeleporter();

		if (!(Vars::Aimbot::Global::AimAt.Value & ToAimAt::SENTRY) && isSentry)
			continue;
		if (!(Vars::Aimbot::Global::AimAt.Value & ToAimAt::DISPENSER) && isDispenser)
			continue;
		if (!(Vars::Aimbot::Global::AimAt.Value & ToAimAt::TELEPORTER) && isTeleporter)
			continue;

		// Check if the Rescue Ranger should shoot at friendly buildings
		if (bIsRescueRanger && (pBuilding->m_iTeamNum() == pLocal->m_iTeamNum()))
		{
			if (pBuilding->m_iBOHealth() >= pBuilding->m_iMaxHealth())
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
	if (Vars::Aimbot::Global::AimAt.Value & ToAimAt::NPC)
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
	if (Vars::Aimbot::Global::AimAt.Value & ToAimAt::BOMB)
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
	F::AimbotGlobal.SortPriority(&validTargets);

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
	switch (pWeapon->m_iItemDefinitionIndex())
	{
	case Demoman_s_StickybombLauncher:
	case Demoman_s_StickybombLauncherR:
	case Demoman_s_FestiveStickybombLauncher: return 0.8f;
	case Demoman_s_TheQuickiebombLauncher: return 0.6f;
	case Demoman_s_TheScottishResistance: return 1.6f;
	}

	return 0.f;
}

int CAimbotProjectile::GetHitboxPriority(int nHitbox, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target)
{
	bool bHeadshot = target.m_TargetType == ETargetType::PLAYER && pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW;
	if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 5) && bHeadshot) // lazy use of hitscan modifier
	{
		float charge = I::GlobalVars->curtime - pWeapon->m_flChargeBeginTime();
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

void SolveProjectileSpeed(CBaseCombatWeapon* pWeapon, const Vec3& vLocalPos, const Vec3& vTargetPos, float& flVelocity, float& flDragTime, const float flGravity)
{
	const float flGrav = flGravity * 800.0f;
	if (!IsPipe(pWeapon))
		return;

	const Vec3 vDelta = vTargetPos - vLocalPos;
	const float flDist = vDelta.Length2D();

	const float flRoot = pow(flVelocity, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flVelocity, 2));
	if (flRoot < 0.f)
		return;

	const float flPitch = atan((pow(flVelocity, 2) - sqrt(flRoot)) / (flGrav * flDist));
	const float flTime = flDist / (cos(flPitch) * flVelocity);

	float flDrag = 0.f;
	if (Vars::Aimbot::Projectile::DragOverride.Value)
		flDrag = Vars::Aimbot::Projectile::DragOverride.Value;
	else
	{
		switch (pWeapon->m_iItemDefinitionIndex()) // the remaps are dumb but they work so /shrug
		{
		case Demoman_m_GrenadeLauncher:
		case Demoman_m_GrenadeLauncherR:
		case Demoman_m_FestiveGrenadeLauncher:
		case Demoman_m_Autumn:
		case Demoman_m_MacabreWeb:
		case Demoman_m_Rainbow:
		case Demoman_m_SweetDreams:
		case Demoman_m_CoffinNail:
		case Demoman_m_TopShelf:
		case Demoman_m_Warhawk:
		case Demoman_m_ButcherBird:
		case Demoman_m_TheIronBomber: flDrag = Math::RemapValClamped(flVelocity, 1217.f, k_flMaxVelocity, 0.115f, 0.195f); break; // 0.115 normal, 0.195 capped, 0.290 v3000
		case Demoman_m_TheLochnLoad: flDrag = Math::RemapValClamped(flVelocity, 1504.f, k_flMaxVelocity, 0.065f, 0.090f); break; // 0.065 normal, 0.090 capped, 0.130 v3000
		case Demoman_m_TheLooseCannon: flDrag = Math::RemapValClamped(flVelocity, 1468.f, k_flMaxVelocity, 0.370f, 0.530f); break; // 0.370 normal, 0.530 capped, 0.800 v3000
		case Demoman_s_StickybombLauncher:
		case Demoman_s_StickybombLauncherR:
		case Demoman_s_FestiveStickybombLauncher:
		case Demoman_s_TheQuickiebombLauncher:
		case Demoman_s_TheScottishResistance: flDrag = Math::RemapValClamped(flVelocity, 922.f, k_flMaxVelocity, 0.080f, 0.190f); break; // 0.080 low, 0.190 capped, 0.225 v2400
		}
	}

	flDragTime = powf(flTime, 2) * flDrag / 2; // rough estimate to prevent m_flTime being too low
	flVelocity = flVelocity - flVelocity * flTime * flDrag;

	if (Vars::Aimbot::Projectile::TimeOverride.Value)
		flDragTime = Vars::Aimbot::Projectile::TimeOverride.Value;
}

bool CAimbotProjectile::CalculateAngle(const Vec3& vLocalPos, const Vec3& vTargetPos, const float flVelocity, const float flGravity, CBaseCombatWeapon* pWeapon, Solution_t& out)
{
	float flNewVel = flVelocity, flDragTime = 0.f;
	SolveProjectileSpeed(pWeapon, vLocalPos, vTargetPos, flNewVel, flDragTime, flGravity);

	const Vec3 vDelta = vTargetPos - vLocalPos;
	const float flDist = vDelta.Length2D();
	const float flGrav = flGravity * 800.f; //g_ConVars.sv_gravity->GetFloat()

	if (!flGrav)
	{
		const Vec3 vAngleTo = Math::CalcAngle(vLocalPos, vTargetPos);
		out.m_flPitch = -DEG2RAD(vAngleTo.x);
		out.m_flYaw = DEG2RAD(vAngleTo.y);
	}
	else
	{ // arch
		const float flRoot = pow(flNewVel, 4) - flGrav * (flGrav * pow(flDist, 2) + 2.f * vDelta.z * pow(flNewVel, 2));
		if (flRoot < 0.f)
			return false;
		out.m_flPitch = atan((pow(flNewVel, 2) - sqrt(flRoot)) / (flGrav * flDist));
		out.m_flYaw = atan2(vDelta.y, vDelta.x);
	}
	out.m_flTime = flDist / (cos(out.m_flPitch) * flNewVel) + flDragTime;

	return true;
}

bool CAimbotProjectile::TestAngle(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, Target_t& target, const Vec3& vOriginal, const Vec3& vPredict, const Vec3& vAngles, const int& iSimTime)
{
	target.m_pEntity->SetAbsOrigin(vOriginal);

	ProjectileInfo projInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, vAngles, projInfo) || !F::ProjSim.Initialize(projInfo))
		return false;

	target.m_pEntity->SetAbsOrigin(vPredict);

	if (Vars::Aimbot::Projectile::AimMethod.Value != 1)
		projInfo.m_hull += Vec3(Vars::Aimbot::Projectile::HullInc.Value, Vars::Aimbot::Projectile::HullInc.Value, Vars::Aimbot::Projectile::HullInc.Value);

	for (int n = 0; n < iSimTime; n++)
	{
		Vec3 Old = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(projInfo);
		Vec3 New = F::ProjSim.GetOrigin();

		CGameTrace trace = {};
		CTraceFilterProjectile filter = {};
		filter.pSkip = pLocal;
		Utils::TraceHull(Old, New, projInfo.m_hull * -1, projInfo.m_hull, MASK_SOLID, &filter, &trace);

		if (trace.DidHit()) // mayb exclude time check for stickies if they hit ground instead
		{
			target.m_pEntity->SetAbsOrigin(vOriginal);

			if (trace.entity == target.m_pEntity && (iSimTime - n < 5 || PrimeTime(pWeapon) > TICKS_TO_TIME(n)))
			{
				G::ProjLines = projInfo.PredictionLines;
				G::ProjLines.push_back({ trace.vEndPos, Math::GetRotatedPosition(trace.vEndPos, Math::VelocityToAngles(F::ProjSim.GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value) });

				// attempted to have a headshot check though this seems more detrimental than useful outside of smooth aimbot
				if (Vars::Aimbot::Projectile::AimMethod.Value == 1 && target.m_nAimedHitbox == HITBOX_HEAD)
				{	// i think this is accurate ?
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

int CAimbotProjectile::CanHit(Target_t& target, CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, float* flTimeTo, std::vector<DrawBox>* bBoxes)
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
	Vec3 vTargetPos = vOriginalPos;
	Vec3 vModVelocity; float flModVelocity, flMaxTime = Vars::Aimbot::Projectile::PredictionTime.Value;
	{
		if (!F::ProjSim.GetInfo(pLocal, pWeapon, {}, projInfo) || !F::ProjSim.Initialize(projInfo))
			return false;

		flMaxTime = std::min(projInfo.m_lifetime, Vars::Aimbot::Projectile::PredictionTime.Value);

		vModVelocity = F::ProjSim.GetVelocity(); flModVelocity = vModVelocity.Length(); // account for up velocity & capped velocity
	}
	//const int iLatency = TIME_TO_TICKS(F::Backtrack.GetReal());
	//const float flLatency = F::Backtrack.GetReal()/*iNetChan->GetLatency(FLOW_OUTGOING)*/ + I::GlobalVars->interval_per_tick;
	const float flLatency = F::Backtrack.GetReal()/*iNetChan->GetLatency(FLOW_OUTGOING)*/;
	const float latOff = I::GlobalVars->interval_per_tick * (Vars::Aimbot::Projectile::LatOff.Value + G::AnticipatedChoke);
	const float phyOff = I::GlobalVars->interval_per_tick * (Vars::Aimbot::Projectile::PhyOff.Value + G::AnticipatedChoke);

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
			vTargetPos = storage.m_MoveData.m_vecOrigin;
		}

		// actually test points
		Solution_t solution;

		int iPriority = -1;
		for (const auto& vPoint : vPoints) // get most ideal point
		{
			iPriority++;
			if (iPriority >= iLowestPriority || !iLowestPriority)
				break;

			if (!CalculateAngle(vEyePos, vTargetPos + vPoint, flModVelocity, projInfo.m_gravity, pWeapon, solution))
				continue;

			if (!iEndTick)
			{
				if (solution.m_flTime + flLatency + latOff > TICKS_TO_TIME(i) || PrimeTime(pWeapon) + flLatency + latOff > TICKS_TO_TIME(i)) // TICKS_TO_TIME(flLatency)
					continue;
				iEndTick = i + vPoints.size() - 1;
			}

			Vec3 vAngles = Aim(G::CurrentUserCmd->viewangles, { -RAD2DEG(solution.m_flPitch), RAD2DEG(solution.m_flYaw), 0.f });
			{	// up velocity fix
				Vec3 vBadAngle = {};
				Math::VectorAngles(vModVelocity, vBadAngle);

				vAngles.x -= vBadAngle.x;
			}

			if (TestAngle(pLocal, pWeapon, target, vOriginalPos, vTargetPos, vAngles + G::PunchAngles, i - TIME_TO_TICKS(flLatency + phyOff)))
			{
				iLowestPriority = iPriority;
				vAngleTo = vAngles;
				*flTimeTo = solution.m_flTime + flLatency + latOff;
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
		if (Vars::Visuals::ShowHitboxes.Value)
		{
			bBoxes->push_back({ vTargetPos, target.m_pEntity->m_vecMins(), target.m_pEntity->m_vecMaxs(), Vec3(), I::GlobalVars->curtime + (Vars::Visuals::TimedLines.Value ? TICKS_TO_TIME(i) : 5.f), Vars::Colors::HitboxEdge.Value, Vars::Colors::HitboxFace.Value, true });

			if (target.m_nAimedHitbox == HITBOX_HEAD) // huntsman head
			{
				const Vec3 vOriginOffset = target.m_pEntity->GetAbsOrigin() - vTargetPos;

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

				bBoxes->push_back({ matrixOrigin - vOriginOffset, bbox->bbmin, bbox->bbmax, bboxAngle, I::GlobalVars->curtime + (Vars::Visuals::TimedLines.Value ? TICKS_TO_TIME(i) : 5.f), Vars::Colors::HitboxEdge.Value, Vars::Colors::HitboxFace.Value });
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
		//I::EngineClient->SetViewAngles(vAngle);
	}
	else if (G::IsAttacking)
	{
		Utils::FixMovement(pCmd, vAngle);
		pCmd->viewangles = vAngle;
		//if (bFlameThrower)
		//	G::SilentAngles = true;
		//else
			G::PSilentAngles = true;
	}
}

Vec3 CAimbotProjectile::Aim(Vec3 vCurAngle, Vec3 vToAngle, int iMethod)
{
	Vec3 vReturn = {};

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
		const float t = 1.f - (float)Vars::Aimbot::Projectile::SmoothingAmount.Value / 100.f;
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

bool CAimbotProjectile::RunMain(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	const int nWeaponID = pWeapon->GetWeaponID();
	const bool bAutomatic = pWeapon->IsStreamingWeapon(), bKeepFiring = bAutomatic && G::LastUserCmd->buttons & IN_ATTACK;

	if (bKeepFiring && !G::CanPrimaryAttack && F::AimbotGlobal.IsKeyDown())
		pCmd->buttons |= IN_ATTACK;

	if (!Vars::Aimbot::Global::Active.Value || !Vars::Aimbot::Projectile::Active.Value ||
		!G::CanPrimaryAttack && Vars::Aimbot::Projectile::AimMethod.Value == 2 /*&& !G::DoubleTap*/ && nWeaponID != TF_WEAPON_PIPEBOMBLAUNCHER && nWeaponID != TF_WEAPON_CANNON)
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
		nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER || nWeaponID == TF_WEAPON_CANNON && Vars::Aimbot::Projectile::ChargeLooseCannon.Value))
	{
		pCmd->buttons |= IN_ATTACK;
		if (!G::CanPrimaryAttack)
			return true;
	}

	for (auto& target : targets)
	{
		float flTimeTo = 0.f; std::vector<DrawBox> bBoxes = {};
		const int result = CanHit(target, pLocal, pWeapon, &flTimeTo, &bBoxes);
		if (!result) continue;

		G::CurrentTarget = { target.m_pEntity->GetIndex(), I::GlobalVars->tickcount };
		if (Vars::Aimbot::Projectile::AimMethod.Value == 2)
			G::AimPos = target.m_vPos;

		if (Vars::Aimbot::Global::AutoShoot.Value)
		{
			pCmd->buttons |= IN_ATTACK;

			if (G::CurItemDefIndex == Soldier_m_TheBeggarsBazooka)
			{
				if (pWeapon->m_iClip1() > 0)
					pCmd->buttons &= ~IN_ATTACK;
			}
			else
			{
				if ((nWeaponID == TF_WEAPON_COMPOUND_BOW || nWeaponID == TF_WEAPON_PIPEBOMBLAUNCHER) && pWeapon->m_flChargeBeginTime() > 0.f)
					pCmd->buttons &= ~IN_ATTACK;
				else if (nWeaponID == TF_WEAPON_CANNON && pWeapon->m_flDetonateTime() > 0.f)
				{
					if (Vars::Aimbot::Projectile::ChargeLooseCannon.Value)
					{
						if (target.m_TargetType == ETargetType::SENTRY ||
							target.m_TargetType == ETargetType::DISPENSER ||
							target.m_TargetType == ETargetType::TELEPORTER ||
							target.m_TargetType == ETargetType::BOMBS) // please DONT double donk buildings or bombs
						{
							pCmd->buttons &= ~IN_ATTACK;
						}

						if (target.m_pEntity->m_iHealth() <= 50) // check if we even need to double donk to kill first
							pCmd->buttons &= ~IN_ATTACK;

						float flDetonateTime = pWeapon->m_flDetonateTime();
						float flDetonateMaxTime = Utils::ATTRIB_HOOK_FLOAT(0.f, "grenade_launcher_mortar_mode", pWeapon);
						float flCharge = Math::RemapValClamped(flDetonateTime - I::GlobalVars->curtime, 0.f, flDetonateMaxTime, 0.f, 1.f);
						if (std::clamp(flCharge - 0.05f, 0.f, 1.f) < flTimeTo)
							pCmd->buttons &= ~IN_ATTACK;
					}
					else
						pCmd->buttons &= ~IN_ATTACK;
				}
			}
		}

		G::IsAttacking = Utils::IsAttacking(pCmd, pWeapon);

		if ((G::IsAttacking || !Vars::Aimbot::Global::AutoShoot.Value) && Vars::Visuals::SimLines.Value /*&& !pWeapon->IsInReload()*/)
		{
			F::Visuals.ClearBulletLines();
			G::LinesStorage.clear();

			G::LinesStorage.push_back({ G::MoveLines, Vars::Visuals::TimedLines.Value ? -int(G::MoveLines.size()) : I::GlobalVars->curtime + 5.f, Vars::Colors::PredictionColor.Value});
			if (G::IsAttacking)
				G::LinesStorage.push_back({ G::ProjLines, Vars::Visuals::TimedLines.Value ? -int(G::ProjLines.size()) - TIME_TO_TICKS(F::Backtrack.GetReal()) : I::GlobalVars->curtime + 5.f, Vars::Colors::ProjectileColor.Value });
			
			G::BoxesStorage.clear();
			for (auto& bBox : bBoxes)
				G::BoxesStorage.push_back(bBox);
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

	/*
	const float flChargeS = pWeapon->m_flChargeBeginTime() > 0.f ? I::GlobalVars->curtime - pWeapon->m_flChargeBeginTime() : 0.f;
	const float flChargeC = pWeapon->m_flDetonateTime() > 0.f ? pWeapon->m_flDetonateTime() - I::GlobalVars->curtime : 1.f;
	const float flAmount = pWeapon->GetWeaponID() != TF_WEAPON_CANNON
		? Math::RemapValClamped(flChargeS, 0.f, Utils::ATTRIB_HOOK_FLOAT(4.f, "stickybomb_charge_rate", pWeapon), 0.f, 1.f)
		: 1.f - Math::RemapValClamped(flChargeC, 0.f, Utils::ATTRIB_HOOK_FLOAT(0.f, "grenade_launcher_mortar_mode", pWeapon), 0.f, 1.f);
	*/
	float flAmount = 0.f;
	if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER || pWeapon->GetWeaponID() == TF_WEAPON_STICKY_BALL_LAUNCHER || pWeapon->GetWeaponID() == TF_WEAPON_GRENADE_STICKY_BALL
		|| pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
	{
		static float flBegin = -1.f;
		if (pCmd->buttons & IN_ATTACK && flBegin < 0.f && G::CanPrimaryAttack)
			flBegin = I::GlobalVars->curtime;

		const float flCharge = flBegin > 0.f ? I::GlobalVars->curtime - flBegin : 0.f;
		flAmount = pWeapon->GetWeaponID() != TF_WEAPON_CANNON
			? Math::RemapValClamped(flCharge, 0.f, Utils::ATTRIB_HOOK_FLOAT(4.f, "stickybomb_charge_rate", pWeapon), 0.f, 1.f)
			: Math::RemapValClamped(flCharge, 0.f, Utils::ATTRIB_HOOK_FLOAT(0.f, "grenade_launcher_mortar_mode", pWeapon), 0.f, 1.f);

		if (!G::CanPrimaryAttack)
			flBegin = -1.f;
	}

	const bool bAutoRelease = Vars::Aimbot::Projectile::AutoRelease.Value && flAmount > float(Vars::Aimbot::Projectile::AutoRelease.Value) / 100;
	const bool bCancel = flAmount > 0.95f;

	// add user toggle to control whether to cancel or not
	if ((bCancel || bEarly && (!(G::Buttons & IN_ATTACK) || bAutoRelease)) && G::LastUserCmd->buttons & IN_ATTACK && bLastTickHeld)
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_COMPOUND_BOW:
			pCmd->buttons |= IN_ATTACK2;
			pCmd->buttons &= ~IN_ATTACK;
			break;
		case TF_WEAPON_CANNON:
			if (auto pSwap = pLocal->GetWeaponFromSlot(SLOT_SECONDARY))
			{
				pCmd->weaponselect = pSwap->GetIndex();
				bLastTickCancel = pWeapon->GetIndex();
			}
			break;
		case TF_WEAPON_PIPEBOMBLAUNCHER:
			if (auto pSwap = pLocal->GetWeaponFromSlot(SLOT_PRIMARY))
			{
				pCmd->weaponselect = pSwap->GetIndex();
				bLastTickCancel = pWeapon->GetIndex();
			}
		}
	}
	else if (bAutoRelease && pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER &&
		!pWeapon->IsInReload() && !bLastTickReload && G::LastUserCmd->buttons & IN_ATTACK && !bLastTickHeld)
	{
		pCmd->buttons &= ~IN_ATTACK;
	}

	bLastTickHeld = bHeld && !bEarly, bLastTickReload = pWeapon->IsInReload();
}