#pragma once
#include "../../Feature.h"

class CRadar
{
	bool ShouldRun();
	void DrawRadar();
	bool GetDrawPosition(int& x, int& y, int& z, CBaseEntity* pEntity);
	void DrawPoints(CBaseEntity* pLocal);

	float LocalCos = 0, LocalSin = 0, Range = 0, LocalYaw = 0;
	Vec3 LocalOrigin;

public:
	void Run();
};

ADD_FEATURE(CRadar, Radar)