#include "Menu.h"

#include "../Vars.h"
#include "../Visuals/Radar/Radar.h"
#include "../Misc/Misc.h"
#include "../Visuals/Chams/DMEChams.h"
#include "../Visuals/Glow/Glow.h"
#include "../Visuals/Visuals.h"
#include "../Backtrack/Backtrack.h"

#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/imgui_stdlib.h>

#include "Fonts/IconsMaterialDesign.h"
#include "Playerlist/Playerlist.h"
#include "MaterialEditor/MaterialEditor.h"

#include "Components.hpp"
#include "ConfigManager/ConfigManager.h"

#include <mutex>

#pragma warning (disable : 4309)

int unuPrimary = 0;
int unuSecondary = 0;

constexpr auto SIZE_FULL_WIDTH = ImVec2(-1, 20);

/* The main menu */
void CMenu::DrawMenu()
{
	ImGui::SetNextWindowSize(ImVec2(700, 700), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 700, 500 });
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

	if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar))
	{
		const auto drawList = ImGui::GetWindowDrawList();
		const auto windowSize = ImGui::GetWindowSize();
		const auto windowPos = ImGui::GetWindowPos();

		ImGui::Dummy({ 0, 2 });

		// Title Text
		{
			ImGui::PushFont(TitleFont);
			const auto titleWidth = ImGui::CalcTextSize(Vars::Menu::CheatName.Value.c_str()).x;
			drawList->AddText(TitleFont, TitleFont->FontSize, { windowPos.x + (windowSize.x / 2) - (titleWidth / 2), windowPos.y }, Accent, Vars::Menu::CheatName.Value.c_str());
			ImGui::PopFont();
		}

		// Icons
		{
			float currentX = windowSize.x;

			// Playerlist Icon
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_PEOPLE))
				Vars::Menu::ShowPlayerlist.Value = !Vars::Menu::ShowPlayerlist.Value;
			ImGui::HelpMarker("Playerlist");

			// Material Editor Icon
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_BRUSH))
				F::MaterialEditor.IsOpen = !F::MaterialEditor.IsOpen;
			ImGui::HelpMarker("Material Editor");
		}

		// Tabbar
		ImGui::SetCursorPos({ 0, TitleHeight });
		ImGui::PushStyleColor(ImGuiCol_ChildBg, BackgroundLight.Value);
		if (ImGui::BeginChild("Tabbar", { windowSize.x + 5, TabHeight + SubTabHeight }, false, ImGuiWindowFlags_NoScrollWithMouse))
		{
			DrawTabbar();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();

		// Main content
		ImGui::SetCursorPos({ 0, TitleHeight + TabHeight + SubTabHeight });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 8.f, 10.f });
		ImGui::PushStyleColor(ImGuiCol_ChildBg, BackgroundDark.Value);
		if (ImGui::BeginChild("Content", { windowSize.x, windowSize.y - (TitleHeight + TabHeight + SubTabHeight) }, false, ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::PushFont(Verdana);
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 3.f, 2.f });

			switch (CurrentTab)
			{
				case MenuTab::Aimbot: { MenuAimbot(); break; }
				case MenuTab::Trigger: { MenuTrigger(); break; }
				case MenuTab::Visuals: { MenuVisuals(); break; }
				case MenuTab::HvH: { MenuHvH(); break; }
				case MenuTab::Misc: { MenuMisc(); break; }
				case MenuTab::Settings: { MenuSettings(); break; }
			}

			ImGui::PopStyleVar();
			ImGui::PopFont();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		// End
		ImGui::End();
	}

	ImGui::PopStyleVar(2);
}

void CMenu::DrawTabbar()
{
	ImGui::PushFont(SectionFont);
	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

	if (ImGui::BeginTable("TabbarTable", 6))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, BackgroundLight.Value);
		ImGui::PushStyleColor(ImGuiCol_Text, TextLight.Value);
		if (ImGui::TabButton("Aimbot", CurrentTab == MenuTab::Aimbot))
			CurrentTab = MenuTab::Aimbot;

		if (ImGui::TabButton("Triggerbot", CurrentTab == MenuTab::Trigger))
			CurrentTab = MenuTab::Trigger;

		if (ImGui::TabButton("Visuals", CurrentTab == MenuTab::Visuals))
			CurrentTab = MenuTab::Visuals;

		if (ImGui::TabButton("HvH", CurrentTab == MenuTab::HvH))
			CurrentTab = MenuTab::HvH;

		if (ImGui::TabButton("Misc", CurrentTab == MenuTab::Misc))
			CurrentTab = MenuTab::Misc;

		if (ImGui::TabButton("Settings", CurrentTab == MenuTab::Settings))
			CurrentTab = MenuTab::Settings;

		ImGui::PopStyleColor(2);
		ImGui::EndTable();
	}

	ImGui::SetCursorPosY(TabHeight);
	if (CurrentTab == MenuTab::Visuals)
	{
		SubTabHeight = 30.f;

		if (ImGui::BeginTable("SubbarTable", 6))
		{
			ImGui::PushStyleColor(ImGuiCol_Button, BackgroundLight.Value);
			ImGui::PushStyleColor(ImGuiCol_Text, TextLight.Value);
			if (ImGui::TabButton("ESP", CurrentVisualsTab == VisualsTab::ESP))
				CurrentVisualsTab = VisualsTab::ESP;

			if (ImGui::TabButton("Chams", CurrentVisualsTab == VisualsTab::Chams))
				CurrentVisualsTab = VisualsTab::Chams;

			if (ImGui::TabButton("Glow", CurrentVisualsTab == VisualsTab::Glow))
				CurrentVisualsTab = VisualsTab::Glow;

			if (ImGui::TabButton("Misc", CurrentVisualsTab == VisualsTab::Misc))
				CurrentVisualsTab = VisualsTab::Misc;

			if (ImGui::TabButton("Radar", CurrentVisualsTab == VisualsTab::Radar))
				CurrentVisualsTab = VisualsTab::Radar;

			if (ImGui::TabButton("Fonts", CurrentVisualsTab == VisualsTab::Font))
				CurrentVisualsTab = VisualsTab::Font;

			ImGui::PopStyleColor(2);
			ImGui::EndTable();
		}
	}
	else
	{
		SubTabHeight = 0.f;
	}

	ImGui::PopStyleVar(3);
	ImGui::PopFont();
}

#pragma region Tabs
/* Tab: Aimbot */
void CMenu::MenuAimbot()
{
	using namespace ImGui;

	if (BeginTable("AimbotTable", 3))
	{
		/* Column 1 */
		if (TableColumnChild("AimbotCol1"))
		{
			SectionTitle("Global");
			WToggle("Aimbot", &Vars::Aimbot::Global::Active.Value); HelpMarker("Aimbot master switch");
			ColorPickerL("Target", Vars::Colors::Target.Value);
			InputKeybind("Aimbot key", Vars::Aimbot::Global::AimKey.Value); HelpMarker("The key to enable aimbot");
			ColorPickerL("Aimbot FOV circle", Vars::Colors::FOVCircle.Value);
			WToggle("Autoshoot###AimbotAutoshoot", &Vars::Aimbot::Global::AutoShoot.Value); HelpMarker("Automatically shoot when a target is found");
			MultiFlags({ "Players", "Sentries", "Dispensers", "Teleporters", "Stickies", "NPCs", "Bombs" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6 },
				&Vars::Aimbot::Global::AimAt.Value, "Aimbot targets###AimbotTargets");
			ColorPickerL("Invulnerable color", Vars::Colors::Invulnerable.Value);
			MultiFlags({ "Invulnerable", "Cloaked", "Dead Ringer", "Friends", "Taunting", "Vaccinator", "Unsimulated Players", "Disguised" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 },
				&Vars::Aimbot::Global::IgnoreOptions.Value, "Ignored targets###AimbotIgnoredTargets");
			if (Vars::Aimbot::Global::IgnoreOptions.Value & (1 << 1))
			{
				WSlider("Cloak Percentage Threshold", &Vars::Aimbot::Global::IgnoreCloakPercentage.Value, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
				HelpMarker("If the player's cloak percentage is greater than the set threshold, it will ignore them. (Set this to 0 to always ignore cloaked spies)");
			}
			WSlider("Max Targets###AimbotMax", &Vars::Aimbot::Global::MaxTargets.Value, 1, 6, "%d", ImGuiSliderFlags_AlwaysClamp);

			if (Vars::Aimbot::Global::IgnoreOptions.Value & (1 << 6))
			{
				WSlider("Tick Tolerance###AimbotUnsimulatedTolerance", &Vars::Aimbot::Global::TickTolerance.Value, 0, 21, "%d", ImGuiSliderFlags_AlwaysClamp);
			}

			SectionTitle("Crits");
			WToggle("Crit hack", &Vars::CritHack::Active.Value);  HelpMarker("Enables the crit hack (BETA)");
			MultiCombo({ "Indicators", "Avoid Random", "Always Melee" }, { &Vars::CritHack::Indicators.Value, &Vars::CritHack::AvoidRandom.Value, &Vars::CritHack::AlwaysMelee.Value }, "Misc###CrithackMiscOptions");
			HelpMarker("Misc options for crithack");
			InputKeybind("Crit key", Vars::CritHack::CritKey.Value); HelpMarker("Will try to force crits when the key is held");

			SectionTitle("Backtrack");
			WToggle("Active", &Vars::Backtrack::Enabled.Value); HelpMarker("If you shoot at the backtrack manually it will attempt to hit it");
			WCombo("Backtrack Method###HitscanBacktrackMethod", &Vars::Backtrack::Method.Value, { "All", "Last", "Prefer OnShot" });
			{
				float flDiv = Vars::Backtrack::Latency.Value / 5;
				WSlider("Fake Latency###BTLatency", &Vars::Backtrack::Latency.Value, 0, 800, "%d", ImGuiSliderFlags_AlwaysClamp);
			}
			WSlider("Fake Interp###BTInterp", &Vars::Backtrack::Interp.Value, 0, 800, "%d", ImGuiSliderFlags_AlwaysClamp); HelpMarker("Will not change latency but only able to modify on spawn");
			WSlider("Window###BTWindow", &Vars::Backtrack::Window.Value, 1, 200, "%d", ImGuiSliderFlags_AlwaysClamp);
			WToggle("Unchoke Prediction", &Vars::Backtrack::UnchokePrediction.Value);
			if (Vars::Debug::Info.Value)
			{
				SectionTitle("debug");
				WSlider("passthrough offset", &Vars::Backtrack::PassthroughOffset.Value, -3, 3);
				WSlider("tickset offset", &Vars::Backtrack::TicksetOffset.Value, -3, 3);
				WSlider("choke pass mod", &Vars::Backtrack::ChokePassMod.Value, -1, 1);
				WSlider("choke set mod", &Vars::Backtrack::ChokeSetMod.Value, -1, 1);
				WSlider("new window sub", &Vars::Backtrack::NWindowSub.Value, 0, 200);
				WSlider("old window sub", &Vars::Backtrack::OWindowSub.Value, 0, 200);
			}
		} EndChild();

		/* Column 2 */
		if (TableColumnChild("AimbotCol2"))
		{
			SectionTitle("Hitscan");
			WToggle("Active ###HitscanAimbotActive", &Vars::Aimbot::Hitscan::Active.Value);
			WSlider("Aim FOV ###HitscanFOV", &Vars::Aimbot::Hitscan::AimFOV.Value, 0.f, 180.f, "%1.f", ImGuiSliderFlags_AlwaysClamp);
			WCombo("Sort method###HitscanSortMethod", &Vars::Aimbot::Hitscan::SortMethod.Value, { "FOV", "Distance" }); HelpMarker("Which method the aimbot uses to decide which target to aim at");
			WCombo("Aim method###HitscanAimMethod", &Vars::Aimbot::Hitscan::AimMethod.Value, { "Plain", "Smooth", "Silent" }); HelpMarker("Which method the aimbot uses to aim at the target");
			if (Vars::Aimbot::Hitscan::AimMethod.Value == 1)
				WSlider("Smooth factor###HitscanSmoothing", &Vars::Aimbot::Hitscan::SmoothingAmount.Value, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp); HelpMarker("Changes how smooth the aimbot will aim at the target");
			WCombo("Tapfire###HitscanTapfire", &Vars::Aimbot::Hitscan::TapFire.Value, { "Off", "Distance", "Always" }); HelpMarker("How/If the aimbot chooses to tapfire enemies.");
			if (Vars::Aimbot::Hitscan::TapFire.Value == 1)
				WSlider("Tap Fire Distance###HitscanTapfireDistance", &Vars::Aimbot::Hitscan::TapFireDist.Value, 250.f, 1000.f, "%.0f", ImGuiSliderFlags_AlwaysClamp); HelpMarker("The distance at which tapfire will activate.");
			{
				static std::vector flagNames{ "Head", "Body", "Pelvis", "Arms", "Legs" };
				static std::vector flagValues{ 0x00000001, 0x00000004, 0x00000002, 0x00000008, 0x00000010 }; // 1<<1 and 1<<2 are swapped because the enum for hitboxes is weird.
				MultiFlags(flagNames, flagValues, &Vars::Aimbot::Hitscan::Hitboxes.Value, "Hitboxes###AimbotHitboxScanning");
			}
			WSlider("Point Scale###HitscanMultipointScale", &Vars::Aimbot::Hitscan::PointScale.Value, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
			WToggle("Wait for headshot", &Vars::Aimbot::Hitscan::WaitForHeadshot.Value); HelpMarker("The aimbot will wait until it can headshot (if applicable)");
			WToggle("Wait for charge", &Vars::Aimbot::Hitscan::WaitForCharge.Value); HelpMarker("The aimbot will wait until the rifle has charged long enough to kill in one shot");
			WToggle("Scoped only", &Vars::Aimbot::Hitscan::ScopedOnly.Value); HelpMarker("The aimbot will only shoot if scoped");
			WToggle("Auto scope", &Vars::Aimbot::Hitscan::AutoScope.Value); HelpMarker("The aimbot will automatically scope in to shoot");
			WToggle("Bodyaim if lethal", &Vars::Aimbot::Global::BAimLethal.Value); HelpMarker("The aimbot will aim for body when damage is lethal to it");
			WToggle("Piss on Team", &Vars::Aimbot::Hitscan::ExtinguishTeam.Value); HelpMarker("Will aim at burning teammates with The Sydney Sleeper");

			SectionTitle("Melee");
			WToggle("Active ###MeleeAimbotActive", &Vars::Aimbot::Melee::Active.Value);
			WSlider("Aim FOV ###MeleeFOV", &Vars::Aimbot::Melee::AimFOV.Value, 0.f, 180.f, "%1.f", ImGuiSliderFlags_AlwaysClamp);
			WCombo("Aim method###MeleeAimMethod", &Vars::Aimbot::Melee::AimMethod.Value, { "Plain", "Smooth", "Silent" }); HelpMarker("Which method the aimbot uses to aim at the target");
			if (Vars::Aimbot::Melee::AimMethod.Value == 1)
				WSlider("Smooth factor###MeleeSmoothing", &Vars::Aimbot::Melee::SmoothingAmount.Value, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp); HelpMarker("How smooth the aimbot should be");
			WToggle("Auto backstab", &Vars::Aimbot::Melee::AutoBackstab.Value);
			WToggle("Ignore razorback", &Vars::Aimbot::Melee::IgnoreRazorback.Value);
			WToggle("Swing prediction", &Vars::Aimbot::Melee::PredictSwing.Value); HelpMarker("Aimbot will attack preemptively, predicting you will be in range of the target");
			WToggle("Whip teammates", &Vars::Aimbot::Melee::WhipTeam.Value); HelpMarker("Aimbot will target teammates if holding the Disciplinary Action");
			if (Vars::Debug::Info.Value)
			{
				SectionTitle("debug");
				WSlider("swing ticks", &Vars::Aimbot::Melee::SwingTicks.Value, 10, 14);
			}
		} EndChild();

		/* Column 3 */
		if (TableColumnChild("AimbotCol3"))
		{
			SectionTitle("Projectile");
			WToggle("Active", &Vars::Aimbot::Projectile::Active.Value);
			WSlider("Aim FOV", &Vars::Aimbot::Projectile::AimFOV.Value, 0.f, 180.f, "%1.f", ImGuiSliderFlags_AlwaysClamp);
			WCombo("Sort method###ProjectileSortMethod", &Vars::Aimbot::Projectile::SortMethod.Value, { "FOV", "Distance" });
			WCombo("Aim method###ProjectileAimMethod", &Vars::Aimbot::Projectile::AimMethod.Value, { "Plain", "Smooth", "Silent" });
			if (Vars::Aimbot::Projectile::AimMethod.Value == 1)
				WSlider("Smooth factor###ProjectileSmoothing", &Vars::Aimbot::Projectile::SmoothingAmount.Value, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp); HelpMarker("How smooth the aimbot should be");
			WSlider("Prediction Time", &Vars::Aimbot::Projectile::PredictionTime.Value, 0.1f, 10.f, "%.1f");
			WToggle("No spread", &Vars::Aimbot::Projectile::NoSpread.Value);
			WToggle("Auto release", &Vars::Aimbot::Projectile::AutoRelease.Value);
			if (Vars::Aimbot::Projectile::AutoRelease.Value)
				WSlider("Auto release at", &Vars::Aimbot::Projectile::AutoReleaseAt.Value, 0.f, 1.f, "%.2f");

			if (Vars::Debug::Info.Value)
			{
				SectionTitle("debug");
				WSlider("samples", &Vars::Aimbot::Projectile::iSamples.Value, 3, 66);
				WSlider("vert shift", &Vars::Aimbot::Projectile::VerticalShift.Value, 0.f, 20.f, "%.1f");
				WSlider("latency offset", &Vars::Aimbot::Projectile::LatOff.Value, -3.f, 3.f, "%.1f");
				WSlider("physic offset", &Vars::Aimbot::Projectile::PhyOff.Value, -3.f, 3.f, "%.1f");
				WCombo("hunterman mode", &Vars::Aimbot::Projectile::HuntermanMode.Value, { "center", "shift head", "shift up", "from top" });
				if (Vars::Aimbot::Projectile::HuntermanMode.Value == 1 || Vars::Aimbot::Projectile::HuntermanMode.Value == 2)
					WSlider("hunterman shift", &Vars::Aimbot::Projectile::HuntermanShift.Value, 0.f, 10.f, "%.1f");
			}

			SectionTitle("Preferences");
			WToggle("Charge loose cannon", &Vars::Aimbot::Projectile::ChargeLooseCannon.Value); HelpMarker("Will charge your loose cannon in order to double donk");
			WToggle("Splash Prediction", &Vars::Aimbot::Projectile::SplashPrediction.Value); HelpMarker("Will shoot the area near the target to hit them with splash damage");

			SectionTitle("Strafe Prediction");
			MultiCombo({ "Air", "Ground" }, { &Vars::Aimbot::Projectile::StrafePredictionAir.Value, &Vars::Aimbot::Projectile::StrafePredictionGround.Value }, "Strafe Prediction");
			WSlider("Minimum deviation", &Vars::Aimbot::Projectile::StrafePredictionMinDifference.Value, 0, 180); HelpMarker("How big the angle difference of the predicted strafe has to be to apply");
			WSlider("Maximum distance", &Vars::Aimbot::Projectile::StrafePredictionMaxDistance.Value, 100.f, 10000.f); HelpMarker("Max distance to apply strafe prediction (lower is better)");
			WSlider("Hit chance", &Vars::Aimbot::Projectile::StrafePredictionHitchance.Value, 0.f, 1.f);
		} EndChild();

		/* End */
		EndTable();
	}
}

/* Tab: Trigger */
void CMenu::MenuTrigger()
{
	using namespace ImGui;
	if (BeginTable("TriggerTable", 2))
	{
		/* Column 1 */
		if (TableColumnChild("TriggerCol1"))
		{
			SectionTitle("Global");
			WToggle("Triggerbot", &Vars::Triggerbot::Global::Active.Value); HelpMarker("Global triggerbot master switch");
			InputKeybind("Trigger key", Vars::Triggerbot::Global::TriggerKey.Value); HelpMarker("The key which activates the triggerbot");
			HelpMarker("Choose which targets the Aimbot should aim at");
			{
				static std::vector flagNames{ "Invulnerable", "Cloaked", "Friends", "Taunting", "Unsimulated Players", "Disguised" };
				static std::vector flagValues{ 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5 };
				MultiFlags(flagNames, flagValues, &Vars::Triggerbot::Global::IgnoreOptions.Value, "Ignored targets###TriggerbotIgnoredTargets");
				HelpMarker("Choose which targets should be ignored");
			}

			SectionTitle("Autouber");
			WToggle("Autouber###Triggeruber", &Vars::Triggerbot::Uber::Active.Value); HelpMarker("Auto uber master switch");
			WToggle("Only uber friends", &Vars::Triggerbot::Uber::OnlyFriends.Value); HelpMarker("Auto uber will only activate if healing steam friends");
			WToggle("Preserve self", &Vars::Triggerbot::Uber::PopLocal.Value); HelpMarker("Auto uber will activate if local player's health falls below the percentage");
			WToggle("Vaccinator resistances", &Vars::Triggerbot::Uber::AutoVacc.Value); HelpMarker("Auto uber will automatically find the best resistance and pop when needed (This doesn't work properly)");
			if (Vars::Triggerbot::Uber::AutoVacc.Value)
			{
				MultiCombo({ "Bullet", "Blast", "Fire" }, { &Vars::Triggerbot::Uber::BulletRes.Value, &Vars::Triggerbot::Uber::BlastRes.Value, &Vars::Triggerbot::Uber::FireRes.Value }, "Allowed Resistances");
			}
			{
				static std::vector Names{ "Scout", "Soldier", "Pyro", "Heavy", "Engineer", "Sniper", "Spy" };
				static std::vector Values{ 1 << 0, 1 << 1, 1 << 2, 1 << 4, 1 << 5, 1 << 7, 1 << 8 };

				MultiFlags(Names, Values, &Vars::Triggerbot::Uber::ReactClasses.Value, "Hitscan React Classes###TriggerbotAutoVaccClasses");
			}
			WSlider("Health left (%)###TriggerUberHealthLeft", &Vars::Triggerbot::Uber::HealthLeft.Value, 1.f, 99.f, "%.0f%%", 1.0f); HelpMarker("The amount of health the heal target must be below to actiavte");
			WSlider("Reaction FoV###TriggerUberReactFoV", &Vars::Triggerbot::Uber::ReactFoV.Value, 0, 90, "%d", 1); HelpMarker("Checks whether you are within a certain FoV from legit players before auto ubering.");
			WToggle("Activate charge trigger", &Vars::Triggerbot::Uber::VoiceCommand.Value); HelpMarker("Will ubercharge regardless of anything if your target says activate charge");

			SectionTitle("Auto Jump");
			InputKeybind("Auto rocket jump", Vars::Triggerbot::Jump::JumpKey.Value); HelpMarker("Unfinished");
			InputKeybind("Auto CTap", Vars::Triggerbot::Jump::CTapKey.Value);
		} EndChild();

		/* Column 2 */
		if (TableColumnChild("TriggerCol2"))
		{
			SectionTitle("Auto Detonate");
			WToggle("Autodetonate###TriggerDet", &Vars::Triggerbot::Detonate::Active.Value);
			{
				static std::vector Names{ "Players", "Sentries", "Dispensers", "Teleporters", "NPCs", "Bombs", "Stickies" };
				static std::vector Values{ 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6 };

				MultiFlags(Names, Values, &Vars::Triggerbot::Detonate::DetonateTargets.Value, "Targets###TriggerbotDetonateTargets");
			}
			WToggle("Explode stickies###TriggerSticky", &Vars::Triggerbot::Detonate::Stickies.Value); HelpMarker("Detonate sticky bombs when a player is in range");
			WToggle("Detonate flares###TriggerFlares", &Vars::Triggerbot::Detonate::Flares.Value); HelpMarker("Detonate detonator flares when a player is in range");
			WSlider("Detonation radius###TriggerDetRadius", &Vars::Triggerbot::Detonate::RadiusScale.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp); HelpMarker("The radius around the projectile that it will detonate if a player is in");

			SectionTitle("Autoblast");
			WToggle("Autoblast###Triggreairblast", &Vars::Triggerbot::Blast::Active.Value); HelpMarker("Auto airblast master switch");
			WToggle("Rage airblast###TriggerAirRage", &Vars::Triggerbot::Blast::Rage.Value); HelpMarker("Will airblast whenever possible, regardless of FoV");
			WToggle("Silent###triggerblastsilent", &Vars::Triggerbot::Blast::Silent.Value); HelpMarker("Aim changes made by the rage mode setting aren't visible");
			WToggle("Extinguish Players###TriggerExtinguishPlayers", &Vars::Triggerbot::Blast::ExtinguishPlayers.Value); HelpMarker("Will automatically extinguish burning players");
			WToggle("Disable on Attack###TriggerDisableOnAttack", &Vars::Triggerbot::Blast::DisableOnAttack.Value); HelpMarker("Will not air blast while attacking");
			WSlider("FOV####AirBlastFov", &Vars::Triggerbot::Blast::Fov.Value, 0.f, 90.f, "%d", ImGuiSliderFlags_AlwaysClamp);
		} EndChild();

		EndTable();
	}
}

/* Tab: Visuals */
void CMenu::MenuVisuals()
{
	using namespace ImGui;

	switch (CurrentVisualsTab)
	{
	// Visuals: ESP
		case VisualsTab::ESP:
		{
			if (BeginTable("VisualsESPTable", 2))
			{
				/* Column 1 */
				if (TableColumnChild("VisualsESPCol1"))
				{
					SectionTitle("Main");
					WToggle("Active###EnableESP", &Vars::ESP::Main::Active.Value); HelpMarker("Global ESP master switch");
					WToggle("Relative colors", &Vars::ESP::Main::EnableTeamEnemyColors.Value); HelpMarker("Chooses colors relative to your team (team/enemy)");
					if (Vars::ESP::Main::EnableTeamEnemyColors.Value)
					{
						ColorPickerL("Enemy color", Vars::Colors::Enemy.Value);
						ColorPickerL("Team color", Vars::Colors::Team.Value, 1);
					}
					else
					{
						ColorPickerL("RED Team color", Vars::Colors::TeamRed.Value);
						ColorPickerL("BLU Team color", Vars::Colors::TeamBlu.Value, 1);
					}
					WToggle("Dormant sound ESP", &Vars::ESP::Main::DormantSoundESP.Value); HelpMarker("Credits: reestart");
					if (Vars::ESP::Main::DormantSoundESP.Value)
						WSlider("Dormant Decay Time###GlobalDormantDecayTime", &Vars::ESP::Main::DormantTime.Value, 0.015f, 5.0f, "%.1f", ImGuiSliderFlags_Logarithmic);

					SectionTitle("Players");
					WToggle("Active###PlayerESP", &Vars::ESP::Players::Active.Value); HelpMarker("Will draw useful information/indicators on players");
					WToggle("Ignore local###SelfESP", &Vars::ESP::Players::IgnoreLocal.Value); ColorPickerL("Local color", Vars::Colors::Local.Value);
					WToggle("Ignore team###IgnoreTeamESPp", &Vars::ESP::Players::IgnoreTeam.Value); ColorPickerL("Friend color", Vars::Colors::Friend.Value);
					WToggle("Ignore cloaked###IgnoreCloakESPp", &Vars::ESP::Players::IgnoreCloaked.Value); ColorPickerL("Cloaked color", Vars::Colors::Cloak.Value);
					WToggle("Name###PlayerNameESP", &Vars::ESP::Players::Name.Value);
					WToggle("Health bar###PlayerHealthBar", &Vars::ESP::Players::HealthBar.Value);
					ColorPickerL("Health Bar Top", Vars::Colors::HealthBar.Value.StartColor, 1);
					ColorPickerL("Health Bar Bottom", Vars::Colors::HealthBar.Value.EndColor);
					WToggle("Health text###PlayerHealthText", &Vars::ESP::Players::HealthText.Value);
					ColorPickerL("Overheal color", Vars::Colors::Overheal.Value);
					WToggle("Uber bar###PlayerUberBar", &Vars::ESP::Players::UberBar.Value);
					ColorPickerL("Uber Bar", Vars::Colors::UberBar.Value);
					WToggle("Uber text###PlayerUberText", &Vars::ESP::Players::UberText.Value);
					WToggle("Class icon###PlayerClassIcon", &Vars::ESP::Players::ClassIcon.Value);
					WToggle("Class text###PlayerClassText", &Vars::ESP::Players::ClassText.Value);
					WToggle("Weapon icon", &Vars::ESP::Players::WeaponIcon.Value);
					WToggle("Weapon text", &Vars::ESP::Players::WeaponText.Value);
					ColorPickerL("Weapon color", Vars::Colors::Weapon.Value);
					WToggle("Distance", &Vars::ESP::Players::Distance.Value);
					WToggle("Box###PlayerBoxESP", &Vars::ESP::Players::Box.Value);
					WToggle("Skeleton###PlayerBones", &Vars::ESP::Players::Bones.Value);
					WToggle("Priority Text", &Vars::ESP::Players::PriorityText.Value);
					MultiCombo({ "Buffs", "Debuffs", "Other Conditions", "Lag Compensation", "KD", "Ping" },
						{ &Vars::ESP::Players::Conditions::Buffs.Value, &Vars::ESP::Players::Conditions::Debuffs.Value, &Vars::ESP::Players::Conditions::Other.Value, 
						  &Vars::ESP::Players::Conditions::LagComp.Value, &Vars::ESP::Players::Conditions::KD.Value, &Vars::ESP::Players::Conditions::Ping.Value },
						  "Condition List" );
					WSlider("ESP alpha###PlayerESPAlpha", &Vars::ESP::Players::Alpha.Value, 0.01f, 1.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsESPCol2"))
				{
					SectionTitle("Buildings");
					WToggle("Active###BuildingESP", &Vars::ESP::Buildings::Active.Value);
					WToggle("Ignore team buildings###BuildingESPIgnoreTeammates", &Vars::ESP::Buildings::IgnoreTeam.Value);
					WToggle("Name###BuildingNameESP", &Vars::ESP::Buildings::Name.Value);
					WToggle("Health bar###Buildinghelathbar", &Vars::ESP::Buildings::HealthBar.Value);
					WToggle("Health text###buildinghealth", &Vars::ESP::Buildings::HealthText.Value);
					WToggle("Distance", &Vars::ESP::Buildings::Distance.Value);
					WToggle("Box###PBuildingBoxESP", &Vars::ESP::Buildings::Box.Value);
					WToggle("Owner###Buildingowner", &Vars::ESP::Buildings::Owner.Value);
					WToggle("Level###Buildinglevel", &Vars::ESP::Buildings::Level.Value);
					WToggle("Condition###Buildingconditions", &Vars::ESP::Buildings::Condition.Value);
					WSlider("ESP alpha###BuildingESPAlpha", &Vars::ESP::Buildings::Alpha.Value, 0.01f, 1.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

					SectionTitle("World");
					WToggle("Active###WorldESP", &Vars::ESP::World::Active.Value);
					WToggle("Healthpack###WorldESPHealthpack", &Vars::ESP::World::Health.Value); ColorPickerL("Healthpack color", Vars::Colors::Health.Value);
					WToggle("Ammopack###WorldESPAmmopack", &Vars::ESP::World::Ammo.Value); ColorPickerL("Ammopack color", Vars::Colors::Ammo.Value);
					WToggle("NPC###WorldESPNPC", &Vars::ESP::World::NPC.Value); ColorPickerL("NPC color", Vars::Colors::NPC.Value);
					WToggle("Bombs###WorldESPBomb", &Vars::ESP::World::Bomb.Value); ColorPickerL("Bomb Color", Vars::Colors::Bomb.Value);
					WToggle("Spellbook###WorldESPSpellbook", &Vars::ESP::World::Spellbook.Value); ColorPickerL("Spellbook Color", Vars::Colors::Spellbook.Value);
					WToggle("Gargoyle###WorldESPGargoyle", &Vars::ESP::World::Gargoyle.Value); ColorPickerL("Gargoyle Color", Vars::Colors::Gargoyle.Value);
					WSlider("ESP alpha###WordlESPAlpha", &Vars::ESP::World::Alpha.Value, 0.01f, 1.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
				} EndChild();

				EndTable();
			}
			break;
		}

		// Visuals: Chams
		case VisualsTab::Chams:
		{
			if (BeginTable("VisualsChamsTable", 2))
			{
				/* Column 1 */
				if (TableColumnChild("VisualsChamsCol1"))
				{
					SectionTitle("Chams Main");
					WToggle("Active###ChamsMasterSwitch", &Vars::Chams::Active.Value);

					static std::vector chamOptions{
						"Local",
						"FakeAngles",
						"Friends",
						"Enemies",
						"Teammates",
						"Target",
						"Ragdolls",
						"ViewModel",
						"VM Weapon"
					};
					static std::vector DMEProxyMaterials{
						"None",
						"Spectrum splattered",
						"Electro skulls",
						"Jazzy",
						"Frozen aurora",
						"Hana",
						"IDK",
						"Ghost thing",
						"Flames",
						"Spook wood",
						"Edgy",
						"Starlight serenity",
						"Fade"
					};
					static std::vector dmeGlowMaterial{
						"None",
						"Fresnel Glow",
						"Wireframe Glow"
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
							case 6: return Vars::Chams::Players::Ragdoll.Value;
							case 7: return Vars::Chams::DME::Hands.Value;
							case 8: return Vars::Chams::DME::Weapon.Value;
							}

							return Vars::Chams::Players::Local.Value;
						}());
					static std::vector DMEChamMaterials{ "Original", "Shaded", "Shiny", "Flat", "Wireframe shaded", "Wireframe shiny", "Wireframe flat", "Fresnel", "Brick", "Custom" };

					//WToggle("Player chams###PlayerChamsBox", &Vars::Chams::Players::Active.Value); HelpMarker("Player chams master switch");

					MultiCombo({ "Render Wearable", "Render Weapon" }, { &Vars::Chams::Players::Wearables.Value, &Vars::Chams::Players::Weapons.Value }, "Flags");
					HelpMarker("Customize Chams");
					WCombo("Config", &currentSelected, chamOptions);
					{
						ColorPickerL("Color", currentStruct.Color);
						MultiCombo({ "Active", "Obstructed" }, { &currentStruct.ChamsActive, &currentStruct.ShowObstructed }, "Options");

						WCombo("Material", &currentStruct.DrawMaterial, DMEChamMaterials); HelpMarker("Which material the chams will apply to the player");
						if (currentStruct.DrawMaterial == 7)
							ColorPickerL("Fresnel base color", currentStruct.FresnelBase);
						if (currentStruct.DrawMaterial == 9)
							MaterialCombo("Custom Material", &currentStruct.CustomMaterial);
						WCombo("Overlay", &currentStruct.OverlayType, dmeGlowMaterial);
						ColorPickerL("Glow Color", currentStruct.OverlayColor);
						WToggle("Rainbow Glow", &currentStruct.OverlayRainbow);
						WToggle("Pulse Glow", &currentStruct.OverlayPulse);
						WSlider("Glow Reduction", &currentStruct.OverlayIntensity, 150.f, 0.1f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

						if (currentSelected == 7 || currentSelected == 8)
						{
							int& proxySkinIndex = currentSelected == 8 ? Vars::Chams::DME::WeaponsProxySkin.Value : Vars::Chams::DME::HandsProxySkin.Value;
							WCombo("Proxy Material", &proxySkinIndex, DMEProxyMaterials);

						}
					}

					SectionTitle("Backtrack Chams");
					WToggle("Active###BacktrackChamsActive", &Vars::Backtrack::BtChams::Enabled.Value); HelpMarker("Draws chams to show where a player is");
					ColorPickerL("Backtrack color", Vars::Backtrack::BtChams::BacktrackColor.Value);
					WToggle("Only draw last tick", &Vars::Backtrack::BtChams::LastOnly.Value); HelpMarker("Only draws the last tick (can save FPS)");
					WToggle("Enemy only", &Vars::Backtrack::BtChams::EnemyOnly.Value); HelpMarker("You CAN backtrack your teammates. (Whip, medigun)");

					WCombo("Material##BtMaterial", &Vars::Backtrack::BtChams::Material.Value, DMEChamMaterials);
					if (Vars::Backtrack::BtChams::Material.Value == 9)
					{
						MaterialCombo("Custom Material##BtCustom", &Vars::Backtrack::BtChams::Custom.Value);
					}
					WCombo("Overlay##BtOverlay", &Vars::Backtrack::BtChams::Overlay.Value, dmeGlowMaterial);
					WToggle("Ignore Z###BtIgnoreZ", &Vars::Backtrack::BtChams::IgnoreZ.Value); HelpMarker("Draws them through walls");
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsChamsCol2"))
				{
					{
						SectionTitle("Building Chams");
						WToggle("Active###BuildingChams", &Vars::Chams::Buildings::Active.Value);

						static std::vector chamOptions{
							"Local",
							"Friends",
							"Enemies",
							"Teammates",
							"Target"
						};
						static std::vector dmeGlowMaterial{
							"None",
							"Fresnel Glow",
							"Wireframe Glow"
						};

						static int currentSelected = 0; //
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
						static std::vector DMEChamMaterials{ "Original", "Shaded", "Shiny", "Flat", "Wireframe shaded", "Wireframe shiny", "Wireframe flat", "Fresnel", "Brick", "Custom" };

						WCombo("Config", &currentSelected, chamOptions);
						{
							ColorPickerL("Color", currentStruct.Color);
							MultiCombo({ "Active", "Obstructed" }, { &currentStruct.ChamsActive, &currentStruct.ShowObstructed }, "Options");

							WCombo("Material", &currentStruct.DrawMaterial, DMEChamMaterials);
							if (currentStruct.DrawMaterial == 7)
							{
								ColorPickerL("Fresnel base color", currentStruct.FresnelBase);
							}
							if (currentStruct.DrawMaterial == 9)
							{
								MaterialCombo("Custom Material", &currentStruct.CustomMaterial);
							}
							WCombo("Glow Overlay", &currentStruct.OverlayType, dmeGlowMaterial);
							ColorPickerL("Glow Color", currentStruct.OverlayColor);
							WToggle("Rainbow Glow", &currentStruct.OverlayRainbow);
							WToggle("Pulse Glow", &currentStruct.OverlayPulse);
							WSlider("Glow Reduction", &currentStruct.OverlayIntensity, 150.f, 0.1f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
						}
					}
					{
						SectionTitle("World Chams");
						WToggle("Active###WorldChams", &Vars::Chams::World::Active.Value);

						static std::vector chamOptions{
							"Healthpacks",
							"Ammopacks",
							"Team Projectiles",
							"Enemy Projectiles",
						};
						static std::vector dmeGlowMaterial{
							"None",
							"Fresnel Glow",
							"Wireframe Glow"
						};

						static int currentSelected = 0; //
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
						static std::vector DMEChamMaterials{ "Original", "Shaded", "Shiny", "Flat", "Wireframe shaded", "Wireframe shiny", "Wireframe flat", "Fresnel", "Brick", "Custom" };

						WCombo("Config", &currentSelected, chamOptions);
						{
							ColorPickerL("Color", currentStruct.Color);
							MultiCombo({ "Active", "Obstructed" }, { &currentStruct.ChamsActive, &currentStruct.ShowObstructed }, "Options");

							WCombo("Material", &currentStruct.DrawMaterial, DMEChamMaterials); HelpMarker("Which material the chams will apply to the player");
							if (currentStruct.DrawMaterial == 7)
							{
								ColorPickerL("Fresnel base color", currentStruct.FresnelBase);
							}
							if (currentStruct.DrawMaterial == 9)
							{
								MaterialCombo("Custom Material", &currentStruct.CustomMaterial);
							}
							WCombo("Glow Overlay", &currentStruct.OverlayType, dmeGlowMaterial);
							ColorPickerL("Glow Color", currentStruct.OverlayColor);
							WToggle("Rainbow Glow", &currentStruct.OverlayRainbow);
							WToggle("Pulse Glow", &currentStruct.OverlayPulse);
							WSlider("Glow Reduction", &currentStruct.OverlayIntensity, 150.f, 0.1f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
						}
					}
				} EndChild();

				EndTable();
			}
			break;
		}

		// Visuals: Glow
		case VisualsTab::Glow:
		{
			if (BeginTable("VisualsGlowTable", 2))
			{
				/* Column 1 */
				if (TableColumnChild("VisualsGlowCol1"))
				{
					SectionTitle("Glow Main");
					WToggle("Active###GlowActive", &Vars::Glow::Main::Active.Value);
					WCombo("Glow Type###GlowTypeSelect", &Vars::Glow::Main::Type.Value, { "Blur", "Stencil" }); HelpMarker("Method in which glow will be rendered");
					WSlider("Glow scale", &Vars::Glow::Main::Scale.Value, 1, 20, "%d", ImGuiSliderFlags_AlwaysClamp);

					SectionTitle("Player Glow");
					WToggle("Active###PlayerGlow", &Vars::Glow::Players::Active.Value); HelpMarker("Player glow master switch");
					WToggle("Self glow###SelfGlow", &Vars::Glow::Players::ShowLocal.Value); HelpMarker("Draw glow on the local player");
					WToggle("Self rainbow glow###SelfGlowRainbow", &Vars::Glow::Players::LocalRainbow.Value); HelpMarker("Homosapien");
					WCombo("Ignore team###IgnoreTeamGlowp", &Vars::Glow::Players::IgnoreTeammates.Value, { "Off", "All", "Only friends" }); HelpMarker("Which teammates the glow will ignore drawing on");
					WToggle("Wearable glow###PlayerWearableGlow", &Vars::Glow::Players::Wearables.Value); HelpMarker("Will draw glow on player cosmetics");
					WToggle("Weapon glow###PlayerWeaponGlow", &Vars::Glow::Players::Weapons.Value); HelpMarker("Will draw glow on player weapons");
					WSlider("Glow alpha###PlayerGlowAlpha", &Vars::Glow::Players::Alpha.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
					WCombo("Glow color###GlowColour", &Vars::Glow::Players::Color.Value, { "Team", "Health" }); HelpMarker("Which color the glow will draw");
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsGlowCol2"))
				{
					SectionTitle("Building Glow");
					WToggle("Active###BuildiongGlow", &Vars::Glow::Buildings::Active.Value);
					WToggle("Ignore team buildings###buildingglowignoreteams", &Vars::Glow::Buildings::IgnoreTeammates.Value);
					WSlider("Glow alpha###BuildingGlowAlpha", &Vars::Glow::Buildings::Alpha.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
					WCombo("Glow color###GlowColourBuildings", &Vars::Glow::Buildings::Color.Value, { "Team", "Health" });

					SectionTitle("World Glow");
					WToggle("Active###WorldGlow", &Vars::Glow::World::Active.Value);
					WToggle("Healthpacks###worldhealthpackglow", &Vars::Glow::World::Health.Value);
					WToggle("Ammopacks###worldammopackglow", &Vars::Glow::World::Ammo.Value);
					WToggle("NPCs###worldnpcs", &Vars::Glow::World::NPCs.Value);
					WToggle("Bombs###worldbombglow", &Vars::Glow::World::Bombs.Value);
					WToggle("Spellbook###worldspellbookglow", &Vars::Glow::World::Spellbook.Value);
					WToggle("Gargoyle###worldgargoyleglow", &Vars::Glow::World::Gargoyle.Value);
					WCombo("Projectile glow###teamprojectileglow", &Vars::Glow::World::Projectiles.Value, { "Off", "All", "Only enemies" });
					WSlider("Glow alpha###WorldGlowAlpha", &Vars::Glow::World::Alpha.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
				} EndChild();

				EndTable();
			}
			break;
		}

		// Visuals: Misc
		case VisualsTab::Misc:
		{
			if (BeginTable("VisualsMiscTable", 2))
			{
				/* Column 1 */
				if (TableColumnChild("VisualsMiscCol1"))
				{
					SectionTitle("UI");
					WSlider("Field of view", &Vars::Visuals::FieldOfView.Value, 30, 150, "%d"); HelpMarker("How many degrees of field of vision you would like");
					WSlider("Zoomed field of view", &Vars::Visuals::ZoomFieldOfView.Value, 30, 150, "%d"); HelpMarker("Degrees of field while zoomed");
					MultiCombo({ "Scope", "Disguises", "Taunts", "Interpolation", "View Punch", "MOTD", "Screen Effects", "Angle Forcing", "Ragdolls", "Screen Overlays", "DSP", "Convar Queries" }, { &Vars::Visuals::RemoveScope.Value, &Vars::Visuals::RemoveDisguises.Value, &Vars::Visuals::RemoveTaunts.Value, &Vars::Misc::DisableInterpolation.Value, &Vars::Visuals::RemovePunch.Value, &Vars::Visuals::RemoveMOTD.Value, &Vars::Visuals::RemoveScreenEffects.Value, &Vars::Visuals::PreventForcedAngles.Value, &Vars::Visuals::RemoveRagdolls.Value, &Vars::Visuals::RemoveScreenOverlays.Value, &Vars::Visuals::RemoveDSP.Value, &Vars::Visuals::RemoveConvarQueries.Value}, "Removals");
					WToggle("Reveal Scoreboard", &Vars::Visuals::RevealScoreboard.Value);
					WToggle("Scoreboard Colours", &Vars::Visuals::ScoreboardColors.Value);
					WToggle("Scoreboard Playerlist", &Vars::Misc::ScoreboardPlayerlist.Value);
					WToggle("Clean Screenshots", &Vars::Visuals::CleanScreenshots.Value);
					WToggle("On Screen Conditions", &Vars::Visuals::DrawOnScreenConditions.Value);
					WToggle("On Screen Ping", &Vars::Visuals::DrawOnScreenPing.Value);
					WToggle("Sniper sightlines", &Vars::Visuals::SniperSightlines.Value);
					WToggle("Pickup Timers", &Vars::Visuals::PickupTimers.Value);
					WToggle("Spectator list", &Vars::Visuals::SpectatorList.Value);
					if (Vars::Visuals::SpectatorList.Value)
						WToggle("Spectator avatars", &Vars::Visuals::SpectatorAvatars.Value);
					WToggle("Post processing", &Vars::Visuals::DoPostProcessing.Value);
					WToggle("No prop fade", &Vars::Visuals::NoStaticPropFade.Value);

					SectionTitle("Viewmodel");
					WToggle("Crosshair aim position", &Vars::Visuals::CrosshairAimPos.Value);
					WToggle("Viewmodel aim position", &Vars::Visuals::AimbotViewmodel.Value);
					WSlider("VM Off X", &Vars::Visuals::VMOffsets.Value.x, -45.f, 45.f, "%.0f");
					WSlider("VM Off Y", &Vars::Visuals::VMOffsets.Value.y, -45.f, 45.f, "%.0f");
					WSlider("VM Off Z", &Vars::Visuals::VMOffsets.Value.z, -45.f, 45.f, "%.0f");
					WSlider("VM Roll", &Vars::Visuals::VMRoll.Value, -180, 180);
					WToggle("Viewmodel sway", &Vars::Visuals::ViewmodelSway.Value);
					if (Vars::Visuals::ViewmodelSway.Value)
					{
						WSlider("Viewmodel Sway Scale", &Vars::Visuals::ViewmodelSwayScale.Value, 0.01, 5, "%.1f");
						WSlider("Viewmodel Sway Interp", &Vars::Visuals::ViewmodelSwayInterp.Value, 0.01, 1, "%.1f"); HelpMarker("How long until the viewmodel returns to its original position (in seconds)");
					}

					SectionTitle("Tracers");
					{
						WCombo("Bullet trail", &Vars::Visuals::Tracers::ParticleTracer.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP Beam 2", "Big Nasty", "Distortion Trail", "Black Ink", "Custom" });
						if (Vars::Visuals::Tracers::ParticleTracer.Value == 9)
							WInputText("Custom Tracer", &Vars::Visuals::Tracers::ParticleName.Value);
						WCombo("Crit trail", &Vars::Visuals::Tracers::ParticleTracerCrits.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Merasmus ZAP Beam 2", "Big Nasty", "Distortion Trail", "Black Ink", "Custom" });
						if (Vars::Visuals::Tracers::ParticleTracerCrits.Value == 9)
							WInputText("Custom Crit Tracer", &Vars::Visuals::Tracers::ParticleNameCrits.Value);
					}

					SectionTitle("Beams");
					{
						using namespace Vars::Visuals;

						WToggle("Enable beams", &Beans::Active.Value); HelpMarker("he loves beans?");
						WToggle("Rainbow beams", &Beans::Rainbow.Value);
						ColorPickerL("Beam color", Beans::BeamColor.Value);
						WToggle("Custom model", &Beans::UseCustomModel.Value);
						if (Beans::UseCustomModel.Value)
						{
							WInputText("Model", &Beans::Model.Value);
						}
						WSlider("Beam lifespan", &Beans::Life.Value, 0.0f, 10.f);
						WSlider("Beam width", &Beans::Width.Value, 0.0f, 10.f);
						WSlider("Beam end width", &Beans::EndWidth.Value, 0.0f, 10.f);
						WSlider("Beam fade length", &Beans::FadeLength.Value, 0.0f, 30.f);
						WSlider("Beam amplitude", &Beans::Amplitude.Value, 0.0f, 10.f);
						WSlider("Beam brightness", &Beans::Brightness.Value, 0.0f, 255.f);
						WSlider("Beam speed", &Beans::Speed.Value, 0.0f, 5.f);
						WSlider("Segments", &Beans::Segments.Value, 1, 10); //what are good values for this

						// TODO: Reward this ugly code
						{
							static std::vector flagNames{ "STARTENTITY", "ENDENTITY","FADEIN","FADEOUT","SINENOISE","SOLID","SHADEIN","SHADEOUT","ONLYNOISEONCE","NOTILE","USE_HITBOXES","STARTVISIBLE","ENDVISIBLE","ISACTIVE","FOREVER","HALOBEAM","REVERSED", };
							static std::vector flagValues{ 0x00000001, 0x00000002,0x00000004,0x00000008,0x00000010,0x00000020,0x00000040,0x00000080,0x00000100,0x00000200,0x00000400,0x00000800,0x00001000,0x00002000,0x00004000,0x00008000,0x00010000 };
							MultiFlags(flagNames, flagValues, &Beans::Flags.Value, "Beam Flags###BeamFlags");
						}
					}

					SectionTitle("Ragdolls");
					WToggle("No Gibs", &Vars::Visuals::RagdollEffects::NoGib.Value);
					WToggle("Enemy only###RagdollEnemyOnly", &Vars::Visuals::RagdollEffects::EnemyOnly.Value); HelpMarker("Only runs it on enemies");
					MultiCombo({ "Burning", "Electrocuted", "Become ash", "Dissolve" }, { &Vars::Visuals::RagdollEffects::Burning.Value, &Vars::Visuals::RagdollEffects::Electrocuted.Value, &Vars::Visuals::RagdollEffects::BecomeAsh.Value, &Vars::Visuals::RagdollEffects::Dissolve.Value }, "Effects###RagdollEffects");
					HelpMarker("Ragdoll particle effects");
					WCombo("Ragdoll model", &Vars::Visuals::RagdollEffects::RagdollType.Value, { "None", "Gold", "Ice" }); HelpMarker("Which ragdoll model should be used");
					HelpMarker("Will make their ragdoll ice");
					WToggle("Use Separate Vector Forces", &Vars::Visuals::RagdollEffects::SeparateVectors.Value);
					if (Vars::Visuals::RagdollEffects::SeparateVectors.Value)
					{
						WSlider("Ragdoll Force Forwards", &Vars::Visuals::RagdollEffects::RagdollForceForwards.Value, -10.f, 10.f, "%.1f"); HelpMarker("Multipler for the force applied to a ragdoll's forward vector, where positive is backwards and negative is forwards.");
						WSlider("Ragdoll Force Sideways", &Vars::Visuals::RagdollEffects::RagdollForceSides.Value, -10.f, 10.f, "%.1f"); HelpMarker("Multipler for the force applied to a ragdoll's side vector, where negative is right and positive is left.");
						WSlider("Ragdoll Force Up", &Vars::Visuals::RagdollEffects::RagdollForceUp.Value, -10.f, 10.f, "%.1f"); HelpMarker("Multipler for the force applied to a ragdoll's up vector, where negative is up and positive is down.");
					}
					else
					{
						WSlider("Ragdoll Force", &Vars::Visuals::RagdollEffects::RagdollForce.Value, -10.f, 10.f, "%.1f"); HelpMarker("Multipler for the force applied to a ragdoll.");
					}
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsMiscCol2"))
				{
					SectionTitle("Bullet");
					WToggle("Bullet tracers", &Vars::Visuals::BulletTracer.Value);
					ColorPickerL("Bullet tracer color", Vars::Colors::BulletTracer.Value);

					SectionTitle("Simulation");
					WToggle("Enabled", &Vars::Visuals::SimLines.Value);
					ColorPickerL("Projectile Line Color", Vars::Colors::ProjectileColor.Value, 1);
					ColorPickerL("Prediction Line Color", Vars::Colors::PredictionColor.Value);
					WToggle("Timed", &Vars::Visuals::TimedLines.Value);
					WToggle("Seperators", &Vars::Visuals::SimSeperators.Value);
					if (Vars::Visuals::SimSeperators.Value)
					{
						WSlider("Seperator Length", &Vars::Visuals::SeperatorLength.Value, 2, 16, "%d", ImGuiSliderFlags_Logarithmic);
						WSlider("Seperator Spacing", &Vars::Visuals::SeperatorSpacing.Value, 1, 64, "%d", ImGuiSliderFlags_Logarithmic);
					}
					WToggle("Swing Prediction Lines", &Vars::Visuals::SwingLines.Value);
					WToggle("Projectile Trajectory", &Vars::Visuals::ProjectileTrajectory.Value);
					if (Vars::Debug::Info.Value)
					{
						SectionTitle("debug");
						WToggle("overwrite", &Vars::Visuals::PTOverwrite.Value);
						WCombo("type", &Vars::Visuals::PTType.Value,
							{ "none", "bullet", "rocket", "pipebomb", "pipebomb remote", "syringe", "flare", "jar", "arrow", "flame rocket", "jar milk",
							"healing bolt", "energy ball", "energy ring", "pipebomb practice", "cleaver", "sticky ball", "cannonball", "building repair bolt", "sentry rocket",
							"festive arrow", "throwable", "spell", "festive jar", "festive healing bolt", "breadmonster jarate", "breadmonster madmilk", "grappling hook", "bread monster", "jar gas",
							"balloffire" }); // to keep the integer easily associated with the enum, a lot of these aren't even used and are probably only used outside of simulation
						WSlider("off x", &Vars::Visuals::PTOffX.Value, -25.f, 25.f, "%.1f");
						WSlider("off y", &Vars::Visuals::PTOffY.Value, -25.f, 25.f, "%.1f");
						WSlider("off z", &Vars::Visuals::PTOffZ.Value, -25.f, 25.f, "%.1f");
						WToggle("pipes", &Vars::Visuals::PTPipes.Value);
						WSlider("hull", &Vars::Visuals::PTHull.Value, 0.f, 10.f, "%.1f");
						WSlider("speed", &Vars::Visuals::PTSpeed.Value, 0.f, 5000.f, "%.0f");
						WSlider("gravity", &Vars::Visuals::PTGravity.Value, 0.f, 2.f, "%.1f");
						WToggle("no spin", &Vars::Visuals::PTNoSpin.Value);
						WSlider("lifetime", &Vars::Visuals::PTLifeTime.Value, 0.f, 10.f, "%.1f");
						WSlider("up vel", &Vars::Visuals::PTUpVelocity.Value, 0.f, 1000.f, "%.0f");
						WSlider("ang vel x", &Vars::Visuals::PTAngVelocityX.Value, -1000.f, 1000.f, "%.0f");
						WSlider("ang vel y", &Vars::Visuals::PTAngVelocityY.Value, -1000.f, 1000.f, "%.0f");
						WSlider("ang vel z", &Vars::Visuals::PTAngVelocityZ.Value, -1000.f, 1000.f, "%.0f");
						WSlider("drag", &Vars::Visuals::PTDrag.Value, 0.f, 2.f, "%.1f");
						WSlider("drag x", &Vars::Visuals::PTDragBasisX.Value, 0.f, 0.1f, "%.6f");
						WSlider("drag y", &Vars::Visuals::PTDragBasisY.Value, 0.f, 0.1f, "%.6f");
						WSlider("drag z", &Vars::Visuals::PTDragBasisZ.Value, 0.f, 0.1f, "%.6f");
						WSlider("ang drag x", &Vars::Visuals::PTAngDragBasisX.Value, 0.f, 0.1f, "%.6f");
						WSlider("ang drag y", &Vars::Visuals::PTAngDragBasisY.Value, 0.f, 0.1f, "%.6f");
						WSlider("ang drag z", &Vars::Visuals::PTAngDragBasisZ.Value, 0.f, 0.1f, "%.6f");
					}

					SectionTitle("Hitbox");
					WToggle("Draw Hitboxes", &Vars::Aimbot::Global::ShowHitboxes.Value); HelpMarker("Shows targeted hitbox");
					ColorPickerL("Hitbox matrix face color", Vars::Colors::HitboxFace.Value);
					ColorPickerL("Hitbox matrix edge color", Vars::Colors::HitboxEdge.Value, 1);

					SectionTitle("Thirdperson");
					WToggle("Thirdperson", &Vars::Visuals::ThirdPerson::Active.Value);
					InputKeybind("Thirdperson key", Vars::Visuals::ThirdPerson::Key.Value);
					WSlider("Thirdperson distance", &Vars::Visuals::ThirdPerson::Distance.Value, 0.f, 500.f, "%.0f", ImGuiSliderFlags_None);
					WSlider("Thirdperson right", &Vars::Visuals::ThirdPerson::Right.Value, -500.f, 500.f, "%.0f", ImGuiSliderFlags_None);
					WSlider("Thirdperson up", &Vars::Visuals::ThirdPerson::Up.Value, -500.f, 500.f, "%.0f", ImGuiSliderFlags_None);
					WToggle("Thirdperson crosshair", &Vars::Visuals::ThirdPerson::Crosshair.Value);

					SectionTitle("Out of FOV arrows");
					WToggle("Active###FOVArrowActive", &Vars::Visuals::Arrows::Active.Value);
					WSlider("Offset###FOVArrowOffset", &Vars::Visuals::Arrows::Offset.Value, 0, 1000, "%d");
					WSlider("Max distance", &Vars::Visuals::Arrows::MaxDist.Value, 0.f, 5000.f, "%.0f");

					SectionTitle("World");
					MultiCombo({ "World", "Sky", "Prop", "Particle" }, { &Vars::Visuals::World::WorldModulation.Value, &Vars::Visuals::World::SkyModulation.Value, &Vars::Visuals::World::PropModulation.Value, &Vars::Visuals::World::ParticleModulation.Value }, "Modulations");
					HelpMarker("Select which types of modulation you want to enable");
					if (ColorPickerL("World modulation", Vars::Colors::WorldModulation.Value, 3) ||
						ColorPickerL("Sky modulation", Vars::Colors::SkyModulation.Value, 2) ||
						ColorPickerL("Prop modulation", Vars::Colors::StaticPropModulation.Value, 1))
					{
						G::ShouldUpdateMaterialCache = true;
					}
					ColorPickerL("Particle modulation", Vars::Colors::ParticleModulation.Value);
					static std::vector skyNames{
						"Custom",
						"sky_tf2_04",
						"sky_upward",
						"sky_dustbowl_01",
						"sky_goldrush_01",
						"sky_granary_01",
						"sky_well_01",
						"sky_gravel_01",
						"sky_badlands_01",
						"sky_hydro_01",
						"sky_night_01",
						"sky_nightfall_01",
						"sky_trainyard_01",
						"sky_stormfront_01",
						"sky_morningsnow_01",
						"sky_alpinestorm_01",
						"sky_harvest_01",
						"sky_harvest_night_01",
						"sky_halloween",
						"sky_halloween_night_01",
						"sky_halloween_night2014_01",
						"sky_island_01",
						"sky_rainbow_01"
					};
					WToggle("Skybox changer", &Vars::Visuals::World::SkyboxChanger.Value); HelpMarker("Will change the skybox, either to a base TF2 one or a custom one");
					WCombo("Skybox", &Vars::Visuals::World::SkyboxNum.Value, skyNames);
					if (Vars::Visuals::World::SkyboxNum.Value == 0)
					{
						WInputText("Custom skybox name", &Vars::Visuals::World::SkyboxName.Value); HelpMarker("Name of the skybox you want to you (tf/materials/skybox)");
					}
					WToggle("World Textures Override", &Vars::Visuals::World::OverrideTextures.Value); HelpMarker("Turn this off when in-game so you don't drop fps :p");
					WToggle("Prop Wireframe", &Vars::Visuals::World::PropWireframe.Value);
				} EndChild();

				EndTable();
			}
			break;
		}

		// Visuals: Radar
		case VisualsTab::Radar:
		{
			if (BeginTable("VisualsRadarTable", 2))
			{
				/* Column 1 */
				if (TableColumnChild("VisualsRadarCol1"))
				{
					SectionTitle("Main");
					WToggle("Enable Radar###RadarActive", &Vars::Radar::Main::Active.Value); HelpMarker("Will show nearby things relative to your player");
					WSlider("Range###RadarRange", &Vars::Radar::Main::Range.Value, 50, 3000, "%d"); HelpMarker("The range of the radar");
					WSlider("Background alpha###RadarBGA", &Vars::Radar::Main::BackAlpha.Value, 0, 255, "%d"); HelpMarker("The background alpha of the radar");
					WSlider("Line alpha###RadarLineA", &Vars::Radar::Main::LineAlpha.Value, 0, 255, "%d"); HelpMarker("The line alpha of the radar");
					WToggle("No Title Gradient", &Vars::Radar::Main::NoTitleGradient.Value);

					SectionTitle("Players");
					WToggle("Show players###RIOJSADFIOSAJIDPFOJASDFOJASOPDFJSAPOFDJOPS", &Vars::Radar::Players::Active.Value); HelpMarker("lol");
					WCombo("Icon###radari", &Vars::Radar::Players::IconType.Value, { "Scoreboard", "Portraits", "Avatar" }); HelpMarker("What sort of icon to represent players with");
					WCombo("Background###radarb", &Vars::Radar::Players::BackGroundType.Value, { "Off", "Rectangle", "Texture" }); HelpMarker("What sort of background to put on players on the radar");
					WToggle("Outline###radaro", &Vars::Radar::Players::Outline.Value); HelpMarker("Will put an outline on players on the radar");
					WCombo("Ignore teammates###radarplayersteam", &Vars::Radar::Players::IgnoreTeam.Value, { "Off", "All", "Keep friends" }); HelpMarker("Which teammates the radar will ignore drawing on");
					WCombo("Ignore cloaked###radarplayerscloaked", &Vars::Radar::Players::IgnoreCloaked.Value, { "Off", "All", "Keep friends" }); HelpMarker("Which cloaked players the radar will ignore drawing on");
					WToggle("Health bar###radarhealt", &Vars::Radar::Players::Health.Value); HelpMarker("Will show players health on the radar");
					WSlider("Icon size###playersizeiconradar", &Vars::Radar::Players::IconSize.Value, 12, 30, "%d"); HelpMarker("The icon size of players on the radar");
					WToggle("Height indicator###RadarPlayersZ", &Vars::Radar::Players::Height.Value); HelpMarker("Shows a little arrow indicating the height of the player");
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsRadarCol2"))
				{
					SectionTitle("Building");
					WToggle("Show buildings###radarbuildingsa", &Vars::Radar::Buildings::Active.Value);
					WToggle("Outline###radarbuildingsao", &Vars::Radar::Buildings::Outline.Value);
					WToggle("Ignore team###radarbuildingsb", &Vars::Radar::Buildings::IgnoreTeam.Value);
					WToggle("Health bar###radarbuildingsc", &Vars::Radar::Buildings::Health.Value);
					WSlider("Icon size###buildingsizeiconradar", &Vars::Radar::Buildings::IconSize.Value, 12, 30, "%d");

					SectionTitle("World");
					WToggle("Active###radarworldd", &Vars::Radar::World::Active.Value);
					WToggle("Healthpack###radarworldda", &Vars::Radar::World::Health.Value);
					WToggle("Ammopack###radarworlddb", &Vars::Radar::World::Ammo.Value);
					WSlider("Icon size###worldsizeiconradar", &Vars::Radar::World::IconSize.Value, 12, 30, "%d");
				} EndChild();

				EndTable();
			}
			break;
		}

		// Visuals: Font
		case VisualsTab::Font:
		{
			if (BeginTable("VisualsFontTable", 2))
			{
				static std::vector fontFlagNames{ "Italic", "Underline", "Strikeout", "Symbol", "Antialias", "Gaussian", "Rotary", "Dropshadow", "Additive", "Outline", "Custom" };
				static std::vector fontFlagValues{ 0x001, 0x002, 0x004, 0x008, 0x010, 0x020, 0x040, 0x080, 0x100, 0x200, 0x400 };

				/* Column 1 */
				if (TableColumnChild("VisualsFontCol1"))
				{
					SectionTitle("ESP Font");
					WInputText("Font name###espfontname", &Vars::Fonts::FONT_ESP::szName.Value);
					WInputInt("Font height###espfontheight", &Vars::Fonts::FONT_ESP::nTall.Value);
					WInputInt("Font weight###espfontweight", &Vars::Fonts::FONT_ESP::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_ESP::nFlags.Value, "Font flags###FONT_ESP");

					SectionTitle("Name Font");
					WInputText("Font name###espfontnamename", &Vars::Fonts::FONT_ESP_NAME::szName.Value);
					WInputInt("Font height###espfontnameheight", &Vars::Fonts::FONT_ESP_NAME::nTall.Value);
					WInputInt("Font weight###espfontnameweight", &Vars::Fonts::FONT_ESP_NAME::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_ESP_NAME::nFlags.Value, "Font flags###FONT_ESP_NAME");

					SectionTitle("Condition Font");
					WInputText("Font name###espfontcondname", &Vars::Fonts::FONT_ESP_COND::szName.Value);
					WInputInt("Font height###espfontcondheight", &Vars::Fonts::FONT_ESP_COND::nTall.Value);
					WInputInt("Font weight###espfontcondweight", &Vars::Fonts::FONT_ESP_COND::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_ESP_COND::nFlags.Value, "Font flags###FONT_ESP_COND");

					if (Button("Apply settings###fontapply"))
						g_Draw.RemakeFonts();
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsFontCol2"))
				{
					SectionTitle("Pickup Font");
					WInputText("Font name###espfontpickupsname", &Vars::Fonts::FONT_ESP_PICKUPS::szName.Value);
					WInputInt("Font height###espfontpickupsheight", &Vars::Fonts::FONT_ESP_PICKUPS::nTall.Value);
					WInputInt("Font weight###espfontpickupsweight", &Vars::Fonts::FONT_ESP_PICKUPS::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_ESP_PICKUPS::nFlags.Value, "Font flags###FONT_ESP_PICKUPS");

					SectionTitle("Menu Font");
					WInputText("Font name###espfontnamenameneby", &Vars::Fonts::FONT_MENU::szName.Value);
					WInputInt("Font height###espfontnameheightafsdfads", &Vars::Fonts::FONT_MENU::nTall.Value);
					WInputInt("Font weight###espfontnameweightasfdafsd", &Vars::Fonts::FONT_MENU::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_MENU::nFlags.Value, "Font flags###FONT_MENU");

					SectionTitle("Indicator Font");
					WInputText("Font name###espfontindicatorname", &Vars::Fonts::FONT_INDICATORS::szName.Value);
					WInputInt("Font height###espfontindicatorheight", &Vars::Fonts::FONT_INDICATORS::nTall.Value);
					WInputInt("Font weight###espfontindicatorweight", &Vars::Fonts::FONT_INDICATORS::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_INDICATORS::nFlags.Value, "Font flags###FONT_INDICATORS");
				} EndChild();

				EndTable();
			}
			break;
		}
	}
}

/* Tab: HvH */
void CMenu::MenuHvH()
{
	using namespace ImGui;
	if (BeginTable("HvHTable", 2))
	{
		/* Column 1 */
		if (TableColumnChild("HvHCol1"))
		{
			const int iVar = g_ConVars.sv_maxusrcmdprocessticks->GetInt();
			const int iTicks = iVar ? iVar : 24;

			/* Section: Tickbase Exploits */
			SectionTitle("Doubletap");
			WToggle("Enabled", &Vars::CL_Move::DoubleTap::Enabled.Value);
			WSlider("Tick limit", &Vars::CL_Move::DoubleTap::TickLimit.Value, 1, iTicks, "%d", ImGuiSliderFlags_AlwaysClamp);
			WSlider("Warp rate", &Vars::CL_Move::DoubleTap::WarpRate.Value, 1, iTicks, "%d", ImGuiSliderFlags_AlwaysClamp);
			WSlider("Passive recharge", &Vars::CL_Move::DoubleTap::PassiveRecharge.Value, 0, iTicks, "%d", ImGuiSliderFlags_AlwaysClamp);
			WCombo("Mode###DTmode", &Vars::CL_Move::DoubleTap::Mode.Value, { "Always", "Hold", "Toggle" });
			if (Vars::CL_Move::DoubleTap::Mode.Value != 0)
				InputKeybind("Doubletap key", Vars::CL_Move::DoubleTap::DoubletapKey.Value);
			InputKeybind("Recharge key", Vars::CL_Move::DoubleTap::RechargeKey.Value);
			InputKeybind("Teleport key", Vars::CL_Move::DoubleTap::TeleportKey.Value);
			MultiCombo({ "Wait for DT", "Anti-warp", "Avoid airborne", "Auto retain", "Auto Recharge", "Recharge While Dead", "Safe Tick", "Safe Tick Airborne" }, { &Vars::CL_Move::DoubleTap::WaitReady.Value, &Vars::CL_Move::DoubleTap::AntiWarp.Value, &Vars::CL_Move::DoubleTap::NotInAir.Value, &Vars::CL_Move::DoubleTap::AutoRetain.Value, &Vars::CL_Move::DoubleTap::AutoRecharge.Value, &Vars::CL_Move::DoubleTap::RechargeWhileDead.Value, &Vars::CL_Move::DoubleTap::SafeTick.Value, &Vars::CL_Move::DoubleTap::SafeTickAirOverride.Value }, "Options");
			WToggle("Indicator", &Vars::CL_Move::DoubleTap::Indicator.Value);
			ColorPickerL("Outline###DoubletapOutline", Vars::Colors::DtOutline.Value);

			SectionTitle("Speed hack");
			WToggle("Speed hack", &Vars::CL_Move::SpeedEnabled.Value); HelpMarker("Speedhack Master Switch");
			if (Vars::CL_Move::SpeedEnabled.Value)
			{
				WSlider("SpeedHack factor", &Vars::CL_Move::SpeedFactor.Value, 1, 66, "%d");
			}
			HelpMarker("High values are not recommended");

			/* Section: Fakelag */
			SectionTitle("Fakelag");
			WToggle("Enable Fakelag", &Vars::CL_Move::FakeLag::Enabled.Value);
			WCombo("Mode###FLmode", &Vars::CL_Move::FakeLag::Mode.Value, { "Always", "Hold", "Toggle" });
			if (Vars::CL_Move::FakeLag::Mode.Value != 0)
				InputKeybind("Fakelag key", Vars::CL_Move::FakeLag::Key.Value);
			WCombo("Type###FLtype", &Vars::CL_Move::FakeLag::Type.Value, { "Plain", "Random", "Adaptive" });

			switch (Vars::CL_Move::FakeLag::Type.Value)
			{
				case 0: WSlider("Fakelag value", &Vars::CL_Move::FakeLag::Value.Value, 1, 22, "%d", ImGuiSliderFlags_AlwaysClamp); break;
				case 1:
				{
					WSlider("Random max###flRandMax", &Vars::CL_Move::FakeLag::Max.Value, Vars::CL_Move::FakeLag::Min.Value + 1, 22, "%d", ImGuiSliderFlags_AlwaysClamp); HelpMarker("Maximum random fakelag value");
					WSlider("Random min###flRandMin", &Vars::CL_Move::FakeLag::Min.Value, 1, Vars::CL_Move::FakeLag::Max.Value - 1, "%d", ImGuiSliderFlags_AlwaysClamp); HelpMarker("Minimum random fakelag value");
					break;
				}
			}	//	add more here if you add your own fakelag modes :D
			MultiCombo({ "While Moving", "While Unducking", "While Airborne" }, { &Vars::CL_Move::FakeLag::WhileMoving.Value, &Vars::CL_Move::FakeLag::WhileUnducking.Value, &Vars::CL_Move::FakeLag::WhileGrounded.Value }, "Flags###FakeLagFlags");
			WToggle("Unchoke On Attack", &Vars::CL_Move::FakeLag::UnchokeOnAttack.Value); HelpMarker("Will exit a fakelag cycle if you are attacking.");

			WToggle("Retain BlastJump", &Vars::CL_Move::FakeLag::RetainBlastJump.Value); HelpMarker("Will attempt to retain the blast jumping condition as soldier and runs independently of fakelag.");
		} EndChild();

		/* Column 2 */
		if (TableColumnChild("HvHCol2"))
		{
			/* Section: Anti Aim */
			SectionTitle("Anti Aim");
			WToggle("Active", &Vars::AntiHack::AntiAim::Active.Value);
			InputKeybind("Anti-aim Key", Vars::AntiHack::AntiAim::ToggleKey.Value); HelpMarker("The key to toggle anti aim");

			WCombo("Real Pitch", &Vars::AntiHack::AntiAim::PitchReal.Value, { "None", "Up", "Down", "Zero" }); HelpMarker("The pitch your hitboxes will be built around.");
			WCombo("Fake Pitch", &Vars::AntiHack::AntiAim::PitchFake.Value, { "None", "Up", "Down" }); HelpMarker("The pitch that other players will see.");
			
			WCombo("Real yaw", &Vars::AntiHack::AntiAim::YawReal.Value, { "None", "Forward", "Left", "Right", "Backwards", "Spin", "Edge" }); HelpMarker("The yaw your hitboxes will be built around.");
			WCombo("Fake yaw", &Vars::AntiHack::AntiAim::YawFake.Value, { "None", "Forward", "Left", "Right", "Backwards", "Spin", "Edge" }); HelpMarker("The yaw that other players will see.");
			if (Vars::AntiHack::AntiAim::YawFake.Value == 5 || Vars::AntiHack::AntiAim::YawReal.Value == 5)
			{
				WSlider("Spin Speed", &Vars::AntiHack::AntiAim::SpinSpeed.Value, -30.f, 30.f, "%.0f", 0); HelpMarker("How many degrees you will rotate in a tick");
			}
			
			WCombo("Real Offset", &Vars::AntiHack::AntiAim::RealYawMode.Value, { "Offset", "FOV Player", "FOV Player + Offset" }); HelpMarker("The yaw that your real yaw is added to.");
			if (Vars::AntiHack::AntiAim::RealYawMode.Value != 1)
				WSlider("Yaw Offset###RealYawOffset", &Vars::AntiHack::AntiAim::RealYawOffset.Value, -180, 180, "%.0f");
			WCombo("Fake Offset", &Vars::AntiHack::AntiAim::FakeYawMode.Value, { "Offset", "FOV Player", "FOV Player + Offset" }); HelpMarker("The yaw that your fake yaw is added to.");
			if (Vars::AntiHack::AntiAim::FakeYawMode.Value != 1)
				WSlider("Yaw Offset###FakeYawOffset", &Vars::AntiHack::AntiAim::FakeYawOffset.Value, -180, 180, "%.0f");

			MultiCombo({ "AntiOverlap", "Jitter Legs", "HidePitchOnShot", "Anti-Backstab" }, { &Vars::AntiHack::AntiAim::AntiOverlap.Value, &Vars::AntiHack::AntiAim::LegJitter.Value, &Vars::AntiHack::AntiAim::InvalidShootPitch.Value, &Vars::AntiHack::AntiAim::AntiBackstab.Value }, "Misc.");

			/* Section: Auto Peek */
			SectionTitle("Auto Peek");
			InputKeybind("Autopeek Key", Vars::CL_Move::AutoPeekKey.Value); HelpMarker("Hold this key while peeking and use A/D to set the peek direction");
			WSlider("Max Distance", &Vars::CL_Move::AutoPeekDistance.Value, 50.f, 400.f, "%.0f"); HelpMarker("Maximum distance that auto peek can walk");
			WToggle("Free move", &Vars::CL_Move::AutoPeekFree.Value); HelpMarker("Allows you to move freely while peeking");

			SectionTitle("Cheater Detection");
			WToggle("Enable Cheater Detection", &Vars::Misc::CheaterDetection::Enabled.Value);
			if (Vars::Misc::CheaterDetection::Enabled.Value)
			{
				{
					static std::vector flagNames{ "Accuracy", "Score", "Simtime Changes", "Packet Choking", "Bunnyhopping", "Aim Flicking", "OOB Angles", "Aimbot", "Duck Speed" };
					static std::vector flagValues{ 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7, 1 << 8 };
					MultiFlags(flagNames, flagValues, &Vars::Misc::CheaterDetection::Methods.Value, "Detection Methods###CheaterDetectionMethods");
					HelpMarker("Methods by which to detect bad actors.");
				}
				{
					static std::vector flagNames{ "Double Scans", "Lagging Client", "Timing Out" };
					static std::vector flagValues{ 1 << 0, 1 << 1, 1 << 2 };
					MultiFlags(flagNames, flagValues, &Vars::Misc::CheaterDetection::Protections.Value, "Ignore Conditions###CheaterDetectionIgnoreMethods");
					HelpMarker("Don't scan in certain scenarios (prevents false positives).");
				}
				WSlider("Suspicion Gate", &Vars::Misc::CheaterDetection::SuspicionGate.Value, 5, 50, "%d"); HelpMarker("Infractions required to mark somebody as a cheater.");

				if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 1))
				{
					WSlider("Analytical High Score Mult", &Vars::Misc::CheaterDetection::ScoreMultiplier.Value, 1.5f, 4.f, "%.1f"); HelpMarker("How much to multiply the average score to treat as a max score per second.");
				}

				if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 3 | 1 << 2))
				{
					WSlider("Packet Manipulation Gate", &Vars::Misc::CheaterDetection::PacketManipGate.Value, 1, 22, "%d"); HelpMarker("Used as the minimum amount of average packet manipulation to infract someone as a cheater.");
				}

				if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 4))
				{
					WSlider("BHop Sensitivity", &Vars::Misc::CheaterDetection::BHopMaxDelay.Value, 1, 5, "%d"); HelpMarker("How many ticks a player can be on the ground before their next jump isn't counted as a bhop.");
					WSlider("BHop Minimum Detections", &Vars::Misc::CheaterDetection::BHopDetectionsRequired.Value, 2, 15, "%d"); HelpMarker("How many concurrent bunnyhops need to be executed before someone is infracted.");
				}

				if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 5))
				{
					WSlider("Minimum Aim-Flick", &Vars::Misc::CheaterDetection::MinimumFlickDistance.Value, 5.f, 30.f, "%.1f"); HelpMarker("The distance someones view angles must flick prior to be being suspected by the cheat detector.");
					WSlider("Maximum Post Flick Noise", &Vars::Misc::CheaterDetection::MaximumNoise.Value, 5.f, 15.f, "%.1f"); HelpMarker("The maximum distance the players mouse can move post-flick before the cheat detector considers it as a legit flick (mouse moved fast, etc).");
				}

				if (Vars::Misc::CheaterDetection::Methods.Value & (1 << 7))
				{
					WSlider("Maximum Scaled Aimbot FoV", &Vars::Misc::CheaterDetection::MaxScaledAimbotFoV.Value, 5.f, 30.f, "%.1f"); HelpMarker("The maximum FoV (post scaling) for the aimbot detection.");
					WSlider("Minimum Aimbot FoV", &Vars::Misc::CheaterDetection::MinimumAimbotFoV.Value, 5.f, 30.f, "%.1f"); HelpMarker("The minimum FoV to infract a player for aimbot.");
				}
			}
			SectionTitle("Resolver");
			WToggle("Enable Resolver", &Vars::AntiHack::Resolver::Resolver.Value);
			if (Vars::AntiHack::Resolver::Resolver.Value)
				WToggle("Ignore in-air", &Vars::AntiHack::Resolver::IgnoreAirborne.Value);
		} EndChild();

		EndTable();
	}
}

/* Tab: Misc */
void CMenu::MenuMisc()
{
	using namespace ImGui;
	if (BeginTable("MiscTable", 2))
	{
		/* Column 1 */
		if (TableColumnChild("MiscCol1"))
		{
			SectionTitle("Movement");
			WCombo("No Push###MovementNoPush", &Vars::Misc::NoPush.Value, { "Off", "Always", "While Moving", "Partial While AFK" });
			WCombo("Fast Stop", &Vars::Misc::AccurateMovement.Value, { "Off", "Legacy", "Instant", "Adaptive" }); HelpMarker("Will stop you from sliding once you stop pressing movement buttons");
			WToggle("Fast Strafe", &Vars::Misc::FastDeltaStrafe.Value); HelpMarker("Allows you to change direction instantly.");
			WToggle("Fast Accel", &Vars::Misc::FastAccel.Value); HelpMarker("Makes you accelerate to full speed faster.");
			WToggle("Crouch Speed", &Vars::Misc::CrouchSpeed.Value); HelpMarker("Allows you to move at full speed while crouched.");
			if (Vars::Misc::CrouchSpeed.Value)
			{
				WToggle("Hide Real Angle", &Vars::Misc::FakeAccelAngle.Value); HelpMarker("Tries to stop your angle from updating while using crouch speed (janky).");
			}
			WToggle("Bunnyhop", &Vars::Misc::AutoJump.Value); HelpMarker("Will jump as soon as you touch the ground again, keeping speed between jumps");
			WCombo("Autostrafe", &Vars::Misc::AutoStrafe.Value, { "Off", "Legit", "Directional" }); HelpMarker("Will strafe for you in air automatically so that you gain speed");
			if (Vars::Misc::AutoStrafe.Value == 2)
			{
				WToggle("Only on movement key", &Vars::Misc::DirectionalOnlyOnMove.Value); HelpMarker("This makes it so that you dont always go forward if u just hold space");
				WToggle("Only on space", &Vars::Misc::DirectionalOnlyOnSpace.Value); HelpMarker("only space ??"); //temporary
			}
			SectionTitle("Automation");
			WToggle("Anti-AFK", &Vars::Misc::AntiAFK.Value); HelpMarker("Will make you jump every now and then so you don't get kicked for idling");
			WToggle("Taunt slide", &Vars::Misc::TauntSlide.Value); HelpMarker("Allows you to input in taunts");
			WToggle("Auto accept item drops", &Vars::Misc::AutoAcceptItemDrops.Value); HelpMarker("Automatically accepts all item drops");

			SectionTitle("Sound");
			MultiFlags({ "Footsteps", "Noisemaker" }, { 1 << 0, 1 << 1 }, &Vars::Misc::SoundBlock.Value, "Block Sounds###SoundRemovals");
		} EndChild();

		/* Column 2 */
		if (TableColumnChild("MiscCol2"))
		{
			SectionTitle("Chat");
			WToggle("Chat Flags", &Vars::Misc::ChatFlags.Value); HelpMarker("Adds advanced prefixes to chat messages");

			SectionTitle("Queueing");
			WToggle("Region selector", &Vars::Misc::RegionChanger.Value);

			MultiFlags({ "Atlanta", "Chicago", "Los Angeles", "Moses Lake", "Seattle", "Virginia", "Washington", "Amsterdam", "Frankfurt", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "Buenos Aires", "Lima", "Santiago", "Sao Paulo", "Chennai", "Dubai", "Guangzhou", "Hong Kong", "Mumbai", "Seoul", "Shanghai", "Singapore", "Tianjin", "Tokyo", "Sydney", "Johannesburg" },
				{		  DC_ATL,	 DC_ORD,	DC_LAX,		   DC_EAT,		 DC_SEA,	DC_IAD,		DC_DFW,		  DC_AMS,	   DC_FRA,		DC_LHR,	  DC_MAD,	DC_PAR,	 DC_STO,	  DC_VIE,	DC_WAW,	  DC_EZE,		  DC_LIM, DC_SCL,	  DC_GRU,	   DC_MAA,	  DC_DXB,  DC_CAN,		DC_HKG,		 DC_BOM,   DC_SEO,	DC_SHA,		DC_SGP,		 DC_TSN,	DC_TYO,	 DC_SYD, DC_JNB			 },
				&Vars::Misc::RegionsAllowed.Value,
				"Regions"
			);
			WToggle("Freeze queue timer", &Vars::Misc::FreezeQueue.Value);
			WCombo("Auto casual queue", &Vars::Misc::AutoCasualQueue.Value, { "Off", "In menu", "Always" }); HelpMarker("Automatically starts queueuing for casual");

			SectionTitle("Exploits");
			WToggle("Cheats bypass", &Vars::Misc::CheatsBypass.Value); HelpMarker("Allows you to use some sv_cheats commands(clientside)");
			WToggle("Pure bypass", &Vars::Misc::BypassPure.Value); HelpMarker("Allows you to load any custom files, even if disallowed by the sv_pure setting");
			WToggle("Ping reducer", &Vars::Misc::PingReducer.Value); HelpMarker("Reduces your ping on the scoreboard");
			if (Vars::Misc::PingReducer.Value)
			{
				WSlider("cl_cmdrate", &Vars::Misc::PingTarget.Value, 1, 66); HelpMarker("Lower values for lower leaderboard ping");
			}
			WToggle("Equip region unlock", &Vars::Visuals::EquipRegionUnlock.Value); HelpMarker("This doesn't let you add the equip regions back once you turn it on."); // why is this in visuals

			if (Vars::Debug::Info.Value)
			{
				{
					SectionTitle("Convar spoofer");
					WInputText("Convar", &Vars::Misc::ConvarName.Value);
					WInputText("Value", &Vars::Misc::ConvarValue.Value);
					if (Button("Send", ImVec2(GetWindowSize().x - 2 * GetStyle().WindowPadding.x, 20)))
					{
						CNetChannel* netChannel = I::EngineClient->GetNetChannelInfo();
						if (netChannel == nullptr) { return; }

						Utils::ConLog("Convar", std::format("Sent {} as {}", Vars::Misc::ConvarName.Value, Vars::Misc::ConvarValue.Value).c_str(), { 255, 0, 255, 255 });
						NET_SetConVar cmd(Vars::Misc::ConvarName.Value.c_str(), Vars::Misc::ConvarValue.Value.c_str());
						netChannel->SendNetMsg(cmd);

						//Vars::Misc::ConvarName = "";
						//Vars::Misc::ConvarValue = "";
					}
				}
			}

			SectionTitle("Steam RPC");
			WToggle("Steam RPC", &Vars::Misc::Steam::EnableRPC.Value); HelpMarker("Enable Steam Rich Presence"); HelpMarker("Enable Steam Rich Presence");
			WCombo("Match group", &Vars::Misc::Steam::MatchGroup.Value, { "Special Event", "MvM Mann Up", "Competitive", "Casual", "MvM Boot Camp" }); HelpMarker("Which match group should be used?");
			WToggle("Override in menu", &Vars::Misc::Steam::OverrideMenu.Value); HelpMarker("Override match group to \"Main Menu\" when in main menu");
			WCombo("Map text", &Vars::Misc::Steam::MapText.Value, { "Custom", "Fedoraware", "Figoraware", "Meowhook.club", "Rathook.cc", "Nitro.tf" }); HelpMarker("Which map text should be used?");
			if (Vars::Misc::Steam::MapText.Value == 0)
			{
				WInputText("Custom map text", &Vars::Misc::Steam::CustomText.Value); HelpMarker(R"(For when "Custom" is selcted in "Map text". Sets custom map text.)");
			}
			WInputInt("Group size", &Vars::Misc::Steam::GroupSize.Value); HelpMarker("Sets party size");
		} EndChild();

		EndTable();
	}
}

/* Tab: Settings */
void CMenu::MenuSettings()
{
	using namespace ImGui;
	if (BeginTable("SettingsTable", 2))
	{
		/* Column 1 */
		if (TableColumnChild("SettingsCol1"))
		{
			SectionTitle("Settings");
			if (ColorPicker("Accent", Vars::Menu::Theme::Accent.Value))
				LoadStyle();
			SameLine(); Text("Accent");

			WInputText("Name", &Vars::Menu::CheatName.Value);
			WInputText("Chat Prefix", &Vars::Menu::CheatPrefix.Value);
			WToggle("Keybinds", &Vars::Menu::ShowKeybinds.Value);

			InputKeybind("Menu key", Vars::Menu::MenuKey.Value, true);

			SectionTitle("Configs");

			if (Button("Open configs folder", SIZE_FULL_WIDTH))
				ShellExecuteA(NULL, NULL, g_CFG.GetConfigPath().c_str(), NULL, NULL, SW_SHOWNORMAL);
			if (Button("Open visuals folder", SIZE_FULL_WIDTH))
				ShellExecuteA(NULL, NULL, g_CFG.GetVisualsPath().c_str(), NULL, NULL, SW_SHOWNORMAL);

			Dummy({ 0, 5 });

			/* Config Tabs */
			ImGui::PushFont(SectionFont);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
			if (ImGui::BeginTable("ConfigTable", 2))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, TextLight.Value);
				if (ImGui::TabButton("General", CurrentConfigTab == ConfigTab::General))
					CurrentConfigTab = ConfigTab::General;

				if (ImGui::TabButton("Visuals", CurrentConfigTab == ConfigTab::Visuals))
					CurrentConfigTab = ConfigTab::Visuals;

				ImGui::PopStyleColor(1);
				ImGui::EndTable();
			}
			ImGui::PopStyleVar(2);
			ImGui::PopFont();

			static std::string selected;
			static std::string selectedvis;

			/* Config list*/
			if (CurrentConfigTab == ConfigTab::General)
			{
				// Current config
				const std::string cfgText = "Loaded: " + g_CFG.GetCurrentConfig();
				Text(cfgText.c_str());

				// Config name field
				std::string newConfigName = {};
				if (InputTextWithHint("###configname", "New config name", &newConfigName, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!std::filesystem::exists(g_CFG.GetConfigPath() + "\\" + newConfigName))
					{
						g_CFG.SaveConfig(newConfigName);
					}
				}

				// Config list
				for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetConfigPath()))
				{
					if (!entry.is_regular_file()) { continue; }
					if (entry.path().extension() != g_CFG.ConfigExtension) { continue; }

					std::string configName = entry.path().filename().string();
					configName.erase(configName.end() - g_CFG.ConfigExtension.size(), configName.end());

					if (configName == selected)
					{
						const ImGuiStyle* style2 = &GetStyle();
						const ImVec4* colors2 = style2->Colors;
						ImVec4 buttonColor = colors2[ImGuiCol_Button];
						buttonColor.w *= .5f;
						PushStyleColor(ImGuiCol_Button, buttonColor);

						// Config name button
						if (Button(configName.c_str(), SIZE_FULL_WIDTH))
						{
							selected = configName;
						}
						PopStyleColor();

						if (BeginTable("ConfigActions", 3))
						{
							// Save config button
							TableNextColumn();
							if (Button("Save", SIZE_FULL_WIDTH))
							{
								if (configName != g_CFG.GetCurrentConfig())
								{
									OpenPopup("Save config?");
								}
								else
								{
									g_CFG.SaveConfig(selected);
									selected.clear();
								}
							}

							// Load config button
							TableNextColumn();
							if (Button("Load", SIZE_FULL_WIDTH))
							{
								g_CFG.LoadConfig(selected);
								selected.clear();
								LoadStyle();
							}

							// Remove config button
							TableNextColumn();
							if (Button("Remove", SIZE_FULL_WIDTH))
							{
								OpenPopup("Remove config?");
							}

							// Dialogs
							{
								// Save config dialog
								if (BeginPopupModal("Save config?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
								{
									Text("Do you really want to override this config?");

									Separator();
									if (Button("Yes, override!", ImVec2(150, 0)))
									{
										g_CFG.SaveConfig(selected);
										selected.clear();
										CloseCurrentPopup();
									}

									SameLine();
									if (Button("No", ImVec2(120, 0)))
									{
										CloseCurrentPopup();
									}
									EndPopup();
								}

								// Delete config dialog
								if (BeginPopupModal("Remove config?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
								{
									Text("Do you really want to delete this config?");

									Separator();
									if (Button("Yes, remove!", ImVec2(150, 0)))
									{
										g_CFG.RemoveConfig(selected);
										selected.clear();
										CloseCurrentPopup();
									}
									SameLine();
									if (Button("No", ImVec2(150, 0)))
									{
										CloseCurrentPopup();
									}
									EndPopup();
								}
							}

							EndTable();
						}
					}
					else if (configName == g_CFG.GetCurrentConfig())
					{
						PushStyleColor(ImGuiCol_Button, GetStyle().Colors[ImGuiCol_ButtonActive]);
						std::string buttonText = "> " + configName + " <";
						if (Button(buttonText.c_str(), SIZE_FULL_WIDTH))
						{
							selected = configName;
						}
						PopStyleColor();
					}
					else
					{
						if (Button(configName.c_str(), SIZE_FULL_WIDTH))
						{
							selected = configName;
						}
					}
				}

			}
			else if (CurrentConfigTab == ConfigTab::Visuals)
			{
				// Current config
				const std::string cfgText = "Loaded: " + g_CFG.GetCurrentVisuals();
				Text(cfgText.c_str());

				// Config name field
				std::string newConfigName = {};

				if (InputTextWithHint("###configname", "New config name", &newConfigName, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!std::filesystem::exists(g_CFG.GetVisualsPath() + "\\" + newConfigName))
					{
						g_CFG.SaveVisual(newConfigName);
					}
				}

				// Visuals list
				for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetVisualsPath()))
				{
					if (!entry.is_regular_file()) { continue; }
					if (entry.path().extension() != g_CFG.ConfigExtension) { continue; }

					std::string configName = entry.path().filename().string();
					configName.erase(configName.end() - g_CFG.ConfigExtension.size(), configName.end());

					if (configName == selected)
					{
						const ImGuiStyle* style2 = &GetStyle();
						const ImVec4* colors2 = style2->Colors;
						ImVec4 buttonColor = colors2[ImGuiCol_Button];
						buttonColor.w *= .5f;
						PushStyleColor(ImGuiCol_Button, buttonColor);

						// Config name button
						if (Button(configName.c_str(), SIZE_FULL_WIDTH))
						{
							selected = configName;
						}
						PopStyleColor();

						// Visuals action buttons
						if (BeginTable("ConfigActions", 3))
						{
							// Visuals config button
							TableNextColumn();
							if (Button("Save", SIZE_FULL_WIDTH))
							{
								if (configName != g_CFG.GetCurrentVisuals())
								{
									OpenPopup("Save visuals?");
								}
								else
								{
									g_CFG.SaveVisual(selected);
									selected.clear();
								}
							}

							// Load visuals button
							TableNextColumn();
							if (Button("Load", SIZE_FULL_WIDTH))
							{
								g_CFG.LoadVisual(selected);
								selected.clear();
								LoadStyle();
							}

							// Remove visual button
							TableNextColumn();
							if (Button("Remove", SIZE_FULL_WIDTH))
							{
								OpenPopup("Remove visuals?");
							}

							// Dialogs
							{
								// Save config dialog
								if (BeginPopupModal("Save visuals?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
								{
									Text("Do you really want to override this config?");

									Separator();
									if (Button("Yes, override!", ImVec2(150, 0)))
									{
										g_CFG.SaveVisual(selected);
										selected.clear();
										CloseCurrentPopup();
									}

									SameLine();
									if (Button("No", ImVec2(120, 0)))
									{
										CloseCurrentPopup();
									}
									EndPopup();
								}

								// Delete config dialog
								if (BeginPopupModal("Remove visuals?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
								{
									Text("Do you really want to delete this config?");

									Separator();
									if (Button("Yes, remove!", ImVec2(150, 0)))
									{
										g_CFG.RemoveVisual(selected);
										selected.clear();
										CloseCurrentPopup();
									}
									SameLine();
									if (Button("No", ImVec2(150, 0)))
									{
										CloseCurrentPopup();
									}
									EndPopup();
								}
							}

							EndTable();
						}
					}
					else if (configName == g_CFG.GetCurrentVisuals())
					{
						PushStyleColor(ImGuiCol_Button, GetStyle().Colors[ImGuiCol_ButtonActive]);
						std::string buttonText = "> " + configName + " <";
						if (Button(buttonText.c_str(), SIZE_FULL_WIDTH))
						{
							selected = configName;
						}
						PopStyleColor();
					}
					else
					{
						if (Button(configName.c_str(), SIZE_FULL_WIDTH))
						{
							selected = configName;
						}
					}
				}
			}
		} EndChild();

		/* Column 2 */
		if (TableColumnChild("SettingsCol2"))
		{
			SectionTitle("Debug");
			WToggle("Debug info", &Vars::Debug::Info.Value);
			WToggle("Debug logging", &Vars::Debug::Logging.Value);
			WToggle("Allow secure servers", I::AllowSecureServers);

			bool* m_bPendingPingRefresh = reinterpret_cast<bool*>(I::TFGCClientSystem + 828);
			WToggle("Pending Ping Refresh", m_bPendingPingRefresh);

			WToggle("Show server hitboxes###tpShowServer", &Vars::Debug::ServerHitbox.Value); HelpMarker("Will show the server angles in thirdperson in localhost servers");
			WToggle("Anti aim lines", &Vars::Debug::AntiAimLines.Value);

			// Particle tester
			if (CollapsingHeader("Particles"))
			{
				static std::string particleName = "ping_circle";

				InputText("Particle name", &particleName);
				const auto& pLocal = g_EntityCache.GetLocal();
				if (Button("Dispatch") && pLocal)
					Particles::DispatchParticleEffect(particleName.c_str(), pLocal->GetAbsOrigin(), { });
			}

			SectionTitle("Logging");
			MultiFlags({ "Votes", "Class Changes", "Damage" }, { 1 << 0, 1 << 1, 1 << 2 }, & Vars::Logging::Logs.Value, "Logs");
			MultiFlags({ "Toasts", "Chat", "Party", "Console" }, { 1 << 0, 1 << 1, 1 << 2, 1 << 3 }, & Vars::Logging::LogTo.Value, "Log to");
			ColorPickerL("Notification background", Vars::Logging::Notification::Background.Value);
			ColorPickerL("Notification outline", Vars::Logging::Notification::Outline.Value, 1);
			ColorPickerL("Notification color", Vars::Logging::Notification::Text.Value, 2);
			WSlider("Notification time", &Vars::Logging::Notification::Lifetime.Value, 0.5f, 3.f, "%.1f");

			SectionTitle("Utilities");
			if (Button("Full update", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("cl_fullupdate");
			if (Button("Reload HUD", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("hud_reloadscheme");
			if (Button("Restart sound", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("snd_restart");
			if (Button("Stop sound", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("stopsound");
			if (Button("Status", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("status");
			if (Button("Ping", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("ping");
			if (Button("Retry", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("retry");
			if (Button("Exit", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("exit");
			if (Button("Console", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("showconsole");
			if (Button("Demo playback", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("demoui");
			if (Button("Demo trackbar", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("demoui2");
			if (Button("Itemtest", SIZE_FULL_WIDTH))
				I::EngineClient->ClientCmd_Unrestricted("itemtest");
			if (Button("Fix Chams", SIZE_FULL_WIDTH))
			{
				F::DMEChams.CreateMaterials();
				F::Glow.CreateMaterials();
				F::MaterialEditor.LoadMaterials();
			}
			if (Button("Print Hashes", SIZE_FULL_WIDTH))
				Hash::PrintHash();
			if (Button("Reveal bullet lines", SIZE_FULL_WIDTH))
				F::Visuals.RevealBulletLines();
			if (Button("Reveal prediction lines", SIZE_FULL_WIDTH))
				F::Visuals.RevealSimLines();
			if (Button("Reveal boxes", SIZE_FULL_WIDTH))
				F::Visuals.RevealBoxes();
#ifdef DEBUG
			if (Button("Dump Classes", SIZE_FULL_WIDTH))
				F::Misc.DumpClassIDS();
#endif
			//if (Button("CPrint", SIZE_FULL_WIDTH))
			//	I::CenterPrint->Print((char*)"niggaz");

			if (!I::EngineClient->IsConnected())
			{
				if (Button("Unlock all achievements", SIZE_FULL_WIDTH))
					F::Misc.UnlockAchievements();
				if (Button("Lock all achievements", SIZE_FULL_WIDTH))
					F::Misc.LockAchievements();
			}
		} EndChild();

		EndTable();
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
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.f, 1.f, 1.f, 1.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, { 100.f, 40.f });

		if (ImGui::Begin(szTitle, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			ImGui::PushFont(VerdanaBold);
			auto size = ImGui::CalcTextSize(szTitle);
			ImGui::SetCursorPos({ (100 - size.x) * 0.5f, (40 - size.y) * 0.5f });
			ImGui::Text(szTitle);
			ImGui::PopFont();

			const ImVec2 winPos = ImGui::GetWindowPos();
			const ImVec2 winSize = ImGui::GetWindowSize();

			info.x = static_cast<int>(winPos.x);
			info.y = static_cast<int>(winPos.y);
			info.w = static_cast<int>(100.f);
			info.h = static_cast<int>(40.f);
			info.c = static_cast<int>(info.x + 50.f);

			ImGui::End();
		}
		ImGui::PopStyleVar(2);
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

	if (ImGui::Begin("Keybinds", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
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
		drawOption("Triggerbot", isActive(Vars::Triggerbot::Global::Active.Value, Vars::Triggerbot::Global::TriggerKey.Value, Vars::Triggerbot::Global::TriggerKey.Value));

		ImGui::End();
	}

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
}

void CMenu::Render(IDirect3DDevice9* pDevice)
{
	if (!ConfigLoaded) { return; }

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
	ImGui::PushFont(Verdana);

	// better looking dt bars
	F::Visuals.DrawTickbaseBars();

	// Window that should always be visible
	DrawKeybinds();
	F::Radar.DrawWindow();

	if (IsOpen)
	{
		DrawMenu();
		AddDraggable("Doubletap", Vars::CL_Move::DoubleTap::Position.Value, Vars::CL_Move::DoubleTap::Indicator.Value);
		AddDraggable("Crithack", Vars::CritHack::IndicatorPos.Value, Vars::CritHack::Indicators.Value);
		AddDraggable("Spectators", Vars::Visuals::SpectatorPos.Value, Vars::Visuals::SpectatorList.Value);
		AddDraggable("Conditions", Vars::Visuals::OnScreenConditions.Value, Vars::Visuals::DrawOnScreenConditions.Value);
		AddDraggable("Ping", Vars::Visuals::OnScreenPing.Value, Vars::Visuals::DrawOnScreenPing.Value);

		F::MaterialEditor.Render();
		F::PlayerList.Render();
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
		ItemWidth = 150.f;

		// https://raais.github.io/ImStudio/
		Accent = ImGui::ColorToVec(Vars::Menu::Theme::Accent.Value);
		AccentDark = ImColor(Accent.Value.x * 0.8f, Accent.Value.y * 0.8f, Accent.Value.z * 0.8f, Accent.Value.w);

		auto& style = ImGui::GetStyle();
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // Center window title
		style.WindowMinSize = ImVec2(100, 100);
		style.WindowPadding = ImVec2(0, 0);
		style.WindowBorderSize = 1.f;
		style.ButtonTextAlign = ImVec2(0.5f, 0.4f); // Center button text
		style.FrameBorderSize = 0.f;
		style.FrameRounding = 2.f;
		style.ChildBorderSize = 1.f;
		style.ChildRounding = 0.f;
		style.GrabMinSize = 15.f;
		style.GrabRounding = 2.f;
		style.ScrollbarSize = 4.f;
		style.ScrollbarRounding = 6.f;
		style.ItemSpacing = ImVec2(8.f, 5.f);

		ImVec4* colors = style.Colors;
		colors[ImGuiCol_Border] = ImColor(110, 110, 128);
		colors[ImGuiCol_WindowBg] = Background;
		colors[ImGuiCol_TitleBg] = BackgroundDark;
		colors[ImGuiCol_TitleBgActive] = BackgroundLight;
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.4f);
		colors[ImGuiCol_Button] = BackgroundLight;
		colors[ImGuiCol_ButtonHovered] = ImColor(69, 69, 77);
		colors[ImGuiCol_ButtonActive] = ImColor(82, 79, 87);
		colors[ImGuiCol_PopupBg] = BackgroundDark;
		colors[ImGuiCol_FrameBg] = ImColor(50, 50, 50);
		colors[ImGuiCol_FrameBgHovered] = ImColor(60, 60, 60);
		colors[ImGuiCol_FrameBgActive] = ImColor(70, 70, 70);
		colors[ImGuiCol_CheckMark] = Accent;
		colors[ImGuiCol_Text] = TextLight;

		colors[ImGuiCol_SliderGrab] = Accent;
		colors[ImGuiCol_SliderGrabActive] = AccentDark;
		colors[ImGuiCol_ResizeGrip] = Accent;
		colors[ImGuiCol_ResizeGripActive] = Accent;
		colors[ImGuiCol_ResizeGripHovered] = Accent;
		colors[ImGuiCol_Header] = ImColor(70, 70, 70);
		colors[ImGuiCol_HeaderActive] = ImColor(40, 40, 40);
		colors[ImGuiCol_HeaderHovered] = ImColor(60, 60, 60);
	}
}

void CMenu::Init(IDirect3DDevice9* pDevice)
{
	// Initialize ImGui and device
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(FindWindowA(nullptr, "Team Fortress 2"));
	ImGui_ImplDX9_Init(pDevice);

	// Fonts
	{
		const auto& io = ImGui::GetIO();

		auto fontConfig = ImFontConfig();
		fontConfig.OversampleH = 2;

		constexpr ImWchar fontRange[]{ 0x0020, 0x00FF, 0x0400, 0x044F, 0 }; // Basic Latin, Latin Supplement and Cyrillic

		VerdanaSmall = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", 12.0f, &fontConfig, fontRange);
		Verdana = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", 14.0f, &fontConfig, fontRange);
		VerdanaBold = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdanab.ttf)", 14.0f, &fontConfig, fontRange);

		SectionFont = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdana.ttf)", 16.0f, &fontConfig, fontRange);
		TitleFont = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\verdanab.ttf)", 20.0f, &fontConfig, fontRange);

		constexpr ImWchar iconRange[]{ ICON_MIN_MD, ICON_MAX_MD, 0 };
		ImFontConfig iconConfig;
		iconConfig.MergeMode = true;
		iconConfig.PixelSnapH = true;
		IconFont = io.Fonts->AddFontFromMemoryCompressedTTF(MaterialFont_compressed_data, MaterialFont_compressed_size, 16.f, &iconConfig, iconRange);

		io.Fonts->Build();
	}

	LoadStyle();
	F::MaterialEditor.Init();
}