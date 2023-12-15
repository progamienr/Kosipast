#pragma once
#include "../Feature.h"

#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/imgui_color_gradient.h>
#include <ImGui/TextEditor.h>

class CMenu
{
	void DrawMenu();

	void MenuAimbot();
	void MenuVisuals();
	void MenuMisc();
	void MenuLogs();
	void MenuSettings();

	void AddDraggable(const char* szTitle, DragBox_t& info, bool bShouldDraw);
	void DrawKeybinds();
	void DrawCameraWindow();

	void LoadStyle();

	int CurrentTab = 0;

	int CurrentAimbotTab = 0;
	int CurrentVisualsTab = 0;
	int CurrentLogsTab = 0;
	int CurrentConfigTab = 0;
	int CurrentConfigType = 0;

	int ChamsConfig{ 0 }; // Real, Fake, Enemy, Team, Friend, Target, Backtrack, ViewmodelArms, ViewmodelWeapon, LocalBuildings, EnemyBuildings, TeamBuildings, FriendBuildings, Health, Ammo, Projectiles, Objective [payload, intelligence, etc], NPC [includes boss], Pickups [lunchbox, money, spell, powerup], Ragdoll
	int CurrentChamsFilter = 0;
	int CurrentChamsTab = 0;

	int GlowConfig{ 0 }; // above^
	int CurrentGlowFilter = 0;

	float TitleHeight = 22.f;
	ImVec2 TabSize = { 65, 72 };
	ImVec2 SubTabSize = { 90, 48 };

	// material editor stuff
	TextEditor TextEditor;
	std::string CurrentMaterial;
	bool LockedMaterial;

public:
	void Render(IDirect3DDevice9* pDevice);
	void Init(IDirect3DDevice9* pDevice);

	bool IsOpen = false;
	bool ConfigLoaded = false;
	bool Unload = false;

	std::string FeatureHint;

	// Colors
	ImColor Accent = { 255, 101, 101 };
	ImColor AccentLight = { 255, 111, 111 };
	ImColor Background = { 45, 46, 53, 200 };
	ImColor Foreground = { 24, 26, 30, 200 };
	ImColor Foremost = { 56, 60, 64, 200 };
	ImColor ForemostLight = { 62, 66, 70, 200 };
	ImColor Inactive = { 200, 200, 200 };
	ImColor Active = { 255, 255, 255 };

	// Fonts
	ImFont* FontSmall = nullptr;
	ImFont* FontRegular = nullptr;
	ImFont* FontBold = nullptr;
	ImFont* FontBlack = nullptr;
	ImFont* FontLarge = nullptr;

	ImFont* IconFontRegular = nullptr;
	ImFont* IconFontLarge = nullptr;
};

ADD_FEATURE(CMenu, Menu)