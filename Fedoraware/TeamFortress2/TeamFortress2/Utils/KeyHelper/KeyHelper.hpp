#pragma once
#include <Windows.h>
#include <chrono>

class KeyHelper {
	enum class KeyState { None, Down };

	int* Key = nullptr;
	KeyState LastState = KeyState::None;
	int LastPress = 0;

public:
	explicit KeyHelper(int* vKey)
	{
		Key = vKey;
	}

	// Is the button currently down?
	bool Down()
	{
		{ //Utils::IsGameWindowInFocus()
			static HWND hwGame = nullptr;

			while (!hwGame) {
				hwGame = FindWindowW(nullptr, L"Team Fortress 2");
				if (!hwGame)
				{
					return false;
				}
			}

			if (GetForegroundWindow() != hwGame) return false;
		}

		if (!*Key)
		{
			LastState = KeyState::None;
			return false;
		}

		const bool isDown = GetAsyncKeyState(*Key) & 0x8000;
		LastState = isDown ? KeyState::Down : KeyState::None;
		return isDown;
	}

	// Was the button just pressed? This will only be true once.
	bool Pressed()
	{
		const bool shouldCheck = LastState == KeyState::None;
		return Down() && shouldCheck;
	}

	// Was the button double clicked? This will only be true once.
	bool Double()
	{
		if (!Pressed()) return false;

		int epoch = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		bool ret = epoch < LastPress + 500;
		LastPress = epoch;

		return ret;
	}

	// Was the button just released? This will only be true once.
	bool Released()
	{
		const bool shouldCheck = LastState == KeyState::Down;
		return !Down() && shouldCheck;
	}
};