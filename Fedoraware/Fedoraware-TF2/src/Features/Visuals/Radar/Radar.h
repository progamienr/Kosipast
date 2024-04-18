#pragma once
#include "../../Feature.h"

class CRadar
{
	bool GetDrawPosition(CBaseEntity* pLocal, CBaseEntity* pEntity, int& x, int& y, int& z);
	void DrawBackground();
	void DrawPoints(CBaseEntity* pLocal);

public:
	void Run(CBaseEntity* pLocal);
};

ADD_FEATURE(CRadar, Radar)