#include "MovementSimulation.h"
#pragma warning (disable : 4018)
#pragma warning (disable : 4552)

//we'll use this to set current player's command, without it CGameMovement::CheckInterval will try to access a nullptr
static CUserCmd DummyCmd = {};

void CMovementSimulation::FillVelocities()
{
	if (Vars::Aimbot::Projectile::StrafePredictionGround.Value || Vars::Aimbot::Projectile::StrafePredictionAir.Value)
	{
		auto FillVelocity = [](CBaseEntity* pEntity, std::map<int, std::deque<VelocityData>>& m_Velocities)
		{
			const int iEntIndex = pEntity->GetIndex();
			if (!pEntity->IsAlive() || pEntity->GetDormant() || pEntity->GetVelocity().IsZero())
			{
				m_Velocities[iEntIndex].clear();
				return;
			}

			const VelocityData vRecord = {
				pEntity->GetVelocity(),
				pEntity->GetSimulationTime(),
				bool(pEntity->GetFlags() & FL_ONGROUND)
			};

			if (m_Velocities[iEntIndex].size() > 0)
			{
				const VelocityData vLast = m_Velocities[iEntIndex][0];
				if (vRecord.m_flSimTime <= vLast.m_flSimTime)
					return;

				const float flLastYaw = Math::VelocityToAngles(vRecord.m_vecVelocity).y;
				const float flYaw = Math::VelocityToAngles(vLast.m_vecVelocity).y;
				const float flDelta = RAD2DEG(Math::AngleDiffRad(DEG2RAD(flLastYaw), DEG2RAD(flYaw)));
				if (fabsf(flDelta) > 10.f)
					m_Velocities[iEntIndex].clear();

				if (vRecord.m_bGrounded != vLast.m_bGrounded)
				{
					m_Velocities[iEntIndex].clear();
					for (int i = 0; i < 2; i++)
						m_Velocities[iEntIndex].push_front(vRecord);
				}
			}

			m_Velocities[iEntIndex].push_front(vRecord);

			if (m_Velocities[iEntIndex].size() > 66)
				m_Velocities[iEntIndex].pop_back();
		};

		const auto pLocal = g_EntityCache.GetLocal();
		if (pLocal)
		{
			for (const auto& pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
			{
				if (pEntity != pLocal) // only appears when in thirdperson ?
					FillVelocity(pEntity, m_Velocities);
			}
			FillVelocity(pLocal, m_Velocities);
		}
	}
	else
	{
		m_Velocities.clear();
	}
}

//const bool shouldPredict = playerStorage.m_pPlayer->OnSolid() ? Vars::Aimbot::Projectile::StrafePredictionGround.Value : Vars::Aimbot::Projectile::StrafePredictionAir.Value;
bool CMovementSimulation::Initialize(CBaseEntity* pPlayer, PlayerStorage& playerStorageOut, bool cancelStrafe)
{
	if (!pPlayer || !pPlayer->IsPlayer() || pPlayer->deadflag())
		return false;

	G::MoveLines.clear();

	//set player
	playerStorageOut.m_pPlayer = pPlayer;

	//set current command
	playerStorageOut.m_pPlayer->SetCurrentCmd(&DummyCmd);

	//store player's data
	playerStorageOut.m_PlayerData.m_vecOrigin = pPlayer->GetAbsOrigin();
	playerStorageOut.m_PlayerData.m_vecVelocity = pPlayer->m_vecVelocity();
	playerStorageOut.m_PlayerData.m_vecBaseVelocity = pPlayer->m_vecBaseVelocity();
	playerStorageOut.m_PlayerData.m_vecViewOffset = pPlayer->m_vecViewOffset();
	playerStorageOut.m_PlayerData.m_hGroundEntity = pPlayer->m_hGroundEntity();
	playerStorageOut.m_PlayerData.m_vecWorldSpaceCenter = pPlayer->GetWorldSpaceCenter();
	playerStorageOut.m_PlayerData.m_fFlags = pPlayer->m_fFlags();
	playerStorageOut.m_PlayerData.m_flDucktime = pPlayer->m_flDucktime();
	playerStorageOut.m_PlayerData.m_flDuckJumpTime = pPlayer->m_flDuckJumpTime();
	playerStorageOut.m_PlayerData.m_bDucked = pPlayer->m_bDucked();
	playerStorageOut.m_PlayerData.m_bDucking = pPlayer->m_bDucking();
	playerStorageOut.m_PlayerData.m_bInDuckJump = pPlayer->m_bInDuckJump();
	playerStorageOut.m_PlayerData.m_flModelScale = pPlayer->m_flModelScale();

	//store vars
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_bOldFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	m_flOldFrametime = I::GlobalVars->frametime;
	bDontPredict = false;

	//the hacks that make it work
	{
		if (pPlayer->m_fFlags() & FL_DUCKING)
		{
			pPlayer->m_fFlags() &= ~FL_DUCKING; //breaks origin's z if FL_DUCKING is not removed
			pPlayer->m_bDucked() = true; //(mins/maxs will be fine when ducking as long as m_bDucked is true)
			pPlayer->m_flDucktime() = 0.0f;
			pPlayer->m_flDuckJumpTime() = 0.0f;
			pPlayer->m_bDucking() = false;
			pPlayer->m_bInDuckJump() = false;
		}

		//if (pPlayer != g_EntityCache.GetLocal())
		//{
		//	pPlayer->m_hGroundEntity() = -1; //this is bs you don't need this
		//} 

		pPlayer->m_flModelScale() -= 0.03125f; //fixes issues with corners

		if (pPlayer->m_fFlags() & FL_ONGROUND)
			pPlayer->SetAbsOrigin(pPlayer->GetAbsOrigin() + Vector(0, 0, 0.03125f)); //to prevent getting stuck in the ground
		else
			pPlayer->m_hGroundEntity() = -1; // fix for velocity being set to 0 even if in air

		//for some reason if xy vel is zero it doesn't predict
		if (fabsf(pPlayer->m_vecVelocity().x) < 0.01f)
		{
			pPlayer->m_vecVelocity().x = 0.015f;
		}

		if (fabsf(pPlayer->m_vecVelocity().y) < 0.01f)
		{
			pPlayer->m_vecVelocity().y = 0.015f;
		}
	}

	// setup move data
	SetupMoveData(playerStorageOut);

	// calculate strafe if desired
	if (!cancelStrafe)
		StrafePrediction(playerStorageOut);

	return true;
}

void CMovementSimulation::SetupMoveData(PlayerStorage& playerStorage)
{
	if (!playerStorage.m_pPlayer)
		return;

	playerStorage.m_MoveData.m_bFirstRunOfFunctions = false;
	playerStorage.m_MoveData.m_bGameCodeMovedPlayer = false;
	playerStorage.m_MoveData.m_nPlayerHandle = reinterpret_cast<IHandleEntity*>(playerStorage.m_pPlayer)->GetRefEHandle();
	playerStorage.m_MoveData.m_vecVelocity = playerStorage.m_pPlayer->m_vecVelocity();	//	m_vecBaseVelocity hits -1950?
	if (playerStorage.m_pPlayer->m_fFlags() & FL_ONGROUND && !playerStorage.m_MoveData.m_vecVelocity.IsZero())
		playerStorage.m_MoveData.m_vecVelocity.z = 0.f; // step fix
	playerStorage.m_MoveData.m_vecAbsOrigin = playerStorage.m_pPlayer->GetAbsOrigin();
	playerStorage.m_MoveData.m_flMaxSpeed = playerStorage.m_pPlayer->TeamFortress_CalculateMaxSpeed();

	if (playerStorage.m_PlayerData.m_fFlags & FL_DUCKING)
	{
		playerStorage.m_MoveData.m_flMaxSpeed *= 0.3333f;
	}

	playerStorage.m_MoveData.m_flClientMaxSpeed = playerStorage.m_MoveData.m_flMaxSpeed;

	//need a better way to determine angles probably
	playerStorage.m_MoveData.m_vecViewAngles = { 0.0f, Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity).y, 0.0f };

	Vec3 vForward = {}, vRight = {};
	Math::AngleVectors(playerStorage.m_MoveData.m_vecViewAngles, &vForward, &vRight, nullptr);

	playerStorage.m_MoveData.m_flForwardMove = (playerStorage.m_MoveData.m_vecVelocity.y - vRight.y / vRight.x * playerStorage.m_MoveData.m_vecVelocity.x) / (vForward.y - vRight.y / vRight.x * vForward.x);
	playerStorage.m_MoveData.m_flSideMove = (playerStorage.m_MoveData.m_vecVelocity.x - vForward.x * playerStorage.m_MoveData.m_flForwardMove) / vRight.x;
}

void CMovementSimulation::StrafePrediction(PlayerStorage& playerStorage)
{
	if (!(playerStorage.m_pPlayer->OnSolid() ? Vars::Aimbot::Projectile::StrafePredictionGround.Value : Vars::Aimbot::Projectile::StrafePredictionAir.Value)) 
		return;

	const int iEntIndex = playerStorage.m_pPlayer->GetIndex();

	const auto& mVelocityRecords = m_Velocities[iEntIndex];

	if (mVelocityRecords.size() < 4)
		return;
	const int iSamples = fmin(3, mVelocityRecords.size());
	if (!iSamples)
		return;

	float flAverageYaw = 0.f;
	float flCompareYaw = playerStorage.m_MoveData.m_vecViewAngles.y; //Math::VelocityToAngles(m_MoveData.m_vecVelocity).y;
	for (int i = 0; i < iSamples; i++)
	{
		const float flRecordYaw = Math::VelocityToAngles(mVelocityRecords.at(i).m_vecVelocity).y;

		const float flDelta = RAD2DEG(Math::AngleDiffRad(DEG2RAD(flCompareYaw), DEG2RAD(flRecordYaw)));
		flAverageYaw += flDelta;

		flCompareYaw = flRecordYaw;
	}
	flAverageYaw /= iSamples;

	//while (flAverageYaw > 360.f) { flAverageYaw -= 360.f; }
	//while (flAverageYaw < -360.f) { flAverageYaw += 360.f; } just no
		fmod(flAverageYaw + 180.0f, 360.0f) - 180.0f;

	if (fabsf(flAverageYaw) < Vars::Aimbot::Projectile::StrafePredictionMinDifference.Value)
	{
		flAverageYaw = 0;
		return;
	}

	auto get_velocity_degree = [](float velocity)
	{
		auto tmp = RAD2DEG(atan(30.0f / velocity));

		#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)
		if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
			return 90.0f;

		else if (tmp < 0.0f)
			return 0.0f;
		else
			return tmp;
	};

	const float flMaxDelta = get_velocity_degree(fabsf(flAverageYaw)) / fmaxf((float)iSamples, 2.f);

	if (fabsf(flAverageYaw) > flMaxDelta) {
		m_Velocities[playerStorage.m_pPlayer->GetIndex()].clear();
		return;
	}	//	ugly fix for sweaty pricks

	if (Vars::Debug::DebugInfo.Value)
	{
		Utils::ConLog("MovementSimulation", tfm::format("flAverageYaw calculated to %f", flAverageYaw).c_str(), { 83, 255, 83, 255 });
	}

	if (flAverageYaw == 0.f)
		return;

	playerStorage.m_flAverageYaw = flAverageYaw;
}

void CMovementSimulation::RunTick(PlayerStorage& playerStorage)
{
	if (!playerStorage.m_pPlayer || bDontPredict)
	{
		return;
	}

	if (playerStorage.m_pPlayer->GetClassID() != ETFClassID::CTFPlayer)
	{
		return;
	}

	playerStorage.PredictionLines.push_back({ playerStorage.m_MoveData.m_vecAbsOrigin, Math::GetRotatedPosition( playerStorage.m_MoveData.m_vecAbsOrigin, Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value ) });

	//make sure frametime and prediction vars are right
	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.0f : TICK_INTERVAL;
	
	float airCorrection = 0.f;
	if (playerStorage.m_flAverageYaw)
	{
		float flYaw = playerStorage.m_flAverageYaw;
		if (!playerStorage.m_pPlayer->OnSolid() && playerStorage.m_flAverageYaw)
			airCorrection += 90.f * (playerStorage.m_flAverageYaw > 0.f ? 1.f : -1.f);
		else
			flYaw *= 1.5f; // dumb fix (?), ground strafing seems a bit slow

		playerStorage.m_MoveData.m_vecViewAngles.y += flYaw + airCorrection;
		//playerStorage.m_MoveData.m_vecViewAngles = { 0.0f, Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity).y, 0.0f };
	}

	I::TFGameMovement->ProcessMovement(playerStorage.m_pPlayer, &playerStorage.m_MoveData);

	playerStorage.m_MoveData.m_vecViewAngles.y -= airCorrection;
}

void CMovementSimulation::Restore(PlayerStorage& playerStorage)
{
	if (!playerStorage.m_pPlayer)
		return;

	playerStorage.m_pPlayer->SetCurrentCmd(nullptr);

	playerStorage.m_pPlayer->SetAbsOrigin(playerStorage.m_PlayerData.m_vecOrigin);
	playerStorage.m_pPlayer->m_vecVelocity() = playerStorage.m_PlayerData.m_vecVelocity;
	playerStorage.m_pPlayer->m_vecBaseVelocity() = playerStorage.m_PlayerData.m_vecBaseVelocity;
	playerStorage.m_pPlayer->m_vecViewOffset() = playerStorage.m_PlayerData.m_vecViewOffset;
	playerStorage.m_pPlayer->m_hGroundEntity() = playerStorage.m_PlayerData.m_hGroundEntity;
	playerStorage.m_pPlayer->m_fFlags() = playerStorage.m_PlayerData.m_fFlags;
	playerStorage.m_pPlayer->m_flDucktime() = playerStorage.m_PlayerData.m_flDucktime;
	playerStorage.m_pPlayer->m_flDuckJumpTime() = playerStorage.m_PlayerData.m_flDuckJumpTime;
	playerStorage.m_pPlayer->m_bDucked() = playerStorage.m_PlayerData.m_bDucked;
	playerStorage.m_pPlayer->m_bDucking() = playerStorage.m_PlayerData.m_bDucking;
	playerStorage.m_pPlayer->m_bInDuckJump() = playerStorage.m_PlayerData.m_bInDuckJump;
	playerStorage.m_pPlayer->m_flModelScale() = playerStorage.m_PlayerData.m_flModelScale;

	I::Prediction->m_bInPrediction = m_bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
	I::GlobalVars->frametime = m_flOldFrametime;

	memset(&playerStorage, 0, sizeof(PlayerStorage));
}