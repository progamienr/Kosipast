#include "Menu.h"

#include "../Vars.h"
#include "../Color.h"
#include "../Misc/Misc.h"
#include "../Visuals/Visuals.h"
#include "../Resolver/Resolver.h"

#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/imgui_stdlib.h>

#include "Fonts/IconsMaterialDesign.h"
#include "Fonts/RobotoMedium.h"
#include "Fonts/RobotoBold.h"
#include "Fonts/RobotoBlack.h"

#include "Playerlist/PlayerUtils.h"

#include "Components.hpp"
#include "ConfigManager/ConfigManager.h"
#include "../CameraWindow/CameraWindow.h"

#include <mutex>

#pragma warning (disable : 4309)

int unuPrimary = 0;
int unuSecondary = 0;

constexpr auto SIZE_FULL_WIDTH = ImVec2(-1, 20);

/* The main menu */
void CMenu::DrawMenu()
{
	using namespace ImGui;

	ImVec2 mainWindowPos = {};
	ImVec2 mainWindowSize = {};

	SetNextWindowSize(ImVec2(750, 500), ImGuiCond_FirstUseEver);
	PushStyleVar(ImGuiStyleVar_WindowMinSize, { 750, 500 });
	if (Begin("MainWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
	{
		const auto windowPos = mainWindowPos = GetWindowPos();
		const auto windowSize = mainWindowSize = GetWindowSize();

		// Main tabs
		FTabs({ "AIMBOT", "VISUALS", "MISC", "LOGS", "SETTINGS" }, &CurrentTab, TabSize, { 0, SubTabSize.y }, true, {ICON_MD_GROUP, ICON_MD_IMAGE, ICON_MD_PUBLIC, ICON_MD_MENU_BOOK, ICON_MD_SETTINGS});
		
		// Sub tabs
		switch (CurrentTab)
		{
		case 0: FTabs({ "GENERAL", "HVH", "AUTOMATION" }, &CurrentAimbotTab, SubTabSize, { TabSize.x, 0 }); break;
		case 1: FTabs({ "ESP", "CHAMS", "GLOW", "MISC##", "RADAR", "MENU" }, &CurrentVisualsTab, SubTabSize, { TabSize.x, 0 }); break;
		case 2: FTabs({ "MISC##" }, nullptr, SubTabSize, { TabSize.x, 0 }); break;
		case 3: FTabs({ "LOGS##", "SETTINGS##" }, &CurrentLogsTab, SubTabSize, { TabSize.x, 0 }); break;
		case 4: FTabs({ "CONFIG", "CONDITIONS", "PLAYERLIST", "MATERIALS" }, &CurrentConfigTab, SubTabSize, { TabSize.x, 0 }); break;
		}

		// Main content
		SetCursorPos({ TabSize.x, SubTabSize.y });
		PushStyleVar(ImGuiStyleVar_WindowPadding, { 8.f, 8.f });
		PushStyleColor(ImGuiCol_ChildBg, {});
		if (BeginChild("Content", { windowSize.x - TabSize.x, windowSize.y - SubTabSize.y }, false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			PushStyleColor(ImGuiCol_ChildBg, Foreground.Value);
			PushStyleVar(ImGuiStyleVar_ChildRounding, 3.f);

			switch (CurrentTab)
			{
			case 0: MenuAimbot(); break;
			case 1: MenuVisuals(); break;
			case 2: MenuMisc(); break;
			case 3: MenuLogs(); break;
			case 4: MenuSettings(); break;
			}

			PopStyleVar();
			PopStyleColor();
		} EndChild();
		PopStyleColor();
		PopStyleVar();

		// End
		End();
	}
	PopStyleVar();

	// Title Text
	if (Vars::Menu::CheatName.Value != "")
	{
		PushFont(FontLarge);
		const auto titleSize = CalcTextSize(Vars::Menu::CheatName.Value.c_str());
		SetNextWindowSize({ std::min(titleSize.x + 26.f, mainWindowSize.x), 40.f });
		SetNextWindowPos({ mainWindowPos.x, mainWindowPos.y - 48.f });
		PushStyleVar(ImGuiStyleVar_WindowMinSize, { 40.f, 40.f });
		if (Begin("TitleWindow", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing))
		{
			const auto windowPos = GetWindowPos();
			GetWindowDrawList()->AddText(FontLarge, FontLarge->FontSize, { windowPos.x + 13.f, windowPos.y + 10.f }, Accent, Vars::Menu::CheatName.Value.c_str());

			End();
		}
		PopStyleVar();
		PopFont();
	}
}

#pragma region Tabs
/* Tab: Aimbot */
void CMenu::MenuAimbot()
{
	using namespace ImGui;

	switch (CurrentAimbotTab)
	{
	// General
	case 0:
		if (BeginTable("AimbotTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Global"))
			{
				FColorPicker("Aimbot FOV circle", &Vars::Colors::FOVCircle.Value);
				FToggle("Aimbot", &Vars::Aimbot::Global::Active.Value);
				FToggle("Autoshoot", &Vars::Aimbot::Global::AutoShoot.Value, FToggle_Middle);
				FKeybind("Aimbot key", Vars::Aimbot::Global::AimKey.Value);
				FDropdown("Target", &Vars::Aimbot::Global::AimAt.Value, { "Players", "Sentries", "Dispensers", "Teleporters", "Stickies", "NPCs", "Bombs" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Ignore", &Vars::Aimbot::Global::IgnoreOptions.Value, { "Invulnerable", "Cloaked", "Dead Ringer", "Friends", "Taunting", "Vaccinator", "Unsimulated Players", "Disguised" }, {}, FDropdown_Multi | FDropdown_Right);
				bTransparent = !(Vars::Aimbot::Global::IgnoreOptions.Value & 1 << 1);
					FSlider("Ignore cloak", &Vars::Aimbot::Global::IgnoreCloakPercentage.Value, 0, 100, 10, "%d%%", FSlider_Clamp);
				bTransparent = false;
				FSlider("Max targets", &Vars::Aimbot::Global::MaxTargets.Value, 1, 6, 1, "%d", FSlider_Clamp);
				bTransparent = !(Vars::Aimbot::Global::IgnoreOptions.Value & 1 << 6);
					FSlider("Tick tolerance", &Vars::Aimbot::Global::TickTolerance.Value, 0, 21, 1, "%d", FSlider_Clamp);
				bTransparent = false;
			} EndSection();
			if (Section("Crits"))
			{
				FToggle("Crit hack", &Vars::CritHack::Active.Value);
				FToggle("Avoid random", &Vars::CritHack::AvoidRandom.Value);
				FToggle("Always melee", &Vars::CritHack::AlwaysMelee.Value, FToggle_Middle);
				FKeybind("Crit key", Vars::CritHack::CritKey.Value);
			} EndSection();
			if (Section("Backtrack"))
			{
				FToggle("Active", &Vars::Backtrack::Enabled.Value);
				FDropdown("Method", &Vars::Backtrack::Method.Value, { "All", "Last", "Prefer OnShot" });
				FSlider("Fake latency", &Vars::Backtrack::Latency.Value, 0, g_ConVars.sv_maxunlag->GetFloat() * 1000 - 200, 5, "%d", FSlider_Clamp);
				FSlider("Fake interp", &Vars::Backtrack::Interp.Value, 0, g_ConVars.sv_maxunlag->GetFloat() * 1000 - 200, 5, "%d", FSlider_Clamp);
				FSlider("Window", &Vars::Backtrack::Window.Value, 1, 200, 5, "%d", FSlider_Clamp);
				FToggle("Unchoke prediction", &Vars::Backtrack::UnchokePrediction.Value);
				FToggle("Allow forward tracking", &Vars::Backtrack::AllowForward.Value, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## backtrack"))
				{
					FSlider("passthrough offset", &Vars::Backtrack::PassthroughOffset.Value, -5, 5);
					FSlider("tickset offset", &Vars::Backtrack::TicksetOffset.Value, -5, 5);
					FSlider("choke pass mod", &Vars::Backtrack::ChokePassMod.Value, -1, 1);
					FSlider("choke set mod", &Vars::Backtrack::ChokeSetMod.Value, -1, 1);
					FSlider("new window sub", &Vars::Backtrack::NWindowSub.Value, 0, 200, 5);
					FSlider("old window sub", &Vars::Backtrack::OWindowSub.Value, 0, 200, 5);
				} EndSection();
			}

			/* Column 2 */ // reduce this column some
			TableNextColumn();
			if (Section("Hitscan"))
			{
				FToggle("Active", &Vars::Aimbot::Hitscan::Active.Value);
				FSlider("Aim FOV## Hitscan", &Vars::Aimbot::Hitscan::AimFOV.Value, 1.f, 180.f, 1.f, "%.0f", FSlider_Right | FSlider_Clamp);
				FDropdown("Aim type", &Vars::Aimbot::Hitscan::AimMethod.Value, { "Plain", "Smooth", "Silent" }, {}, FDropdown_Left);
				FDropdown("Target selection", &Vars::Aimbot::Hitscan::SortMethod.Value, { "FOV", "Distance" }, {}, FDropdown_Right);
				FDropdown("Hitboxes", &Vars::Aimbot::Hitscan::Hitboxes.Value, { "Head", "Body", "Pelvis", "Arms", "Legs" }, { 1 << 0, 1 << 2, 1 << 1, 1 << 3, 1 << 4 }, FDropdown_Multi);
				FDropdown("Modifiers", &Vars::Aimbot::Hitscan::Modifiers.Value, { "Tapfire", "Wait for heatshot", "Wait for charge", "Scoped only", "Auto scope", "Bodyaim if lethal", "Extinguish team" }, {}, FDropdown_Multi);
				bTransparent = Vars::Aimbot::Hitscan::AimMethod.Value != 1;
					FSlider("Smooth factor## Hitscan", &Vars::Aimbot::Hitscan::SmoothingAmount.Value, 0, 100, 1, "%d", FSlider_Clamp);
				bTransparent = false;
				FSlider("Point scale", &Vars::Aimbot::Hitscan::PointScale.Value, 0, 100, 5, "%d%%", FSlider_Clamp | FSlider_Precision);
				bTransparent = !(Vars::Aimbot::Hitscan::Modifiers.Value & 1 << 0);
					FSlider("Tapfire distance", &Vars::Aimbot::Hitscan::TapFireDist.Value, 250.f, 1000.f, 50.f, "%.0f", FSlider_Clamp | FSlider_Precision);
				bTransparent = false;
			} EndSection();
			if (Section("Projectile"))
			{
				FToggle("Active", &Vars::Aimbot::Projectile::Active.Value);
				FSlider("Aim FOV## Projectile", &Vars::Aimbot::Projectile::AimFOV.Value, 1.f, 180.f, 1.f, "%1.f", FSlider_Right | FSlider_Clamp);
				FDropdown("Aim type", &Vars::Aimbot::Projectile::AimMethod.Value, { "Plain", "Smooth", "Silent" }, {}, FDropdown_Left);
				FDropdown("Target selection", &Vars::Aimbot::Projectile::SortMethod.Value, { "FOV", "Distance" }, {}, FDropdown_Right);
				FDropdown("Predict", &Vars::Aimbot::Projectile::StrafePrediction.Value, { "Air strafing", "Ground strafing" }, {}, FDropdown_Multi);
				bTransparent = Vars::Aimbot::Projectile::AimMethod.Value != 1;
					FSlider("Smooth factor## Projectile", &Vars::Aimbot::Projectile::SmoothingAmount.Value, 0, 100, 1, "%d", FSlider_Clamp);
				bTransparent = false;
				FSlider("Hit chance", &Vars::Aimbot::Projectile::StrafePredictionHitchance.Value, 0, 100, 5, "%d%%", FSlider_Clamp | FSlider_Precision);
				FSlider("Max simulation time", &Vars::Aimbot::Projectile::PredictionTime.Value, 0.1f, 10.f, 0.1f, "%.1fs");
				FSlider("Auto release", &Vars::Aimbot::Projectile::AutoRelease.Value, 0, 100, 5, "%d%%", FSlider_Clamp | FSlider_Precision);
				FToggle("Splash prediction", &Vars::Aimbot::Projectile::SplashPrediction.Value);
				FToggle("Charge loose cannon", &Vars::Aimbot::Projectile::ChargeLooseCannon.Value, FToggle_Middle);
				FToggle("No spread", &Vars::Aimbot::Projectile::NoSpread.Value);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## projectile"))
				{
					FSlider("ground samples", &Vars::Aimbot::Projectile::iGroundSamples.Value, 3, 66);
					FSlider("air samples", &Vars::Aimbot::Projectile::iAirSamples.Value, 3, 66);
					FSlider("vert shift", &Vars::Aimbot::Projectile::VerticalShift.Value, 0.f, 20.f, 0.5f, "%.1f");
					FSlider("latency offset", &Vars::Aimbot::Projectile::LatOff.Value, -3.f, 3.f, 0.1f, "%.1f");
					FSlider("physic offset", &Vars::Aimbot::Projectile::PhyOff.Value, -3.f, 3.f, 0.1f, "%.1f");
					FDropdown("hunterman mode", &Vars::Aimbot::Projectile::HuntermanMode.Value, { "center", "shift head", "shift up", "from top", "lerp to top" });
					if (Vars::Aimbot::Projectile::HuntermanMode.Value == 1 || Vars::Aimbot::Projectile::HuntermanMode.Value == 2)
						FSlider("hunterman shift", &Vars::Aimbot::Projectile::HuntermanShift.Value, 0.f, 10.f, 0.5f, "%.1f");
					if (Vars::Aimbot::Projectile::HuntermanMode.Value == 4)
						FSlider("hunterman lerp", &Vars::Aimbot::Projectile::HuntermanLerp.Value, 0.f, 100.f, 1.f, "%.0f%%");
					FToggle("strafe rate check", &Vars::Aimbot::Projectile::StrafeRate.Value);
					FToggle("strafe adjustments", &Vars::Aimbot::Projectile::StrafeAdjust.Value);
				} EndSection();
			}
			if (Section("Melee"))
			{
				FToggle("Active", &Vars::Aimbot::Melee::Active.Value);
				FSlider("Aim FOV## Melee", &Vars::Aimbot::Melee::AimFOV.Value, 1.f, 180.f, 1.f, "%1.f", FSlider_Right | FSlider_Clamp);
				FDropdown("Aim type", &Vars::Aimbot::Melee::AimMethod.Value, { "Plain", "Smooth", "Silent" });
				bTransparent = Vars::Aimbot::Melee::AimMethod.Value != 1;
					FSlider("Smooth factor## Melee", &Vars::Aimbot::Melee::SmoothingAmount.Value, 0, 100, 1, "%d", FSlider_Clamp);
				bTransparent = false;
				FToggle("Auto backstab", &Vars::Aimbot::Melee::AutoBackstab.Value);
				FToggle("Ignore razorback", &Vars::Aimbot::Melee::IgnoreRazorback.Value, FToggle_Middle); // this does not work, pPlayer->GetWeaponFromSlot(i) only returns equippables
				FToggle("Swing prediction", &Vars::Aimbot::Melee::SwingPrediction.Value);
				FToggle("Whip teammates", &Vars::Aimbot::Melee::WhipTeam.Value, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## melee"))
					FSlider("swing ticks", &Vars::Aimbot::Melee::SwingTicks.Value, 10, 14);
				EndSection();
			}

			EndTable();
		}
		break;
	// HvH
	case 1:
		if (BeginTable("HvHTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Doubletap"))
			{
				const int iVar = g_ConVars.sv_maxusrcmdprocessticks->GetInt();
				const int iTicks = iVar ? iVar : 24;

				FToggle("Enabled", &Vars::CL_Move::DoubleTap::Enabled.Value);
				FSlider("Tick limit", &Vars::CL_Move::DoubleTap::TickLimit.Value, 1, iTicks, 1, "%d", FSlider_Clamp);
				FSlider("Warp rate", &Vars::CL_Move::DoubleTap::WarpRate.Value, 1, iTicks, 1, "%d", FSlider_Clamp);
				FSlider("Passive recharge", &Vars::CL_Move::DoubleTap::PassiveRecharge.Value, 0, iTicks, 1, "%d", FSlider_Clamp);
				FDropdown("Mode", &Vars::CL_Move::DoubleTap::Mode.Value, { "Always", "Hold", "Toggle" });
				bTransparent = Vars::CL_Move::DoubleTap::Mode.Value == 0;
					FKeybind("Doubletap key", Vars::CL_Move::DoubleTap::DoubletapKey.Value);
				bTransparent = false;
				FKeybind("Recharge key", Vars::CL_Move::DoubleTap::RechargeKey.Value);
				FKeybind("Teleport key", Vars::CL_Move::DoubleTap::TeleportKey.Value);
				FDropdown("Options", &Vars::CL_Move::DoubleTap::Options.Value, { "Anti-warp", "Avoid airborne", "Auto retain", "Auto Recharge", "Recharge While Dead" }, {}, FDropdown_Multi);
			} EndSection();
			if (Section("Fakelag"))
			{
				FToggle("Enabled", &Vars::CL_Move::FakeLag::Enabled.Value);
				FDropdown("Mode", &Vars::CL_Move::FakeLag::Mode.Value, { "Always", "Hold", "Toggle" });
				bTransparent = Vars::CL_Move::FakeLag::Mode.Value == 0;
					FKeybind("Fakelag key", Vars::CL_Move::FakeLag::Key.Value);
				bTransparent = false;
				FDropdown("Type", &Vars::CL_Move::FakeLag::Type.Value, { "Plain", "Random", "Adaptive" });
				FDropdown("Options", &Vars::CL_Move::FakeLag::Options.Value, { "While Moving", "While Unducking", "While Airborne" }, {}, FDropdown_Multi);
				bTransparent = Vars::CL_Move::FakeLag::Type.Value != 0;
					FSlider("Plain value", &Vars::CL_Move::FakeLag::Value.Value, 1, 22, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = Vars::CL_Move::FakeLag::Type.Value != 1;
					FSlider("Random value", &Vars::CL_Move::FakeLag::Min.Value, &Vars::CL_Move::FakeLag::Max.Value, 1, 22, 1, "%d - %d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
				FToggle("Unchoke on attack", &Vars::CL_Move::FakeLag::UnchokeOnAttack.Value);
				FToggle("Retain blastjump", &Vars::CL_Move::FakeLag::RetainBlastJump.Value, FToggle_Middle);
			} EndSection();

			if (Section("Speedhack"))
			{
				FToggle("Speedhack", &Vars::CL_Move::SpeedEnabled.Value);
				bTransparent = !Vars::CL_Move::SpeedEnabled.Value;
					FSlider("SpeedHack factor", &Vars::CL_Move::SpeedFactor.Value, 1, 50, 1);
				bTransparent = false;
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Anti Aim"))
			{
				FToggle("Active", &Vars::AntiHack::AntiAim::Active.Value);
				FKeybind("Anti-aim Key", Vars::AntiHack::AntiAim::ToggleKey.Value);
				FDropdown("Real pitch", &Vars::AntiHack::AntiAim::PitchReal.Value, { "None", "Up", "Down", "Zero" }, {}, FDropdown_Left);
				FDropdown("Fake pitch", &Vars::AntiHack::AntiAim::PitchFake.Value, { "None", "Up", "Down" }, {}, FDropdown_Right);
				FDropdown("Real yaw", &Vars::AntiHack::AntiAim::YawReal.Value, { "None", "Forward", "Left", "Right", "Backwards", "Spin", "Edge" }, {}, FDropdown_Left);
				FDropdown("Fake yaw", &Vars::AntiHack::AntiAim::YawFake.Value, { "None", "Forward", "Left", "Right", "Backwards", "Spin", "Edge" }, {}, FDropdown_Right);
				FDropdown("Real offset", &Vars::AntiHack::AntiAim::RealYawMode.Value, { "View", "Target" }, {}, FDropdown_Left);
				FDropdown("Fake offset", &Vars::AntiHack::AntiAim::FakeYawMode.Value, { "View", "Target" }, {}, FDropdown_Right);
				FSlider("Real offset## Offset", &Vars::AntiHack::AntiAim::RealYawOffset.Value, -180, 180, 5, "%d", FSlider_Left | FSlider_Clamp | FSlider_Precision);
				FSlider("Fake offset## Offset", &Vars::AntiHack::AntiAim::FakeYawOffset.Value, -180, 180, 5, "%d", FSlider_Right | FSlider_Clamp | FSlider_Precision);
				bTransparent = Vars::AntiHack::AntiAim::YawFake.Value != 5 && Vars::AntiHack::AntiAim::YawReal.Value != 5;
					FSlider("Spin Speed", &Vars::AntiHack::AntiAim::SpinSpeed.Value, -30.f, 30.f, 1.f, "%.0f", FSlider_Left);
				bTransparent = false;
				FToggle("Anti-overlap", &Vars::AntiHack::AntiAim::AntiOverlap.Value, FToggle_Middle);
				Dummy({ 0, 8 });
				//FToggle("Hide pitch on shot", &Vars::AntiHack::AntiAim::InvalidShootPitch.Value);
			} EndSection();
			if (Section("Auto Peek"))
			{
				FKeybind("Autopeek Key", Vars::CL_Move::AutoPeekKey.Value);
			} EndSection();
			if (Section("Cheater Detection"))
			{
				FToggle("Enabled", &Vars::CheaterDetection::Enabled.Value);
				bTransparent = !Vars::CheaterDetection::Enabled.Value;
					FDropdown("Detection methods", &Vars::CheaterDetection::Methods.Value, { "Accuracy", "Score", "Simtime Changes", "Packet Choking", "Bunnyhopping", "Aim Flicking", "OOB Angles", "Aimbot", "Duck Speed" }, {}, FDropdown_Multi);
					FDropdown("Ignore conditions", &Vars::CheaterDetection::Protections.Value, { "Double Scans", "Lagging Client", "Timing Out" }, {}, FDropdown_Multi);
					FSlider("Suspicion gate", &Vars::CheaterDetection::SuspicionGate.Value, 5, 50, 1);

					bTransparent = !(Vars::CheaterDetection::Methods.Value & 1 << 1) || bTransparent;
						FSlider("High score multiplier", &Vars::CheaterDetection::ScoreMultiplier.Value, 1.5f, 4.f, 0.1f, "%.1f");
					bTransparent = !Vars::CheaterDetection::Enabled.Value;

					bTransparent = !(Vars::CheaterDetection::Methods.Value & (1 << 3 | 1 << 2)) || bTransparent;
						FSlider("Packet manipulation gate", &Vars::CheaterDetection::PacketManipGate.Value, 1, 22, 1);
					bTransparent = !Vars::CheaterDetection::Enabled.Value;

					bTransparent = !(Vars::CheaterDetection::Methods.Value & 1 << 4) || bTransparent;
						FSlider("Bunnyhop sensitivity", &Vars::CheaterDetection::BHopMaxDelay.Value, 1, 5, 1, "%d", FSlider_Left);
						FSlider("Minimum detections", &Vars::CheaterDetection::BHopDetectionsRequired.Value, 2, 15, 1, "%d", FSlider_Right);
					bTransparent = !Vars::CheaterDetection::Enabled.Value;

					bTransparent = !(Vars::CheaterDetection::Methods.Value & 1 << 5) || bTransparent;
						FSlider("Minimum aim-flick", &Vars::CheaterDetection::MinimumFlickDistance.Value, 5.f, 30.f, 0.1f, "%.1f", FSlider_Left);
						FSlider("Maximum noise", &Vars::CheaterDetection::MaximumNoise.Value, 5.f, 15.f, 0.1f, "%.1f", FSlider_Right);
					bTransparent = !Vars::CheaterDetection::Enabled.Value;

					bTransparent = !(Vars::CheaterDetection::Methods.Value & 1 << 7) || bTransparent;
						FSlider("Maximum aimbot FOV", &Vars::CheaterDetection::MaxScaledAimbotFoV.Value, 5.f, 30.f, 0.1f, "%.1f", FSlider_Left);
						FSlider("Minimum aimbot FOV", &Vars::CheaterDetection::MinimumAimbotFoV.Value, 5.f, 30.f, 0.1f, "%.1f", FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Resolver"))
			{
				FToggle("Enabled", &Vars::AntiHack::Resolver::Resolver.Value);
				bTransparent = !Vars::AntiHack::Resolver::Resolver.Value;
					FToggle("Ignore in-air", &Vars::AntiHack::Resolver::IgnoreAirborne.Value, FToggle_Middle);
				bTransparent = false;
			} EndSection();

			EndTable();
		}
		break;
	// Trigger
	case 2:
		if (BeginTable("TriggerTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Global"))
			{
				FToggle("Active", &Vars::Auto::Global::Active.Value);
				FKeybind("Trigger key", Vars::Auto::Global::AutoKey.Value);
				FDropdown("Ignore", &Vars::Auto::Global::IgnoreOptions.Value, { "Invulnerable", "Cloaked", "Taunting", "Unsimulated Players", "Disguised" }, {}, FDropdown_Multi);
			} EndSection();
			if (Section("Autouber"))
			{
				FToggle("Active", &Vars::Auto::Uber::Active.Value);
				FToggle("Only uber friends", &Vars::Auto::Uber::OnlyFriends.Value);
				FToggle("Preserve self", &Vars::Auto::Uber::PopLocal.Value);
				FDropdown("Auto vaccinator", &Vars::Auto::Uber::AutoVaccinator.Value, { "Bullet", "Blast", "Fire" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Bullet react classes", &Vars::Auto::Uber::ReactClasses.Value, { "Scout", "Soldier", "Pyro", "Heavy", "Engineer", "Sniper", "Spy" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 4, 1 << 5, 1 << 7, 1 << 8 }, FDropdown_Multi | FDropdown_Right);
				FSlider("Health left", &Vars::Auto::Uber::HealthLeft.Value, 1.f, 99.f, 1.f, "%.0f%%", FSlider_Clamp);
				FSlider("Reaction FOV", &Vars::Auto::Uber::ReactFOV.Value, 0, 90, 1, "%d", FSlider_Clamp);
				FToggle("Activate charge trigger", &Vars::Auto::Uber::VoiceCommand.Value);
			} EndSection();
			if (Section("Auto Jump"))
			{
				FKeybind("Auto rocket jump", Vars::Auto::Jump::JumpKey.Value);
				FKeybind("Auto CTap", Vars::Auto::Jump::CTapKey.Value);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug"))
				{
					FSlider("timing offset", &Vars::Auto::Jump::TimingOffset.Value, -1, 1);
					FSlider("apply timing offset above", &Vars::Auto::Jump::ApplyAbove.Value, 0, 8);
				}
				EndSection();
			}

			/* Column 2 */
			TableNextColumn();
			if (Section("Auto Detonate"))
			{
				FToggle("Active", &Vars::Auto::Detonate::Active.Value);
				FDropdown("Target", &Vars::Auto::Detonate::DetonateTargets.Value, { "Players", "Sentry", "Dispenser", "Teleporter", "NPCs", "Bombs", "Stickies" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6 }, FDropdown_Multi);
				FToggle("Stickies", &Vars::Auto::Detonate::Stickies.Value);
				FToggle("Flares", &Vars::Auto::Detonate::Flares.Value, FToggle_Middle);
				FSlider("Detonation radius", &Vars::Auto::Detonate::RadiusScale.Value, 0, 100, 1, "%d%%", FSlider_Clamp);
			} EndSection();
			if (Section("Autoblast"))
			{
				FToggle("Active", &Vars::Auto::Airblast::Active.Value);
				FToggle("Rage", &Vars::Auto::Airblast::Rage.Value);
				FToggle("Silent", &Vars::Auto::Airblast::Silent.Value);
				FToggle("Extinguish players", &Vars::Auto::Airblast::ExtinguishPlayers.Value);
				FToggle("Disable on attack", &Vars::Auto::Airblast::DisableOnAttack.Value);
				FSlider("FOV", &Vars::Auto::Airblast::Fov.Value, 0, 90, 1, "%d", FSlider_Clamp);
			} EndSection();

			EndTable();
		}
	}
}

/* Tab: Visuals */
void CMenu::MenuVisuals()
{
	using namespace ImGui;

	switch (CurrentVisualsTab)
	{
	// ESP
	case 0:
		if (BeginTable("VisualsESPTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("ESP"))
			{
				FDropdown("Draw", &Vars::ESP::Draw.Value, { "Enemy", "Team", "Friends", "Local", "NPCs", "Health", "Ammo", "Bombs", "Spellbook", "Gargoyle" }, {}, FDropdown_Multi);
				FDropdown("Player", &Vars::ESP::Player.Value, { "Name", "Health bar", "Health text", "Uber bar", "Uber text", "Class icon", "Class text", "Weapon icon", "Weapon text", "Distance", "Box", "Bones", "Priority", "Labels", "Buffs", "Debuffs", "Misc", "Lag compensation", "Ping", "KDR" }, {}, FDropdown_Multi);
				FDropdown("Building", &Vars::ESP::Building.Value, { "Name", "Health bar", "Health text", "Distance", "Box", "Owner", "Level", "Conditions" }, {}, FDropdown_Multi);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Colors"))
			{
				FToggle("Relative colors", &Vars::Colors::Relative.Value);
				if (Vars::Colors::Relative.Value)
				{
					FColorPicker("Enemy color", &Vars::Colors::Enemy.Value, 0, FColorPicker_Left);
					FColorPicker("Team color", &Vars::Colors::Team.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);
				}
				else
				{
					FColorPicker("RED color", &Vars::Colors::TeamRed.Value, 0, FColorPicker_Left);
					FColorPicker("BLU color", &Vars::Colors::TeamBlu.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);
				}

				FColorPicker("Health bar top", &Vars::Colors::HealthBar.Value.EndColor, 0, FColorPicker_Left);
				FColorPicker("Health bar bottom", &Vars::Colors::HealthBar.Value.StartColor, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Uber bar", &Vars::Colors::UberBar.Value, 0, FColorPicker_Left);
				FColorPicker("Invulnerable color", &Vars::Colors::Invulnerable.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Overheal color", &Vars::Colors::Overheal.Value, 0, FColorPicker_Left);
				FColorPicker("Cloaked color", &Vars::Colors::Cloak.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Local color", &Vars::Colors::Local.Value, 0, FColorPicker_Left);
				FColorPicker("Target color", &Vars::Colors::Target.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);

				FColorPicker("Healthpack color", &Vars::Colors::Health.Value, 0, FColorPicker_Left);
				FColorPicker("Ammopack color", &Vars::Colors::Ammo.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("NPC color", &Vars::Colors::NPC.Value, 0, FColorPicker_Left);
				FColorPicker("Bomb color", &Vars::Colors::Bomb.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);
				FColorPicker("Spellbook color", &Vars::Colors::Spellbook.Value, 0, FColorPicker_Left);
				FColorPicker("Gargoyle color", &Vars::Colors::Gargoyle.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);

				FSlider("Active alpha", &Vars::ESP::ActiveAlpha.Value, 0, 255, 5, "%d", FSlider_Clamp);
				FSlider("Dormant alpha", &Vars::ESP::DormantAlpha.Value, 0, 255, 5, "%d", FSlider_Clamp);
				FSlider("Dormant Decay Time", &Vars::ESP::DormantTime.Value, 0.015f, 5.0f, 0.1f, "%.1f", FSlider_Left | FSlider_Clamp);
				FToggle("Dormant priority only", &Vars::ESP::DormantPriority.Value, FToggle_Middle); Dummy({ 0, 8 });
			} EndSection();

			EndTable();
		}
		break;
	// Chams
	case 1:
		if (BeginTable("VisualsChamsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Friendly"))
			{
				FToggle("Players", &Vars::Chams::Friendly::Players.Value);
				FToggle("Ragdolls", &Vars::Chams::Friendly::Ragdolls.Value, FToggle_Middle);
				FToggle("Buildings", &Vars::Chams::Friendly::Buildings.Value);
				FToggle("Projectiles", &Vars::Chams::Friendly::Projectiles.Value, FToggle_Middle);

				FMDropdown("Visible material", &Vars::Chams::Friendly::Chams.Value.VisibleMaterial, FSDropdown_Left, 1);
				FColorPicker("Visible color", &Vars::Chams::Friendly::Chams.Value.VisibleColor, 0, FColorPicker_Dropdown);
				FMDropdown("Occluded material", &Vars::Chams::Friendly::Chams.Value.OccludedMaterial, FSDropdown_Right, 1);
				FColorPicker("Occluded color", &Vars::Chams::Friendly::Chams.Value.OccludedColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("Enemy"))
			{
				FToggle("Players", &Vars::Chams::Enemy::Players.Value);
				FToggle("Ragdolls", &Vars::Chams::Enemy::Ragdolls.Value, FToggle_Middle);
				FToggle("Buildings", &Vars::Chams::Enemy::Buildings.Value);
				FToggle("Projectiles", &Vars::Chams::Enemy::Projectiles.Value, FToggle_Middle);

				FMDropdown("Visible material", &Vars::Chams::Enemy::Chams.Value.VisibleMaterial, FSDropdown_Left, 1);
				FColorPicker("Visible color", &Vars::Chams::Enemy::Chams.Value.VisibleColor, 0, FColorPicker_Dropdown);
				FMDropdown("Occluded material", &Vars::Chams::Enemy::Chams.Value.OccludedMaterial, FSDropdown_Right, 1);
				FColorPicker("Occluded color", &Vars::Chams::Enemy::Chams.Value.OccludedColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("World"))
			{
				FToggle("NPCs", &Vars::Chams::World::NPCs.Value);
				FToggle("Pickups", &Vars::Chams::World::Pickups.Value, FToggle_Middle);
				FToggle("Bombs", &Vars::Chams::World::Bombs.Value);

				FMDropdown("Visible material", &Vars::Chams::World::Chams.Value.VisibleMaterial, FSDropdown_Left, 1);
				FColorPicker("Visible color", &Vars::Chams::World::Chams.Value.VisibleColor, 0, FColorPicker_Dropdown);
				FMDropdown("Occluded material", &Vars::Chams::World::Chams.Value.OccludedMaterial, FSDropdown_Right, 1);
				FColorPicker("Occluded color", &Vars::Chams::World::Chams.Value.OccludedColor, 0, FColorPicker_Dropdown);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Backtrack"))
			{
				FToggle("Active", &Vars::Chams::Backtrack::Active.Value);
				SameLine(GetWindowSize().x / 2 + 4); SetCursorPosY(GetCursorPosY() - 24);
				FDropdown("Draw", &Vars::Chams::Backtrack::Draw.Value, { "Last", "Last + first", "All" }, {}, FDropdown_Left);

				FMDropdown("Material", &Vars::Chams::Backtrack::Chams.Value.VisibleMaterial, FSDropdown_None, 1);
				FColorPicker("Color", &Vars::Chams::Backtrack::Chams.Value.VisibleColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("Fake Angle"))
			{
				FToggle("Active", &Vars::Chams::FakeAngle::Active.Value);

				FMDropdown("Material", &Vars::Chams::FakeAngle::Chams.Value.VisibleMaterial, FSDropdown_None, 1);
				FColorPicker("Color", &Vars::Chams::FakeAngle::Chams.Value.VisibleColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Weapon", &Vars::Chams::Viewmodel::Weapon.Value);
				FToggle("Hands", &Vars::Chams::Viewmodel::Hands.Value, FToggle_Middle);

				FMDropdown("Material", &Vars::Chams::Viewmodel::Chams.Value.VisibleMaterial, FSDropdown_None, 1);
				FColorPicker("Color", &Vars::Chams::Viewmodel::Chams.Value.VisibleColor, 0, FColorPicker_Dropdown);
			} EndSection();

			EndTable();
		}
		break;
	// Glow
	case 2:
		if (BeginTable("VisualsGlowTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Friendly"))
			{
				FToggle("Players", &Vars::Glow::Friendly::Players.Value);
				FToggle("Ragdolls", &Vars::Glow::Friendly::Ragdolls.Value, FToggle_Middle);
				FToggle("Buildings", &Vars::Glow::Friendly::Buildings.Value);
				FToggle("Projectiles", &Vars::Glow::Friendly::Projectiles.Value, FToggle_Middle);
				Dummy({ 0, 8 });

				FToggle("Stencil", &Vars::Glow::Friendly::Glow.Value.Stencil);
				FToggle("Blur", &Vars::Glow::Friendly::Glow.Value.Blur, FToggle_Middle);
				bTransparent = !Vars::Glow::Friendly::Glow.Value.Stencil;
					FSlider("Stencil scale## Friendly", &Vars::Glow::Friendly::Glow.Value.StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !Vars::Glow::Friendly::Glow.Value.Blur;
					FSlider("Blur scale## Friendly", &Vars::Glow::Friendly::Glow.Value.BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Enemy"))
			{
				FToggle("Players", &Vars::Glow::Enemy::Players.Value);
				FToggle("Ragdolls", &Vars::Glow::Enemy::Ragdolls.Value, FToggle_Middle);
				FToggle("Buildings", &Vars::Glow::Enemy::Buildings.Value);
				FToggle("Projectiles", &Vars::Glow::Enemy::Projectiles.Value, FToggle_Middle);
				Dummy({ 0, 8 });

				FToggle("Stencil", &Vars::Glow::Enemy::Glow.Value.Stencil);
				FToggle("Blur", &Vars::Glow::Enemy::Glow.Value.Blur, FToggle_Middle);
				bTransparent = !Vars::Glow::Enemy::Glow.Value.Stencil;
					FSlider("Stencil scale## Enemy", &Vars::Glow::Enemy::Glow.Value.StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !Vars::Glow::Enemy::Glow.Value.Blur;
					FSlider("Blur scale## Enemy", &Vars::Glow::Enemy::Glow.Value.BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("World"))
			{
				FToggle("NPCs", &Vars::Glow::World::NPCs.Value);
				FToggle("Pickups", &Vars::Glow::World::Pickups.Value, FToggle_Middle);
				FToggle("Bombs", &Vars::Glow::World::Bombs.Value);
				Dummy({ 0, 8 });

				FToggle("Stencil", &Vars::Glow::World::Glow.Value.Stencil);
				FToggle("Blur", &Vars::Glow::World::Glow.Value.Blur, FToggle_Middle);
				bTransparent = !Vars::Glow::World::Glow.Value.Stencil;
					FSlider("Stencil scale## World", &Vars::Glow::World::Glow.Value.StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !Vars::Glow::World::Glow.Value.Blur;
					FSlider("Blur scale## World", &Vars::Glow::World::Glow.Value.BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Backtrack"))
			{
				FToggle("Active", &Vars::Glow::Backtrack::Active.Value);
				SameLine(GetWindowSize().x / 2 + 4); SetCursorPosY(GetCursorPosY() - 24);
				FDropdown("Draw", &Vars::Glow::Backtrack::Draw.Value, { "Last", "Last + first", "All" }, {}, FDropdown_Left);
				Dummy({ 0, 8 });

				FToggle("Stencil", &Vars::Glow::Backtrack::Glow.Value.Stencil);
				FToggle("Blur", &Vars::Glow::Backtrack::Glow.Value.Blur, FToggle_Middle);
				bTransparent = !Vars::Glow::Backtrack::Glow.Value.Stencil;
					FSlider("Stencil scale## Backtrack", &Vars::Glow::Backtrack::Glow.Value.StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !Vars::Glow::Backtrack::Glow.Value.Blur;
					FSlider("Blur scale## Backtrack", &Vars::Glow::Backtrack::Glow.Value.BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Fake Angle"))
			{
				FToggle("Active", &Vars::Glow::FakeAngle::Active.Value);
				Dummy({ 0, 8 });

				FToggle("Stencil", &Vars::Glow::FakeAngle::Glow.Value.Stencil);
				FToggle("Blur", &Vars::Glow::FakeAngle::Glow.Value.Blur, FToggle_Middle);
				bTransparent = !Vars::Glow::FakeAngle::Glow.Value.Stencil;
					FSlider("Stencil scale## FakeAngle", &Vars::Glow::FakeAngle::Glow.Value.StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !Vars::Glow::FakeAngle::Glow.Value.Blur;
					FSlider("Blur scale## FakeAngle", &Vars::Glow::FakeAngle::Glow.Value.BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Weapon", &Vars::Glow::Viewmodel::Weapon.Value);
				FToggle("Hands", &Vars::Glow::Viewmodel::Hands.Value, FToggle_Middle);
				Dummy({ 0, 8 });

				FToggle("Stencil", &Vars::Glow::Viewmodel::Glow.Value.Stencil);
				FToggle("Blur", &Vars::Glow::Viewmodel::Glow.Value.Blur, FToggle_Middle);
				bTransparent = !Vars::Glow::Viewmodel::Glow.Value.Stencil;
					FSlider("Stencil scale## Viewmodel", &Vars::Glow::Viewmodel::Glow.Value.StencilScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Left);
				bTransparent = !Vars::Glow::Viewmodel::Glow.Value.Blur;
					FSlider("Blur scale## Viewmodel", &Vars::Glow::Viewmodel::Glow.Value.BlurScale, 1, 10, 1, "%d", FSlider_Clamp | FSlider_Right);
				bTransparent = false;
			} EndSection();

			EndTable();
		}
		break;
	// Misc
	case 3:
		if (BeginTable("VisualsMiscTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Removals"))
			{
				FToggle("Scope", &Vars::Visuals::RemoveScope.Value);
				FToggle("Interpolation", &Vars::Visuals::RemoveInterpolation.Value, FToggle_Middle);
				FToggle("Disguises", &Vars::Visuals::RemoveDisguises.Value);
				FToggle("Screen overlays", &Vars::Visuals::RemoveScreenOverlays.Value, FToggle_Middle);
				FToggle("Taunts", &Vars::Visuals::RemoveTaunts.Value);
				FToggle("Screen effects", &Vars::Visuals::RemoveScreenEffects.Value, FToggle_Middle);
				FToggle("View punch", &Vars::Visuals::RemovePunch.Value);
				FToggle("Angle forcing", &Vars::Visuals::PreventForcedAngles.Value, FToggle_Middle);
				FToggle("MOTD", &Vars::Visuals::RemoveMOTD.Value);
				FToggle("Convar queries", &Vars::Visuals::RemoveConvarQueries.Value, FToggle_Middle);
				FToggle("Post processing", &Vars::Visuals::RemovePostProcessing.Value);
				FToggle("DSP", &Vars::Visuals::RemoveDSP.Value, FToggle_Middle);
			} EndSection();
			if (Section("UI"))
			{
				FSlider("Field of view", &Vars::Visuals::FieldOfView.Value, 0, 160, 1, "%d", FSlider_Clamp);
				FSlider("Zoomed field of view", &Vars::Visuals::ZoomFieldOfView.Value, 0, 160, 1, "%d", FSlider_Clamp);
				FToggle("Reveal scoreboard", &Vars::Visuals::RevealScoreboard.Value);
				FToggle("Scoreboard colors", &Vars::Visuals::ScoreboardColors.Value, FToggle_Middle);
				FToggle("Clean screenshots", &Vars::Visuals::CleanScreenshots.Value);
				FToggle("Scoreboard playerlist", &Vars::Visuals::ScoreboardPlayerlist.Value, FToggle_Middle);
				FToggle("Sniper sightlines", &Vars::Visuals::SniperSightlines.Value);
				FToggle("Pickup timers", &Vars::Visuals::PickupTimers.Value, FToggle_Middle);
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Crosshair aim position", &Vars::Visuals::CrosshairAimPos.Value);
				FToggle("Viewmodel aim position", &Vars::Visuals::AimbotViewmodel.Value, FToggle_Middle);
				FSlider("Offset X", &Vars::Visuals::VMOffsetX.Value, -45, 45, 5, "%d", FSlider_Clamp | FSlider_Precision);
				FSlider("Offset Y", &Vars::Visuals::VMOffsetY.Value, -45, 45, 5, "%d", FSlider_Clamp | FSlider_Precision);
				FSlider("Offset Z", &Vars::Visuals::VMOffsetZ.Value, -45, 45, 5, "%d", FSlider_Clamp | FSlider_Precision);
				FSlider("Roll", &Vars::Visuals::VMRoll.Value, -180, 180, 5, "%d", FSlider_Clamp | FSlider_Precision);
				FToggle("Sway", &Vars::Visuals::ViewmodelSway.Value);
				bTransparent = !Vars::Visuals::ViewmodelSway.Value;
					FSlider("Sway scale", &Vars::Visuals::ViewmodelSwayScale.Value, 0.01f, 5.f, 0.1f, "%.1f", FSlider_Left);
					FSlider("Sway interp", &Vars::Visuals::ViewmodelSwayInterp.Value, 0.01f, 1.f, 0.1f, "%.1f", FSlider_Right);
				bTransparent = false;
			} EndSection();
			if (Section("Tracers"))
			{
				FSDropdown("Bullet trail", &Vars::Visuals::Tracers::ParticleTracer.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP 2", "Big Nasty", "Distortion Trail", "Black Ink" }, FSDropdown_Custom | FSDropdown_Left);
				FSDropdown("Crit trail", &Vars::Visuals::Tracers::ParticleTracerCrits.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP 2", "Big Nasty", "Distortion Trail", "Black Ink" }, FSDropdown_Custom | FSDropdown_Right);
			} EndSection();
			if (Section("Ragdolls"))
			{
				FToggle("No ragdolls", &Vars::Visuals::Ragdolls::NoRagdolls.Value);
				FToggle("No gibs", &Vars::Visuals::Ragdolls::NoGib.Value, FToggle_Middle);
				FToggle("Mods", &Vars::Visuals::Ragdolls::Active.Value);
				bTransparent = !Vars::Visuals::Ragdolls::Active.Value;
					FToggle("Enemy only", &Vars::Visuals::Ragdolls::EnemyOnly.Value, FToggle_Middle);
					FDropdown("Ragdoll effects", &Vars::Visuals::Ragdolls::Effects.Value, { "Burning", "Electrocuted", "Ash", "Dissolve" }, {}, FDropdown_Multi | FDropdown_Left);
					FDropdown("Ragdoll model", &Vars::Visuals::Ragdolls::Type.Value, { "None", "Gold", "Ice" }, {}, FDropdown_Right);
					FSlider("Ragdoll force", &Vars::Visuals::Ragdolls::Force.Value, -10.f, 10.f, 0.5f, "%.1f", FSlider_Precision);
					FSlider("Horizontal force", &Vars::Visuals::Ragdolls::ForceHorizontal.Value, -10.f, 10.f, 0.5f, "%.1f", FSlider_Precision);
					FSlider("Vertical force", &Vars::Visuals::Ragdolls::ForceVertical.Value, -10.f, 10.f, 0.5f, "%.1f", FSlider_Precision);
				bTransparent = false;
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Bullet"))
			{
				FColorPicker("Bullet tracer color", &Vars::Colors::BulletTracer.Value);
				FToggle("Bullet tracers", &Vars::Visuals::BulletTracer.Value);
			} EndSection();
			if (Section("Simulation"))
			{
				FColorPicker("Prediction line color", &Vars::Colors::PredictionColor.Value, 1); FColorPicker("Projectile line color", &Vars::Colors::ProjectileColor.Value);
				FToggle("Enabled", &Vars::Visuals::SimLines.Value);
				FToggle("Timed", &Vars::Visuals::TimedLines.Value, FToggle_Middle);
				FToggle("Seperators", &Vars::Visuals::SimSeperators.Value);
				bTransparent = !Vars::Visuals::SimSeperators.Value;
					FSlider("Seperator length", &Vars::Visuals::SeperatorLength.Value, 2, 16, 1, "%d", FSlider_Left);
					FSlider("Seperator spacing", &Vars::Visuals::SeperatorSpacing.Value, 1, 16, 1, "%d", FSlider_Right);
				bTransparent = false;
				FColorPicker("Clipped line color", &Vars::Colors::ClippedColor.Value);
				FToggle("Projectile trajectory", &Vars::Visuals::ProjectileTrajectory.Value);
				FToggle("Projectile camera", &Vars::Visuals::ProjectileCamera.Value, FToggle_Middle);
				FToggle("Trajectory on shot", &Vars::Visuals::TrajectoryOnShot.Value);
				FToggle("Swing prediction lines", &Vars::Visuals::SwingLines.Value, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug"))
				{
					FToggle("overwrite", &Vars::Visuals::PTOverwrite.Value);
					FDropdown("type", &Vars::Visuals::PTType.Value,
						{ "none", "bullet", "rocket", "pipebomb", "pipebomb remote", "syringe", "flare", "jar", "arrow", "flame rocket", "jar milk",
						"healing bolt", "energy ball", "energy ring", "pipebomb practice", "cleaver", "sticky ball", "cannonball", "building repair bolt", "sentry rocket",
						"festive arrow", "throwable", "spell", "festive jar", "festive healing bolt", "breadmonster jarate", "breadmonster madmilk", "grappling hook", "bread monster", "jar gas",
						"balloffire" }); // to keep the integer easily associated with the enum, a lot of these aren't even used and are probably only used outside of simulation
					FSlider("off x", &Vars::Visuals::PTOffX.Value, -25.f, 25.f, 0.1f, "%.1f");
					FSlider("off y", &Vars::Visuals::PTOffY.Value, -25.f, 25.f, 0.1f, "%.1f");
					FSlider("off z", &Vars::Visuals::PTOffZ.Value, -25.f, 25.f, 0.1f, "%.1f");
					FToggle("pipes", &Vars::Visuals::PTPipes.Value);
					FSlider("hull", &Vars::Visuals::PTHull.Value, 0.f, 10.f, 0.1f, "%.1f");
					FSlider("speed", &Vars::Visuals::PTSpeed.Value, 0.f, 5000.f);
					FSlider("gravity", &Vars::Visuals::PTGravity.Value, 0.f, 2.f, 0.1f, "%.1f");
					FToggle("no spin", &Vars::Visuals::PTNoSpin.Value);
					FSlider("lifetime", &Vars::Visuals::PTLifeTime.Value, 0.f, 10.f, 0.1f, "%.1f");
					FSlider("up vel", &Vars::Visuals::PTUpVelocity.Value, 0.f, 1000.f);
					FSlider("ang vel x", &Vars::Visuals::PTAngVelocityX.Value, -1000.f, 1000.f);
					FSlider("ang vel y", &Vars::Visuals::PTAngVelocityY.Value, -1000.f, 1000.f);
					FSlider("ang vel z", &Vars::Visuals::PTAngVelocityZ.Value, -1000.f, 1000.f);
					FSlider("drag", &Vars::Visuals::PTDrag.Value, 0.f, 2.f, 0.1f, "%.1f");
					FSlider("drag x", &Vars::Visuals::PTDragBasisX.Value, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("drag y", &Vars::Visuals::PTDragBasisY.Value, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("drag z", &Vars::Visuals::PTDragBasisZ.Value, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("ang drag x", &Vars::Visuals::PTAngDragBasisX.Value, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("ang drag y", &Vars::Visuals::PTAngDragBasisY.Value, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
					FSlider("ang drag z", &Vars::Visuals::PTAngDragBasisZ.Value, 0.f, 0.1f, 0.01f, "%.2f", FSlider_Precision);
				} EndSection();
			}
			if (Section("Hitbox"))
			{
				FColorPicker("Edge color", &Vars::Colors::HitboxEdge.Value, 1); FColorPicker("Face color", &Vars::Colors::HitboxFace.Value);
				FToggle("Draw Hitboxes", &Vars::Visuals::ShowHitboxes.Value);
			} EndSection();
			if (Section("Thirdperson"))
			{
				FToggle("Thirdperson", &Vars::Visuals::ThirdPerson::Active.Value);
				FKeybind("Thirdperson key", Vars::Visuals::ThirdPerson::Key.Value);
				FSlider("Thirdperson distance", &Vars::Visuals::ThirdPerson::Distance.Value, 0.f, 500.f, 5.f, "%.0f", FSlider_Precision);
				FSlider("Thirdperson right", &Vars::Visuals::ThirdPerson::Right.Value, -500.f, 500.f, 5.f, "%.0f", FSlider_Precision);
				FSlider("Thirdperson up", &Vars::Visuals::ThirdPerson::Up.Value, -500.f, 500.f, 5.f, "%.0f", FSlider_Precision);
				FToggle("Thirdperson crosshair", &Vars::Visuals::ThirdPerson::Crosshair.Value);
			} EndSection();
			if (Section("Out of FOV arrows"))
			{
				FToggle("Active", &Vars::Visuals::Arrows::Active.Value);
				FSlider("Offset", &Vars::Visuals::Arrows::Offset.Value, 0, 500, 25, "%d", FSlider_Precision);
				FSlider("Max distance", &Vars::Visuals::Arrows::MaxDist.Value, 0.f, 5000.f, 50.f, "%.0f", FSlider_Precision);
			} EndSection();
			if (Section("World"))
			{
				FDropdown("Modulations", &Vars::Visuals::World::Modulations.Value, { "World", "Sky", "Prop", "Particle", "Fog" }, { }, FDropdown_Left | FDropdown_Multi);
				static std::vector skyNames = {
					"Off", "sky_tf2_04", "sky_upward", "sky_dustbowl_01", "sky_goldrush_01", "sky_granary_01", "sky_well_01", "sky_gravel_01", "sky_badlands_01",
					"sky_hydro_01", "sky_night_01", "sky_nightfall_01", "sky_trainyard_01", "sky_stormfront_01", "sky_morningsnow_01","sky_alpinestorm_01",
					"sky_harvest_01", "sky_harvest_night_01", "sky_halloween", "sky_halloween_night_01", "sky_halloween_night2014_01", "sky_island_01", "sky_rainbow_01"
				};
				FSDropdown("Skybox changer", &Vars::Visuals::World::SkyboxChanger.Value, skyNames, FSDropdown_Custom | FSDropdown_Right);
				bTransparent = !(Vars::Visuals::World::Modulations.Value & 1 << 0);
					bool bUpdate = FColorPicker("World modulation", &Vars::Colors::WorldModulation.Value, 0, FColorPicker_Left);
				bTransparent = !(Vars::Visuals::World::Modulations.Value & 1 << 1);
					bUpdate = FColorPicker("Sky modulation", &Vars::Colors::SkyModulation.Value, 0, FColorPicker_Middle | FColorPicker_SameLine) || bUpdate;
				bTransparent = !(Vars::Visuals::World::Modulations.Value & 1 << 2);
					bUpdate = FColorPicker("Prop modulation", &Vars::Colors::PropModulation.Value, 0, FColorPicker_Left) || bUpdate;
				bTransparent = !(Vars::Visuals::World::Modulations.Value & 1 << 3);
					FColorPicker("Particle modulation", &Vars::Colors::ParticleModulation.Value, 0, FColorPicker_Middle | FColorPicker_SameLine);
				bTransparent = !(Vars::Visuals::World::Modulations.Value & 1 << 4);
					FColorPicker("Fog modulation", &Vars::Colors::FogModulation.Value, 0, FColorPicker_Left);
				bTransparent = false;
				if (bUpdate)
					G::ShouldUpdateMaterialCache = true;
				FToggle("Near prop fade", &Vars::Visuals::World::NearPropFade.Value, FToggle_Middle);
				FToggle("Prop wireframe", &Vars::Visuals::World::PropWireframe.Value);
				FToggle("No prop fade", &Vars::Visuals::World::NoPropFade.Value, FToggle_Middle);
				FSDropdown("World texture", &Vars::Visuals::World::WorldTexture.Value, { "Default", "Dev", "Camo", "Black", "White", "Flat" }, FSDropdown_Custom);
			} EndSection();

			EndTable();
		}
		break;
	// Radar
	case 4:
		if (BeginTable("VisualsRadarTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Main"))
			{
				FToggle("Active", &Vars::Radar::Main::Active.Value);
				FSlider("Range", &Vars::Radar::Main::Range.Value, 50, 3000, 50, "%d", FSlider_Precision);
				FSlider("Background alpha", &Vars::Radar::Main::BackAlpha.Value, 0, 255, 1, "%d", FSlider_Clamp);
				FSlider("Line alpha", &Vars::Radar::Main::LineAlpha.Value, 0, 255, 1, "%d", FSlider_Clamp);
				FToggle("Draw out of range", &Vars::Radar::Main::AlwaysDraw.Value);
			} EndSection();
			if (Section("Player"))
			{
				FToggle("Active", &Vars::Radar::Players::Active.Value);
				FDropdown("Icon", &Vars::Radar::Players::IconType.Value, { "Scoreboard", "Portraits", "Avatar" }, {}, FDropdown_Left);
				FDropdown("Background", &Vars::Radar::Players::BackGroundType.Value, { "Off", "Rectangle", "Texture" }, {}, FDropdown_Right);
				FToggle("Outline", &Vars::Radar::Players::Outline.Value);
				FDropdown("Ignore teammates", &Vars::Radar::Players::IgnoreTeam.Value, { "Off", "All", "Keep friends" }, {}, FDropdown_Left);
				FDropdown("Ignore cloaked", &Vars::Radar::Players::IgnoreCloaked.Value, { "Off", "All", "Keep friends" }, {}, FDropdown_Right);
				FSlider("Icon size## Player", &Vars::Radar::Players::IconSize.Value, 12, 30);
				FToggle("Health bar", &Vars::Radar::Players::Health.Value);
				FToggle("Height indicator", &Vars::Radar::Players::Height.Value, FToggle_Middle);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Building"))
			{
				FToggle("Active", &Vars::Radar::Buildings::Active.Value);
				FToggle("Ignore team", &Vars::Radar::Buildings::IgnoreTeam.Value);
				FToggle("Outline", &Vars::Radar::Buildings::Outline.Value);
				FToggle("Health bar", &Vars::Radar::Buildings::Health.Value, FToggle_Middle);
				FSlider("Icon size## Building", &Vars::Radar::Buildings::IconSize.Value, 12, 30);
			} EndSection();
			if (Section("World"))
			{
				FToggle("Active", &Vars::Radar::World::Active.Value);
				FToggle("Health", &Vars::Radar::World::Health.Value);
				FToggle("Ammo", &Vars::Radar::World::Ammo.Value, FToggle_Middle);
				FSlider("Icon size## World", &Vars::Radar::World::IconSize.Value, 12, 30);
			} EndSection();

			EndTable();
		}
		break;
	// Menu
	case 5:
	{
		bool bUpdate = false;

		if (BeginTable("MenuTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("General"))
			{
				bUpdate = FColorPicker("Accent color", &Vars::Menu::Theme::Accent.Value, 0, FColorPicker_Left);
				bUpdate = FColorPicker("Foremost color", &Vars::Menu::Theme::Foremost.Value, 0, FColorPicker_Middle | FColorPicker_SameLine) || bUpdate;
				bUpdate = FColorPicker("Background color", &Vars::Menu::Theme::Background.Value, 0, FColorPicker_Left) || bUpdate;
				bUpdate = FColorPicker("Foreground color", &Vars::Menu::Theme::Foreground.Value, 0, FColorPicker_Middle | FColorPicker_SameLine) || bUpdate;
				bUpdate = FColorPicker("Active color", &Vars::Menu::Theme::Active.Value, 0, FColorPicker_Left) || bUpdate;
				bUpdate = FColorPicker("Inactive color", &Vars::Menu::Theme::Inactive.Value, 0, FColorPicker_Middle | FColorPicker_SameLine) || bUpdate;

				FSDropdown("Cheat title", &Vars::Menu::CheatName.Value, {}, FSDropdown_AutoUpdate | FSDropdown_Left);
				FSDropdown("Chat info prefix", &Vars::Menu::CheatPrefix.Value, {}, FSDropdown_Right);
				FKeybind("Menu key", Vars::Menu::MenuKey.Value, true);
			} EndSection();
			if (Section("Indicators"))
			{
				FDropdown("Indicators", &Vars::Menu::Indicators.Value, { "Ticks", "Crit hack", "Spectators", "Ping", "Conditions" }, {}, FDropdown_Multi);
				if (FSlider("DPI", &Vars::Menu::DPI.Value, 0.8f, 1.8f, 0.2f, "%.1f", FSlider_Precision))
				{
					g_Draw.RemakeFonts();
					for (const auto var : g_Vars)
					{
						if (var->m_iType == typeid(DragBox_t).hash_code())
							var->GetVar<DragBox_t>()->Value.update = true;
					}
				}
			} EndSection();

			/* Column 2 */
			TableNextColumn();

			EndTable();
		}

		if (bUpdate)
			LoadStyle();
	}
	}
}

/* Tab: Misc */
void CMenu::MenuMisc()
{
	using namespace ImGui;

	if (BeginTable("MiscTable", 2))
	{
		/* Column 1 */
		TableNextColumn();
		if (Section("Movement"))
		{
			FDropdown("Autostrafe", &Vars::Misc::AutoStrafe.Value, { "Off", "Legit", "Directional" });
			bTransparent = Vars::Misc::AutoStrafe.Value != 2;
				FToggle("Only on movement key", &Vars::Misc::DirectionalOnlyOnMove.Value);
				FToggle("Only on space", &Vars::Misc::DirectionalOnlyOnSpace.Value, FToggle_Middle);
			bTransparent = false;
			FToggle("Bunnyhop", &Vars::Misc::AutoJump.Value);
			FToggle("Auto jumpbug", &Vars::Misc::AutoJumpbug.Value, FToggle_Middle); // this is unreliable without setups, do not depend on it!
			FToggle("Fast stop", &Vars::Misc::FastStop.Value);
			FToggle("Fast accelerate", &Vars::Misc::FastAccel.Value, FToggle_Middle);
			FToggle("Fast strafe", &Vars::Misc::FastStrafe.Value);
			FToggle("No push", &Vars::Misc::NoPush.Value, FToggle_Middle);
			FToggle("Crouch speed", &Vars::Misc::CrouchSpeed.Value);
		} EndSection();
		if (Section("Exploits"))
		{
			FToggle("Cheats bypass", &Vars::Misc::CheatsBypass.Value);
			FToggle("Pure bypass", &Vars::Misc::BypassPure.Value, FToggle_Middle);
			FToggle("Ping reducer", &Vars::Misc::PingReducer.Value);
			bTransparent = !Vars::Misc::PingReducer.Value;
				FSlider("cl_cmdrate", &Vars::Misc::PingTarget.Value, 1, 66, 1, "%d", FSlider_Right | FSlider_Clamp);
			bTransparent = false;
			SetCursorPosY(GetCursorPosY() - 8);
			FToggle("Equip region unlock", &Vars::Misc::EquipRegionUnlock.Value);
		} EndSection();
		if (Vars::Debug::Info.Value)
		{
			if (Section("Convar spoofer"))
			{
				FSDropdown("Convar", &Vars::Misc::ConvarName.Value, {}, FSDropdown_Left);
				FSDropdown("Value", &Vars::Misc::ConvarValue.Value, {}, FSDropdown_Right);
				if (FButton("Send"))
				{
					CNetChannel* netChannel = I::EngineClient->GetNetChannelInfo();
					if (netChannel)
					{
						Utils::ConLog("Convar", std::format("Sent {} as {}", Vars::Misc::ConvarName.Value, Vars::Misc::ConvarValue.Value).c_str(), Vars::Menu::Theme::Accent.Value);
						NET_SetConVar cmd(Vars::Misc::ConvarName.Value.c_str(), Vars::Misc::ConvarValue.Value.c_str());
						netChannel->SendNetMsg(cmd);

						//Vars::Misc::ConvarName = "";
						//Vars::Misc::ConvarValue = "";
					}
				}
			} EndSection();
		}
		if (Section("Automation"))
		{
			FToggle("Anti-backstab", &Vars::Misc::AntiBackstab.Value);
			FToggle("Anti-AFK", &Vars::Misc::AntiAFK.Value, FToggle_Middle);
			FToggle("Anti autobalance", &Vars::Misc::AntiAutobalance.Value);
			FToggle("Auto accept item drops", &Vars::Misc::AutoAcceptItemDrops.Value, FToggle_Middle);
			FToggle("Taunt control", &Vars::Misc::TauntControl.Value);
			FToggle("Kart control", &Vars::Misc::KartControl.Value, FToggle_Middle);
			FToggle("Backpack expander", &Vars::Misc::BackpackExpander.Value);
		} EndSection();
		if (Section("Sound"))
		{
			FDropdown("Block sounds", &Vars::Misc::SoundBlock.Value, { "Footsteps", "Noisemaker" }, {}, FDropdown_Multi);
			FToggle("Giant weapon sounds", &Vars::Misc::GiantWeaponSounds.Value);
		} EndSection();
		if (Section("Mann vs. Machine"))
		{
			FToggle("Instant respawn", &Vars::Misc::InstantRespawn.Value);
			FToggle("Instant revive", &Vars::Misc::InstantRevive.Value, FToggle_Middle);
		} EndSection();

		/* Column 2 */
		TableNextColumn();
		if (Section("Game"))
		{
			FToggle("Network fix", &Vars::Misc::NetworkFix.Value);
			FToggle("Prediction error jitter fix", &Vars::Misc::PredictionErrorJitterFix.Value, FToggle_Middle);
			FToggle("SetupBones optimization", &Vars::Misc::SetupBonesOptimization.Value);
		} EndSection();
		if (Section("Queueing"))
		{
			FDropdown("Force regions", &Vars::Misc::ForceRegions.Value,
				{ "Atlanta", "Chicago", "Los Angeles", "Moses Lake", "Seattle", "Virginia", "Washington", "Amsterdam", "Frankfurt", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "Buenos Aires", "Lima", "Santiago", "Sao Paulo", "Chennai", "Dubai", "Guangzhou", "Hong Kong", "Mumbai", "Seoul", "Shanghai", "Singapore", "Tianjin", "Tokyo", "Sydney", "Johannesburg" },
				{ DC_ATL,	 DC_ORD,	DC_LAX,		   DC_EAT,		 DC_SEA,	DC_IAD,		DC_DFW,		  DC_AMS,	   DC_FRA,		DC_LHR,	  DC_MAD,	DC_PAR,	 DC_STO,	  DC_VIE,	DC_WAW,	  DC_EZE,		  DC_LIM, DC_SCL,	  DC_GRU,	   DC_MAA,	  DC_DXB,  DC_CAN,		DC_HKG,		 DC_BOM,   DC_SEO,	DC_SHA,		DC_SGP,		 DC_TSN,	DC_TYO,	 DC_SYD,   DC_JNB },
				FDropdown_Multi
			);
			FDropdown("Auto queue", &Vars::Misc::AutoCasualQueue.Value, { "Off", "In menu", "Always" });
			FToggle("Freeze queue", &Vars::Misc::FreezeQueue.Value);
		} EndSection();
		if (Section("Chat"))
		{
			FToggle("Chat tags", &Vars::Misc::ChatTags.Value);
		} EndSection();
		if (Section("Steam RPC"))
		{
			FToggle("Steam RPC", &Vars::Misc::Steam::EnableRPC.Value);
			FDropdown("Match group", &Vars::Misc::Steam::MatchGroup.Value, { "Special Event", "MvM Mann Up", "Competitive", "Casual", "MvM Boot Camp" });
			FSDropdown("Map text", &Vars::Misc::Steam::MapText.Value, { "Fedoraware", "Figoraware", "Meowhook.club", "Rathook.cc", "Nitro.tf" }, FSDropdown_Custom);
			FSlider("Group size", &Vars::Misc::Steam::GroupSize.Value, 0, 6);
			FToggle("Override in menu", &Vars::Misc::Steam::OverrideMenu.Value);
		} EndSection();

		EndTable();
	}
}

/* Tab: Settings */
void CMenu::MenuLogs()
{
	using namespace ImGui;

	switch (CurrentLogsTab)
	{
	// Logs
	case 0:
		// Eventually put all logs here, regardless of any settings
		break;
	// Settings
	case 1:
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Logging"))
			{
				FDropdown("Logs", &Vars::Logging::Logs.Value, { "Vote start", "Vote cast", "Class changes", "Damage", "Cheat detection", "Tags" }, {}, FDropdown_Multi);
				FSlider("Notification time", &Vars::Logging::Lifetime.Value, 0.5f, 5.f, 0.5f, "%.1f");
			} EndSection();
			if (Section("Vote Start"))
			{
				bTransparent = !(Vars::Logging::Logs.Value & 1 << 0);
					FDropdown("Log to", &Vars::Logging::VoteStart::LogTo.Value, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();
			if (Section("Vote Cast"))
			{
				bTransparent = !(Vars::Logging::Logs.Value & 1 << 1);
					FDropdown("Log to", &Vars::Logging::VoteCast::LogTo.Value, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();
			if (Section("Class Change"))
			{
				bTransparent = !(Vars::Logging::Logs.Value & 1 << 2);
					FDropdown("Log to", &Vars::Logging::ClassChange::LogTo.Value, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Damage"))
			{
				bTransparent = !(Vars::Logging::Logs.Value & 1 << 3);
					FDropdown("Log to", &Vars::Logging::Damage::LogTo.Value, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();
			if (Section("Cheat Detection"))
			{
				bTransparent = !(Vars::Logging::Logs.Value & 1 << 4);
					FDropdown("Log to", &Vars::Logging::CheatDetection::LogTo.Value, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();
			if (Section("Tags"))
			{
				bTransparent = !(Vars::Logging::Logs.Value & 1 << 5);
					FDropdown("Log to", &Vars::Logging::Tags::LogTo.Value, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Multi);
				bTransparent = false;
			} EndSection();

			EndTable();
		}
	}
}

/* Tab: Config */
void CMenu::MenuSettings()
{
	using namespace ImGui;

	switch (CurrentConfigTab)
	{
	// Settings
	case 0:
		if (BeginTable("ConfigSettingsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Config"))
			{
				if (FButton("Configs folder", FButton_Left))
					ShellExecuteA(NULL, NULL, g_CFG.GetConfigPath().c_str(), NULL, NULL, SW_SHOWNORMAL);
				if (FButton("Visuals folder", FButton_Right | FButton_SameLine))
					ShellExecuteA(NULL, NULL, g_CFG.GetVisualsPath().c_str(), NULL, NULL, SW_SHOWNORMAL);

				FTabs({ "GENERAL", "VISUALS", }, &CurrentConfigType, { GetColumnWidth() / 2 + 2, SubTabSize.y }, { 6, GetCursorPos().y }, false);

				switch (CurrentConfigType)
				{
				// General
				case 0:
				{
					static std::string newName;
					FSDropdown("Config name", &newName, {}, FSDropdown_AutoUpdate | FSDropdown_Left);
					if (FButton("Create", FButton_Fit | FButton_SameLine | FButton_Large) && newName.length() > 0)
					{
						if (!std::filesystem::exists(g_CFG.GetConfigPath() + "\\" + newName))
							g_CFG.SaveConfig(newName);
						newName.clear();
					}

					for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetConfigPath()))
					{
						if (!entry.is_regular_file() || entry.path().extension() != g_CFG.ConfigExtension)
							continue;

						std::string configName = entry.path().filename().string();
						configName.erase(configName.end() - g_CFG.ConfigExtension.size(), configName.end());

						const auto current = GetCursorPos().y;

						SetCursorPos({ 14, current + 11 });
						TextColored(configName == g_CFG.GetCurrentConfig() ? Active.Value : Inactive.Value, configName.c_str());

						int o = 26;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## DeleteConfig{}", configName).c_str());
						o += 25;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_SAVE))
						{
							if (configName != g_CFG.GetCurrentConfig() || g_CFG.GetCurrentVisuals() != "")
								OpenPopup(std::format("Confirmation## SaveConfig{}", configName).c_str());
							else
								g_CFG.SaveConfig(configName);
						}
						o += 25;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DOWNLOAD))
						{
							g_CFG.LoadConfig(configName);
							LoadStyle();
						}

						// Dialogs
						{
							// Save config dialog
							if (BeginPopupModal(std::format("Confirmation## SaveConfig{}", configName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								Text(std::format("Do you really want to override '{}'?", configName).c_str());

								if (FButton("Yes, override", FButton_Left))
								{
									g_CFG.SaveConfig(configName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}

							// Delete config dialog
							if (BeginPopupModal(std::format("Confirmation## DeleteConfig{}", configName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								Text(std::format("Do you really want to delete '{}'?", configName).c_str());

								if (FButton("Yes, delete", FButton_Left))
								{
									g_CFG.RemoveConfig(configName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}
						}

						SetCursorPos({ 6, current }); DebugDummy({ 0, 28 });
					}
					break;
				}
				// Visuals
				case 1:
				{
					static std::string newName;
					FSDropdown("Config name", &newName, {}, FSDropdown_AutoUpdate | FSDropdown_Left);
					if (FButton("Create", FButton_Fit | FButton_SameLine | FButton_Large) && newName.length() > 0)
					{
						if (!std::filesystem::exists(g_CFG.GetVisualsPath() + "\\" + newName))
							g_CFG.SaveVisual(newName);
						newName.clear();
					}

					for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetVisualsPath()))
					{
						if (!entry.is_regular_file() || entry.path().extension() != g_CFG.ConfigExtension)
							continue;

						std::string configName = entry.path().filename().string();
						configName.erase(configName.end() - g_CFG.ConfigExtension.size(), configName.end());

						const auto current = GetCursorPos().y;

						SetCursorPos({ 14, current + 11 });
						TextColored(configName == g_CFG.GetCurrentVisuals() ? Active.Value : Inactive.Value, configName.c_str());

						int o = 26;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## DeleteVisual{}", configName).c_str());
						o += 25;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_SAVE))
						{
							if (configName != g_CFG.GetCurrentVisuals())
								OpenPopup(std::format("Confirmation## SaveVisual{}", configName).c_str());
							else
								g_CFG.SaveVisual(configName);
						}
						o += 25;

						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DOWNLOAD))
						{
							g_CFG.LoadVisual(configName);
							LoadStyle();
						}

						// Dialogs
						{
							// Save config dialog
							if (BeginPopupModal(std::format("Confirmation## SaveVisual{}", configName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								Text(std::format("Do you really want to override '{}'?", configName).c_str());

								if (FButton("Yes, override", FButton_Left))
								{
									g_CFG.SaveVisual(configName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}

							// Delete config dialog
							if (BeginPopupModal(std::format("Confirmation## DeleteVisual{}", configName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
							{
								Text(std::format("Do you really want to delete '{}'?", configName).c_str());

								if (FButton("Yes, delete", FButton_Left))
								{
									g_CFG.RemoveVisual(configName);
									CloseCurrentPopup();
								}
								if (FButton("No", FButton_Right | FButton_SameLine))
									CloseCurrentPopup();

								EndPopup();
							}
						}

						SetCursorPos({ 6, current }); DebugDummy({ 0, 28 });
					}
				}
				}
			} EndSection();
			SetCursorPosX(GetCursorPosX() + 8);
			PushStyleColor(ImGuiCol_Text, Inactive.Value);
			Text("Built @ %s, %s", __DATE__, __TIME__);
			PopStyleColor();

			/* Column 2 */
			TableNextColumn();
			if (Section("Debug"))
			{
				FToggle("Debug info", &Vars::Debug::Info.Value);
				FToggle("Debug logging", &Vars::Debug::Logging.Value, FToggle_Middle);
				FToggle("Allow secure servers", I::AllowSecureServers);
				bool* m_bPendingPingRefresh = reinterpret_cast<bool*>(I::TFGCClientSystem + 828);
				FToggle("Pending ping refresh", m_bPendingPingRefresh, FToggle_Middle);
				FToggle("Show server hitboxes", &Vars::Debug::ServerHitbox.Value); HelpMarker("localhost servers");
				FToggle("Anti aim lines", &Vars::Debug::AntiAimLines.Value, FToggle_Middle);
				static std::string particleName = "ping_circle";
				FSDropdown("Particle name", &particleName, {}, FSDropdown_Left);
				const auto& pLocal = g_EntityCache.GetLocal();
				if (FButton("Dispatch", FButton_Right | FButton_SameLine | FButton_Large) && pLocal)
					Particles::DispatchParticleEffect(particleName.c_str(), pLocal->GetAbsOrigin(), { });
			} EndSection();
			if (Section("Extra"))
			{
				if (FButton("cl_fullupdate", FButton_Left))
					I::EngineClient->ClientCmd_Unrestricted("cl_fullupdate");
				if (FButton("retry", FButton_Right | FButton_SameLine))
					I::EngineClient->ClientCmd_Unrestricted("retry");
				if (FButton("Console", FButton_Left))
					I::EngineClient->ClientCmd_Unrestricted("showconsole");
				if (FButton("Fix Chams", FButton_Right | FButton_SameLine))
					F::Materials.ReloadMaterials();
#ifdef DEBUG
				if (FButton("Dump Classes", FButton_Left))
					F::Misc.DumpClassIDS();
#endif
				if (!I::EngineClient->IsConnected())
				{
					if (FButton("Unlock achievements", FButton_Left))
						OpenPopup("Unlock achievements?");
					if (FButton("Lock achievements", FButton_Right | FButton_SameLine))
						OpenPopup("Lock achievements?");

					if (BeginPopupModal("Unlock achievements?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						Text("Do you really want to unlock all achievements?");

						Separator();
						if (Button("Yes, unlock", ImVec2(150, 0)))
						{
							F::Misc.UnlockAchievements();
							CloseCurrentPopup();
						}
						SameLine();
						if (Button("No", ImVec2(150, 0)))
							CloseCurrentPopup();
						EndPopup();
					}
					if (BeginPopupModal("Lock achievements?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						Text("Do you really want to lock all achievements?");

						Separator();
						if (Button("Yes, lock", ImVec2(150, 0)))
						{
							F::Misc.LockAchievements();
							CloseCurrentPopup();
						}
						SameLine();
						if (Button("No", ImVec2(150, 0)))
							CloseCurrentPopup();
						EndPopup();
					}
				}
				if (Vars::Debug::Info.Value)
				{
					if (FButton("Reveal bullet lines", FButton_Left))
						F::Visuals.RevealBulletLines();
					if (FButton("Reveal prediction lines", FButton_Right | FButton_SameLine))
						F::Visuals.RevealSimLines();
					if (FButton("Reveal boxes", FButton_Left))
						F::Visuals.RevealBoxes();
					if (FButton("Print hashes", FButton_Right | FButton_SameLine))
						Hash::PrintHash();
				}
			} EndSection();

			EndTable();
		}
		break;
	// Conditions
	case 1:
		if (BeginTable("ConfigConditionsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Settings"))
			{
				FToggle("Show keybind menu", &Vars::Menu::ShowKeybinds.Value);
				FToggle("Show keybind changes", &Vars::Menu::ShowKBChanges.Value, FToggle_Middle);
			} EndSection();

			TableNextColumn();

			EndTable();
		}
		break;
	// PlayerList
	case 2:
		if (Section("Players"))
		{
			if (I::EngineClient->IsInGame())
			{
				std::lock_guard lock(F::PlayerUtils.mutex);
				const auto& playerCache = F::PlayerUtils.vPlayerCache;

				auto getTeamColor = [](int team, bool alive)
					{
						switch (team)
						{
						case 3: return Color_t(50, 75, 100, alive ? 255 : 127);
						case 2: return Color_t(125, 50, 50, alive ? 255 : 127);
						}
						return Color_t(100, 100, 100, 255);
					};
				auto drawPlayer = [getTeamColor](const ListPlayer& player, int x, int y)
					{
						bool bClicked = false, bAdd = false, bPitch = false, bYaw = false;

						const Color_t teamColor = getTeamColor(player.Team, player.Alive);
						const ImColor imColor = ColorToVec(teamColor);

						const ImVec2 restorePos = { x ? GetWindowSize().x / 2 + 4.f : 8.f, 32.f + 36.f * y };

						// background
						const float width = GetWindowSize().x / 2 - 12; const auto windowPos = GetWindowPos();
						GetWindowDrawList()->AddRectFilled({ windowPos.x + restorePos.x, windowPos.y + restorePos.y }, { windowPos.x + restorePos.x + width, windowPos.y + restorePos.y + 28 }, imColor, 3);

						// text + icons
						if (player.Local)
						{
							SetCursorPos({ restorePos.x + 7, restorePos.y + 5 });
							IconImage(ICON_MD_PERSON);
						}
						else if (player.Friend)
						{
							SetCursorPos({ restorePos.x + 7, restorePos.y + 5 });
							IconImage(ICON_MD_GROUP);
						}
						int lOffset = player.Local || player.Friend ? 29 : 10;
						SetCursorPos({ restorePos.x + lOffset, restorePos.y + 7});
						TextUnformatted(player.Name);
						lOffset += CalcTextSize(player.Name).x + 8;

						// buttons
						if (!player.Fake)
						{
							// right
							SetCursorPos({ restorePos.x + width - 22, restorePos.y + 5 });
							bAdd = IconButton(ICON_MD_ADD);
							if (Vars::AntiHack::Resolver::Resolver.Value && !player.Local)
							{
								SetCursorPos({ restorePos.x + width - 42, restorePos.y + 5 });
								bYaw = IconButton(ICON_MD_ARROW_FORWARD);

								SetCursorPos({ restorePos.x + width - 62, restorePos.y + 5 });
								bPitch = IconButton(ICON_MD_ARROW_UPWARD);
							}

							// tag bar
							SetCursorPos({ restorePos.x + lOffset, restorePos.y });
							if (BeginChild(std::format("TagBar{}", player.FriendsID).c_str(), { width - lOffset - (Vars::AntiHack::Resolver::Resolver.Value ? 68 : 28), 28}, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
							{
								PushFont(F::Menu.FontSmall);

								const auto childPos = GetWindowPos();
								float tOffset = 0;
								for (const auto& sTag : G::PlayerTags[player.FriendsID])
								{
									PriorityLabel plTag;
									if (!F::PlayerUtils.GetTag(sTag, &plTag))
										continue;

									const ImColor tagColor = ColorToVec(plTag.Color);
									const float tagWidth = CalcTextSize(sTag.c_str()).x + 25;
									const ImVec2 tagPos = { tOffset, 4 };

									GetWindowDrawList()->AddRectFilled({ childPos.x + tagPos.x, childPos.y + tagPos.y }, { childPos.x + tagPos.x + tagWidth, childPos.y + tagPos.y + 20 }, tagColor, 3);
									SetCursorPos({ tagPos.x + 5, tagPos.y + 4 });
									TextUnformatted(sTag.c_str());
									SetCursorPos({ tagPos.x + tagWidth - 18, tagPos.y + 2 });
									if (IconButton(ICON_MD_CANCEL))
										F::PlayerUtils.RemoveTag(player.FriendsID, sTag, true, player.Name);

									tOffset += tagWidth + 4;
								}
								PopFont();
							} EndChild();

							if (!player.Local)
							{
								//bClicked = IsItemClicked();
								bClicked = IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) || bClicked;

								SetCursorPos(restorePos);
								/*bClicked = */Button(std::format("##{}", player.Name).c_str(), { width, 28 }) || bClicked;
								bClicked = IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) || bClicked;
							}
						}

						SetCursorPos(restorePos);
						DebugDummy({ 0, 28 });

						if (bClicked)
							OpenPopup(std::format("Clicked{}", player.FriendsID).c_str());
						else if (bAdd)
							OpenPopup(std::format("Add{}", player.FriendsID).c_str());
						else if (bPitch)
							OpenPopup(std::format("Pitch{}", player.FriendsID).c_str());
						else if (bYaw)
							OpenPopup(std::format("Yaw{}", player.FriendsID).c_str());
						
						// popups
						if (FBeginPopup(std::format("Clicked{}", player.FriendsID).c_str()))
						{
							if (FSelectable("Profile"))
								g_SteamInterfaces.Friends->ActivateGameOverlayToUser("steamid", CSteamID(0x0110000100000000ULL + player.FriendsID));

							if (FSelectable("Votekick"))
								I::EngineClient->ClientCmd_Unrestricted(std::format("callvote kick {}", player.UserID).c_str());

							FEndPopup();
						}
						else if (FBeginPopup(std::format("Add{}", player.FriendsID).c_str()))
						{
							for (const auto& [sTag, plTag] : F::PlayerUtils.vTags)
							{
								if (!plTag.Assignable || F::PlayerUtils.HasTag(player.FriendsID, sTag))
									continue;

								auto imColor = ColorToVec(plTag.Color);
								PushStyleColor(ImGuiCol_Text, imColor);
								imColor.x /= 3; imColor.y /= 3; imColor.z /= 3;
								if (FSelectable(sTag.c_str(), imColor))
									F::PlayerUtils.AddTag(player.FriendsID, sTag, true, player.Name);
								PopStyleColor();
							}

							FEndPopup();
						}
						else if (FBeginPopup(std::format("Pitch{}", player.FriendsID).c_str()))
						{
							for (size_t i = 0; i < F::PlayerUtils.vListPitch.size(); i++)
							{
								if (FSelectable(F::PlayerUtils.vListPitch[i]))
									F::Resolver.mResolverMode[player.FriendsID].second = i;
							}

							FEndPopup();
						}
						else if (FBeginPopup(std::format("Yaw{}", player.FriendsID).c_str()))
						{
							for (size_t i = 0; i < F::PlayerUtils.vListYaw.size(); i++)
							{
								if (FSelectable(F::PlayerUtils.vListYaw[i]))
									F::Resolver.mResolverMode[player.FriendsID].second = i;
							}

							FEndPopup();
						}
					};

				// display players
				int iBlu = 0, iRed = 0;
				for (const auto& player : playerCache)
				{
					switch (player.Team)
					{
					case 3:
						drawPlayer(player, 0, iBlu);
						iBlu++; break;
					case 2:
						drawPlayer(player, 1, iRed);
						iRed++; break;
					}
				}
				int iOther = 0; const int iMax = std::max(iBlu, iRed);
				for (const auto& player : playerCache)
				{
					if (player.Team != 3 && player.Team != 2)
					{
						drawPlayer(player, iOther % 2, iMax + iOther / 2);
						iOther++;
					}
				}
			}
			else
			{
				SetCursorPos({ 18, 39 });
				Text("Not ingame");
				DebugDummy({ 0, 8 });
			}
		} EndSection();
		if (Section("Tags"))
		{
			static std::string tagName = "";
			static PriorityLabel tagData = {};

			if (BeginTable("TagTable", 2))
			{
				/* Column 1 */
				TableNextColumn(); if (BeginChild("TagTable1", { GetColumnWidth(), 48 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
				{
					FSDropdown("Name", &tagName, {}, FSDropdown_Left, 1);
					FColorPicker("Color", &tagData.Color, 0, FColorPicker_Dropdown);

					bTransparent = tagData.Label; // transparent if we want a label, user can still use to sort
						SetCursorPosY(GetCursorPos().y - 37); Dummy({});
						FSlider("Priority", &tagData.Priority, -10, 10, 1, "%d", FSlider_Right);
					bTransparent = false;
				} EndChild();

				/* Column 2 */
				TableNextColumn(); if (BeginChild("TagTable2", { GetColumnWidth(), 48 }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground))
				{
					bDisabled = tagName == "Default" || tagName == "Ignored"; // force nonlabel on default & ignored
						Dummy({ 0, 11 }); FToggle("Label", &tagData.Label);
						if (bDisabled)
							tagData.Label = false;
					bDisabled = false;

					bool bCreate = false, bClear = false;

					// create/modify button
					PushStyleColor(ImGuiCol_Button, Foremost.Value);
					SetCursorPos({ GetWindowSize().x - 88, 8 });
					if (tagName != "")
					{
						PushStyleColor(ImGuiCol_ButtonHovered, ForemostLight.Value);
						PushStyleColor(ImGuiCol_ButtonActive, ForemostLight.Value);
						bCreate = Button("##CreateButton", { 40, 40 });
					}
					else
					{
						PushStyleColor(ImGuiCol_ButtonHovered, Foremost.Value);
						PushStyleColor(ImGuiCol_ButtonActive, Foremost.Value);
						Button("##CreateButton", { 40, 40 });
					}
					PopStyleColor(3);
					SetCursorPos({ GetWindowSize().x - 75, 20 });
					if (tagName != "")
					{
						bool bMatch = false;
						for (const auto& [sTag, _] : F::PlayerUtils.vTags)
						{
							if (sTag == tagName)
								bMatch = true;
						}
						IconImage(bMatch ? ICON_MD_SETTINGS : ICON_MD_ADD);
					}
					else
					{
						bTransparent = true;
							IconImage(ICON_MD_ADD);
						bTransparent = false;
					}

					// clear button
					SetCursorPos({ GetWindowSize().x - 40, 8 });
					PushStyleColor(ImGuiCol_Button, Foremost.Value);
					PushStyleColor(ImGuiCol_ButtonHovered, ForemostLight.Value);
					PushStyleColor(ImGuiCol_ButtonActive, ForemostLight.Value);
					bClear = Button("##ClearButton", { 40, 40 });
					PopStyleColor(3);
					SetCursorPos({ GetWindowSize().x - 27, 20 });
					IconImage(ICON_MD_CLEAR);

					if (bCreate)
					{
						F::PlayerUtils.bSaveTags = true;
						F::PlayerUtils.vTags[tagName].Color = tagData.Color;
						F::PlayerUtils.vTags[tagName].Priority = tagData.Priority;
						F::PlayerUtils.vTags[tagName].Label = tagData.Label;
					}
					if (bCreate || bClear)
					{
						tagName = "";
						tagData = {};
					}
				} EndChild();

				EndTable();
			}

			auto drawTag = [](const std::string sTag, const PriorityLabel& plTag, int y)
				{
					bool bClicked = false, bDelete = false;

					ImColor imColor = ColorToVec(plTag.Color);
					imColor.Value.x /= 3; imColor.Value.y /= 3; imColor.Value.z /= 3;

					const ImVec2 restorePos = { plTag.Label ? GetWindowSize().x * 2 / 3 + 4.f : 8.f, 96.f + 36.f * y };

					// background
					const float width = GetWindowSize().x * (plTag.Label ? 1.f / 3 : 2.f / 3) - 12; const auto windowPos = GetWindowPos();
					GetWindowDrawList()->AddRectFilled({ windowPos.x + restorePos.x, windowPos.y + restorePos.y }, { windowPos.x + restorePos.x + width, windowPos.y + restorePos.y + 28 }, imColor, 3);

					// text
					SetCursorPos({ restorePos.x + 10, restorePos.y + 7 });
					TextUnformatted(sTag.c_str());

					// buttons
					if (!plTag.Locked)
					{
						SetCursorPos({ restorePos.x + width - 22, restorePos.y + 5 });
						bDelete = IconButton(ICON_MD_DELETE);
					}
					if (!plTag.Label)
					{
						SetCursorPos({ restorePos.x + width / 2, restorePos.y + 7 });
						TextUnformatted(std::format("{}", plTag.Priority).c_str());
					}
					SetCursorPos(restorePos);
					bClicked = Button(std::format("##{}", sTag).c_str(), { width, 28 });

					if (bClicked)
					{
						tagName = sTag;
						tagData.Color = plTag.Color;
						tagData.Priority = plTag.Priority;
						tagData.Label = plTag.Label;
					}
					if (bDelete)
					{
						F::PlayerUtils.vTags.erase(sTag);
						F::PlayerUtils.bSaveTags = true;
						if (tagName == sTag)
						{
							tagName = "";
							tagData = {};
						}
					}
				};

			PushStyleColor(ImGuiCol_Text, Inactive.Value);
			SetCursorPos({ 14, 80 }); FText("Priorities");
			SetCursorPos({ GetWindowSize().x * 2 / 3 + 10, 80 }); FText("Labels");
			PopStyleColor();

			std::vector<std::pair<std::string, PriorityLabel>> vPriorities = {}, vLabels = {};
			for (const auto& [sTag, plTag] : F::PlayerUtils.vTags)
			{
				if (!plTag.Label)
					vPriorities.push_back({ sTag, plTag });
				else
					vLabels.push_back({ sTag, plTag });
			}

			std::sort(vPriorities.begin(), vPriorities.end(), [&](const auto& a, const auto& b) -> bool
				{
					// override for default tag
					if (a.first == "Default")
						return true;
					if (b.first == "Default")
						return false;

					// sort by priority if unequal
					if (a.second.Priority != b.second.Priority)
						return a.second.Priority > b.second.Priority;

					return a.first < b.first;
				});
			std::sort(vLabels.begin(), vLabels.end(), [&](const auto& a, const auto& b) -> bool
				{
					// sort by priority if unequal
					if (a.second.Priority != b.second.Priority)
						return a.second.Priority > b.second.Priority;

					return a.first < b.first;
				});

			// display tags
			int iPriorities = 0, iLabels = 0;
			for (const auto& pair : vPriorities)
			{
				drawTag(pair.first, pair.second, iPriorities);
				iPriorities++;
			}
			for (const auto& pair : vLabels)
			{
				drawTag(pair.first, pair.second, iLabels);
				iLabels++;
			}
			SetCursorPos({ 0, 60.f + 36.f * std::max(iPriorities, iLabels)}); DebugDummy({ 0, 28 });
		} EndSection();
		break;
	// MaterialManager
	case 3:
		if (BeginTable("MaterialsTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("Manager"))
			{
				static std::string newName;
				FSDropdown("Material name", &newName, {}, FSDropdown_AutoUpdate | FSDropdown_Left);
				if (FButton("Create", FButton_Fit | FButton_SameLine | FButton_Large) && newName.length() > 0)
				{
					F::Materials.AddMaterial(newName);
					newName.clear();
				}

				if (FButton("Folder", FButton_Fit | FButton_SameLine | FButton_Large))
					ShellExecuteA(nullptr, "open", MaterialFolder.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);

				for (auto const& mat : F::Materials.vChamMaterials)
				{
					const auto current = GetCursorPos().y;

					SetCursorPos({ 14, current + 11 });
					TextColored(mat.bLocked ? Inactive.Value : Active.Value, mat.sName.c_str());

					int o = 26;

					if (!mat.bLocked)
					{
						SetCursorPos({ GetWindowSize().x - o, current + 9 });
						if (IconButton(ICON_MD_DELETE))
							OpenPopup(std::format("Confirmation## DeleteMat{}", mat.sName).c_str());
						if (BeginPopupModal(std::format("Confirmation## DeleteMat{}", mat.sName).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysUseWindowPadding))
						{
							Text(std::format("Do you really want to delete '{}'?", mat.sName).c_str());

							if (FButton("Yes", FButton_Left))
							{
								F::Materials.RemoveMaterial(mat.sName);
								CloseCurrentPopup();
							}
							if (FButton("No", FButton_Right | FButton_SameLine))
								CloseCurrentPopup();

							EndPopup();
						}
						o += 25;
					}

					SetCursorPos({ GetWindowSize().x - o, current + 9 });
					if (IconButton(ICON_MD_EDIT))
					{
						CurrentMaterial = mat.sName;
						LockedMaterial = mat.bLocked;

						TextEditor.SetText(F::Materials.GetVMT(CurrentMaterial));
						TextEditor.SetReadOnly(LockedMaterial);
					}

					SetCursorPos({ 6, current }); DebugDummy({ 0, 28 });
				}
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (CurrentMaterial != "")
			{
				if (Section("Editor", GetContentRegionMax().y - 18, true))
				{
					// Toolbar
					if (!LockedMaterial)
					{
						if (FButton("Save", FButton_Fit))
						{
							auto text = TextEditor.GetText();
							text.erase(text.end() - 1, text.end()); // get rid of random newline
							F::Materials.EditMaterial(CurrentMaterial, text);
						}
						SameLine();
					}
					if (FButton("Close", FButton_Fit))
						CurrentMaterial = "";
					SameLine(); SetCursorPosY(GetCursorPosY() + 27);
					PushStyleColor(ImGuiCol_Text, Inactive.Value);
					FText(LockedMaterial ? std::format("Viewing: {}", CurrentMaterial).c_str() : std::format("Editing: {}", CurrentMaterial).c_str(), FText_Right);
					PopStyleColor();

					// Text editor
					Dummy({ 0, 8 });
					TextEditor.Render("TextEditor");
				} EndSection();
			}

			EndTable();
		}
		break;
	}
}
#pragma endregion

void CMenu::AddDraggable(const char* szTitle, DragBox_t& info, bool bShouldDraw)
{
	if (bShouldDraw)
	{
		const float sizeX = 100.f * Vars::Menu::DPI.Value, sizeY = 40.f * Vars::Menu::DPI.Value;

		if (info.update)
		{
			ImGui::SetNextWindowSize({ sizeX, sizeY }, ImGuiCond_Always);
			ImGui::SetNextWindowPos({ float(info.x - sizeX / 2), float(info.y) }, ImGuiCond_Always);
			info.update = false;
		}

		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorToVec(Vars::Menu::Theme::Active.Value));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { sizeX, sizeY });
		if (ImGui::Begin(szTitle, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
		{
			ImGui::PushFont(FontBlack);
			auto size = ImGui::CalcTextSize(szTitle);
			ImGui::SetCursorPos({ (sizeX - size.x) * 0.5f, (sizeY - size.y) * 0.5f });
			ImGui::Text(szTitle);
			ImGui::PopFont();

			const ImVec2 winPos = ImGui::GetWindowPos();
			info.x = winPos.x + sizeX / 2;
			info.y = winPos.y;

			ImGui::End();
		}
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(2);
	}
}

void CMenu::DrawKeybinds()
{
	if (!Vars::Menu::ShowKeybinds.Value)
		return;

	DragBox_t& info = Vars::Menu::KeybindsDisplay.Value;
	if (info.update)
	{
		ImGui::SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);
		info.update = false;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 4.f, 4.f });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	ImGui::SetNextWindowSize({ 200.f, 0.f });

	if (ImGui::Begin("Keybinds", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		auto drawOption = [](const char* name, bool active)
			{
				ImGui::Text(name);
				ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x - ImGui::CalcTextSize(active ? "[On]" : "[Off]").x);
				ImGui::Text(active ? "[On]" : "[Off]");
			};

		auto isActive = [](bool active, bool needsKey, int key)
			{
				return active && (!needsKey || GetAsyncKeyState(key) & 0x8000);
			};

		drawOption("Aimbot", isActive(Vars::Aimbot::Global::Active.Value, Vars::Aimbot::Global::AimKey.Value, Vars::Aimbot::Global::AimKey.Value));
		drawOption("Auto Shoot", Vars::Aimbot::Global::AutoShoot.Value);
		drawOption("Double Tap", isActive(Vars::CL_Move::DoubleTap::Enabled.Value, Vars::CL_Move::DoubleTap::Mode.Value == 1, Vars::CL_Move::DoubleTap::DoubletapKey.Value));
		drawOption("Anti Aim", Vars::AntiHack::AntiAim::Active.Value);
		drawOption("Fakelag", isActive(Vars::CL_Move::FakeLag::Enabled.Value, Vars::CL_Move::FakeLag::Mode.Value == 1, Vars::CL_Move::FakeLag::Key.Value));
		drawOption("Triggerbot", isActive(Vars::Auto::Global::Active.Value, Vars::Auto::Global::AutoKey.Value, Vars::Auto::Global::AutoKey.Value));

		const ImVec2 winPos = ImGui::GetWindowPos();
		info.x = winPos.x;
		info.y = winPos.y;

		ImGui::End();
	}

	ImGui::PopStyleVar(2);
}

/* Window for the camera feature */
void CMenu::DrawCameraWindow()
{
	if (!Vars::Visuals::ProjectileCamera.Value)
		return;

	WindowBox_t& info = Vars::Visuals::ProjectileWindow.Value;
	if (info.update)
	{
		ImGui::SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);
		ImGui::SetNextWindowSize({ float(info.w), float(info.h) }, ImGuiCond_Always);
		info.update = false;
	}

	ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
	ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorToVec(Vars::Menu::Theme::Active.Value));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 100.f, 100.f });
	if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		const ImVec2 winPos = ImGui::GetWindowPos();
		const ImVec2 winSize = ImGui::GetWindowSize();

		ImGui::PushFont(FontBlack);
		auto size = ImGui::CalcTextSize("Camera");
		ImGui::SetCursorPos({ (winSize.x - size.x) * 0.5f, (winSize.y - size.y) * 0.5f });
		ImGui::Text("Camera");
		ImGui::PopFont();

		info.x = int(winPos.x);
		info.y = int(winPos.y);
		info.w = int(winSize.x);
		info.h = int(winSize.y);

		ImGui::End();
	}
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(2);
}

static void SquareConstraints(ImGuiSizeCallbackData* data) { data->DesiredSize.x = data->DesiredSize.y = std::max(data->DesiredSize.x, data->DesiredSize.y); }
void CMenu::DrawRadar()
{
	if (!Vars::Radar::Main::Active.Value)
		return;

	WindowBox_t& info = Vars::Radar::Main::Window.Value;
	if (info.update)
	{
		ImGui::SetNextWindowPos({ float(info.x), float(info.y) }, ImGuiCond_Always);
		ImGui::SetNextWindowSize({ float(info.w), float(info.w) }, ImGuiCond_Always);
		info.update = false;
	}

	ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
	ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorToVec(Vars::Menu::Theme::Active.Value));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
	ImGui::SetNextWindowSizeConstraints({ 100.f, 100.f }, { 400.f, 400.f }, SquareConstraints);
	if (ImGui::Begin("Radar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
	{
		const ImVec2 winPos = ImGui::GetWindowPos();
		const ImVec2 winSize = ImGui::GetWindowSize();

		ImGui::PushFont(FontBlack);
		auto size = ImGui::CalcTextSize("Radar");
		ImGui::SetCursorPos({ (winSize.x - size.x) * 0.5f, (winSize.y - size.y) * 0.5f });
		ImGui::Text("Radar");
		ImGui::PopFont();

		info.x = static_cast<int>(winPos.x);
		info.y = static_cast<int>(winPos.y);
		info.w = static_cast<int>(winSize.x);

		ImGui::End();
	}
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(2);
}

void CMenu::Render(IDirect3DDevice9* pDevice)
{
	if (!ConfigLoaded)
		return;

	static std::once_flag initFlag;
	std::call_once(initFlag, [&]
		{
			Init(pDevice);
		});

	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

	// Toggle menu (defaults are 'insert' and 'F3', another can be added in menu)
	if (Utils::IsGameWindowInFocus() && (F::KeyHandler.Pressed(VK_INSERT) || F::KeyHandler.Pressed(VK_F3) || F::KeyHandler.Pressed(Vars::Menu::MenuKey.Value)))
		I::VGuiSurface->SetCursorAlwaysVisible(IsOpen = !IsOpen);

	// Begin current frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(FontRegular);

	// better looking dt bars
	F::Visuals.DrawTickbaseBars();

	// Window that should always be visible
	DrawKeybinds();

	if (IsOpen)
	{
		DrawMenu();

		DrawCameraWindow();
		DrawRadar();

		AddDraggable("Ticks", Vars::Menu::TicksDisplay.Value, Vars::Menu::Indicators.Value & (1 << 0));
		AddDraggable("Crit hack", Vars::Menu::CritsDisplay.Value, Vars::Menu::Indicators.Value & (1 << 1));
		AddDraggable("Spectators", Vars::Menu::SpectatorsDisplay.Value, Vars::Menu::Indicators.Value & (1 << 2));
		AddDraggable("Conditions", Vars::Menu::ConditionsDisplay.Value, Vars::Menu::Indicators.Value & (1 << 3));
		AddDraggable("Ping", Vars::Menu::PingDisplay.Value, Vars::Menu::Indicators.Value & (1 << 4));

		Cursor = ImGui::GetMouseCursor();
	}

	// End frame and render
	ImGui::PopFont();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
}

void CMenu::LoadStyle()
{
	// Style & Colors
	{
		Accent = ImGui::ColorToVec(Vars::Menu::Theme::Accent.Value);
		AccentLight = ImColor(Accent.Value.x * 1.1f, Accent.Value.y * 1.1f, Accent.Value.z * 1.1f, Accent.Value.w);
		Background = ImGui::ColorToVec(Vars::Menu::Theme::Background.Value);
		Foreground = ImGui::ColorToVec(Vars::Menu::Theme::Foreground.Value);
		Foremost = ImGui::ColorToVec(Vars::Menu::Theme::Foremost.Value);
		ForemostLight = ImColor(Foremost.Value.x * 1.1f, Foremost.Value.y * 1.1f, Foremost.Value.z * 1.1f, Foremost.Value.w);
		Inactive = ImGui::ColorToVec(Vars::Menu::Theme::Inactive.Value);
		Active = ImGui::ColorToVec(Vars::Menu::Theme::Active.Value);

		auto& style = ImGui::GetStyle();
		style.ButtonTextAlign = { 0.5f, 0.5f }; // Center button text
		style.CellPadding = { 4, 0 };
		style.ChildBorderSize = 0.f;
		style.ChildRounding = 0.f;
		style.FrameBorderSize = 0.f;
		style.FramePadding = { 0, 0 };
		style.FrameRounding = 3.f;
		style.ItemInnerSpacing = { 0, 0 };
		style.ItemSpacing = { 8, 8 };
		style.PopupBorderSize = 0.f;
		style.PopupRounding = 3.f;
		style.ScrollbarSize = 9.f;
		style.ScrollbarRounding = 0.f;
		style.WindowBorderSize = 0.f;
		style.WindowMinSize = { 100, 100 };
		style.WindowPadding = { 0, 0 };
		style.WindowRounding = 3.f;

		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Button] = {};
		colors[ImGuiCol_ButtonHovered] = {};
		colors[ImGuiCol_ButtonActive] = {};
		colors[ImGuiCol_FrameBg] = Foremost;
		colors[ImGuiCol_FrameBgHovered] = ForemostLight;
		colors[ImGuiCol_FrameBgActive] = Foremost;
		colors[ImGuiCol_Header] = {};
		colors[ImGuiCol_HeaderHovered] = ForemostLight;
		colors[ImGuiCol_HeaderActive] = {};
		colors[ImGuiCol_ModalWindowDimBg] = { Background.Value.x, Background.Value.y, Background.Value.z, 0.4f };
		colors[ImGuiCol_PopupBg] = ForemostLight;
		colors[ImGuiCol_ResizeGrip] = {};
		colors[ImGuiCol_ResizeGripActive] = {};
		colors[ImGuiCol_ResizeGripHovered] = {};
		colors[ImGuiCol_ScrollbarBg] = {};
		colors[ImGuiCol_SliderGrab] = Accent;
		colors[ImGuiCol_SliderGrabActive] = AccentLight;
		colors[ImGuiCol_Text] = Active;
		colors[ImGuiCol_WindowBg] = Background;
	}
}

// i think this will occasionally fail and only draw half of the menu w/o any input
void CMenu::Init(IDirect3DDevice9* pDevice)
{
	// Initialize ImGui and device
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(FindWindowA(nullptr, "Team Fortress 2"));
	ImGui_ImplDX9_Init(pDevice);

	// Fonts
	{
		const auto& io = ImGui::GetIO();

		ImFontConfig fontConfig;
		fontConfig.OversampleH = 2;
		constexpr ImWchar fontRange[]{ 0x0020, 0x00FF, 0x0400, 0x044F, 0 }; // Basic Latin, Latin Supplement and Cyrillic

		FontSmall = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 11.f, &fontConfig, fontRange);
		FontRegular = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 13.f, &fontConfig, fontRange);
		FontBold = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoBold_compressed_data, RobotoBold_compressed_size, 13.f, &fontConfig, fontRange);
		FontBlack = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoBlack_compressed_data, RobotoBlack_compressed_size, 15.f, &fontConfig, fontRange);
		FontLarge = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 20.f, &fontConfig, fontRange);

		ImFontConfig iconConfig;
		iconConfig.PixelSnapH = true;
		constexpr ImWchar iconRange[]{ ICON_MIN_MD, ICON_MAX_MD, 0 };

		IconFontRegular = io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data, MaterialIcons_compressed_size, 15.f, &iconConfig, iconRange);
		IconFontLarge = io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data, MaterialIcons_compressed_size, 16.f, &iconConfig, iconRange);

		io.Fonts->Build();
	}

	LoadStyle();
}