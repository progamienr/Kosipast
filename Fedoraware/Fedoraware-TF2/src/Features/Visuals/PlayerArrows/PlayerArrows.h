#pragma once
#include "../../Feature.h"

class CPlayerArrows
{
private:
	bool ShouldRun(CBaseEntity* pLocal);
	void DrawArrowTo(const Vec3& vecFromPos, const Vec3& vecToPos, Color_t color);

public:
	void Run(CBaseEntity* pLocal);
};

ADD_FEATURE(CPlayerArrows, PlayerArrows)