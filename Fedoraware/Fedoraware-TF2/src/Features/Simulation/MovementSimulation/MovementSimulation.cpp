#include "MovementSimulation.h"

#include "../../Backtrack/Backtrack.h"

#pragma warning (disable : 4018)
#pragma warning (disable : 4552)

//we'll use this to set current player's command, without it CGameMovement::CheckInterval will try to access a nullptr
static CUserCmd DummyCmd = {};



void CMovementSimulation::Store(PlayerStorage& playerStorage)
{
	playerStorage.m_PlayerData.m_vecOrigin = playerStorage.m_pPlayer->m_vecOrigin();
	playerStorage.m_PlayerData.m_vecVelocity = playerStorage.m_pPlayer->m_vecVelocity();
	playerStorage.m_PlayerData.m_vecBaseVelocity = playerStorage.m_pPlayer->m_vecBaseVelocity();
	playerStorage.m_PlayerData.m_vecViewOffset = playerStorage.m_pPlayer->m_vecViewOffset();
	playerStorage.m_PlayerData.m_hGroundEntity = playerStorage.m_pPlayer->m_hGroundEntity();
	playerStorage.m_PlayerData.m_fFlags = playerStorage.m_pPlayer->m_fFlags();
	playerStorage.m_PlayerData.m_flDucktime = playerStorage.m_pPlayer->m_flDucktime();
	playerStorage.m_PlayerData.m_flDuckJumpTime = playerStorage.m_pPlayer->m_flDuckJumpTime();
	playerStorage.m_PlayerData.m_bDucked = playerStorage.m_pPlayer->m_bDucked();
	playerStorage.m_PlayerData.m_bDucking = playerStorage.m_pPlayer->m_bDucking();
	playerStorage.m_PlayerData.m_bInDuckJump = playerStorage.m_pPlayer->m_bInDuckJump();
	playerStorage.m_PlayerData.m_flModelScale = playerStorage.m_pPlayer->m_flModelScale();
	playerStorage.m_PlayerData.m_nButtons = playerStorage.m_pPlayer->m_nButtons();
	playerStorage.m_PlayerData.m_flLastMovementStunChange = playerStorage.m_pPlayer->m_flLastMovementStunChange();
	playerStorage.m_PlayerData.m_flStunLerpTarget = playerStorage.m_pPlayer->m_flStunLerpTarget();
	playerStorage.m_PlayerData.m_bStunNeedsFadeOut = playerStorage.m_pPlayer->m_bStunNeedsFadeOut();
	playerStorage.m_PlayerData.m_flPrevTauntYaw = playerStorage.m_pPlayer->m_flPrevTauntYaw();
	playerStorage.m_PlayerData.m_flTauntYaw = playerStorage.m_pPlayer->m_flTauntYaw();
	playerStorage.m_PlayerData.m_flCurrentTauntMoveSpeed = playerStorage.m_pPlayer->m_flCurrentTauntMoveSpeed();
	playerStorage.m_PlayerData.m_iKartState = playerStorage.m_pPlayer->m_iKartState();
	playerStorage.m_PlayerData.m_flVehicleReverseTime = playerStorage.m_pPlayer->m_flVehicleReverseTime();
	playerStorage.m_PlayerData.m_flHypeMeter = playerStorage.m_pPlayer->m_flHypeMeter();
	playerStorage.m_PlayerData.m_flMaxspeed = playerStorage.m_pPlayer->m_flMaxspeed();
	playerStorage.m_PlayerData.m_nAirDucked = playerStorage.m_pPlayer->m_nAirDucked();
	playerStorage.m_PlayerData.m_bJumping = playerStorage.m_pPlayer->m_bJumping();
	playerStorage.m_PlayerData.m_iAirDash = playerStorage.m_pPlayer->m_iAirDash();
	playerStorage.m_PlayerData.m_flWaterJumpTime = playerStorage.m_pPlayer->m_flWaterJumpTime();
	playerStorage.m_PlayerData.m_flSwimSoundTime = playerStorage.m_pPlayer->m_flSwimSoundTime();
	playerStorage.m_PlayerData.m_surfaceProps = playerStorage.m_pPlayer->m_surfaceProps();
	playerStorage.m_PlayerData.m_pSurfaceData = playerStorage.m_pPlayer->m_pSurfaceData();
	playerStorage.m_PlayerData.m_surfaceFriction = playerStorage.m_pPlayer->m_surfaceFriction();
	playerStorage.m_PlayerData.m_chTextureType = playerStorage.m_pPlayer->m_chTextureType();
	playerStorage.m_PlayerData.m_vecPunchAngle = playerStorage.m_pPlayer->m_vecPunchAngle();
	playerStorage.m_PlayerData.m_vecPunchAngleVel = playerStorage.m_pPlayer->m_vecPunchAngleVel();
	playerStorage.m_PlayerData.m_MoveType = playerStorage.m_pPlayer->m_MoveType();
	playerStorage.m_PlayerData.m_MoveCollide = playerStorage.m_pPlayer->m_MoveCollide();
	playerStorage.m_PlayerData.m_vecLadderNormal = playerStorage.m_pPlayer->m_vecLadderNormal();
	playerStorage.m_PlayerData.m_flGravity = playerStorage.m_pPlayer->m_flGravity();
	playerStorage.m_PlayerData.m_nWaterLevel = playerStorage.m_pPlayer->m_nWaterLevel();
	playerStorage.m_PlayerData.m_nWaterType = playerStorage.m_pPlayer->m_nWaterType();
	playerStorage.m_PlayerData.m_flFallVelocity = playerStorage.m_pPlayer->m_flFallVelocity();
	playerStorage.m_PlayerData.m_nPlayerCond = playerStorage.m_pPlayer->m_nPlayerCond();
	playerStorage.m_PlayerData.m_nPlayerCondEx = playerStorage.m_pPlayer->m_nPlayerCondEx();
	playerStorage.m_PlayerData.m_nPlayerCondEx2 = playerStorage.m_pPlayer->m_nPlayerCondEx2();
	playerStorage.m_PlayerData.m_nPlayerCondEx3 = playerStorage.m_pPlayer->m_nPlayerCondEx3();
	playerStorage.m_PlayerData.m_nPlayerCondEx4 = playerStorage.m_pPlayer->m_nPlayerCondEx4();
	playerStorage.m_PlayerData._condition_bits = playerStorage.m_pPlayer->_condition_bits();
}

void CMovementSimulation::Reset(PlayerStorage& playerStorage)
{
	playerStorage.m_pPlayer->m_vecOrigin() = playerStorage.m_PlayerData.m_vecOrigin;
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
	playerStorage.m_pPlayer->m_nButtons() = playerStorage.m_PlayerData.m_nButtons;
	playerStorage.m_pPlayer->m_flLastMovementStunChange() = playerStorage.m_PlayerData.m_flLastMovementStunChange;
	playerStorage.m_pPlayer->m_flStunLerpTarget() = playerStorage.m_PlayerData.m_flStunLerpTarget;
	playerStorage.m_pPlayer->m_bStunNeedsFadeOut() = playerStorage.m_PlayerData.m_bStunNeedsFadeOut;
	playerStorage.m_pPlayer->m_flPrevTauntYaw() = playerStorage.m_PlayerData.m_flPrevTauntYaw;
	playerStorage.m_pPlayer->m_flTauntYaw() = playerStorage.m_PlayerData.m_flTauntYaw;
	playerStorage.m_pPlayer->m_flCurrentTauntMoveSpeed() = playerStorage.m_PlayerData.m_flCurrentTauntMoveSpeed;
	playerStorage.m_pPlayer->m_iKartState() = playerStorage.m_PlayerData.m_iKartState;
	playerStorage.m_pPlayer->m_flVehicleReverseTime() = playerStorage.m_PlayerData.m_flVehicleReverseTime;
	playerStorage.m_pPlayer->m_flHypeMeter() = playerStorage.m_PlayerData.m_flHypeMeter;
	playerStorage.m_pPlayer->m_flMaxspeed() = playerStorage.m_PlayerData.m_flMaxspeed;
	playerStorage.m_pPlayer->m_nAirDucked() = playerStorage.m_PlayerData.m_nAirDucked;
	playerStorage.m_pPlayer->m_bJumping() = playerStorage.m_PlayerData.m_bJumping;
	playerStorage.m_pPlayer->m_iAirDash() = playerStorage.m_PlayerData.m_iAirDash;
	playerStorage.m_pPlayer->m_flWaterJumpTime() = playerStorage.m_PlayerData.m_flWaterJumpTime;
	playerStorage.m_pPlayer->m_flSwimSoundTime() = playerStorage.m_PlayerData.m_flSwimSoundTime;
	playerStorage.m_pPlayer->m_surfaceProps() = playerStorage.m_PlayerData.m_surfaceProps;
	playerStorage.m_pPlayer->m_pSurfaceData() = playerStorage.m_PlayerData.m_pSurfaceData;
	playerStorage.m_pPlayer->m_surfaceFriction() = playerStorage.m_PlayerData.m_surfaceFriction;
	playerStorage.m_pPlayer->m_chTextureType() = playerStorage.m_PlayerData.m_chTextureType;
	playerStorage.m_pPlayer->m_vecPunchAngle() = playerStorage.m_PlayerData.m_vecPunchAngle;
	playerStorage.m_pPlayer->m_vecPunchAngleVel() = playerStorage.m_PlayerData.m_vecPunchAngleVel;
	playerStorage.m_pPlayer->m_MoveType() = playerStorage.m_PlayerData.m_MoveType;
	playerStorage.m_pPlayer->m_MoveCollide() = playerStorage.m_PlayerData.m_MoveCollide;
	playerStorage.m_pPlayer->m_vecLadderNormal() = playerStorage.m_PlayerData.m_vecLadderNormal;
	playerStorage.m_pPlayer->m_flGravity() = playerStorage.m_PlayerData.m_flGravity;
	playerStorage.m_pPlayer->m_nWaterLevel() = playerStorage.m_PlayerData.m_nWaterLevel;
	playerStorage.m_pPlayer->m_nWaterType() = playerStorage.m_PlayerData.m_nWaterType;
	playerStorage.m_pPlayer->m_flFallVelocity() = playerStorage.m_PlayerData.m_flFallVelocity;
	playerStorage.m_pPlayer->m_nPlayerCond() = playerStorage.m_PlayerData.m_nPlayerCond;
	playerStorage.m_pPlayer->m_nPlayerCondEx() = playerStorage.m_PlayerData.m_nPlayerCondEx;
	playerStorage.m_pPlayer->m_nPlayerCondEx2() = playerStorage.m_PlayerData.m_nPlayerCondEx2;
	playerStorage.m_pPlayer->m_nPlayerCondEx3() = playerStorage.m_PlayerData.m_nPlayerCondEx3;
	playerStorage.m_pPlayer->m_nPlayerCondEx4() = playerStorage.m_PlayerData.m_nPlayerCondEx4;
	playerStorage.m_pPlayer->_condition_bits() = playerStorage.m_PlayerData._condition_bits;
}



void CMovementSimulation::FillVelocities()
{
	const auto pLocal = g_EntityCache.GetLocal();
	if (!pLocal)
	{
		mVelocities.clear();
		return;
	}

	if (Vars::Aimbot::Projectile::StrafePrediction.Value)
	{
		auto FillVelocity = [this](CBaseEntity* pEntity, Vec3 vVelocity)
			{
				const int iEntIndex = pEntity->GetIndex();

				if (!pEntity->IsAlive() || pEntity->IsAGhost() || pEntity->GetDormant() || vVelocity.IsZero())
				{
					mVelocities[iEntIndex].clear();
					return;
				}

				const float flSimTime = pEntity->m_flSimulationTime(), flOldSimTime = pEntity->m_flOldSimulationTime();
				if (TIME_TO_TICKS(flSimTime - flOldSimTime) <= 0)
					return;

				mVelocities[iEntIndex].push_front(vVelocity);

				if (mVelocities[iEntIndex].size() > 66)
					mVelocities[iEntIndex].pop_back();
			};

		for (const auto& pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
		{
			if (pEntity && pEntity != pLocal) // only appears when in thirdperson
				FillVelocity(pEntity, pEntity->m_vecVelocity());
		}
		FillVelocity(pLocal, pLocal->m_vecVelocity());
	}
	else
		mVelocities.clear();
}



bool CMovementSimulation::Initialize(CBaseEntity* pPlayer, PlayerStorage& playerStorageOut, bool useHitchance, bool cancelStrafe)
{
	if (!pPlayer || !pPlayer->IsPlayer() || pPlayer->deadflag())
	{
		playerStorageOut.m_bInitFailed = playerStorageOut.m_bFailed = true;
		return false;
	}

	G::MoveLines.clear();

	//set player
	playerStorageOut.m_pPlayer = pPlayer;

	//set current command
	playerStorageOut.m_pPlayer->SetCurrentCmd(&DummyCmd);

	//store player's data
	Store(playerStorageOut);

	// store vars
	m_bOldInPrediction = I::Prediction->m_bInPrediction;
	m_bOldFirstTimePredicted = I::Prediction->m_bFirstTimePredicted;
	m_flOldFrametime = I::GlobalVars->frametime;

	// the hacks that make it work
	{
		if (pPlayer->m_fFlags() & FL_DUCKING)
		{
			pPlayer->m_fFlags() &= ~FL_DUCKING; // breaks origin's z if FL_DUCKING is not removed
			pPlayer->m_bDucked() = true; // (mins/maxs will be fine when ducking as long as m_bDucked is true)
			pPlayer->m_flDucktime() = 0.f;
			pPlayer->m_flDuckJumpTime() = 0.f;
			pPlayer->m_bDucking() = false;
			pPlayer->m_bInDuckJump() = false;
		}

		if (pPlayer != g_EntityCache.GetLocal())
			pPlayer->m_hGroundEntity() = -1; // fix for velocity.z being set to 0 even if in air

		if (pPlayer->IsOnGround())
			pPlayer->m_vecOrigin().z += 0.03125f; // to prevent getting stuck in the ground

		pPlayer->m_flModelScale() -= 0.03125f; // fixes issues with corners

		//for some reason if xy vel is zero it doesn't predict
		if (fabsf(pPlayer->m_vecVelocity().x) < 0.01f)
			pPlayer->m_vecVelocity().x = 0.015f;
		if (fabsf(pPlayer->m_vecVelocity().y) < 0.01f)
			pPlayer->m_vecVelocity().y = 0.015f;

		if (pPlayer->OnSolid())
			pPlayer->m_vecVelocity().z = 0.f; // step fix

		pPlayer->m_vecBaseVelocity() = Vec3(); // i think residual basevelocity causes issues
	}

	// setup move data
	if (!SetupMoveData(playerStorageOut))
	{
		playerStorageOut.m_bFailed = true;
		return false;
	}

	const int iStrafeSamples = pPlayer->OnSolid() ? Vars::Aimbot::Projectile::iGroundSamples.Value : Vars::Aimbot::Projectile::iAirSamples.Value;

	// calculate strafe if desired
	bool bCalculated = cancelStrafe ? false : StrafePrediction(playerStorageOut, iStrafeSamples);

	// really hope this doesn't work like shit
	if (useHitchance && bCalculated && !pPlayer->m_vecVelocity().IsZero())
	{
		const auto& mVelocityRecords = mVelocities[playerStorageOut.m_pPlayer->GetIndex()];
		const int iSamples = mVelocityRecords.size();

		float flCurrentChance = 1.f, flAverageYaw = 0.f, flCompareYaw = Math::VelocityToAngles(playerStorageOut.m_PlayerData.m_vecVelocity).y;
		for (int i = 0; i < iSamples; i++)
		{
			const float flRecordYaw = Math::VelocityToAngles(mVelocityRecords[i]).y;

			const float flDelta = RAD2DEG(Math::AngleDiffRad(DEG2RAD(flCompareYaw), DEG2RAD(flRecordYaw)));
			flAverageYaw += flDelta;

			if ((i + 1) % iStrafeSamples == 0 || i == iSamples - 1)
			{
				flAverageYaw /= i % iStrafeSamples + 1;
				if (fabsf(playerStorageOut.m_flAverageYaw - flAverageYaw) > 0.5f)
					flCurrentChance -= 1.f / ((iSamples - 1) / float(iStrafeSamples) + 1);
				flAverageYaw = 0.f;
			}

			flCompareYaw = flRecordYaw;
		}

		if (flCurrentChance + 0.001f < float(Vars::Aimbot::Projectile::StrafePredictionHitchance.Value) / 100)
		{
			Utils::ConLog("MovementSimulation", std::format("Hitchance ({}% < {}%)", flCurrentChance * 100, Vars::Aimbot::Projectile::StrafePredictionHitchance.Value).c_str(), { 125, 255, 83, 255 }, Vars::Debug::Logging.Value);

			playerStorageOut.m_bFailed = true;
			return false;
		}
	}

	return true;
}

bool CMovementSimulation::SetupMoveData(PlayerStorage& playerStorage)
{
	if (!playerStorage.m_pPlayer)
		return false;

	playerStorage.m_MoveData.m_bFirstRunOfFunctions = false;
	playerStorage.m_MoveData.m_bGameCodeMovedPlayer = false;
	playerStorage.m_MoveData.m_nPlayerHandle = reinterpret_cast<IHandleEntity*>(playerStorage.m_pPlayer)->GetRefEHandle();

	playerStorage.m_MoveData.m_vecOrigin = playerStorage.m_pPlayer->m_vecOrigin();
	playerStorage.m_MoveData.m_vecVelocity = playerStorage.m_pPlayer->m_vecVelocity();
	playerStorage.m_MoveData.m_flMaxSpeed = playerStorage.m_pPlayer->TeamFortress_CalculateMaxSpeed();
	if (playerStorage.m_PlayerData.m_fFlags & FL_DUCKING)
		playerStorage.m_MoveData.m_flMaxSpeed *= 0.3333f;

	playerStorage.m_MoveData.m_vecViewAngles = { 0.f, Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity).y, 0.f };

	Vec3 vForward = {}, vRight = {};
	Math::AngleVectors(playerStorage.m_MoveData.m_vecViewAngles, &vForward, &vRight, nullptr);
	playerStorage.m_MoveData.m_flForwardMove = (playerStorage.m_MoveData.m_vecVelocity.y - vRight.y / vRight.x * playerStorage.m_MoveData.m_vecVelocity.x) / (vForward.y - vRight.y / vRight.x * vForward.x);
	playerStorage.m_MoveData.m_flSideMove = (playerStorage.m_MoveData.m_vecVelocity.x - vForward.x * playerStorage.m_MoveData.m_flForwardMove) / vRight.x;
	if (playerStorage.m_pPlayer->m_vecVelocity().Length2D() <= playerStorage.m_MoveData.m_flMaxSpeed * 0.1f)
		playerStorage.m_MoveData.m_flForwardMove = playerStorage.m_MoveData.m_flSideMove = 0.f;

	playerStorage.m_MoveData.m_flClientMaxSpeed = playerStorage.m_MoveData.m_flMaxSpeed;
	playerStorage.m_MoveData.m_vecAngles = playerStorage.m_MoveData.m_vecOldAngles = playerStorage.m_MoveData.m_vecViewAngles;
	//if (playerStorage.m_pPlayer->m_hConstraintEntity())
	//	playerStorage.m_MoveData.m_vecConstraintCenter = I::ClientEntityList->GetClientEntityFromHandle(playerStorage.m_pPlayer->m_hConstraintEntity())->GetAbsOrigin();
	///else
	playerStorage.m_MoveData.m_vecConstraintCenter = playerStorage.m_pPlayer->m_vecConstraintCenter();
	playerStorage.m_MoveData.m_flConstraintRadius = playerStorage.m_pPlayer->m_flConstraintRadius();
	playerStorage.m_MoveData.m_flConstraintWidth = playerStorage.m_pPlayer->m_flConstraintWidth();
	playerStorage.m_MoveData.m_flConstraintSpeedFactor = playerStorage.m_pPlayer->m_flConstraintSpeedFactor();

	return true;
}

bool CMovementSimulation::GetYawDifference(const std::deque<Vec3>& mVelocityRecords, const int iTime, float* flYaw)
{
	if (mVelocityRecords.size() <= iTime + 2)
		return false;

	const float flRecordYaw1 = Math::VelocityToAngles(mVelocityRecords[iTime]).y;
	const float flRecordYaw2 = Math::VelocityToAngles(mVelocityRecords[iTime + 1]).y;

	*flYaw = fmodf(flRecordYaw1 - flRecordYaw2 + 180.f, 360.f);
	*flYaw += *flYaw < 0 ? 180.f : -180.f;

	static int iSign = 0;
	const int iLastSign = iSign;
	const int iCurSign = iSign = *flYaw > 0 ? 1 : -1;
	if (fabsf(*flYaw) < 640.f / mVelocityRecords[iTime].Length2D()) // dumb way to get straight bool
		return false;

	return !iTime || iLastSign == iCurSign ? true : false;
}

float CMovementSimulation::GetAverageYaw(const int iIndex, const int iSamples)
{
	const auto& mVelocityRecords = mVelocities[iIndex];

	float flAverageYaw = 0.f; int i = 0;
	for (; i < iSamples; i++)
	{
		float flYaw;
		if (!GetYawDifference(mVelocityRecords, i, &flYaw))
			break;

		flAverageYaw += flYaw;
	}
	if (i < 2) // valid strafes too low
		return 0.f;

	flAverageYaw /= i;

	Utils::ConLog("MovementSimulation", std::format("flAverageYaw calculated to {} from {}", flAverageYaw, i).c_str(), { 83, 255, 83, 255 }, Vars::Debug::Logging.Value);

	return flAverageYaw;
}

bool CMovementSimulation::StrafePrediction(PlayerStorage& playerStorage, const int iSamples)
{
	if (playerStorage.m_pPlayer->OnSolid() ? !(Vars::Aimbot::Projectile::StrafePrediction.Value & (1 << 1)) : !(Vars::Aimbot::Projectile::StrafePrediction.Value & (1 << 0)))
		return false;

	float flAverageYaw = GetAverageYaw(playerStorage.m_pPlayer->GetIndex(), iSamples);

	//if (playerStorage.m_pPlayer->OnSolid() && flAverageYaw)
	//	playerStorage.m_MoveData.m_vecViewAngles.y += 22.5f * (flAverageYaw > 0.f ? 1.f : -1.f); // fix for ground strafe delay
	playerStorage.m_flAverageYaw = flAverageYaw;

	return true;
}

void CMovementSimulation::RunTick(PlayerStorage& playerStorage)
{
	if (playerStorage.m_bFailed || !playerStorage.m_pPlayer || !playerStorage.m_pPlayer->IsPlayer())
		return;

	playerStorage.PredictionLines.push_back({ playerStorage.m_MoveData.m_vecOrigin, Math::GetRotatedPosition(playerStorage.m_MoveData.m_vecOrigin, Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value) });

	//make sure frametime and prediction vars are right
	I::Prediction->m_bInPrediction = true;
	I::Prediction->m_bFirstTimePredicted = false;
	I::GlobalVars->frametime = I::Prediction->m_bEnginePaused ? 0.f : TICK_INTERVAL;

	float flCorrection = 0.f;
	if (playerStorage.m_flAverageYaw)
	{
		if (!playerStorage.m_pPlayer->OnSolid())
			flCorrection = 90.f * (playerStorage.m_flAverageYaw > 0.f ? 1.f : -1.f);

		playerStorage.m_MoveData.m_vecViewAngles.y += playerStorage.m_flAverageYaw + flCorrection;
	}
	//else
	//	playerStorage.m_MoveData.m_vecViewAngles.y = Math::VelocityToAngles(playerStorage.m_MoveData.m_vecVelocity).y;

	I::TFGameMovement->ProcessMovement(playerStorage.m_pPlayer, &playerStorage.m_MoveData);

	playerStorage.m_MoveData.m_vecViewAngles.y -= flCorrection;
}

void CMovementSimulation::Restore(PlayerStorage& playerStorage)
{
	if (playerStorage.m_bInitFailed || !playerStorage.m_pPlayer)
		return;

	playerStorage.m_pPlayer->SetCurrentCmd(nullptr);

	Reset(playerStorage);

	I::Prediction->m_bInPrediction = m_bOldInPrediction;
	I::Prediction->m_bFirstTimePredicted = m_bOldFirstTimePredicted;
	I::GlobalVars->frametime = m_flOldFrametime;

	const bool bInitFailed = playerStorage.m_bInitFailed, bFailed = playerStorage.m_bFailed;
	memset(&playerStorage, 0, sizeof(PlayerStorage));
	playerStorage.m_bInitFailed = bInitFailed, playerStorage.m_bFailed = bFailed;
}