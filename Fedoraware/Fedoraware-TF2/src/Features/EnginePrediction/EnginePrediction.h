#pragma once
#include "../Feature.h"

class CEnginePrediction
{
private:
	CMoveData m_MoveData = {};
private:
	int GetTickbase(CUserCmd* pCmd, CBaseEntity* pLocal);

	int m_nOldTickCount = 0;
	float m_fOldCurrentTime = 0.f;
	float m_fOldFrameTime = 0.f;

public:
	void Simulate(CUserCmd* pCmd);
	void Start(CUserCmd* pCmd);
	void End(CUserCmd* pCmd);

	Vec3 m_vOldOrigin = {};
	Vec3 m_vOldVelocity = {};
};

ADD_FEATURE(CEnginePrediction, EnginePrediction)