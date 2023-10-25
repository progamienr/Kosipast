#pragma once

#include <Windows.h>
#include <unordered_map>
#include <chrono>

struct KeyStorage
{
	bool bIsDown = false;
	bool bIsPressed = false;
	bool bIsDouble = false;
	int iPressTime = 0;
	bool bIsReleased = false;
};

class CKeyHandler
{
	int Key = 0;
	std::unordered_map<int, KeyStorage> StorageMap;

	void StoreKey()
	{
		// init if not found
		if (StorageMap.find(Key) == StorageMap.end())
			StorageMap[Key] = {};

		// down
		bool bDown = GetAsyncKeyState(Key) & 0x8000;
		if (bDown)
		{ //Utils::IsGameWindowInFocus()
			static HWND hwGame = nullptr;

			while (!hwGame)
			{
				hwGame = FindWindowW(nullptr, L"Team Fortress 2");
				if (!hwGame)
					bDown = false;
			}

			if (GetForegroundWindow() != hwGame)
				bDown = false;
		}
		if (!Key)
			bDown = false;

		// pressed
		const bool bPressed = bDown && !StorageMap[Key].bIsDown;

		// double click
		const int iEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		const bool bDouble = bPressed && iEpoch < StorageMap[Key].iPressTime + 500;

		// released
		const bool bReleased = !bDown && StorageMap[Key].bIsDown;
		
		StorageMap[Key].bIsDown = bDown;
		StorageMap[Key].bIsPressed = bPressed;
		StorageMap[Key].bIsDouble = bDouble;
		StorageMap[Key].iPressTime = iEpoch;
		StorageMap[Key].bIsReleased = bReleased;
	}

public:
	void SetKey(int iKey)
	{
		if (iKey != -1)
			Key = iKey;
	}

	// Is the button currently down?
	bool Down(int iKey = -1)
	{
		SetKey(iKey);
		StoreKey();
		
		return StorageMap[Key].bIsDown;
	}

	// Was the button just pressed? This will only be true once.
	bool Pressed(int iKey = -1)
	{
		SetKey(iKey);
		StoreKey();

		return StorageMap[Key].bIsPressed;
	}

	// Was the button double clicked? This will only be true once.
	bool Double(int iKey = -1)
	{
		SetKey(iKey);
		StoreKey();

		return StorageMap[Key].bIsDouble;
	}

	// Was the button just released? This will only be true once.
	bool Released(int iKey = -1)
	{
		SetKey(iKey);
		StoreKey();

		return StorageMap[Key].bIsReleased;
	}
};

namespace F {
	inline CKeyHandler KeyHandler;
}