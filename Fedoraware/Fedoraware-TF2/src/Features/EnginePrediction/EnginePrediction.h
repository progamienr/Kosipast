#pragma once
#include "../Feature.h"

class CEnginePrediction
{
private:
	CMoveData m_MoveData = {};
private:
	int GetTickbase(CUserCmd* pCmd, CBaseEntity* pLocal);

public:
	void Start(CUserCmd* pCmd);
	void End(CUserCmd* pCmd);

	int m_nOldTickCount = 0;
	float m_fOldCurrentTime = 0.0f;
	float m_fOldFrameTime = 0.0f;
};

ADD_FEATURE(CEnginePrediction, EnginePrediction)