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
	std::unordered_map<int, KeyStorage> StorageMap;

public:
	void StoreKey(int iKey)
	{
		// down
		bool bDown = GetAsyncKeyState(iKey) & 0x8000;
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
		if (!iKey)
			bDown = false;

		// pressed
		const bool bPressed = bDown && !StorageMap[iKey].bIsDown;

		// double click
		const int iEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		const bool bDouble = bPressed && iEpoch < StorageMap[iKey].iPressTime + 500;

		// released
		const bool bReleased = !bDown && StorageMap[iKey].bIsDown;

		StorageMap[iKey].bIsDown = bDown;
		StorageMap[iKey].bIsPressed = bPressed;
		StorageMap[iKey].bIsDouble = bDouble;
		StorageMap[iKey].iPressTime = iEpoch;
		StorageMap[iKey].bIsReleased = bReleased;
	}

	// Is the button currently down?
	bool Down(int iKey, const bool bStore = true)
	{
		if (bStore)
			StoreKey(iKey);
		
		return StorageMap[iKey].bIsDown;
	}

	// Was the button just pressed? This will only be true once.
	bool Pressed(int iKey, const bool bStore = true)
	{
		if (bStore)
			StoreKey(iKey);

		return StorageMap[iKey].bIsPressed;
	}

	// Was the button double clicked? This will only be true once.
	bool Double(int iKey, const bool bStore = true)
	{
		if (bStore)
			StoreKey(iKey);

		return StorageMap[iKey].bIsDouble;
	}

	// Was the button just released? This will only be true once.
	bool Released(int iKey, const bool bStore = true)
	{
		if (bStore)
			StoreKey(iKey);

		return StorageMap[iKey].bIsReleased;
	}
};

namespace F {
	inline CKeyHandler KeyHandler;
}