#pragma once

#include "../../../SDK/SDK.h"

struct PositionData
{
	Vec3 m_vecOrigin = {};
	float m_flSimTime = 0.f;
};
struct VelocityData
{
	Vec3 m_vecVelocity = {};
	bool m_bGrounded = true;
};
struct PlayerData
{
	Vec3 m_vecOrigin = {};
	Vec3 m_vecVelocity = {};
	Vec3 m_vecBaseVelocity = {};
	Vec3 m_vecViewOffset = {};
	Vec3 m_vecWorldSpaceCenter = {};
	int m_hGroundEntity = 0;
	int m_fFlags = 0;
	float m_flDucktime = 0.0f;
	float m_flDuckJumpTime = 0.0f;
	bool m_bDucked = false;
	bool m_bDucking = false;
	bool m_bInDuckJump = false;
	float m_flModelScale = 0.0f;
};
struct PlayerStorage
{
	CBaseEntity* m_pPlayer = nullptr;
	CMoveData m_MoveData = {};
	PlayerData m_PlayerData = {};

	float m_flAverageYaw = 0.f;

	std::deque<std::pair<Vec3, Vec3>> PredictionLines;

	bool m_bFailed = false;
	bool m_bInitFailed = false;
};

class CMovementSimulation
{
private:
	//void SetCurrentCommand(CBaseEntity* pPlayer, CUserCmd* pCmd);
	bool SetupMoveData(PlayerStorage& playerStorage);
	float GetAverageYaw(const int iSamples, PlayerStorage& playerStorage);
	void StrafePrediction(PlayerStorage& playerStorage);

private:
	bool m_bOldInPrediction = false;
	bool m_bOldFirstTimePredicted = false;
	float m_flOldFrametime = 0.0f;
	int iTick = 0;

	std::map<int, std::deque<VelocityData>> m_Velocities;

public:
	void FillInfo();

	bool Initialize(CBaseEntity* pPlayer, PlayerStorage& playerStorageOut, bool useHitchance = true, bool cancelStrafe = false);
	void RunTick(PlayerStorage& playerStorage);
	void Restore(PlayerStorage& playerStorage);
};

ADD_FEATURE(CMovementSimulation, MoveSim)