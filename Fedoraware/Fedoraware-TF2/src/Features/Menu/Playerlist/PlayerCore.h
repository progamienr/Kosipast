#pragma once
#include "../../Feature.h"

#include "../../Feature.h"

#include "PlayerUtils.h"
#include "../ConfigManager/ConfigManager.h"

class CPlayerlistCore
{
	void LoadPlayers();
	void SavePlayers();
	void LoadTags();
	void SaveTags();

public:
	void Run();
};

ADD_FEATURE(CPlayerlistCore, PlayerCore)