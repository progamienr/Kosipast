#pragma once

class CCore
{
	void OnLoaded();

	bool bHasFailed = false;

public:
	void Load();
	void Unload();

	bool ShouldUnload();
};

inline CCore g_Core;