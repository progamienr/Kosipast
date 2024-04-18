#pragma once
#include "../Feature.h"

class CEnginePrediction
{
private:
	CMoveData m_MoveData = {};
private:
	int GetTickbase(CUserCmd* pCmd, CBaseEntity* pLocal);
	void Simulate(CBaseEntity* pLocal, CUserCmd* pCmd);

	int m_nOldTickCount = 0;
	float m_fOldCurrentTime = 0.f;
	float m_fOldFrameTime = 0.f;

	bool bSimulated = false;

public:
	void Start(CBaseEntity* pLocal, CUserCmd* pCmd);
	void End(CBaseEntity* pLocal, CUserCmd* pCmd);
};

ADD_FEATURE(CEnginePrediction, EnginePrediction)