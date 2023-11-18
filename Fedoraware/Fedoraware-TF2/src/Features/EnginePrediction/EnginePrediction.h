#pragma once
#include "../Feature.h"

class CEnginePrediction
{
private:
	CMoveData m_MoveData = {};
private:
	int GetTickbase(CUserCmd* pCmd, CBaseEntity* pLocal);

	int m_nOldTickCount = 0;
	float m_fOldCurrentTime = 0.0f;
	float m_fOldFrameTime = 0.0f;

	Vec3 m_vOldOrigin = {};
	Vec3 m_vOldVelocity = {};

public:
	void Simulate(CUserCmd* pCmd);
	void Start(CUserCmd* pCmd);
	void End(CUserCmd* pCmd);
};

ADD_FEATURE(CEnginePrediction, EnginePrediction)