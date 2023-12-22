#include "Menu.h"

#include "../Vars.h"
#include "../Visuals/Radar/Radar.h"
#include "../Misc/Misc.h"
#include "../Visuals/Visuals.h"

#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/imgui_stdlib.h>

#include "Fonts/IconsMaterialDesign.h"
#include "Fonts/RobotoMedium.h"
#include "Fonts/RobotoBold.h"
#include "Fonts/RobotoBlack.h"

#include "Playerlist/Playerlist.h"

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
	ImGui::SetNextWindowSize(ImVec2(750, 550), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 750, 550 });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

	if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
	{
		const auto drawList = ImGui::GetWindowDrawList();
		const auto windowSize = ImGui::GetWindowSize();
		const auto windowPos = ImGui::GetWindowPos();

		// Title Text
		if (Vars::Menu::CheatName.Value != "")
		{
			ImGui::Dummy({ 0, 2 });
			ImGui::PushFont(FontLarge);
			const auto titleWidth = ImGui::CalcTextSize(Vars::Menu::CheatName.Value.c_str()).x;
			drawList->AddText(FontLarge, FontLarge->FontSize, { windowPos.x + (windowSize.x / 2) - (titleWidth / 2), windowPos.y }, Accent, Vars::Menu::CheatName.Value.c_str());
			ImGui::PopFont();
		}

		// Main tabs
		ImGui::FTabs({ "AIMBOT", "VISUALS", "MISC", "LOGS", "SETTINGS" }, &CurrentTab, TabSize, { 0, (Vars::Menu::CheatName.Value != "") * TitleHeight + SubTabSize.y }, true, {ICON_MD_GROUP, ICON_MD_IMAGE, ICON_MD_PUBLIC, ICON_MD_MENU_BOOK, ICON_MD_SETTINGS});
		
		// Sub tabs
		switch (CurrentTab)
		{
		case 0: ImGui::FTabs({ "GENERAL", "HVH", "AUTOMATION" }, &CurrentAimbotTab, SubTabSize, { TabSize.x, (Vars::Menu::CheatName.Value != "") * TitleHeight }); break;
		case 1: ImGui::FTabs({ "ESP", "CHAMS", "GLOW", "MISC##", "RADAR", "MENU" }, &CurrentVisualsTab, SubTabSize, { TabSize.x, (Vars::Menu::CheatName.Value != "") * TitleHeight }); break;
		case 2: ImGui::FTabs({ "MISC##" }, nullptr, SubTabSize, { TabSize.x, (Vars::Menu::CheatName.Value != "") * TitleHeight }); break;
		case 3: ImGui::FTabs({ "LOGS##", "SETTINGS##" }, &CurrentLogsTab, SubTabSize, { TabSize.x, (Vars::Menu::CheatName.Value != "") * TitleHeight }); break;
		case 4: ImGui::FTabs({ "CONFIG", "CONDITIONS", "PLAYERLIST", "MATERIALS" }, &CurrentConfigTab, SubTabSize, { TabSize.x, (Vars::Menu::CheatName.Value != "") * TitleHeight }); break;
		}

		// Main content
		ImGui::SetCursorPos({ TabSize.x, (Vars::Menu::CheatName.Value != "") * TitleHeight + SubTabSize.y });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8.f, 8.f });
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(0, 0, 0, 0).Value);
		if (ImGui::BeginChild("Content", { windowSize.x - TabSize.x, windowSize.y - (Vars::Menu::CheatName.Value != "") * TitleHeight - SubTabSize.y }, false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, Foreground.Value);
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.f);

			switch (CurrentTab)
			{
			case 0: MenuAimbot(); break;
			case 1: MenuVisuals(); break;
			case 2: MenuMisc(); break;
			case 3: MenuLogs(); break;
			case 4: MenuSettings(); break;
			}

			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		// End
		ImGui::End();
	}

	ImGui::PopStyleVar(2);
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
				FColorPicker("Target", &Vars::Colors::Target.Value);
				FToggle("Aimbot", &Vars::Aimbot::Global::Active.Value);
				FColorPicker("Aimbot FOV circle", &Vars::Colors::FOVCircle.Value);
				FKeybind("Aimbot key", Vars::Aimbot::Global::AimKey.Value);
				FToggle("Autoshoot## AimbotAutoshoot", &Vars::Aimbot::Global::AutoShoot.Value);
				FColorPicker("Invulnerable color", &Vars::Colors::Invulnerable.Value);
				FDropdown("Aimbot targets## AimbotTargets", &Vars::Aimbot::Global::AimAt.Value, { "Players", "Sentries", "Dispensers", "Teleporters", "Stickies", "NPCs", "Bombs" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Ignored targets## AimbotIgnoredTargets", &Vars::Aimbot::Global::IgnoreOptions.Value, { "Invulnerable", "Cloaked", "Dead Ringer", "Friends", "Taunting", "Vaccinator", "Unsimulated Players", "Disguised" }, {}, FDropdown_Multi | FDropdown_Right);
				if (Vars::Aimbot::Global::IgnoreOptions.Value & (1 << 1))
					FSlider("Ignore cloak", &Vars::Aimbot::Global::IgnoreCloakPercentage.Value, 0, 100, 10, "%d%%", FSlider_Clamp);
				FSlider("Max targets## AimbotMax", &Vars::Aimbot::Global::MaxTargets.Value, 1, 6, 1, "%d", FSlider_Clamp);
				if (Vars::Aimbot::Global::IgnoreOptions.Value & (1 << 6))
					FSlider("Tick tolerance## AimbotUnsimulatedTolerance", &Vars::Aimbot::Global::TickTolerance.Value, 0, 21, 1, "%d", FSlider_Clamp);
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
				FDropdown("Backtrack method## HitscanBacktrackMethod", &Vars::Backtrack::Method.Value, { "All", "Last", "Prefer OnShot" });
				FSlider("Fake latency## BTLatency", &Vars::Backtrack::Latency.Value, 0, g_ConVars.sv_maxunlag->GetFloat() * 1000 - 200, 5, "%d", FSlider_Clamp);
				FSlider("Fake interp## BTInterp", &Vars::Backtrack::Interp.Value, 0, g_ConVars.sv_maxunlag->GetFloat() * 1000 - 200, 5, "%d", FSlider_Clamp);
				FSlider("Window## BTWindow", &Vars::Backtrack::Window.Value, 1, 200, 5, "%d", FSlider_Clamp); // might switch to range slider
				FToggle("Unchoke prediction", &Vars::Backtrack::UnchokePrediction.Value);
				FToggle("Allow forward tracking", &Vars::Backtrack::AllowForward.Value, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## backtrack"))
				{
					FSlider("passthrough offset", &Vars::Backtrack::PassthroughOffset.Value, -3, 3);
					FSlider("tickset offset", &Vars::Backtrack::TicksetOffset.Value, -3, 3);
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
				FToggle("Active## HitscanAimbotActive", &Vars::Aimbot::Hitscan::Active.Value);
				FSlider("Aim FOV## HitscanFOV", &Vars::Aimbot::Hitscan::AimFOV.Value, 1.f, 180.f, 1.f, "%.0f", FSlider_Clamp);
				FDropdown("Sort method## HitscanSortMethod", &Vars::Aimbot::Hitscan::SortMethod.Value, { "FOV", "Distance" }, {}, FDropdown_Left);
				FDropdown("Aim method## HitscanAimMethod", &Vars::Aimbot::Hitscan::AimMethod.Value, { "Plain", "Smooth", "Silent" }, {}, FDropdown_Right);
				if (Vars::Aimbot::Hitscan::AimMethod.Value == 1)
					FSlider("Smooth factor## HitscanSmoothing", &Vars::Aimbot::Hitscan::SmoothingAmount.Value, 0, 100, 1, "%d", FSlider_Clamp);
				FDropdown("Hitboxes## AimbotHitboxScanning", &Vars::Aimbot::Hitscan::Hitboxes.Value, { "Head", "Body", "Pelvis", "Arms", "Legs" }, { 1 << 0, 1 << 2, 1 << 1, 1 << 3, 1 << 4 }, FDropdown_Multi);
				FSlider("Point scale## HitscanMultipointScale", &Vars::Aimbot::Hitscan::PointScale.Value, 0.f, 100.f, 5.f, "%.0f%%", FSlider_Clamp); // this seems to occasionally change on it's own and i don't know why
				FDropdown("Modifiers", &Vars::Aimbot::Hitscan::Modifiers.Value, { "Tapfire", "Wait for heatshot", "Wait for charge", "Scoped only", "Auto scope", "Bodyaim if lethal", "Extinguish team" }, {}, FDropdown_Multi);
				if (Vars::Aimbot::Hitscan::Modifiers.Value & (1 << 0))
					FSlider("Tapfire distance## HitscanTapfireDistance", &Vars::Aimbot::Hitscan::TapFireDist.Value, 250.f, 1000.f, 1.f, "%.0f", FSlider_Clamp);
			} EndSection();
			if (Section("Projectile"))
			{
				FToggle("Active", &Vars::Aimbot::Projectile::Active.Value);
				FSlider("Aim FOV", &Vars::Aimbot::Projectile::AimFOV.Value, 1.f, 180.f, 1.f, "%1.f", FSlider_Clamp);
				FDropdown("Sort method## ProjectileSortMethod", &Vars::Aimbot::Projectile::SortMethod.Value, { "FOV", "Distance" }, {}, FDropdown_Left);
				FDropdown("Aim method## ProjectileAimMethod", &Vars::Aimbot::Projectile::AimMethod.Value, { "Plain", "Smooth", "Silent" }, {}, FDropdown_Right);
				if (Vars::Aimbot::Projectile::AimMethod.Value == 1)
					FSlider("Smooth factor## ProjectileSmoothing", &Vars::Aimbot::Projectile::SmoothingAmount.Value, 0, 100, 1, "%d", FSlider_Clamp);
				FSlider("Max simulation time", &Vars::Aimbot::Projectile::PredictionTime.Value, 0.1f, 10.f, 0.1f, "%.1fs");
				FToggle("No spread", &Vars::Aimbot::Projectile::NoSpread.Value);
				FToggle("Auto release", &Vars::Aimbot::Projectile::AutoRelease.Value, FToggle_Middle);
				if (Vars::Aimbot::Projectile::AutoRelease.Value)
					FSlider("Auto release at", &Vars::Aimbot::Projectile::AutoReleaseAt.Value, 0.f, 100.f, 1.f, "%.0f%%");
				FDropdown("Predict", &Vars::Aimbot::Projectile::StrafePrediction.Value, { "Air strafing", "Ground strafing" }, {}, FDropdown_Multi);
				FSlider("Hit chance", &Vars::Aimbot::Projectile::StrafePredictionHitchance.Value, 0.f, 100.f, 5.f, "%.0f%%", FSlider_Clamp);
				FToggle("Splash prediction", &Vars::Aimbot::Projectile::SplashPrediction.Value);
				FToggle("Charge loose cannon", &Vars::Aimbot::Projectile::ChargeLooseCannon.Value, FToggle_Middle);
			} EndSection();
			if (Vars::Debug::Info.Value)
			{
				if (Section("debug## projectile"))
				{
					FSlider("samples", &Vars::Aimbot::Projectile::iSamples.Value, 3, 66);
					FSlider("vert shift", &Vars::Aimbot::Projectile::VerticalShift.Value, 0.f, 20.f, 0.5f, "%.1f");
					FSlider("latency offset", &Vars::Aimbot::Projectile::LatOff.Value, -3.f, 3.f, 0.1f, "%.1f");
					FSlider("physic offset", &Vars::Aimbot::Projectile::PhyOff.Value, -3.f, 3.f, 0.1f, "%.1f");
					FDropdown("hunterman mode", &Vars::Aimbot::Projectile::HuntermanMode.Value, { "center", "shift head", "shift up", "from top", "lerp to top" });
					if (Vars::Aimbot::Projectile::HuntermanMode.Value == 1 || Vars::Aimbot::Projectile::HuntermanMode.Value == 2)
						FSlider("hunterman shift", &Vars::Aimbot::Projectile::HuntermanShift.Value, 0.f, 10.f, 0.5f, "%.1f");
					if (Vars::Aimbot::Projectile::HuntermanMode.Value == 4)
						FSlider("hunterman lerp", &Vars::Aimbot::Projectile::HuntermanLerp.Value, 0.f, 100.f, 1.f, "%.0f%%");
				} EndSection();
			}
			if (Section("Melee"))
			{
				FToggle("Active## MeleeAimbotActive", &Vars::Aimbot::Melee::Active.Value);
				FSlider("Aim FOV## MeleeFOV", &Vars::Aimbot::Melee::AimFOV.Value, 1.f, 180.f, 1.f, "%1.f", FSlider_Clamp);
				FDropdown("Aim method## MeleeAimMethod", &Vars::Aimbot::Melee::AimMethod.Value, { "Plain", "Smooth", "Silent" });
				if (Vars::Aimbot::Melee::AimMethod.Value == 1)
					FSlider("Smooth factor## MeleeSmoothing", &Vars::Aimbot::Melee::SmoothingAmount.Value, 0, 100, 1, "%d", FSlider_Clamp);
				FToggle("Auto backstab", &Vars::Aimbot::Melee::AutoBackstab.Value);
				FToggle("Ignore razorback", &Vars::Aimbot::Melee::IgnoreRazorback.Value, FToggle_Middle);
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

				FToggle("Enabled## DoubleTapEnabled", &Vars::CL_Move::DoubleTap::Enabled.Value);
				FSlider("Tick limit", &Vars::CL_Move::DoubleTap::TickLimit.Value, 1, iTicks, 1, "%d", FSlider_Clamp);
				FSlider("Warp rate", &Vars::CL_Move::DoubleTap::WarpRate.Value, 1, iTicks, 1, "%d", FSlider_Clamp);
				FSlider("Passive recharge", &Vars::CL_Move::DoubleTap::PassiveRecharge.Value, 0, iTicks, 1, "%d", FSlider_Clamp);
				FDropdown("Mode## DTmode", &Vars::CL_Move::DoubleTap::Mode.Value, { "Always", "Hold", "Toggle" });
				if (Vars::CL_Move::DoubleTap::Mode.Value != 0)
					FKeybind("Doubletap key", Vars::CL_Move::DoubleTap::DoubletapKey.Value);
				FKeybind("Recharge key", Vars::CL_Move::DoubleTap::RechargeKey.Value);
				FKeybind("Teleport key", Vars::CL_Move::DoubleTap::TeleportKey.Value);
				FDropdown("Options", &Vars::CL_Move::DoubleTap::Options.Value, { "Anti-warp", "Avoid airborne", "Auto retain", "Auto Recharge", "Recharge While Dead" }, {}, FDropdown_Multi);
			} EndSection();
			if (Section("Fakelag"))
			{
				FToggle("Enabled## FakelagEnabled", &Vars::CL_Move::FakeLag::Enabled.Value);
				FDropdown("Mode## FLmode", &Vars::CL_Move::FakeLag::Mode.Value, { "Always", "Hold", "Toggle" });
				if (Vars::CL_Move::FakeLag::Mode.Value != 0)
					FKeybind("Fakelag key", Vars::CL_Move::FakeLag::Key.Value);
				FDropdown("Type## FLtype", &Vars::CL_Move::FakeLag::Type.Value, { "Plain", "Random", "Adaptive" });

				switch (Vars::CL_Move::FakeLag::Type.Value)
				{
				case 0: FSlider("Fakelag value", &Vars::CL_Move::FakeLag::Value.Value, 1, 22, 1, "%d", FSlider_Clamp); break;
				case 1: FSlider("Random value", &Vars::CL_Move::FakeLag::Min.Value, &Vars::CL_Move::FakeLag::Max.Value, 1, 22, 1, "%d - %d", FSlider_Clamp);
				}
				FDropdown("Options", &Vars::CL_Move::FakeLag::Options.Value, { "While Moving", "While Unducking", "While Airborne" }, {}, FDropdown_Multi);
				FToggle("Unchoke on attack", &Vars::CL_Move::FakeLag::UnchokeOnAttack.Value);
				FToggle("Retain blastjump", &Vars::CL_Move::FakeLag::RetainBlastJump.Value, FToggle_Middle);
			} EndSection();

			if (Section("Speedhack"))
			{
				FToggle("Speedhack", &Vars::CL_Move::SpeedEnabled.Value);
				if (Vars::CL_Move::SpeedEnabled.Value)
					FSlider("SpeedHack factor", &Vars::CL_Move::SpeedFactor.Value, 1, 50, 1);
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
				if (Vars::AntiHack::AntiAim::YawFake.Value == 5 || Vars::AntiHack::AntiAim::YawReal.Value == 5)
					FSlider("Spin Speed", &Vars::AntiHack::AntiAim::SpinSpeed.Value, -30.f, 30.f, 1.f, "%.0f", 0);
				FDropdown("Real offset", &Vars::AntiHack::AntiAim::RealYawMode.Value, { "Offset", "FOV Player", "FOV Player + Offset" }, {}, FDropdown_Left);
				FDropdown("Fake offset", &Vars::AntiHack::AntiAim::FakeYawMode.Value, { "Offset", "FOV Player", "FOV Player + Offset" }, {}, FDropdown_Right);
				//if (Vars::AntiHack::AntiAim::RealYawMode.Value != 1)
					FSlider("Real offset## RealYawOffset", &Vars::AntiHack::AntiAim::RealYawOffset.Value, -180.f, 180.f, 5.f, "%.0f", FSlider_Left);
				//if (Vars::AntiHack::AntiAim::FakeYawMode.Value != 1)
					FSlider("Fake offset## FakeYawOffset", &Vars::AntiHack::AntiAim::FakeYawOffset.Value, -180.f, 180.f, 5.f, "%.0f", FSlider_Right);
				FToggle("Anti-overlap", &Vars::AntiHack::AntiAim::AntiOverlap.Value);
				FToggle("Hide pitch on shot", &Vars::AntiHack::AntiAim::InvalidShootPitch.Value, FToggle_Middle);
			} EndSection();
			if (Section("Auto Peek"))
			{
				FKeybind("Autopeek Key", Vars::CL_Move::AutoPeekKey.Value);
				FSlider("Max distance", &Vars::CL_Move::AutoPeekDistance.Value, 50.f, 400.f, 10.f); // probably get rid of this, doesn't work well
				FToggle("Free move", &Vars::CL_Move::AutoPeekFree.Value);
			} EndSection();
			if (Section("Cheater Detection"))
			{
				FToggle("Enabled## CheaterDetectionEnabled", &Vars::Misc::CheaterDetection::Enabled.Value);
				if (Vars::Misc::CheaterDetection::Enabled.Value)
				{
					FDropdown("Detection methods## CheaterDetectionMethods", &Vars::Misc::CheaterDetection::Methods.Value, { "Accuracy", "Score", "Simtime Changes", "Packet Choking", "Bunnyhopping", "Aim Flicking", "OOB Angles", "Aimbot", "Duck Speed" }, {}, FDropdown_Multi);
					FDropdown("Ignore conditions## CheaterDetectionIgnoreMethods", &Vars::Misc::CheaterDetection::Protections.Value, { "Double Scans", "Lagging Client", "Timing Out" }, {}, FDropdown_Multi);
					FSlider("Suspicion gate", &Vars::Misc::CheaterDetection::SuspicionGate.Value, 5, 50, 1);
					if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 1))
						FSlider("Analytical high score mult", &Vars::Misc::CheaterDetection::ScoreMultiplier.Value, 1.5f, 4.f, 0.1f, "%.1f");
					if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 3 | 1 << 2))
						FSlider("Packet manipulation gate", &Vars::Misc::CheaterDetection::PacketManipGate.Value, 1, 22, 1);
					if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 4))
					{
						FSlider("BHop sensitivity", &Vars::Misc::CheaterDetection::BHopMaxDelay.Value, 1, 5, 1);
						FSlider("BHop minimum detections", &Vars::Misc::CheaterDetection::BHopDetectionsRequired.Value, 2, 15, 1);
					}
					if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 5))
					{
						FSlider("Minimum aim-flick", &Vars::Misc::CheaterDetection::MinimumFlickDistance.Value, 5.f, 30.f, 0.1f, "%.1f");
						FSlider("Maximum post flick noise", &Vars::Misc::CheaterDetection::MaximumNoise.Value, 5.f, 15.f, 0.1f, "%.1f");
					}
					if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 7))
					{
						FSlider("Maximum scaled aimbot FOV", &Vars::Misc::CheaterDetection::MaxScaledAimbotFoV.Value, 5.f, 30.f, 0.1f, "%.1f");
						FSlider("Minimum aimbot FOV", &Vars::Misc::CheaterDetection::MinimumAimbotFoV.Value, 5.f, 30.f, 0.1f, "%.1f");
					}
				}
			} EndSection();
			if (Section("Resolver"))
			{
				FToggle("Enabled## ResolverEnabled", &Vars::AntiHack::Resolver::Resolver.Value);
				if (Vars::AntiHack::Resolver::Resolver.Value)
					FToggle("Ignore in-air", &Vars::AntiHack::Resolver::IgnoreAirborne.Value, FToggle_Middle);
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
				FToggle("Active## TriggerMain", &Vars::Auto::Global::Active.Value);
				FKeybind("Trigger key", Vars::Auto::Global::AutoKey.Value);
				FDropdown("Ignored targets## IgnoredTargets", &Vars::Auto::Global::IgnoreOptions.Value, { "Invulnerable", "Cloaked", "Friends", "Taunting", "Unsimulated Players", "Disguised" }, {}, FDropdown_Multi);
			} EndSection();
			if (Section("Autouber"))
			{
				FToggle("Active## TriggerUber", &Vars::Auto::Uber::Active.Value);
				FToggle("Only uber friends", &Vars::Auto::Uber::OnlyFriends.Value);
				FToggle("Preserve self", &Vars::Auto::Uber::PopLocal.Value);
				FDropdown("Auto vaccinator", &Vars::Auto::Uber::AutoVaccinator.Value, { "Bullet", "Blast", "Fire" }, {}, FDropdown_Multi | FDropdown_Left);
				FDropdown("Bullet react classes## AutoVaccClasses", &Vars::Auto::Uber::ReactClasses.Value, { "Scout", "Soldier", "Pyro", "Heavy", "Engineer", "Sniper", "Spy" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 4, 1 << 5, 1 << 7, 1 << 8 }, FDropdown_Multi | FDropdown_Right);
				FSlider("Health left## TriggerUberHealthLeft", &Vars::Auto::Uber::HealthLeft.Value, 1.f, 99.f, 1.f, "%.0f%%", 1.0f);
				FSlider("Reaction FOV## TriggerUberReactFoV", &Vars::Auto::Uber::ReactFOV.Value, 0, 90, 1, "%d", 1);
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
				FToggle("Active## TriggerDet", &Vars::Auto::Detonate::Active.Value);
				FDropdown("Targets## TriggerbotDetonateTargets", &Vars::Auto::Detonate::DetonateTargets.Value, { "Players", "Sentries", "Dispensers", "Teleporters", "NPCs", "Bombs", "Stickies" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6 }, FDropdown_Multi);
				FToggle("Stickies## TriggerSticky", &Vars::Auto::Detonate::Stickies.Value);
				FToggle("Flares## TriggerFlares", &Vars::Auto::Detonate::Flares.Value, FToggle_Middle);
				FSlider("Detonation radius## TriggerDetRadius", &Vars::Auto::Detonate::RadiusScale.Value, 0.f, 100.f, 1.f, "%.0f%%", FSlider_Clamp);
			} EndSection();
			if (Section("Autoblast"))
			{
				FToggle("Active## TriggreAirblast", &Vars::Auto::Airblast::Active.Value);
				FToggle("Rage## TriggerAirRage", &Vars::Auto::Airblast::Rage.Value);
				FToggle("Silent## triggerblastsilent", &Vars::Auto::Airblast::Silent.Value);
				FToggle("Extinguish players## TriggerExtinguishPlayers", &Vars::Auto::Airblast::ExtinguishPlayers.Value);
				FToggle("Disable on attack## TriggerDisableOnAttack", &Vars::Auto::Airblast::DisableOnAttack.Value);
				FSlider("FOV## AirBlastFov", &Vars::Auto::Airblast::Fov.Value, 0, 90, 1, "%d", FSlider_Clamp);
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
				FToggle("Active## EnableESP", &Vars::ESP::Main::Active.Value);
				if (Vars::ESP::Main::EnableTeamEnemyColors.Value)
				{
					FColorPicker("Enemy color", &Vars::Colors::Enemy.Value);
					FColorPicker("Team color", &Vars::Colors::Team.Value, 1);
				}
				else
				{
					FColorPicker("RED Team color", &Vars::Colors::TeamRed.Value);
					FColorPicker("BLU Team color", &Vars::Colors::TeamBlu.Value, 1);
				}
				FToggle("Relative colors", &Vars::ESP::Main::EnableTeamEnemyColors.Value);
				FToggle("Dormant sound ESP", &Vars::ESP::Main::DormantSoundESP.Value);
				if (Vars::ESP::Main::DormantSoundESP.Value)
					FSlider("Dormant Decay Time## GlobalDormantDecayTime", &Vars::ESP::Main::DormantTime.Value, 0.015f, 5.0f, 0.1f, "%.1f", ImGuiSliderFlags_Logarithmic);
			} EndSection();
			if (Section("Player"))
			{
				FToggle("Active## PlayerESP", &Vars::ESP::Players::Active.Value);
				FColorPicker("Local color", &Vars::Colors::Local.Value, 1); FColorPicker("Friend color", &Vars::Colors::Friend.Value);
				FToggle("Ignore local## SelfESP", &Vars::ESP::Players::IgnoreLocal.Value);
				FToggle("Ignore team## IgnoreTeamESP", &Vars::ESP::Players::IgnoreTeam.Value, FToggle_Middle);
				FColorPicker("Cloaked color", &Vars::Colors::Cloak.Value);
				FToggle("Ignore cloaked## IgnoreCloakESP", &Vars::ESP::Players::IgnoreCloaked.Value);
				FToggle("Name## PlayerNameESP", &Vars::ESP::Players::Name.Value);
				FColorPicker("Health Bar Top", &Vars::Colors::HealthBar.Value.StartColor, 2); FColorPicker("Health Bar Bottom", &Vars::Colors::HealthBar.Value.EndColor, 1); FColorPicker("Overheal color", &Vars::Colors::Overheal.Value);
				FToggle("Health bar## PlayerHealthBar", &Vars::ESP::Players::HealthBar.Value);
				FToggle("Health text## PlayerHealthText", &Vars::ESP::Players::HealthText.Value, FToggle_Middle);
				FColorPicker("Uber Bar", &Vars::Colors::UberBar.Value);
				FToggle("Uber bar## PlayerUberBar", &Vars::ESP::Players::UberBar.Value);
				FToggle("Uber text## PlayerUberText", &Vars::ESP::Players::UberText.Value, FToggle_Middle);
				FToggle("Class icon## PlayerClassIcon", &Vars::ESP::Players::ClassIcon.Value);
				FToggle("Class text## PlayerClassText", &Vars::ESP::Players::ClassText.Value, FToggle_Middle);
				FColorPicker("Weapon color", &Vars::Colors::Weapon.Value);
				FToggle("Weapon icon", &Vars::ESP::Players::WeaponIcon.Value);
				FToggle("Weapon text", &Vars::ESP::Players::WeaponText.Value, FToggle_Middle);
				FToggle("Distance", &Vars::ESP::Players::Distance.Value);
				FToggle("Box## PlayerBoxESP", &Vars::ESP::Players::Box.Value);
				FToggle("Skeleton## PlayerBones", &Vars::ESP::Players::Bones.Value, FToggle_Middle);
				FToggle("Priority Text", &Vars::ESP::Players::PriorityText.Value);
				FDropdown("Conditions", &Vars::ESP::Players::Conditions.Value, { "Buffs", "Debuffs", "Other Conditions", "Lag Compensation", "KD", "Ping" }, {}, FDropdown_Multi);
				FSlider("Alpha## PlayerESPAlpha", &Vars::ESP::Players::Alpha.Value, 0.01f, 1.0f, 0.1f, "%.1f", FSlider_Clamp);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Building"))
			{
				FToggle("Active## BuildingESP", &Vars::ESP::Buildings::Active.Value);
				FToggle("Ignore team## BuildingESPIgnoreTeammates", &Vars::ESP::Buildings::IgnoreTeam.Value);
				FToggle("Name## BuildingNameESP", &Vars::ESP::Buildings::Name.Value);
				FToggle("Health bar## Buildinghelathbar", &Vars::ESP::Buildings::HealthBar.Value);
				FToggle("Health text## buildinghealth", &Vars::ESP::Buildings::HealthText.Value, FToggle_Middle);
				FToggle("Distance", &Vars::ESP::Buildings::Distance.Value);
				FToggle("Box## PBuildingBoxESP", &Vars::ESP::Buildings::Box.Value, FToggle_Middle);
				FToggle("Owner## Buildingowner", &Vars::ESP::Buildings::Owner.Value);
				FToggle("Level## Buildinglevel", &Vars::ESP::Buildings::Level.Value, FToggle_Middle);
				FToggle("Condition## Buildingconditions", &Vars::ESP::Buildings::Condition.Value);
				FSlider("Alpha## BuildingESPAlpha", &Vars::ESP::Buildings::Alpha.Value, 0.01f, 1.0f, 0.1f, "%.1f", FSlider_Clamp);
			} EndSection();
			if (Section("World"))
			{
				FToggle("Active## WorldESP", &Vars::ESP::World::Active.Value);
				FColorPicker("Healthpack color", &Vars::Colors::Health.Value);
				FToggle("Healthpack## WorldESPHealthpack", &Vars::ESP::World::Health.Value);
				FColorPicker("Ammopack color", &Vars::Colors::Ammo.Value);
				FToggle("Ammopack## WorldESPAmmopack", &Vars::ESP::World::Ammo.Value);
				FColorPicker("NPC color", &Vars::Colors::NPC.Value);
				FToggle("NPC## WorldESPNPC", &Vars::ESP::World::NPC.Value);
				FColorPicker("Bomb color", &Vars::Colors::Bomb.Value);
				FToggle("Bombs## WorldESPBomb", &Vars::ESP::World::Bomb.Value);
				FColorPicker("Spellbook color", &Vars::Colors::Spellbook.Value);
				FToggle("Spellbook## WorldESPSpellbook", &Vars::ESP::World::Spellbook.Value);
				FColorPicker("Gargoyle color", &Vars::Colors::Gargoyle.Value);
				FToggle("Gargoyle## WorldESPGargoyle", &Vars::ESP::World::Gargoyle.Value);
				FSlider("Alpha## WorldESPAlpha", &Vars::ESP::World::Alpha.Value, 0.01f, 1.0f, 0.1f, "%.1f", FSlider_Clamp);
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
			if (Section("Chams"))
				FToggle("Active## ChamsMasterSwitch", &Vars::Chams::Active.Value);
			EndSection();

			if (Section("Player"))
			{
				static std::vector chamOptions{
					"Local",
					"FakeAngles",
					"Friends",
					"Enemies",
					"Teammates",
					"Target",
					"Backtrack",
					"Weapon",
					"Hands",
					"Ragdoll"
				};
				static int currentSelected = 0;
				Chams_t& currentStruct = ([&]() -> Chams_t&
					{
						switch (currentSelected)
						{
						case 0: return Vars::Chams::Players::Local.Value;
						case 1: return Vars::Chams::Players::FakeAng.Value;
						case 2: return Vars::Chams::Players::Friend.Value;
						case 3: return Vars::Chams::Players::Enemy.Value;
						case 4: return Vars::Chams::Players::Team.Value;
						case 5: return Vars::Chams::Players::Target.Value;
						case 6: return Vars::Chams::Players::Backtrack.Value;
						case 7: return Vars::Chams::Players::Weapon.Value;
						case 8: return Vars::Chams::Players::Hands.Value;
						case 9: return Vars::Chams::Players::Ragdoll.Value;
						}

						return Vars::Chams::Players::Local.Value;
					}());

				FDropdown("Config", &currentSelected, chamOptions);
				
				if (currentSelected != 7 && currentSelected != 8) 
				{
					// hide for viewmodel stuff
					// has to be done for some reason, otherwise won't work
					FToggle("Active", &currentStruct.ChamsActive);
					FToggle("Obstructed", &currentStruct.IgnoreZ, FToggle_Middle);
				}

				FMDropdown("Base material", &currentStruct.Material, "Original", FSDropdown_Left, 1);
				FColorPicker("Base color", &currentStruct.Color, 0, FColorPicker_Dropdown);
				FMDropdown("Overlay material", &currentStruct.OverlayMaterial, "None", FSDropdown_Right, 1);
				FColorPicker("Overlay color", &currentStruct.OverlayColor, 0, FColorPicker_Dropdown);

				if (currentSelected == 6)
					FToggle("Only last", &Vars::Backtrack::LastOnly.Value);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Building"))
			{
				static std::vector chamOptions{
					"Local",
					"Friends",
					"Enemies",
					"Teammates",
					"Target"
				};
				static int currentSelected = 0;
				Chams_t& currentStruct = ([&]() -> Chams_t&
					{
						switch (currentSelected)
						{
						case 0: return Vars::Chams::Buildings::Local.Value;
						case 1: return Vars::Chams::Buildings::Friend.Value;
						case 2: return Vars::Chams::Buildings::Enemy.Value;
						case 3: return Vars::Chams::Buildings::Team.Value;
						case 4: return Vars::Chams::Buildings::Target.Value;
						}

						return Vars::Chams::Buildings::Local.Value;
					}());

				FDropdown("Config## BuildingConfig", &currentSelected, chamOptions);
				
				FToggle("Active## BuildingActive", &currentStruct.ChamsActive);
				FToggle("Obstructed## BuildingObstructed", &currentStruct.IgnoreZ, FToggle_Middle);

				FMDropdown("Base material## BuildingBase", &currentStruct.Material, "Original", FSDropdown_Left, 1);
				FColorPicker("Base color## BuildingBaseColor", &currentStruct.Color, 0, FColorPicker_Dropdown);
				FMDropdown("Overlay material## BuildingOverlay", &currentStruct.OverlayMaterial, "None", FSDropdown_Right, 1);
				FColorPicker("Overlay color## BuildingOverlayColor", &currentStruct.OverlayColor, 0, FColorPicker_Dropdown);
			} EndSection();
			if (Section("World"))
			{
				static std::vector chamOptions{
					"Healthpacks",
					"Ammopacks",
					"Team Projectiles",
					"Enemy Projectiles",
				};
				static int currentSelected = 0;
				Chams_t& currentStruct = ([&]() -> Chams_t&
					{
						switch (currentSelected)
						{
						case 0: return Vars::Chams::World::Health.Value;
						case 1: return Vars::Chams::World::Ammo.Value;
						case 2: return Vars::Chams::World::Projectiles::Team.Value;
						case 3: return Vars::Chams::World::Projectiles::Enemy.Value;
						}

						return Vars::Chams::World::Health.Value;
					}());

				FDropdown("Config## WorldConfig", &currentSelected, chamOptions);
				
				FToggle("Active## WorldActive", &currentStruct.ChamsActive);
				FToggle("Obstructed## WorldObstructed", &currentStruct.IgnoreZ, FToggle_Middle);

				FMDropdown("Base material## WorldBase", &currentStruct.Material, "Original", FSDropdown_Left, 1);
				FColorPicker("Base color## WorldBaseColor", &currentStruct.Color, 0, FColorPicker_Dropdown);
				FMDropdown("Overlay material## WorldOverlay", &currentStruct.OverlayMaterial, "None", FSDropdown_Right, 1);
				FColorPicker("Overlay color## WorldOverlayColor", &currentStruct.OverlayColor, 0, FColorPicker_Dropdown);
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
			if (Section("Glow"))
			{
				FToggle("Active## GlowActive", &Vars::Glow::Main::Active.Value);
				FDropdown("Type## GlowTypeSelect", &Vars::Glow::Main::Type.Value, { "Blur", "Stencil" });
				FSlider("Scale", &Vars::Glow::Main::Scale.Value, 1, 10, 1, "%d", FSlider_Clamp);
			} EndSection();
			if (Section("Player"))
			{
				FToggle("Active## PlayerGlow", &Vars::Glow::Players::Active.Value);
				FToggle("Self glow## SelfGlow", &Vars::Glow::Players::ShowLocal.Value);
				FDropdown("Ignore team## IgnoreTeamGlow", &Vars::Glow::Players::IgnoreTeammates.Value, { "Off", "All", "Only friends" });
			} EndSection();
			if (Section("Building"))
			{
				FToggle("Active## BuildiongGlow", &Vars::Glow::Buildings::Active.Value);
				FToggle("Ignore team## IgnoreTeamBuildingGlow", &Vars::Glow::Buildings::IgnoreTeammates.Value);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("World"))
			{
				FToggle("Active## WorldGlow", &Vars::Glow::World::Active.Value);
				FToggle("Healthpacks## WorldHealthpackGlow", &Vars::Glow::World::Health.Value);
				FToggle("Ammopacks## WorldAmmopackGlow", &Vars::Glow::World::Ammo.Value);
				FToggle("NPCs## WorldNPCGlow", &Vars::Glow::World::NPCs.Value);
				FToggle("Bombs## WorldBombGlow", &Vars::Glow::World::Bombs.Value);
				FToggle("Spellbook## WorldSpellbookGlow", &Vars::Glow::World::Spellbook.Value);
				FToggle("Gargoyle## WorldGargoyleGlow", &Vars::Glow::World::Gargoyle.Value);
				FDropdown("Projectiles## ProjectileGlow", &Vars::Glow::World::Projectiles.Value, { "Off", "All", "Only enemies" });
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
				FToggle("Screen overlays", &Vars::Visuals::RemoveScreenOverlays.Value, FToggle_Middle);
				FToggle("Interpolation", &Vars::Misc::DisableInterpolation.Value);
				FToggle("Screen effects", &Vars::Visuals::RemoveScreenEffects.Value, FToggle_Middle);
				FToggle("View punch", &Vars::Visuals::RemovePunch.Value);
				FToggle("Angle forcing", &Vars::Visuals::PreventForcedAngles.Value, FToggle_Middle);
				FToggle("MOTD", &Vars::Visuals::RemoveMOTD.Value);
				FToggle("Convar queries", &Vars::Visuals::RemoveConvarQueries.Value, FToggle_Middle);
				FToggle("Disguises", &Vars::Visuals::RemoveDisguises.Value);
				FToggle("Taunts", &Vars::Visuals::RemoveTaunts.Value, FToggle_Middle);
				FToggle("Ragdolls", &Vars::Visuals::RemoveRagdolls.Value);
				FToggle("DSP", &Vars::Visuals::RemoveDSP.Value, FToggle_Middle);
			} EndSection();
			if (Section("UI"))
			{
				FSlider("Field of view", &Vars::Visuals::FieldOfView.Value, 30, 150, 1);
				FSlider("Zoomed field of view", &Vars::Visuals::ZoomFieldOfView.Value, 30, 150, 1);
				FToggle("Reveal scoreboard", &Vars::Visuals::RevealScoreboard.Value);
				FToggle("Scoreboard colors", &Vars::Visuals::ScoreboardColors.Value);
				FToggle("Scoreboard playerlist", &Vars::Misc::ScoreboardPlayerlist.Value, FToggle_Middle);
				FToggle("Clean screenshots", &Vars::Visuals::CleanScreenshots.Value);
				FToggle("Sniper sightlines", &Vars::Visuals::SniperSightlines.Value);
				FToggle("Pickup timers", &Vars::Visuals::PickupTimers.Value, FToggle_Middle);
				FToggle("Post processing", &Vars::Visuals::DoPostProcessing.Value);
				FToggle("No prop fade", &Vars::Visuals::NoStaticPropFade.Value, FToggle_Middle);
			} EndSection();
			if (Section("Viewmodel"))
			{
				FToggle("Crosshair aim position", &Vars::Visuals::CrosshairAimPos.Value);
				FToggle("Viewmodel aim position", &Vars::Visuals::AimbotViewmodel.Value, FToggle_Middle);
				FSlider("VM Off X", &Vars::Visuals::VMOffsets.Value.x, -45.f, 45.f);
				FSlider("VM Off Y", &Vars::Visuals::VMOffsets.Value.y, -45.f, 45.f);
				FSlider("VM Off Z", &Vars::Visuals::VMOffsets.Value.z, -45.f, 45.f);
				FSlider("VM Roll", &Vars::Visuals::VMRoll.Value, -180, 180);
				FToggle("Viewmodel sway", &Vars::Visuals::ViewmodelSway.Value);
				if (Vars::Visuals::ViewmodelSway.Value)
				{
					FSlider("Sway scale", &Vars::Visuals::ViewmodelSwayScale.Value, 0.01f, 5.f, 0.1f, "%.1f");
					FSlider("Sway interp", &Vars::Visuals::ViewmodelSwayInterp.Value, 0.01f, 1.f, 0.1f, "%.1f");
				}
			} EndSection();
			if (Section("Tracers"))
			{
				FSDropdown("Bullet trail", &Vars::Visuals::Tracers::ParticleTracer.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP Beam 2", "Big Nasty", "Distortion Trail", "Black Ink" }, FSDropdown_Custom | FSDropdown_Left);
				FSDropdown("Crit trail", &Vars::Visuals::Tracers::ParticleTracerCrits.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP Beam 2", "Big Nasty", "Distortion Trail", "Black Ink" }, FSDropdown_Custom | FSDropdown_Right);
			} EndSection();
			if (Section("Ragdolls"))
			{
				FToggle("Enemy only## RagdollEnemyOnly", &Vars::Visuals::Ragdolls::EnemyOnly.Value);
				FToggle("No gibs", &Vars::Visuals::Ragdolls::NoGib.Value, FToggle_Middle);
				FDropdown("Effects## RagdollEffects", &Vars::Visuals::Ragdolls::Effects.Value, { "Burning", "Electrocuted", "Become ash", "Dissolve" }, {}, FDropdown_Multi);
				FDropdown("Ragdoll model", &Vars::Visuals::Ragdolls::Type.Value, { "None", "Gold", "Ice" });
				FSlider("Ragdoll force", &Vars::Visuals::Ragdolls::Force.Value, -10.f, 10.f, 0.5f, "%.1f");
				FSlider("Horizontal force", &Vars::Visuals::Ragdolls::ForceHorizontal.Value, -10.f, 10.f, 0.5f, "%.1f");
				FSlider("Vertical force", &Vars::Visuals::Ragdolls::ForceVertical.Value, -10.f, 10.f, 0.5f, "%.1f");
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
				if (Vars::Visuals::SimSeperators.Value)
				{
					FSlider("Seperator length", &Vars::Visuals::SeperatorLength.Value, 2, 16, 1, "%d", FSlider_Left);
					FSlider("Seperator spacing", &Vars::Visuals::SeperatorSpacing.Value, 1, 64, 1, "%d", FSlider_Right);
				}
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
				FToggle("Draw Hitboxes", &Vars::Aimbot::Global::ShowHitboxes.Value);
			} EndSection();
			if (Section("Thirdperson"))
			{
				FToggle("Thirdperson", &Vars::Visuals::ThirdPerson::Active.Value);
				FKeybind("Thirdperson key", Vars::Visuals::ThirdPerson::Key.Value);
				FSlider("Thirdperson distance", &Vars::Visuals::ThirdPerson::Distance.Value, 0.f, 500.f, 5.f);
				FSlider("Thirdperson right", &Vars::Visuals::ThirdPerson::Right.Value, -500.f, 500.f, 5.f);
				FSlider("Thirdperson up", &Vars::Visuals::ThirdPerson::Up.Value, -500.f, 500.f, 5.f);
				FToggle("Thirdperson crosshair", &Vars::Visuals::ThirdPerson::Crosshair.Value);
			} EndSection();
			if (Section("Out of FOV arrows"))
			{
				FToggle("Active## FOVArrowActive", &Vars::Visuals::Arrows::Active.Value);
				FSlider("Offset## FOVArrowOffset", &Vars::Visuals::Arrows::Offset.Value, 0, 1000);
				FSlider("Max distance", &Vars::Visuals::Arrows::MaxDist.Value, 0.f, 5000.f, 10.f);
			} EndSection();
			if (Section("World"))
			{
				if (FColorPicker("World modulation", &Vars::Colors::WorldModulation.Value, 3) ||
					FColorPicker("Sky modulation", &Vars::Colors::SkyModulation.Value, 2) ||
					FColorPicker("Prop modulation", &Vars::Colors::StaticPropModulation.Value, 1))
				{
					G::ShouldUpdateMaterialCache = true;
				}
				FColorPicker("Particle modulation", &Vars::Colors::ParticleModulation.Value);
				FDropdown("Modulations", &Vars::Visuals::World::Modulations.Value, { "World", "Sky", "Prop", "Particle" }, { }, FDropdown_Left | FDropdown_Multi);
				static std::vector skyNames = {
					"Off", "sky_tf2_04", "sky_upward", "sky_dustbowl_01", "sky_goldrush_01", "sky_granary_01", "sky_well_01", "sky_gravel_01", "sky_badlands_01",
					"sky_hydro_01", "sky_night_01", "sky_nightfall_01", "sky_trainyard_01", "sky_stormfront_01", "sky_morningsnow_01","sky_alpinestorm_01",
					"sky_harvest_01", "sky_harvest_night_01", "sky_halloween", "sky_halloween_night_01", "sky_halloween_night2014_01", "sky_island_01", "sky_rainbow_01"
				};
				FSDropdown("Skybox changer", &Vars::Visuals::World::SkyboxChanger.Value, skyNames, FSDropdown_Custom | FSDropdown_Right);
				FToggle("World textures override", &Vars::Visuals::World::OverrideTextures.Value);
				FToggle("Prop wireframe", &Vars::Visuals::World::PropWireframe.Value, FToggle_Middle);
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
				FToggle("Active## RadarActive", &Vars::Radar::Main::Active.Value);
				FSlider("Range", &Vars::Radar::Main::Range.Value, 50, 3000);
				FSlider("Background alpha", &Vars::Radar::Main::BackAlpha.Value, 0, 255);
				FSlider("Line alpha", &Vars::Radar::Main::LineAlpha.Value, 0, 255);
			} EndSection();
			if (Section("Player"))
			{
				FToggle("Active## PlayerActive", &Vars::Radar::Players::Active.Value);
				FDropdown("Icon", &Vars::Radar::Players::IconType.Value, { "Scoreboard", "Portraits", "Avatar" });
				FDropdown("Background", &Vars::Radar::Players::BackGroundType.Value, { "Off", "Rectangle", "Texture" });
				FToggle("Outline", &Vars::Radar::Players::Outline.Value);
				FDropdown("Ignore teammates", &Vars::Radar::Players::IgnoreTeam.Value, { "Off", "All", "Keep friends" });
				FDropdown("Ignore cloaked", &Vars::Radar::Players::IgnoreCloaked.Value, { "Off", "All", "Keep friends" });
				FToggle("Health bar", &Vars::Radar::Players::Health.Value);
				FSlider("Icon size", &Vars::Radar::Players::IconSize.Value, 12, 30);
				FToggle("Height indicator", &Vars::Radar::Players::Height.Value);
			} EndSection();

			/* Column 2 */
			TableNextColumn();
			if (Section("Building"))
			{
				FToggle("Active## BuildingActive", &Vars::Radar::Buildings::Active.Value);
				FToggle("Outline", &Vars::Radar::Buildings::Outline.Value);
				FToggle("Ignore team", &Vars::Radar::Buildings::IgnoreTeam.Value);
				FToggle("Health bar", &Vars::Radar::Buildings::Health.Value);
				FSlider("Icon size## BuildingIconSize", &Vars::Radar::Buildings::IconSize.Value, 12, 30);
			} EndSection();
			if (Section("World"))
			{
				FToggle("Active## WorldActive", &Vars::Radar::World::Active.Value);
				FToggle("Healthpack", &Vars::Radar::World::Health.Value);
				FToggle("Ammopack", &Vars::Radar::World::Ammo.Value);
				FSlider("Icon size## WorldIconSize", &Vars::Radar::World::IconSize.Value, 12, 30);
			} EndSection();

			EndTable();
		}
		break;
	// Menu
	case 5:
		if (BeginTable("MenuTable", 2))
		{
			/* Column 1 */
			TableNextColumn();
			if (Section("General"))
			{
				if (FColorPicker("Accent color", &Vars::Menu::Theme::Accent.Value, 0, FColorPicker_Left))
					LoadStyle();
				if (FColorPicker("Foremost color", &Vars::Menu::Theme::Foremost.Value, 0, FColorPicker_Middle | FColorPicker_SameLine))
					LoadStyle();
				if (FColorPicker("Background color", &Vars::Menu::Theme::Background.Value, 0, FColorPicker_Left))
					LoadStyle();
				if (FColorPicker("Foreground color", &Vars::Menu::Theme::Foreground.Value, 0, FColorPicker_Middle | FColorPicker_SameLine))
					LoadStyle();
				if (FColorPicker("Active color", &Vars::Menu::Theme::Active.Value, 0, FColorPicker_Left))
					LoadStyle();
				if (FColorPicker("Inactive color", &Vars::Menu::Theme::Inactive.Value, 0, FColorPicker_Middle | FColorPicker_SameLine))
					LoadStyle();

				FSDropdown("Cheat title", &Vars::Menu::CheatName.Value, {}, FSDropdown_Left);
				FSDropdown("Chat info prefix", &Vars::Menu::CheatPrefix.Value, {}, FSDropdown_Right);
				FKeybind("Menu key", Vars::Menu::MenuKey.Value, true);
			} EndSection();
			if (Section("Indicators"))
				FDropdown("Indicators## MenuIndicators", &Vars::Menu::Indicators.Value, { "Ticks", "Crit hack", "Spectators", "Ping", "Conditions" }, {}, FDropdown_Multi);
			EndSection();

			/* Column 2 */
			TableNextColumn();

			EndTable();
		break;
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
			FToggle("Bunnyhop", &Vars::Misc::AutoJump.Value);
			FToggle("Auto jumpbug", &Vars::Misc::AutoJumpbug.Value, FToggle_Middle); // this is unreliable without setups, do not depend on it!
			FDropdown("Autostrafe", &Vars::Misc::AutoStrafe.Value, { "Off", "Legit", "Directional" });
			if (Vars::Misc::AutoStrafe.Value == 2)
			{
				FToggle("Only on movement key", &Vars::Misc::DirectionalOnlyOnMove.Value);
				FToggle("Only on space", &Vars::Misc::DirectionalOnlyOnSpace.Value, FToggle_Middle);
			}
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
			if (Vars::Misc::PingReducer.Value)
				FSlider("cl_cmdrate", &Vars::Misc::PingTarget.Value, 1, 66, 1, "%d", FSlider_Right);
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
		if (Section("Game"))
		{
			FToggle("Network fix", &Vars::Misc::NetworkFix.Value);
			FToggle("Prediction error jitter fix", &Vars::Misc::PredictionErrorJitterFix.Value);
			FToggle("SetupBones optimization", &Vars::Misc::SetupBonesOptimization.Value);
		} EndSection();

		/* Column 2 */
		TableNextColumn();
		if (Section("Queueing"))
		{
			FDropdown("Force regions", &Vars::Misc::ForceRegions.Value,
				{ "Atlanta", "Chicago", "Los Angeles", "Moses Lake", "Seattle", "Virginia", "Washington", "Amsterdam", "Frankfurt", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "Buenos Aires", "Lima", "Santiago", "Sao Paulo", "Chennai", "Dubai", "Guangzhou", "Hong Kong", "Mumbai", "Seoul", "Shanghai", "Singapore", "Tianjin", "Tokyo", "Sydney", "Johannesburg" },
				{ DC_ATL,	 DC_ORD,	DC_LAX,		   DC_EAT,		 DC_SEA,	DC_IAD,		DC_DFW,		  DC_AMS,	   DC_FRA,		DC_LHR,	  DC_MAD,	DC_PAR,	 DC_STO,	  DC_VIE,	DC_WAW,	  DC_EZE,		  DC_LIM, DC_SCL,	  DC_GRU,	   DC_MAA,	  DC_DXB,  DC_CAN,		DC_HKG,		 DC_BOM,   DC_SEO,	DC_SHA,		DC_SGP,		 DC_TSN,	DC_TYO,	 DC_SYD,   DC_JNB },
				FDropdown_Multi
			);
			FToggle("Freeze queue", &Vars::Misc::FreezeQueue.Value);
			FDropdown("Auto queue", &Vars::Misc::AutoCasualQueue.Value, { "Off", "In menu", "Always" });
		} EndSection();
		if (Section("Automation"))
		{
			FToggle("Anti-backstab", &Vars::Misc::AntiBackstab.Value);
			FToggle("Anti-AFK", &Vars::Misc::AntiAFK.Value, FToggle_Middle); // i don't think this works
			FToggle("Taunt control", &Vars::Misc::TauntControl.Value);
			FToggle("Kart control", &Vars::Misc::KartControl.Value, FToggle_Middle);
			FToggle("Auto accept item drops", &Vars::Misc::AutoAcceptItemDrops.Value);
		} EndSection();
		if (Section("Sound"))
		{
			FDropdown("Block sounds## SoundRemovals", &Vars::Misc::SoundBlock.Value, { "Footsteps", "Noisemaker" }, {}, FDropdown_Multi);
		} EndSection();
		if (Section("Chat"))
		{
			FToggle("Chat tags", &Vars::Misc::ChatTags.Value);
		} EndSection();
		if (Section("Steam RPC"))
		{
			FToggle("Steam RPC", &Vars::Misc::Steam::EnableRPC.Value);
			FDropdown("Match group", &Vars::Misc::Steam::MatchGroup.Value, { "Special Event", "MvM Mann Up", "Competitive", "Casual", "MvM Boot Camp" });
			FToggle("Override in menu", &Vars::Misc::Steam::OverrideMenu.Value);
			FSDropdown("Map text", &Vars::Misc::Steam::MapText.Value, { "Fedoraware", "Figoraware", "Meowhook.club", "Rathook.cc", "Nitro.tf" }, FSDropdown_Custom);
			FSlider("Group size", &Vars::Misc::Steam::GroupSize.Value, 0, 6);
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
			{	// i will probably allow logto to be changed for each individually
				FDropdown("Logs", &Vars::Logging::Logs.Value, { "Vote start", "Vote cast", "Class Changes", "Damage" }, {}, FDropdown_Left | FDropdown_Multi);
				FDropdown("Log to", &Vars::Logging::LogTo.Value, { "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, FDropdown_Right | FDropdown_Multi);
				FColorPicker("Notification background", &Vars::Logging::Notification::Background.Value);
				FColorPicker("Notification outline", &Vars::Logging::Notification::Outline.Value, 1);
				FColorPicker("Notification color", &Vars::Logging::Notification::Text.Value, 2);
				FSlider("Notification time", &Vars::Logging::Notification::Lifetime.Value, 0.5f, 3.f, 0.5f, "%.1f");
			} EndSection();

			/* Column 2 */
			TableNextColumn();

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

				FTabs({ "GENERAL", "VISUALS", }, &CurrentConfigType, { ImGui::GetColumnWidth() / 2 + 2, SubTabSize.y }, { 6, GetCursorPos().y }, false);

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
						TextColored(configName == g_CFG.GetCurrentConfig() ? F::Menu.Active.Value : F::Menu.Inactive.Value, configName.c_str());

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
						TextColored(configName == g_CFG.GetCurrentVisuals() ? F::Menu.Active.Value : F::Menu.Inactive.Value, configName.c_str());



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
			PushStyleColor(ImGuiCol_Text, F::Menu.Inactive.Value);
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
		F::PlayerList.Render();
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

				for (auto const& mat : F::Materials.m_ChamMaterials)
				{
					const auto current = GetCursorPos().y;

					SetCursorPos({ 14, current + 11 });
					TextColored(mat.bLocked ? F::Menu.Inactive.Value : F::Menu.Active.Value, mat.sName.c_str());

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
					PushStyleColor(ImGuiCol_Text, F::Menu.Inactive.Value);
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
		if (info.update)
		{
			ImGui::SetNextWindowSize({ 100.f, 40.f }, ImGuiCond_Always);
			ImGui::SetNextWindowPos({ static_cast<float>(info.x), static_cast<float>(info.y) }, ImGuiCond_Always);
			info.update = false;
		}

		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_Border, { 1.f, 1.f, 1.f, 1.f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 100.f, 40.f });
		if (ImGui::Begin(szTitle, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
		{
			ImGui::PushFont(FontBlack);
			auto size = ImGui::CalcTextSize(szTitle);
			ImGui::SetCursorPos({ (100 - size.x) * 0.5f, (40 - size.y) * 0.5f });
			ImGui::Text(szTitle);
			ImGui::PopFont();

			const ImVec2 winPos = ImGui::GetWindowPos();
			const ImVec2 winSize = ImGui::GetWindowSize();

			info.x = static_cast<int>(winPos.x);
			info.y = static_cast<int>(winPos.y);

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

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.35f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.f, 0.f, 0.f, 0.5f));
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

		ImGui::End();
	}

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
}

/* Window for the camera feature */
void CMenu::DrawCameraWindow()
{
	if (Vars::Visuals::ProjectileCamera.Value)
	{
		ImGui::SetNextWindowSize({ static_cast<float>(F::CameraWindow.ViewRect.w), static_cast<float>(F::CameraWindow.ViewRect.h) }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos({ static_cast<float>(F::CameraWindow.ViewRect.x), static_cast<float>(F::CameraWindow.ViewRect.y) }, ImGuiCond_FirstUseEver);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
		ImGui::PushStyleColor(ImGuiCol_Border, { 1.f, 1.f, 1.f, 1.f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 100.f, 100.f });
		if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
		{
			const ImVec2 winPos = ImGui::GetWindowPos();
			const ImVec2 winSize = ImGui::GetWindowSize();

			F::CameraWindow.ViewRect.x = static_cast<int>(winPos.x);
			F::CameraWindow.ViewRect.y = static_cast<int>(winPos.y);
			F::CameraWindow.ViewRect.w = static_cast<int>(winSize.x);
			F::CameraWindow.ViewRect.h = static_cast<int>(winSize.y);

			ImGui::PushFont(FontBlack);
			auto size = ImGui::CalcTextSize("Camera");
			ImGui::SetCursorPos({ (winSize.x - size.x) * 0.5f, (winSize.y - size.y) * 0.5f });
			ImGui::Text("Camera");
			ImGui::PopFont();

			ImGui::End();
		}
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(2);
	}
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
	{
		F::Menu.IsOpen = !F::Menu.IsOpen;
		I::ViewRender->SetScreenOverlayMaterial(nullptr);
		I::VGuiSurface->SetCursorAlwaysVisible(F::Menu.IsOpen);
		if (!F::Menu.IsOpen) // dumb
			I::InputSystem->ResetInputStateVFunc();
	}

	// Begin current frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::PushFont(FontRegular);

	// better looking dt bars
	F::Visuals.DrawTickbaseBars();

	// Window that should always be visible
	DrawKeybinds();
	F::Radar.DrawWindow();

	if (IsOpen)
	{
		DrawMenu();

		DrawCameraWindow();

		AddDraggable("Ticks", Vars::Menu::TicksDisplay.Value, Vars::Menu::Indicators.Value & (1 << 0));
		AddDraggable("Crit hack", Vars::Menu::CritsDisplay.Value, Vars::Menu::Indicators.Value & (1 << 1));
		AddDraggable("Spectators", Vars::Menu::SpectatorsDisplay.Value, Vars::Menu::Indicators.Value & (1 << 2));
		AddDraggable("Conditions", Vars::Menu::ConditionsDisplay.Value, Vars::Menu::Indicators.Value & (1 << 3));
		AddDraggable("Ping", Vars::Menu::PingDisplay.Value, Vars::Menu::Indicators.Value & (1 << 4));
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
		style.GrabMinSize = 1.f;
		style.GrabRounding = 3.f;
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
		style.WindowTitleAlign = { 0.5f, 0.5f }; // Center window title

		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Button] = { 0, 0, 0, 0 };
		colors[ImGuiCol_ButtonHovered] = { 0, 0, 0, 0 };
		colors[ImGuiCol_ButtonActive] = { 0, 0, 0, 0 };
		colors[ImGuiCol_CheckMark] = { Foreground.Value.x, Foreground.Value.y, Foreground.Value.z, 1.f };
		colors[ImGuiCol_FrameBg] = Foremost;
		colors[ImGuiCol_FrameBgHovered] = ForemostLight;
		colors[ImGuiCol_FrameBgActive] = Foremost;
		colors[ImGuiCol_Header] = { 0, 0, 0, 0 };
		colors[ImGuiCol_HeaderHovered] = ForemostLight;
		colors[ImGuiCol_HeaderActive] = { 0, 0, 0, 0 };
		colors[ImGuiCol_ModalWindowDimBg] = { Background.Value.x, Background.Value.y, Background.Value.z, 0.4f };
		colors[ImGuiCol_PopupBg] = ForemostLight;
		colors[ImGuiCol_ResizeGrip] = { 0, 0, 0, 0 };
		colors[ImGuiCol_ResizeGripActive] = { 0, 0, 0, 0 };
		colors[ImGuiCol_ResizeGripHovered] = { 0, 0, 0, 0 };
		colors[ImGuiCol_ScrollbarBg] = { 0, 0, 0, 0 };
		colors[ImGuiCol_SliderGrab] = Accent;
		colors[ImGuiCol_SliderGrabActive] = AccentLight;
		colors[ImGuiCol_Text] = Active;
		colors[ImGuiCol_TitleBg] = Background;
		colors[ImGuiCol_TitleBgActive] = Background;
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