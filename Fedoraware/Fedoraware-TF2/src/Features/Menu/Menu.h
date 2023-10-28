#pragma once
#include "../Feature.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_color_gradient.h"

class CMenu
{
	void DrawMenu();
	void DrawTabbar();

	void MenuAimbot();
	void MenuTrigger();
	void MenuVisuals();
	void MenuHvH();
	void MenuMisc();
	void MenuSettings();

	void AddDraggable(const char* szTitle, DragBox_t& info, bool bShouldDraw);

	void DrawKeybinds();
	void LoadStyle();

	enum class MenuTab
	{
		Aimbot,
		Trigger,
		Visuals,
		HvH,
		Misc,
		Settings
	};

	enum class VisualsTab
	{
		ESP,
		Chams,
		Glow,
		Misc,
		Radar,
		Font
	};

	enum class ConfigTab
	{
		General,
		Visuals
	};

	MenuTab CurrentTab = MenuTab::Aimbot;
	VisualsTab CurrentVisualsTab = VisualsTab::ESP;
	ConfigTab CurrentConfigTab = ConfigTab::General;

public:
	void Render(IDirect3DDevice9* pDevice);
	void Init(IDirect3DDevice9* pDevice);

	bool IsOpen = false;
	bool ConfigLoaded = false;
	bool Unload = false;

	float TitleHeight = 22.f;
	float TabHeight = 30.f;
	float SubTabHeight = 0.f;
	float FooterHeight = 20.f;
	float ItemWidth = 130.f;

	std::string FeatureHint;

	// Colors
	ImColor Accent = { 255, 101, 101 };
	ImColor AccentDark = { 217, 87, 87 };
	ImColor Background = { 23, 23, 23, 250 };	// Title bar
	ImColor BackgroundLight = { 51, 51, 56 };		// Tab bar
	ImColor BackgroundDark = { 31, 31, 31 };		// Background
	ImColor TextLight = { 240, 240, 240 };

	// Fonts
	ImFont* VerdanaSmall = nullptr;	// 12px
	ImFont* Verdana = nullptr;		// 14px
	ImFont* VerdanaBold = nullptr;	// 14px

	ImFont* SectionFont = nullptr;	// 16px
	ImFont* TitleFont = nullptr;	// 20px
	ImFont* IconFont = nullptr;		// 16px
};

ADD_FEATURE(CMenu, Menu)