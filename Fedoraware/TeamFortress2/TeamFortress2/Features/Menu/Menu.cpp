#include "Menu.h"

#include "../Vars.h"
#include "../Radar/Radar.h"
#include "../Misc/Misc.h"
#include "../Chams/DMEChams.h"
#include "../Glow/Glow.h"
#include "../Visuals/Visuals.h"
#include "../Backtrack/Backtrack.h"

#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_stdlib.h"
#include "Fonts/IconsMaterialDesign.h"
#include "Playerlist/Playerlist.h"
#include "MaterialEditor/MaterialEditor.h"

#include "Components.hpp"
#include "ConfigManager/ConfigManager.h"

#include <mutex>

#pragma warning (disable : 4309)

int unuPrimary = 0;
int unuSecondary = 0;

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

		// Title gradient setup
		{
			TitleGradient.ClearMarks();
			TitleGradient.AddMark(0.f, ImGui::ColorToVec(Color_t{ 0,0,0,0 }));
			TitleGradient.AddMark(0.25f, ImGui::ColorToVec(Color_t{ 0,0,0,0 }));
			TitleGradient.AddMark(0.5f, ImGui::ColorToVec(Vars::Menu::MenuAccent));
			TitleGradient.AddMark(0.75f, ImGui::ColorToVec(Color_t{ 0,0,0,0 }));
			TitleGradient.AddMark(1.0f, ImGui::ColorToVec(Color_t{ 0,0,0,0 }));
		}
		ImGui::GradientRect(&TitleGradient, { windowPos.x, windowPos.y }, windowSize.x, 3.f);
		ImGui::Dummy({ 0, 2 });

		// Title Text
		{
			ImGui::PushFont(TitleFont);
			const auto titleWidth = ImGui::CalcTextSize(Vars::Menu::CheatName.c_str()).x;
			drawList->AddText(TitleFont, TitleFont->FontSize, { windowPos.x + (windowSize.x / 2) - (titleWidth / 2), windowPos.y }, Accent, Vars::Menu::CheatName.c_str());
			ImGui::PopFont();
		}

		// Icons
		{
			float currentX = windowSize.x;

			// Settings Icon
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_SETTINGS))
			{
				ShowSettings = !ShowSettings;
			}
			ImGui::HelpMarker("Settings");

			// Playerlist Icon
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_PEOPLE))
			{
				Vars::Menu::ShowPlayerlist = !Vars::Menu::ShowPlayerlist;
			}
			ImGui::HelpMarker("Playerlist");

			// Keybinds Icon
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_KEYBOARD))
			{
				Vars::Menu::ShowKeybinds = !Vars::Menu::ShowKeybinds;
			}
			ImGui::HelpMarker("Keybinds");

			// Material Editor Icon
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_BRUSH))
			{
				F::MaterialEditor.IsOpen = !F::MaterialEditor.IsOpen;
			}
			ImGui::HelpMarker("Material Editor");

			// Debug Menu
			ImGui::SetCursorPos({ currentX -= 25, 0 });
			if (ImGui::IconButton(ICON_MD_BUG_REPORT))
			{
				ShowDebugMenu = !ShowDebugMenu;
			}
			ImGui::HelpMarker("Debug Menu");
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
			}

			ImGui::PopStyleVar();
			ImGui::PopFont();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		// Footer
		{
			if (!Vars::Menu::ModernDesign)
			{
				const auto hintHeight = ImGui::CalcTextSize(FeatureHint.c_str()).y;
				drawList->AddText(Verdana, Verdana->FontSize, { windowPos.x + 10, windowPos.y + windowSize.y - (hintHeight + ImGui::GetStyle().ItemInnerSpacing.y) }, TextLight, FeatureHint.c_str());
			}
		}

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

	if (ImGui::BeginTable("TabbarTable", 5))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, BackgroundLight.Value);
		ImGui::PushStyleColor(ImGuiCol_Text, TextLight.Value);
		if (ImGui::TabButton("Aimbot", CurrentTab == MenuTab::Aimbot))
		{
			CurrentTab = MenuTab::Aimbot;
		}

		if (ImGui::TabButton("Triggerbot", CurrentTab == MenuTab::Trigger))
		{
			CurrentTab = MenuTab::Trigger;
		}

		if (ImGui::TabButton("Visuals", CurrentTab == MenuTab::Visuals))
		{
			CurrentTab = MenuTab::Visuals;
		}

		if (ImGui::TabButton("HvH", CurrentTab == MenuTab::HvH))
		{
			CurrentTab = MenuTab::HvH;
		}

		if (ImGui::TabButton("Misc", CurrentTab == MenuTab::Misc))
		{
			CurrentTab = MenuTab::Misc;
		}

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
			if (ImGui::TabButton("Players", CurrentVisualsTab == VisualsTab::Players))
			{
				CurrentVisualsTab = VisualsTab::Players;
			}

			if (ImGui::TabButton("Buildings", CurrentVisualsTab == VisualsTab::Buildings))
			{
				CurrentVisualsTab = VisualsTab::Buildings;
			}

			if (ImGui::TabButton("World", CurrentVisualsTab == VisualsTab::World))
			{
				CurrentVisualsTab = VisualsTab::World;
			}

			if (ImGui::TabButton("Fonts", CurrentVisualsTab == VisualsTab::Font))
			{
				CurrentVisualsTab = VisualsTab::Font;
			}

			if (ImGui::TabButton("Misc", CurrentVisualsTab == VisualsTab::Misc))
			{
				CurrentVisualsTab = VisualsTab::Misc;
			}

			if (ImGui::TabButton("Radar", CurrentVisualsTab == VisualsTab::Radar))
			{
				CurrentVisualsTab = VisualsTab::Radar;
			}

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
			ColorPickerL("Target", Colors::Target);
			InputKeybind("Aimbot key", Vars::Aimbot::Global::AimKey); HelpMarker("The key to enable aimbot");
			ColorPickerL("Aimbot FOV circle", Colors::FOVCircle);
			WToggle("Autoshoot###AimbotAutoshoot", &Vars::Aimbot::Global::AutoShoot.Value); HelpMarker("Automatically shoot when a target is found");
			{
				static std::vector targetNames{ "Players", "Sentries", "Dispensers", "Teleporters", "Stickies", "NPCs", "Bombs" };
				static std::vector targetValues{ 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6 };
				MultiFlags(targetNames, targetValues, &Vars::Aimbot::Global::AimAt.Value, "Aimbot targets###AimbotTargets");
				HelpMarker("Choose which targets the Aimbot should aim at");

				static std::vector ignoreNames{ "Invulnerable", "Cloaked", "Dead Ringer", "Friends", "Taunting", "Vaccinator", "Unsimulated Players", "Disguised" };
				static std::vector ignoreValues{ 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7 };
				MultiFlags(ignoreNames, ignoreValues, &Vars::Aimbot::Global::IgnoreOptions.Value, "Ignored targets###AimbotIgnoredTargets");
				HelpMarker("Choose which targets should be ignored");
				if (Vars::Aimbot::Global::IgnoreOptions.Value & (1 << 1))
				{
					WSlider("Cloak Percentage Threshold", &Vars::Aimbot::Global::IgnoreCloakPercentage.Value, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
					HelpMarker("If the player's cloak percentage is greater than the set threshold, it will ignore them. (Set this to 0 to always ignore cloaked spies)");
				}
				WSlider("Max Targets###AimbotMax", &Vars::Aimbot::Global::MaxTargets.Value, 1, 6, "%d", ImGuiSliderFlags_AlwaysClamp);
				// remake hitscan aimbot, curb records rather than checking them (and support last & onhit)
			}
			ColorPickerL("Invulnerable colour", Colors::Invuln);

			if (Vars::Aimbot::Global::IgnoreOptions.Value & (1 << 6))
			{
				WSlider("Tick Tolerance###AimbotUnsimulatedTolerance", &Vars::Aimbot::Global::TickTolerance.Value, 0, 21, "%d", ImGuiSliderFlags_AlwaysClamp);
			}

			SectionTitle("Crits");
			WToggle("Crit hack", &Vars::CritHack::Active.Value);  HelpMarker("Enables the crit hack (BETA)");
			MultiCombo({ "Indicators", "Avoid Random", "Always Melee" }, { &Vars::CritHack::Indicators.Value, &Vars::CritHack::AvoidRandom.Value, &Vars::CritHack::AlwaysMelee.Value }, "Misc###CrithackMiscOptions");
			HelpMarker("Misc options for crithack");
			InputKeybind("Crit key", Vars::CritHack::CritKey); HelpMarker("Will try to force crits when the key is held");

			SectionTitle("Backtrack");
			WToggle("Active", &Vars::Backtrack::Enabled.Value); HelpMarker("If you shoot at the backtrack manually it will attempt to hit it");
			WCombo("Backtrack Method###HitscanBacktrackMethod", &Vars::Backtrack::Method.Value, { "All", "Last", "Prefer OnShot" });
			{
				float flDiv = Vars::Backtrack::Latency.Value / 5;
				WSlider("Fake Latency###BTLatency", &Vars::Backtrack::Latency.Value, 0, 800, "%d", ImGuiSliderFlags_AlwaysClamp);
			}
			WSlider("Fake Interp###BTInterp", &Vars::Backtrack::Interp.Value, 0, 800, "%d", ImGuiSliderFlags_AlwaysClamp); HelpMarker("Will not change latency but only able to modify on spawn");
			WToggle("Unchoke Prediction", &Vars::Backtrack::UnchokePrediction.Value);
			WSlider("Backtrack Window###BTProtect", &Vars::Backtrack::Protect.Value, 0, 13, "%d", ImGuiSliderFlags_AlwaysClamp); HelpMarker("Reduces the backtrack window by n ticks");
			WSlider("passthrough offset", &Vars::Backtrack::PassthroughOffset.Value, -3, 3, "%d"); HelpMarker("r");
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
			WSlider("latency offset", &Vars::Aimbot::Projectile::LatOff.Value, -3.f, 3.f, "%.1f");
			WSlider("physic offset", &Vars::Aimbot::Projectile::PhyOff.Value, -3.f, 3.f, "%.1f");

			SectionTitle("Preferences");
			WToggle("Charge loose cannon", &Vars::Aimbot::Projectile::ChargeLooseCannon.Value); HelpMarker("Will charge your loose cannon in order to double donk");
			WToggle("Splash Prediction", &Vars::Aimbot::Projectile::SplashPrediction.Value); HelpMarker("Will shoot the area near the target to hit them with splash damage");

			SectionTitle("Strafe Prediction");
			MultiCombo({ "Air", "Ground" }, { &Vars::Aimbot::Projectile::StrafePredictionAir.Value, &Vars::Aimbot::Projectile::StrafePredictionGround.Value }, "Strafe Prediction");
			WSlider("Minimum deviation", &Vars::Aimbot::Projectile::StrafePredictionMinDifference.Value, 0, 180); HelpMarker("How big the angle difference of the predicted strafe has to be to apply");
			WSlider("Maximum distance", &Vars::Aimbot::Projectile::StrafePredictionMaxDistance.Value, 100.f, 10000.f); HelpMarker("Max distance to apply strafe prediction (lower is better)");
		} EndChild();

		/* End */
		EndTable();
	}
}

/* Tab: Trigger */
void CMenu::MenuTrigger()
{
	using namespace ImGui;
	if (BeginTable("TriggerTable", 3))
	{
		/* Column 1 */
		if (TableColumnChild("TriggerCol1"))
		{
			SectionTitle("Global");
			WToggle("Triggerbot", &Vars::Triggerbot::Global::Active.Value); HelpMarker("Global triggerbot master switch");
			InputKeybind("Trigger key", Vars::Triggerbot::Global::TriggerKey); HelpMarker("The key which activates the triggerbot");
			HelpMarker("Choose which targets the Aimbot should aim at");
			{
				static std::vector flagNames{ "Invulnerable", "Cloaked", "Friends", "Taunting", "Unsimulated Players", "Disguised" };
				static std::vector flagValues{ 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5 };
				MultiFlags(flagNames, flagValues, &Vars::Triggerbot::Global::IgnoreOptions.Value, "Ignored targets###TriggerbotIgnoredTargets");
				HelpMarker("Choose which targets should be ignored");
			}

			SectionTitle("Autoshoot");
			WToggle("Autoshoot###AutoshootTrigger", &Vars::Triggerbot::Shoot::Active.Value); HelpMarker("Shoots if mouse is over a target");
			MultiCombo({ "Players", "Buildings" }, { &Vars::Triggerbot::Shoot::TriggerPlayers.Value, &Vars::Triggerbot::Shoot::TriggerBuildings.Value }, "Trigger targets");
			HelpMarker("Choose which target the triggerbot should shoot at");
			WToggle("Head only###TriggerHeadOnly", &Vars::Triggerbot::Shoot::HeadOnly.Value); HelpMarker("Auto shoot will only shoot if you are aiming at the head");
			WToggle("Wait for Headshot###TriggerbotWaitForHeadshot", &Vars::Triggerbot::Shoot::WaitForHeadshot.Value); HelpMarker("Auto shoot will only shoot if the sniper is charged enough to headshot");
			WToggle("Scoped Only###TriggerbotScopedOnly", &Vars::Triggerbot::Shoot::ScopeOnly.Value); HelpMarker("Auto shoot will only shoot while scoped");
			WSlider("Head scale###TriggerHeadScale", &Vars::Triggerbot::Shoot::HeadScale.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp); HelpMarker("The scale at which the auto shoot will try to shoot the targets head");
		} EndChild();

		/* Column 2 */
		if (TableColumnChild("TriggerCol2"))
		{
			SectionTitle("Autoblast");
			WToggle("Autoblast###Triggreairblast", &Vars::Triggerbot::Blast::Active.Value); HelpMarker("Auto airblast master switch");
			WToggle("Rage airblast###TriggerAirRage", &Vars::Triggerbot::Blast::Rage.Value); HelpMarker("Will airblast whenever possible, regardless of FoV");
			WToggle("Silent###triggerblastsilent", &Vars::Triggerbot::Blast::Silent.Value); HelpMarker("Aim changes made by the rage mode setting aren't visible");
			WToggle("Extinguish Players###TriggerExtinguishPlayers", &Vars::Triggerbot::Blast::ExtinguishPlayers.Value); HelpMarker("Will automatically extinguish burning players");
			WToggle("Disable on Attack###TriggerDisableOnAttack", &Vars::Triggerbot::Blast::DisableOnAttack.Value); HelpMarker("Will not air blast while attacking");
			WSlider("FOV####AirBlastFov", &Vars::Triggerbot::Blast::Fov.Value, 0.f, 90.f, "%.f", ImGuiSliderFlags_AlwaysClamp);

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
		} EndChild();

		/* Column 3 */
		if (TableColumnChild("TriggerCol3"))
		{
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
	// Visuals: Players
		case VisualsTab::Players:
		{
			if (BeginTable("VisualsPlayersTable", 3))
			{
				/* Column 1 */
				if (TableColumnChild("VisualsPlayersCol1"))
				{
					SectionTitle("ESP Main");
					WToggle("ESP###EnableESP", &Vars::ESP::Main::Active.Value); HelpMarker("Global ESP master switch");
					WToggle("Relative colours", &Vars::ESP::Main::EnableTeamEnemyColors.Value); HelpMarker("Chooses colors relative to your team (team/enemy)");
					if (Vars::ESP::Main::EnableTeamEnemyColors.Value)
					{
						ColorPickerL("Enemy color", Colors::Enemy);
						ColorPickerL("Team color", Colors::rTeam, 1);
					}
					else
					{
						ColorPickerL("RED Team color", Colors::TeamRed);
						ColorPickerL("BLU Team color", Colors::TeamBlu, 1);
					}
					WToggle("Dormant sound ESP", &Vars::ESP::Main::DormantSoundESP.Value); HelpMarker("Credits: reestart");
					if (Vars::ESP::Main::DormantSoundESP.Value)
					{
						WSlider("Dormant Decay Time###GlobalDormantDecayTime", &Vars::ESP::Main::DormantTime.Value, 0.015f, 5.0f, "%.1f", ImGuiSliderFlags_Logarithmic);
					}

					SectionTitle("Player ESP");
					WToggle("Player ESP###EnablePlayerESP", &Vars::ESP::Players::Active.Value); HelpMarker("Will draw useful information/indicators on players");
					WToggle("Name ESP###PlayerNameESP", &Vars::ESP::Players::Name.Value); HelpMarker("Will draw the players name");
					WToggle("Custom Name Color", &Vars::ESP::Players::NameCustom.Value); HelpMarker("Custom color for name esp");
					if (Vars::ESP::Players::NameCustom.Value)
					{
						ColorPickerL("Name ESP Color", Vars::ESP::Players::NameColor);
					}
					WToggle("Ignore local###SelfESP", &Vars::ESP::Players::IgnoreLocal.Value);
					ColorPickerL("Local colour", Colors::Local);
					WToggle("Ignore teammates###IgnoreTeamESPp", &Vars::ESP::Players::IgnoreTeammates.Value);
					ColorPickerL("Friend colour", Colors::Friend);
					WToggle("Ignore cloaked###IgnoreCloakESPp", &Vars::ESP::Players::IgnoreCloaked.Value);
					ColorPickerL("Cloaked colour", Colors::Cloak);
					WCombo("Ubercharge###PlayerUber", &Vars::ESP::Players::Uber.Value, { "Off", "Text", "Bar", "Both" }); HelpMarker("Will draw how much ubercharge a medic has");
					ColorPickerL("Ubercharge colour", Colors::UberColor);
					WCombo("Class###PlayerIconClass", &Vars::ESP::Players::Class.Value, { "Off", "Icon", "Text", "Both" }); HelpMarker("Will draw the class the player is");
					WToggle("Weapon text", &Vars::ESP::Players::WeaponText.Value);
					WToggle("Weapon icons", &Vars::ESP::Players::WeaponIcon.Value); HelpMarker("Shows an icon for the weapon that the player has currently equipped");
					ColorPickerL("Weapon icon/text colour", Colors::WeaponIcon);
					WToggle("Health bar###ESPPlayerHealthBar", &Vars::ESP::Players::HealthBar.Value); HelpMarker("Will draw a bar visualizing how much health the player has");
					if (Vars::ESP::Players::HealthBarStyle.Value == 0)
					{
						ColorPickerL("Health Bar Top", Colors::GradientHealthBar.startColour);
						ColorPickerL("Health Bar Bottom", Colors::GradientHealthBar.endColour, 1);
					}

					WCombo("Health bar style", &Vars::ESP::Players::HealthBarStyle.Value, { "Gradient", "Old" }); HelpMarker("How to draw the health bar");
					if (Vars::ESP::Players::HealthBarStyle.Value == 0)
					{
						ColorPickerL("Overheal Bar Top", Colors::GradientOverhealBar.startColour);
						ColorPickerL("Overheal Bar Bottom", Colors::GradientOverhealBar.endColour, 1);
					}
					if (Vars::ESP::Players::HealthBarStyle.Value == 1)
					{
						ColorPickerL("Overheal Colour", Colors::Overheal);
					}
					WCombo("Health Text###ESPPlayerHealthText", &Vars::ESP::Players::HealthText.Value, { "Off", "Default", "Bar" }); HelpMarker("Draws the player health as a text");
					WToggle("Distance", &Vars::ESP::Players::Distance.Value); HelpMarker("Shows the distance from you to the player in meters");
					WToggle("Conditions", &Vars::ESP::Players::Conditions::Enabled.Value);
					if (Vars::ESP::Players::Conditions::Enabled.Value)
					{
						MultiCombo({ "Buffs", "Debuffs", "Other Conditions", "Lag Compensation", "KD", "Ping"/*, "Dormant"*/},
								   { &Vars::ESP::Players::Conditions::Buffs.Value, &Vars::ESP::Players::Conditions::Debuffs.Value, &Vars::ESP::Players::Conditions::Other.Value, 
									 &Vars::ESP::Players::Conditions::LagComp.Value, &Vars::ESP::Players::Conditions::KD.Value, &Vars::ESP::Players::Conditions::Ping.Value/*,& Vars::ESP::Players::Conditions::Dormant.Value*/},
								     "Condition List");
						ColorPickerL("Condition colour", Colors::Cond);
					}
					WToggle("Priority Text", &Vars::ESP::Players::PriorityText.Value); HelpMarker("Will show you what priority the enemy is. (Attempts to automatically mark cheaters.)");
					WCombo("Box###PlayerBoxESP", &Vars::ESP::Players::Box.Value, { "Off", "Bounding", "Cornered", "3D" }); HelpMarker("What sort of box to draw on players");
					WCombo("Skeleton###PlayerSkellington", &Vars::ESP::Players::Bones.Value, { "Off", "Custom colour", "Health" }); HelpMarker("Will draw the bone structure of the player");
					ColorPickerL("Skellington colour", Colors::Bones);
					WSlider("ESP alpha###PlayerESPAlpha", &Vars::ESP::Players::Alpha.Value, 0.01f, 1.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
					WToggle("Sniper sightlines", &Vars::ESP::Players::SniperSightlines.Value);
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsPlayersCol2"))
				{
					SectionTitle("Chams Main");
					WToggle("Chams###ChamsMasterSwitch", &Vars::Chams::Main::Active.Value); HelpMarker("Chams master switch");

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
													  case 0:
													  {
														  return Vars::Chams::Players::Local;
													  }
													  case 1:
													  {
														  return Vars::Chams::Players::FakeAng;
													  }
													  case 2:
													  {
														  return Vars::Chams::Players::Friend;
													  }
													  case 3:
													  {
														  return Vars::Chams::Players::Enemy;
													  }
													  case 4:
													  {
														  return Vars::Chams::Players::Team;
													  }
													  case 5:
													  {
														  return Vars::Chams::Players::Target;
													  }
													  case 6:
													  {
														  return Vars::Chams::Players::Ragdoll;
													  }
													  case 7:
													  {
														  return Vars::Chams::DME::Hands;
													  }
													  case 8:
													  {
														  return Vars::Chams::DME::Weapon;
													  }
												  }

												  return Vars::Chams::Players::Local;
											  }());
					static std::vector DMEChamMaterials{ "Original", "Shaded", "Shiny", "Flat", "Wireframe shaded", "Wireframe shiny", "Wireframe flat", "Fresnel", "Brick", "Custom" };

					//WToggle("Player chams###PlayerChamsBox", &Vars::Chams::Players::Active.Value); HelpMarker("Player chams master switch");

					MultiCombo({ "Render Wearable", "Render Weapon" }, { &Vars::Chams::Players::Wearables.Value, &Vars::Chams::Players::Weapons.Value }, "Flags");
					HelpMarker("Customize Chams");
					WCombo("Config", &currentSelected, chamOptions);
					{
						ColorPickerL("Colour", currentStruct.colour);
						MultiCombo({ "Active", "Obstructed" }, { &currentStruct.chamsActive, &currentStruct.showObstructed }, "Options");

						WCombo("Material", &currentStruct.drawMaterial, DMEChamMaterials); HelpMarker("Which material the chams will apply to the player");
						if (currentStruct.drawMaterial == 7)
						{
							ColorPickerL("Fresnel base colour", currentStruct.fresnelBase, 1);
						}
						if (currentStruct.drawMaterial == 9)
						{
							MaterialCombo("Custom Material", &currentStruct.customMaterial);
						}
						WCombo("Overlay", &currentStruct.overlayType, dmeGlowMaterial);
						ColorPickerL("Glow Colour", currentStruct.overlayColour);

						if (currentSelected == 7 || currentSelected == 8)
						{
							int& proxySkinIndex = currentSelected == 8 ? Vars::Chams::DME::WeaponsProxySkin.Value : Vars::Chams::DME::HandsProxySkin.Value;
							WCombo("Proxy Material", &proxySkinIndex, DMEProxyMaterials);

						}
					}

					SectionTitle("Chams Misc");

					WToggle("DME chams###dmeactive", &Vars::Chams::DME::Active.Value); HelpMarker("DME chams master switch");

					SectionTitle("Backtrack chams");
					WToggle("Backtrack chams", &Vars::Backtrack::BtChams::Enabled.Value); HelpMarker("Draws chams to show where a player is");
					ColorPickerL("Backtrack colour", Vars::Backtrack::BtChams::BacktrackColor);
					WToggle("Only draw last tick", &Vars::Backtrack::BtChams::LastOnly.Value); HelpMarker("Only draws the last tick (can save FPS)");
					WToggle("Enemy only", &Vars::Backtrack::BtChams::EnemyOnly.Value); HelpMarker("You CAN backtrack your teammates. (Whip, medigun)");

					WCombo("Material##BtMaterial", &Vars::Backtrack::BtChams::Material.Value, DMEChamMaterials);
					if (Vars::Backtrack::BtChams::Material.Value == 9)
					{
						MaterialCombo("Custom Material##BtCustom", &Vars::Backtrack::BtChams::Custom);
					}
					WCombo("Overlay##BtOverlay", &Vars::Backtrack::BtChams::Overlay.Value, dmeGlowMaterial);
					WToggle("Ignore Z###BtIgnoreZ", &Vars::Backtrack::BtChams::IgnoreZ.Value); HelpMarker("Draws them through walls");
				} EndChild();

				/* Column 3 */
				if (TableColumnChild("VisualsPlayersCol3"))
				{
					SectionTitle("Glow Main");
					WToggle("Glow", &Vars::Glow::Main::Active.Value);
					WCombo("Glow Type###GlowTypeSelect", &Vars::Glow::Main::Type.Value, { "Blur", "Stencil" }); HelpMarker("Method in which glow will be rendered");
					WSlider("Glow scale", &Vars::Glow::Main::Scale.Value, 1, 20, "%d", ImGuiSliderFlags_AlwaysClamp);

					SectionTitle("Player Glow");
					WToggle("Player glow###PlayerGlowButton", &Vars::Glow::Players::Active.Value); HelpMarker("Player glow master switch");
					WToggle("Self glow###SelfGlow", &Vars::Glow::Players::ShowLocal.Value); HelpMarker("Draw glow on the local player");
					WToggle("Self rainbow glow###SelfGlowRainbow", &Vars::Glow::Players::LocalRainbow.Value); HelpMarker("Homosapien");
					WCombo("Ignore team###IgnoreTeamGlowp", &Vars::Glow::Players::IgnoreTeammates.Value, { "Off", "All", "Only friends" }); HelpMarker("Which teammates the glow will ignore drawing on");
					WToggle("Wearable glow###PlayerWearableGlow", &Vars::Glow::Players::Wearables.Value); HelpMarker("Will draw glow on player cosmetics");
					WToggle("Weapon glow###PlayerWeaponGlow", &Vars::Glow::Players::Weapons.Value); HelpMarker("Will draw glow on player weapons");
					WSlider("Glow alpha###PlayerGlowAlpha", &Vars::Glow::Players::Alpha.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
					WCombo("Glow colour###GlowColour", &Vars::Glow::Players::Color.Value, { "Team", "Health" }); HelpMarker("Which colour the glow will draw");
				} EndChild();

				EndTable();
			}
			break;
		}

		// Visuals: Building
		case VisualsTab::Buildings:
		{
			if (BeginTable("VisualsBuildingsTable", 3))
			{
				/* Column 1 */
				if (TableColumnChild("VisualsBuildingsCol1"))
				{
					SectionTitle("Building ESP");
					WToggle("Building ESP###BuildinGESPSwioifas", &Vars::ESP::Buildings::Active.Value); HelpMarker("Will draw useful information/indicators on buildings");
					WToggle("Ignore team buildings###BuildingESPIgnoreTeammates", &Vars::ESP::Buildings::IgnoreTeammates.Value); HelpMarker("Whether or not to draw ESP on your teams buildings");
					WToggle("Name ESP###BuildingNameESP", &Vars::ESP::Buildings::Name.Value); HelpMarker("Will draw the players name");
					WToggle("Custom Name Color", &Vars::ESP::Buildings::NameCustom.Value); HelpMarker("Custom color for name esp");
					if (Vars::ESP::Buildings::NameCustom.Value)
					{
						ColorPickerL("Name ESP Color", Vars::ESP::Buildings::NameColor);
					}
					WToggle("Health bar###Buildinghelathbar", &Vars::ESP::Buildings::HealthBar.Value); HelpMarker("Will draw a bar visualizing how much health the building has");
					WToggle("Health text###buildinghealth", &Vars::ESP::Buildings::Health.Value); HelpMarker("Will draw the building's health, as well as its max health");
					WToggle("Distance", &Vars::ESP::Buildings::Distance.Value); HelpMarker("Shows the distance from you to the building in meters");
					WToggle("Building owner###Buildingowner", &Vars::ESP::Buildings::Owner.Value); HelpMarker("Shows who built the building");
					WToggle("Building level###Buildinglevel", &Vars::ESP::Buildings::Level.Value); HelpMarker("Will draw what level the building is");
					WToggle("Building condition###Buildingconditions", &Vars::ESP::Buildings::Cond.Value); HelpMarker("Will draw what conditions the building is under");
					WCombo("Box###PBuildingBoxESP", &Vars::ESP::Buildings::Box.Value, { "Off", "Bounding", "Cornered", "3D" }); HelpMarker("What sort of box to draw on buildings");
					WSlider("ESP alpha###BuildingESPAlpha", &Vars::ESP::Buildings::Alpha.Value, 0.01f, 1.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp); HelpMarker("How transparent the ESP should be");
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsBuildingsCol2"))
				{
					SectionTitle("Building Chams");
					WToggle("Building chams###BuildingChamsBox", &Vars::Chams::Buildings::Active.Value); HelpMarker("Building chams master switch");

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
													  case 0:
													  {
														  return Vars::Chams::Buildings::Local;
													  }
													  case 1:
													  {
														  return Vars::Chams::Buildings::Friend;
													  }
													  case 2:
													  {
														  return Vars::Chams::Buildings::Enemy;
													  }
													  case 3:
													  {
														  return Vars::Chams::Buildings::Team;
													  }
													  case 4:
													  {
														  return Vars::Chams::Buildings::Target;
													  }
												  }

												  return Vars::Chams::Buildings::Local;
											  }());
					static std::vector DMEChamMaterials{ "Original", "Shaded", "Shiny", "Flat", "Wireframe shaded", "Wireframe shiny", "Wireframe flat", "Fresnel", "Brick", "Custom" };

					WCombo("Config", &currentSelected, chamOptions);
					{
						ColorPickerL("Colour", currentStruct.colour, 1);
						MultiCombo({ "Active", "Obstructed" }, { &currentStruct.chamsActive, &currentStruct.showObstructed }, "Options");

						WCombo("Material", &currentStruct.drawMaterial, DMEChamMaterials); HelpMarker("Which material the chams will apply to the player");
						if (currentStruct.drawMaterial == 7)
						{
							ColorPickerL("Fresnel base colour", currentStruct.fresnelBase, 1);
						}
						if (currentStruct.drawMaterial == 9)
						{
							MaterialCombo("Custom Material", &currentStruct.customMaterial);
						}
						WCombo("Glow Overlay", &currentStruct.overlayType, dmeGlowMaterial);
						ColorPickerL("Glow Colour", currentStruct.overlayColour, 1);
						WToggle("Rainbow Glow", &currentStruct.overlayRainbow);
						WToggle("Pulse Glow", &currentStruct.overlayPulse);
						WSlider("Glow Reduction", &currentStruct.overlayIntensity, 150.f, 0.1f, "%.1f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_ClampOnInput);
					}
				} EndChild();

				/* Column 3 */
				if (TableColumnChild("VisualsBuildingsCol3"))
				{
					SectionTitle("Building Glow");
					WToggle("Building glow###BuildiongGlowButton", &Vars::Glow::Buildings::Active.Value);
					WToggle("Ignore team buildings###buildingglowignoreteams", &Vars::Glow::Buildings::IgnoreTeammates.Value);
					WSlider("Glow alpha###BuildingGlowAlpha", &Vars::Glow::Buildings::Alpha.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
					WCombo("Glow colour###GlowColourBuildings", &Vars::Glow::Buildings::Color.Value, { "Team", "Health" });
				} EndChild();

				EndTable();
			}
			break;
		}

		// Visuals: World
		case VisualsTab::World:
		{
			if (BeginTable("VisualsWorldTable", 3))
			{
				/* Column 1 */
				if (TableColumnChild("VisualsWorldCol1"))
				{
					SectionTitle("World ESP");

					WToggle("World ESP###WorldESPActive", &Vars::ESP::World::Active.Value); HelpMarker("World ESP master switch");
					WSlider("ESP alpha###WordlESPAlpha", &Vars::ESP::World::Alpha.Value, 0.01f, 1.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp); HelpMarker("How transparent the world ESP should be");

					SectionTitle("Healthpack");
					WToggle("Name###WorldESPHealthpackName", &Vars::ESP::World::HealthName.Value); HelpMarker("Will draw ESP on healthpacks");
					WToggle("Line###WorldESPHealthpackLine", &Vars::ESP::World::HealthLine.Value); HelpMarker("Will draw a line to healthpacks");
					WToggle("Health Distance", &Vars::ESP::World::HealthDistance.Value); HelpMarker("Shows the distance from you to the health pack in meters");
					WCombo("Box###WorldESPHealthpackBox", &Vars::ESP::World::HealthBox.Value, { "Off", "Bounding", "Cornered", "3D" }); HelpMarker("What sort of box to draw on healthpacks");
					ColorPickerL("Healthpack colour", Colors::Health); HelpMarker("Color for healthpack ESP");

					SectionTitle("Ammopack");
					WToggle("Name###WorldESPAmmopackName", &Vars::ESP::World::AmmoName.Value); HelpMarker("Will draw ESP on ammopacks");
					WToggle("Line###WorldESPAmmopackLine", &Vars::ESP::World::AmmoLine.Value); HelpMarker("Will draw a line to ammopacks");
					WToggle("Ammo Distance", &Vars::ESP::World::AmmoDistance.Value); HelpMarker("Shows the distance from you to the ammo box in meters");
					WCombo("Box###WorldESPAmmopackBox", &Vars::ESP::World::AmmoBox.Value, { "Off", "Bounding", "Cornered", "3D" }); HelpMarker("What sort of box to draw on ammopacks");
					ColorPickerL("Ammopack colour", Colors::Ammo); HelpMarker("Color for ammopack ESP");

					SectionTitle("NPC");
					WToggle("Name###WorldESPNPCName", &Vars::ESP::World::NPCName.Value); HelpMarker("Will draw ESP on NPCs");
					WToggle("Line###WorldESPNPCLine", &Vars::ESP::World::NPCLine.Value); HelpMarker("Will draw a line to NPCs");
					WToggle("NPC Distance", &Vars::ESP::World::NPCDistance.Value); HelpMarker("Shows the distance from you to the NPC in meters");
					WCombo("Box###WorldESPNPCBox", &Vars::ESP::World::NPCBox.Value, { "Off", "Bounding", "Cornered", "3D" }); HelpMarker("What sort of box to draw on NPCs");
					ColorPickerL("NPC colour", Colors::NPC); HelpMarker("Color for NPC ESP");

					SectionTitle("Bombs");
					WToggle("Name###WorldESPBombName", &Vars::ESP::World::BombName.Value); HelpMarker("Will draw ESP on Bombs");
					WToggle("Line###WorldESPBombLine", &Vars::ESP::World::BombLine.Value); HelpMarker("Will draw a line to Bombs");
					WToggle("Bomb Distance", &Vars::ESP::World::BombDistance.Value); HelpMarker("Shows the distance from you to the bomb in meters");
					WCombo("Box###WorldESPBombBox", &Vars::ESP::World::BombBox.Value, { "Off", "Bounding", "Cornered", "3D" }); HelpMarker("What sort of box to draw on Bombs");
					ColorPickerL("Bomb Colour", Colors::Bomb); HelpMarker("Color for bomb ESP");

				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsWorldCol2"))
				{
					SectionTitle("World Chams");
					WToggle("World chams###woldchamsbut", &Vars::Chams::World::Active.Value);				

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
													  case 0:
													  {
														  return Vars::Chams::World::Health;
													  }
													  case 1:
													  {
														  return Vars::Chams::World::Ammo;
													  }
													  case 2:
													  {
														  return Vars::Chams::World::Projectiles::Team;
													  }
													  case 3:
													  {
														  return Vars::Chams::World::Projectiles::Enemy;
													  }
												  }

												  return Vars::Chams::World::Health;
											  }());
					static std::vector DMEChamMaterials{ "Original", "Shaded", "Shiny", "Flat", "Wireframe shaded", "Wireframe shiny", "Wireframe flat", "Fresnel", "Brick", "Custom" };

					WCombo("Config", &currentSelected, chamOptions);
					{
						ColorPickerL("Colour", currentStruct.colour, 1);
						MultiCombo({ "Active", "Obstructed" }, { &currentStruct.chamsActive, &currentStruct.showObstructed }, "Options");

						WCombo("Material", &currentStruct.drawMaterial, DMEChamMaterials); HelpMarker("Which material the chams will apply to the player");
						if (currentStruct.drawMaterial == 7)
						{
							ColorPickerL("Fresnel base colour", currentStruct.fresnelBase, 1);
						}
						if (currentStruct.drawMaterial == 9)
						{
							MaterialCombo("Custom Material", &currentStruct.customMaterial);
						}
						WCombo("Glow Overlay", &currentStruct.overlayType, dmeGlowMaterial);
						ColorPickerL("Glow Colour", currentStruct.overlayColour, 1);
						WToggle("Rainbow Glow", &currentStruct.overlayRainbow);
						WToggle("Pulse Glow", &currentStruct.overlayPulse);
						WSlider("Glow Reduction", &currentStruct.overlayIntensity, 150.f, 0.1f, "%.1f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_ClampOnInput);
					}
				} EndChild();

				/* Column 3 */
				if (TableColumnChild("VisualsWorldCol3"))
				{
					SectionTitle("World Glow");
					WToggle("World glow###Worldglowbutton", &Vars::Glow::World::Active.Value);
					WToggle("Healthpacks###worldhealthpackglow", &Vars::Glow::World::Health.Value);
					WToggle("Ammopacks###worldammopackglow", &Vars::Glow::World::Ammo.Value);
					WToggle("NPCs###worldnpcs", &Vars::Glow::World::NPCs.Value);
					WToggle("Bombs###worldbombglow", &Vars::Glow::World::Bombs.Value);
					WCombo("Projectile glow###teamprojectileglow", &Vars::Glow::World::Projectiles.Value, { "Off", "All", "Only enemies" });
					WSlider("Glow alpha###WorldGlowAlpha", &Vars::Glow::World::Alpha.Value, 0.f, 1.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
				} EndChild();

				EndTable();
			}
			break;
		}

		// Visuals: Font
		case VisualsTab::Font:
		{
			if (BeginTable("VisualsFontTable", 3))
			{
				static std::vector fontFlagNames{ "Italic", "Underline", "Strikeout", "Symbol", "Antialias", "Gaussian", "Rotary", "Dropshadow", "Additive", "Outline", "Custom" };
				static std::vector fontFlagValues{ 0x001, 0x002, 0x004, 0x008, 0x010, 0x020, 0x040, 0x080, 0x100, 0x200, 0x400 };

				/* Column 1 */
				if (TableColumnChild("VisualsFontCol1"))
				{
					SectionTitle("ESP Font");
					WInputText("Font name###espfontname", &Vars::Fonts::FONT_ESP::szName);
					WInputInt("Font height###espfontheight", &Vars::Fonts::FONT_ESP::nTall.Value);
					WInputInt("Font weight###espfontweight", &Vars::Fonts::FONT_ESP::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_ESP::nFlags.Value, "Font flags###FONT_ESP");

					SectionTitle("Name Font");
					WInputText("Font name###espfontnamename", &Vars::Fonts::FONT_ESP_NAME::szName);
					WInputInt("Font height###espfontnameheight", &Vars::Fonts::FONT_ESP_NAME::nTall.Value);
					WInputInt("Font weight###espfontnameweight", &Vars::Fonts::FONT_ESP_NAME::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_ESP_NAME::nFlags.Value, "Font flags###FONT_ESP_NAME");
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsFontCol2"))
				{
					SectionTitle("Condition Font");
					WInputText("Font name###espfontcondname", &Vars::Fonts::FONT_ESP_COND::szName);
					WInputInt("Font height###espfontcondheight", &Vars::Fonts::FONT_ESP_COND::nTall.Value);
					WInputInt("Font weight###espfontcondweight", &Vars::Fonts::FONT_ESP_COND::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_ESP_COND::nFlags.Value, "Font flags###FONT_ESP_COND");

					SectionTitle("Pickup Font");
					WInputText("Font name###espfontpickupsname", &Vars::Fonts::FONT_ESP_PICKUPS::szName);
					WInputInt("Font height###espfontpickupsheight", &Vars::Fonts::FONT_ESP_PICKUPS::nTall.Value);
					WInputInt("Font weight###espfontpickupsweight", &Vars::Fonts::FONT_ESP_PICKUPS::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_ESP_PICKUPS::nFlags.Value, "Font flags###FONT_ESP_PICKUPS");
				} EndChild();

				/* Column 3 */
				if (TableColumnChild("VisualsFontCol3"))
				{
					SectionTitle("Menu Font");
					WInputText("Font name###espfontnamenameneby", &Vars::Fonts::FONT_MENU::szName);
					WInputInt("Font height###espfontnameheightafsdfads", &Vars::Fonts::FONT_MENU::nTall.Value);
					WInputInt("Font weight###espfontnameweightasfdafsd", &Vars::Fonts::FONT_MENU::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_MENU::nFlags.Value, "Font flags###FONT_MENU");

					SectionTitle("Indicator Font");
					WInputText("Font name###espfontindicatorname", &Vars::Fonts::FONT_INDICATORS::szName);
					WInputInt("Font height###espfontindicatorheight", &Vars::Fonts::FONT_INDICATORS::nTall.Value);
					WInputInt("Font weight###espfontindicatorweight", &Vars::Fonts::FONT_INDICATORS::nWeight.Value);
					MultiFlags(fontFlagNames, fontFlagValues, &Vars::Fonts::FONT_INDICATORS::nFlags.Value, "Font flags###FONT_INDICATORS");

					if (Button("Apply settings###fontapply"))
					{
						const Font_t fontEsp = {
							0x0,
							Vars::Fonts::FONT_ESP::szName.c_str(),
							Vars::Fonts::FONT_ESP::nTall.Value,
							Vars::Fonts::FONT_ESP::nWeight.Value,
							Vars::Fonts::FONT_ESP::nFlags.Value
						};
						const Font_t fontEspName = {
							0x0,
							Vars::Fonts::FONT_ESP_NAME::szName.c_str(),
							Vars::Fonts::FONT_ESP_NAME::nTall.Value,
							Vars::Fonts::FONT_ESP_NAME::nWeight.Value,
							Vars::Fonts::FONT_ESP_NAME::nFlags.Value
						};
						const Font_t fontEspCond = {
							0x0,
							Vars::Fonts::FONT_ESP_COND::szName.c_str(),
							Vars::Fonts::FONT_ESP_COND::nTall.Value,
							Vars::Fonts::FONT_ESP_COND::nWeight.Value,
							Vars::Fonts::FONT_ESP_COND::nFlags.Value
						};
						const Font_t fontIndicator = {
							0x0,
							Vars::Fonts::FONT_INDICATORS::szName.c_str(),
							Vars::Fonts::FONT_INDICATORS::nTall.Value,
							Vars::Fonts::FONT_INDICATORS::nWeight.Value,
							Vars::Fonts::FONT_INDICATORS::nFlags.Value
						};
						const Font_t fontEspPickups = {
							0x0,
							Vars::Fonts::FONT_ESP_PICKUPS::szName.c_str(),
							Vars::Fonts::FONT_ESP_PICKUPS::nTall.Value,
							Vars::Fonts::FONT_ESP_PICKUPS::nWeight.Value,
							Vars::Fonts::FONT_ESP_PICKUPS::nFlags.Value
						};
						const Font_t fontMenu = {
							0x0,
							Vars::Fonts::FONT_MENU::szName.c_str(),
							Vars::Fonts::FONT_MENU::nTall.Value,
							Vars::Fonts::FONT_MENU::nWeight.Value,
							Vars::Fonts::FONT_MENU::nFlags.Value
						};

						const std::vector <Font_t> fonts = {
							fontEsp,
							fontEspName,
							fontEspCond,
							fontEspPickups,
							fontMenu,
							fontIndicator,
							{ 0x0, "Verdana", 18, 800, FONTFLAG_ANTIALIAS},
							{ 0x0, "Verdana", 12, 800, FONTFLAG_DROPSHADOW},
						};

						g_Draw.RemakeFonts(fonts);
					}
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
					SectionTitle("World & UI");
					WSlider("Field of view", &Vars::Visuals::FieldOfView.Value, 30, 150, "%d"); HelpMarker("How many degrees of field of vision you would like");
					WSlider("Zoomed field of view", &Vars::Visuals::ZoomFieldOfView.Value, 30, 150, "%d"); HelpMarker("Degrees of field while zoomed");
					MultiCombo({ "World", "Sky", "Prop Wireframe" }, { &Vars::Visuals::WorldModulation.Value, &Vars::Visuals::SkyModulation.Value, &Vars::Visuals::PropWireframe.Value }, "Modulations");
					HelpMarker("Select which types of modulation you want to enable");
					if (ColorPickerL("World modulation colour", Colors::WorldModulation) ||
						ColorPickerL("Sky modulation colour", Colors::SkyModulation, 1) ||
						ColorPickerL("Prop modulation colour", Colors::StaticPropModulation, 2))
					{
						G::ShouldUpdateMaterialCache = true;
					}
					MultiCombo({ "Scope", /*"Zoom",*/ "Disguises", "Taunts", "Interpolation", "View Punch", "MOTD", "Screen Effects", "Angle Forcing", "Ragdolls", "Screen Overlays", "DSP", "Convar Queries" }, { &Vars::Visuals::RemoveScope.Value, /*&Vars::Visuals::RemoveZoom.Value,*/ &Vars::Visuals::RemoveDisguises.Value, &Vars::Visuals::RemoveTaunts.Value, &Vars::Misc::DisableInterpolation.Value, &Vars::Visuals::RemovePunch.Value, &Vars::Visuals::RemoveMOTD.Value, &Vars::Visuals::RemoveScreenEffects.Value, &Vars::Visuals::PreventForcedAngles.Value, &Vars::Visuals::RemoveRagdolls.Value, &Vars::Visuals::RemoveScreenOverlays.Value, &Vars::Visuals::RemoveDSP.Value, &Vars::Visuals::RemoveConvarQueries.Value}, "Removals");
					HelpMarker("Select what you want to remove");
					WToggle("Scoreboard Colours", &Vars::Visuals::ScoreboardColours.Value);
					WToggle("Scoreboard Playerlist", &Vars::Misc::ScoreboardPlayerlist.Value);
					WToggle("Clean Screenshots", &Vars::Visuals::CleanScreenshots.Value);
					WToggle("Crosshair aim position", &Vars::Visuals::CrosshairAimPos.Value);
					WToggle("Viewmodel aim position", &Vars::Visuals::AimbotViewmodel.Value);
					WToggle("Viewmodel sway", &Vars::Visuals::ViewmodelSway.Value);
					if (Vars::Visuals::ViewmodelSway.Value)
					{
						WSlider("Viewmodel Sway Scale", &Vars::Visuals::ViewmodelSwayScale.Value, 0.01, 5, "%.2f");
						WSlider("Viewmodel Sway Interp", &Vars::Visuals::ViewmodelSwayInterp.Value, 0.01, 1, "%.2f"); HelpMarker("How long until the viewmodel returns to its original position (in seconds)");
					}
					{
						static std::vector flagNames{ "Text", "Console", "Chat", "Party", "Verbose" };
						static std::vector flagValues{ 1, 2, 4, 8, 32 };
						MultiFlags(flagNames, flagValues, &Vars::Misc::VotingOptions.Value, "Vote Logger###VoteLoggingOptions");
					}
					MultiCombo({ "Damage Logs (Console)", "Damage Logs (Text)", "Damage Logs (Chat)", "Class Changes (Text)", "Class Changes (Chat)" }, { &Vars::Visuals::DamageLoggerConsole.Value, &Vars::Visuals::DamageLoggerText.Value, &Vars::Visuals::DamageLoggerChat.Value, &Vars::Visuals::ChatInfoText.Value, &Vars::Visuals::ChatInfoChat.Value }, "Event Logging");
					HelpMarker("What & How should events be logged");
					ColorPickerL("GUI Notif Background", Colors::NotifBG);
					ColorPickerL("GUI Notif Outline", Colors::NotifOutline, 1);
					ColorPickerL("GUI Notif Colour", Colors::NotifText, 2);
					WSlider("GUI Notif Time", &Vars::Visuals::NotificationLifetime.Value, 0.5f, 3.f, "%.1f");
					WToggle("On Screen Local Conditions", &Vars::Visuals::DrawOnScreenConditions.Value); HelpMarker("Render your local conditions on your screen");
					WToggle("On Screen Ping", &Vars::Visuals::DrawOnScreenPing.Value); HelpMarker("Render your ping and your scoreboard ping on the screen");
					WToggle("Pickup Timers", &Vars::Visuals::PickupTimers.Value); HelpMarker("Displays the respawn time of health and ammopacks");
					WToggle("Spectator list", &Vars::Visuals::SpectatorList.Value);
					if (Vars::Visuals::SpectatorList.Value)
					{
						WToggle("Spectator avatars", &Vars::Visuals::SpectatorAvatars.Value);
					}
					WToggle("Post processing", &Vars::Visuals::DoPostProcessing.Value); HelpMarker("Toggle post processing effects");
					WToggle("No prop fade", &Vars::Visuals::NoStaticPropFade.Value); HelpMarker("Make props not fade");

					SectionTitle("Bullet");
					WToggle("Bullet tracers", &Vars::Visuals::BulletTracer.Value);
					ColorPickerL("Bullet tracer colour", Colors::BulletTracer);

					SectionTitle("Simulation");
					WToggle("Enabled", &Vars::Visuals::SimLines.Value);
					ColorPickerL("Prediction Line Color", Vars::Aimbot::Projectile::PredictionColor);
					ColorPickerL("Projectile Line Color", Vars::Aimbot::Projectile::ProjectileColor, 1);
					WToggle("Clear Lines", &Vars::Visuals::ClearLines.Value);
					WToggle("Seperators", &Vars::Visuals::SimSeperators.Value);
					if (Vars::Visuals::SimSeperators.Value)
					{
						WSlider("Seperator Length", &Vars::Visuals::SeperatorLength.Value, 2, 16, "%d", ImGuiSliderFlags_Logarithmic);
						WSlider("Seperator Spacing", &Vars::Visuals::SeperatorSpacing.Value, 1, 64, "%d", ImGuiSliderFlags_Logarithmic);
					}
					WToggle("Swing Prediction Lines", &Vars::Visuals::SwingLines.Value);
					WToggle("Projectile Trajectory", &Vars::Visuals::ProjectileTrajectory.Value);

					SectionTitle("Hitbox");
					WToggle("Draw Hitboxes", &Vars::Aimbot::Global::ShowHitboxes.Value); HelpMarker("Shows targeted hitbox");
					ColorPickerL("Hitbox matrix face colour", Colors::HitboxFace);
					ColorPickerL("Hitbox matrix edge colour", Colors::HitboxEdge, 1);

					SectionTitle("Particles");
					{
						WCombo("Particle Colors", &Vars::Visuals::Particles::Colors::Type.Value, { "Disabled", "Color Picker", "Rainbow" });
						if (Vars::Visuals::Particles::Colors::Type.Value == 1)
						{
							ColorPickerL("Particle Color", Colors::ParticleColor);
						}
						if (Vars::Visuals::Particles::Colors::Type.Value == 2)
						{
							WSlider("Rainbow Speed", &Vars::Visuals::Particles::Colors::RainbowSpeed.Value, 0, 5, "%.2f");
						}
						WCombo("Particle tracer", &Vars::Visuals::Particles::Tracers::ParticleTracer.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Big Nasty", "Distortion Trail", "Black Ink", "Custom" });
						if (Vars::Visuals::Particles::Tracers::ParticleTracer.Value == 8)
						{
							WInputText("Custom Tracer", &Vars::Visuals::Particles::Tracers::ParticleName);
						}
						WCombo("Particle tracer (Crits)", &Vars::Visuals::Particles::Tracers::ParticleTracerCrits.Value, { "Off", "Machina", "C.A.P.P.E.R", "Short Circuit", "Merasmus ZAP", "Big Nasty", "Distortion Trail", "Black Ink", "Custom" });
						if (Vars::Visuals::Particles::Tracers::ParticleTracerCrits.Value == 8)
						{
							WInputText("Custom Crit Tracer", &Vars::Visuals::Particles::Tracers::ParticleNameCrits);
						}
						WToggle("Halloween Spell Footsteps", &Vars::Visuals::Particles::Feet::Enabled.Value);
						if (Vars::Visuals::Particles::Feet::Enabled.Value)
						{
							if (!Vars::Visuals::Particles::Colors::Type.Value)
							{
								WCombo("Color Mode", &Vars::Visuals::Particles::Feet::ColorType.Value, { "Color Picker", "HHH", "Team Colored", "Rainbow" });
								if (Vars::Visuals::Particles::Feet::ColorType.Value == 0)
								{
									ColorPickerL("Footstep Color", Colors::FeetColor);
								}
							}
							else { HelpMarker("Turn off particle colors to be able to change the color of this."); }
						}
					}

					SectionTitle("Beams");
					{
						using namespace Vars::Visuals;

						WToggle("Enable beams", &Beans::Active.Value); HelpMarker("he loves beans?");
						WToggle("Rainbow beams", &Beans::Rainbow.Value);
						ColorPickerL("Beam colour", Beans::BeamColour);
						WToggle("Custom model", &Beans::UseCustomModel.Value);
						if (Beans::UseCustomModel.Value)
						{
							WInputText("Model", &Beans::Model);
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

					SectionTitle("Viewmodel Offset");
					WSlider("VM Off X", &Vars::Visuals::VMOffsets.x, -45.f, 45.f);
					WSlider("VM Off Y", &Vars::Visuals::VMOffsets.y, -45.f, 45.f);
					WSlider("VM Off Z", &Vars::Visuals::VMOffsets.z, -45.f, 45.f);
					WSlider("VM Roll", &Vars::Visuals::VMRoll.Value, -180, 180);

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
						WSlider("Ragdoll Force Forwards", &Vars::Visuals::RagdollEffects::RagdollForceForwards.Value, -10.f, 10.f, "%.2f"); HelpMarker("Multipler for the force applied to a ragdoll's forward vector, where positive is backwards and negative is forwards.");
						WSlider("Ragdoll Force Sideways", &Vars::Visuals::RagdollEffects::RagdollForceSides.Value, -10.f, 10.f, "%.2f"); HelpMarker("Multipler for the force applied to a ragdoll's side vector, where negative is right and positive is left.");
						WSlider("Ragdoll Force Up", &Vars::Visuals::RagdollEffects::RagdollForceUp.Value, -10.f, 10.f, "%.2f"); HelpMarker("Multipler for the force applied to a ragdoll's up vector, where negative is up and positive is down.");
					}
					else
					{
						WSlider("Ragdoll Force", &Vars::Visuals::RagdollEffects::RagdollForce.Value, -10.f, 10.f, "%.2f"); HelpMarker("Multipler for the force applied to a ragdoll.");
					}
				} EndChild();

				/* Column 2 */
				if (TableColumnChild("VisualsMiscCol2"))
				{
					SectionTitle("Skybox & Textures");
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
					WToggle("Skybox changer", &Vars::Visuals::SkyboxChanger.Value); HelpMarker("Will change the skybox, either to a base TF2 one or a custom one");
					WCombo("Skybox", &Vars::Skybox::SkyboxNum, skyNames);
					if (Vars::Skybox::SkyboxNum == 0)
					{
						WInputText("Custom skybox name", &Vars::Skybox::SkyboxName); HelpMarker("Name of the skybox you want to you (tf/materials/skybox)");
					}
					WToggle("World Textures Override", &Vars::Visuals::OverrideWorldTextures.Value); HelpMarker("Turn this off when in-game so you don't drop fps :p");
					WToggle("Bypass sv_pure", &Vars::Misc::BypassPure.Value); HelpMarker("Allows you to load any custom files, even if disallowed by the sv_pure setting");

					SectionTitle("Thirdperson");
					WToggle("Thirdperson", &Vars::Visuals::ThirdPerson.Value); HelpMarker("Will move your camera to be in a thirdperson view");
					InputKeybind("Thirdperson key", Vars::Visuals::ThirdPersonKey); HelpMarker("What key to toggle thirdperson, press ESC if no bind is desired");

					WSlider("Thirdperson distance", &Vars::Visuals::ThirdpersonDist.Value, -500.f, 500.f, "%.1f", ImGuiSliderFlags_None);
					WSlider("Thirdperson right", &Vars::Visuals::ThirdpersonRight.Value, -500.f, 500.f, "%.1f", ImGuiSliderFlags_None);
					WSlider("Thirdperson up", &Vars::Visuals::ThirdpersonUp.Value, -500.f, 500.f, "%.1f", ImGuiSliderFlags_None);
					WToggle("Thirdperson crosshair", &Vars::Visuals::ThirdpersonCrosshair.Value);

					SectionTitle("Out of FOV arrows");
					WToggle("Active###fovar", &Vars::Visuals::OutOfFOVArrows.Value); HelpMarker("Will draw arrows to players who are outside of the range of your FoV");
					WSlider("Distance from center", &Vars::Visuals::FovArrowsDist.Value, 0, 1000, "%d"); HelpMarker("How far from the center of the screen the arrows will draw");
					WSlider("Max distance", &Vars::Visuals::MaxDist.Value, 0.f, 5000.f, "%.0f"); HelpMarker("How far until the arrows will not show");
				} EndChild();

				EndTable();
			}
			break;
		}

		case VisualsTab::Radar:
		{
			if (BeginTable("VisualsRadarTable", 3))
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
				} EndChild();

				/* Column 3 */
				if (TableColumnChild("VisualsRadarCol3"))
				{
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
	}
}

/* Tab: HvH */
void CMenu::MenuHvH()
{
	using namespace ImGui;
	if (BeginTable("HvHTable", 3))
	{
		/* Column 1 */
		if (TableColumnChild("HvHCol1"))
		{
			/* Section: Tickbase Exploits */
			SectionTitle("Tickbase Exploits");
			WToggle("Enable Tickbase Exploits", &Vars::Misc::CL_Move::Enabled.Value); HelpMarker("Allows tickbase shifting");
			WToggle("Indicator", &Vars::Misc::CL_Move::Indicator.Value);
			ColorPickerL("DT bar outline colour", Colors::DtOutline);
			InputKeybind("Recharge key", Vars::Misc::CL_Move::RechargeKey); HelpMarker("Recharges ticks for shifting");
			InputKeybind("Teleport key", Vars::Misc::CL_Move::TeleportKey); HelpMarker("Shifts ticks to warp");
			if (Vars::Misc::CL_Move::DTMode.Value == 0 || Vars::Misc::CL_Move::DTMode.Value == 2)
			{
				InputKeybind("Doubletap key", Vars::Misc::CL_Move::DoubletapKey); HelpMarker("Only doubletap when the key is pressed. Leave as (None) for always active.");
			}

			WCombo("Teleport Mode", &Vars::Misc::CL_Move::TeleportMode.Value, { "Plain", "Smooth" }); HelpMarker("How the teleport should be done");
			if (Vars::Misc::CL_Move::TeleportMode.Value)
			{
				WSlider("Smooth Teleport Factor", &Vars::Misc::CL_Move::TeleportFactor.Value, 2, 6, "%d");
			}
			MultiCombo({ "Recharge While Dead", "Auto Recharge", "Wait for DT", "Anti-warp", "Avoid airborne", "Retain Fakelag", "Stop Recharge Movement", "Safe Tick", "Safe Tick Airborne", "Auto Retain" }, { &Vars::Misc::CL_Move::RechargeWhileDead.Value, &Vars::Misc::CL_Move::AutoRecharge.Value, &Vars::Misc::CL_Move::WaitForDT.Value, &Vars::Misc::CL_Move::AntiWarp.Value, &Vars::Misc::CL_Move::NotInAir.Value, &Vars::Misc::CL_Move::RetainFakelag.Value, &Vars::Misc::CL_Move::StopMovement.Value, &Vars::Misc::CL_Move::SafeTick.Value, &Vars::Misc::CL_Move::SafeTickAirOverride.Value, &Vars::Misc::CL_Move::AutoRetain.Value }, "Options"); HelpMarker("Enable various features regarding tickbase exploits");
			WCombo("Doubletap Mode", &Vars::Misc::CL_Move::DTMode.Value, { "On key", "Always", "Disable on key", "Disabled" }); HelpMarker("How should DT behave");
			const int ticksMax = g_ConVars.sv_maxusrcmdprocessticks->GetInt();
			WSlider("Ticks to shift", &Vars::Misc::CL_Move::DTTicks.Value, 1, ticksMax ? ticksMax : 22, "%d"); HelpMarker("How many ticks to shift");
			WSlider("Passive Recharge Factor", &Vars::Misc::CL_Move::PassiveRecharge.Value, 0, 22, "%d");
			WToggle("SpeedHack", &Vars::Misc::CL_Move::SEnabled.Value); HelpMarker("Speedhack Master Switch");
			if (Vars::Misc::CL_Move::SEnabled.Value)
			{
				WSlider("SpeedHack Factor", &Vars::Misc::CL_Move::SFactor.Value, 1, 66, "%d");
			}
			HelpMarker("High values are not recommended");

			/* Section: Fakelag */
			SectionTitle("Fakelag");
			WToggle("Retain BlastJump", &Vars::Misc::CL_Move::RetainBlastJump.Value); HelpMarker("Will attempt to retain the blast jumping condition as soldier and runs independently of fakelag.");
			WToggle("Enable Fakelag", &Vars::Misc::CL_Move::Fakelag.Value);
			MultiCombo({ "While Moving", "On Key", "While Visible", "Predict Visibility", "While Unducking", "While Airborne" }, { &Vars::Misc::CL_Move::WhileMoving.Value, &Vars::Misc::CL_Move::FakelagOnKey.Value, &Vars::Misc::CL_Move::WhileVisible.Value, &Vars::Misc::CL_Move::PredictVisibility.Value, &Vars::Misc::CL_Move::WhileUnducking.Value, &Vars::Misc::CL_Move::WhileInAir.Value }, "Flags###FakeLagFlags");
			if (Vars::Misc::CL_Move::FakelagOnKey.Value)
			{
				InputKeybind("Fakelag key", Vars::Misc::CL_Move::FakelagKey); HelpMarker("The key to activate fakelag as long as it's held");
			}
			WCombo("Fakelag Mode###FLmode", &Vars::Misc::CL_Move::FakelagMode.Value, { "Plain", "Random", "Adaptive" }); HelpMarker("Controls how fakelag will be controlled.");

			switch (Vars::Misc::CL_Move::FakelagMode.Value)
			{
				case 0: { WSlider("Fakelag value", &Vars::Misc::CL_Move::FakelagValue.Value, 1, ticksMax - 3, "%d"); HelpMarker("How much lag you should fake(?)"); break; }
				case 1:
				{
					WSlider("Random max###flRandMax", &Vars::Misc::CL_Move::FakelagMax.Value, Vars::Misc::CL_Move::FakelagMin.Value + 1, ticksMax, "%d"); HelpMarker("Maximum random fakelag value");
					WSlider("Random min###flRandMin", &Vars::Misc::CL_Move::FakelagMin.Value, 1, Vars::Misc::CL_Move::FakelagMax.Value - 1, "%d"); HelpMarker("Minimum random fakelag value");
					break;
				}
			}	//	add more here if you add your own fakelag modes :D
			WToggle("Unchoke On Attack", &Vars::Misc::CL_Move::UnchokeOnAttack.Value); HelpMarker("Will exit a fakelag cycle if you are attacking.");


		} EndChild();

		if (TableColumnChild("HvHCol2"))
		{
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
					WSlider("Analytical High Score Mult", &Vars::Misc::CheaterDetection::ScoreMultiplier.Value, 1.5f, 4.f, "%.2f"); HelpMarker("How much to multiply the average score to treat as a max score per second.");
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
			WToggle("Enable Resolver", &Vars::AntiHack::Resolver::Resolver.Value); HelpMarker("Enables the anti-aim resolver.");
			if (Vars::AntiHack::Resolver::Resolver.Value)
			{
				WToggle("Ignore in-air", &Vars::AntiHack::Resolver::IgnoreAirborne.Value); HelpMarker("Doesn't resolve players who are in the air.");
			}
		} EndChild();

		/* Column 3 */
		if (TableColumnChild("HvHCol3"))
		{
			/* Section: Anti Aim */
			SectionTitle("Anti Aim");
			WToggle("Enable Anti-aim", &Vars::AntiHack::AntiAim::Active.Value);
			InputKeybind("Anti-aim Key", Vars::AntiHack::AntiAim::ToggleKey); HelpMarker("The key to toggle anti aim");
			WCombo("Pitch", &Vars::AntiHack::AntiAim::Pitch.Value, { "None", "Zero", "Up", "Down", "Fake up", "Fake down", "Random", "Half Up", "Jitter", "Fake Up Custom", "Fake Down Custom" }); HelpMarker("Which way to look up/down");
			WCombo("Base Yaw", &Vars::AntiHack::AntiAim::BaseYawMode.Value, { "Offset", "FOV Player", "FOV Player + Offset" });
			WCombo("Real yaw", &Vars::AntiHack::AntiAim::YawReal.Value, { "None", "Forward", "Left", "Right", "Backwards", "Random", "Spin", "Edge", "On Hurt", "Custom", "Invert", "Jitter", "Jitter Random", "Jitter Flip", "Manual" }); HelpMarker("Which way to look horizontally");
			WCombo("Fake yaw", &Vars::AntiHack::AntiAim::YawFake.Value, { "None", "Forward", "Left", "Right", "Backwards", "Random", "Spin", "Edge", "On Hurt", "Custom", "Invert", "Jitter", "Jitter Random", "Jitter Flip", "Manual" }); HelpMarker("Which way to appear to look horizontally");
			if (Vars::AntiHack::AntiAim::Pitch.Value == 9 || Vars::AntiHack::AntiAim::Pitch.Value == 10)
			{
				WSlider("Custom Real Pitch", &Vars::AntiHack::AntiAim::CustomRealPitch.Value, -89.f, 89.f, "%.1f", 0);
			}
			if (Vars::AntiHack::AntiAim::Pitch.Value == 6 || Vars::AntiHack::AntiAim::YawFake.Value == 6 || Vars::AntiHack::AntiAim::YawReal.Value == 6)
			{
				WSlider("Spin Speed", &Vars::AntiHack::AntiAim::SpinSpeed.Value, -30.f, 30.f, "%.1f", 0); HelpMarker("You spin me right 'round, baby, right 'round");
			}
			if (Vars::AntiHack::AntiAim::Pitch.Value == 6 || Vars::AntiHack::AntiAim::YawFake.Value == 5 || Vars::AntiHack::AntiAim::YawReal.Value == 5)
			{
				WSlider("Random Interval", &Vars::AntiHack::AntiAim::RandInterval.Value, 0, 100, "%d"); HelpMarker("How often the random Anti-Aim should update");
			}
			if (Vars::AntiHack::AntiAim::BaseYawMode.Value != 1)
			{
				WSlider("Base Yaw Offset", &Vars::AntiHack::AntiAim::BaseYawOffset.Value, -180, 180);
			}
			if (Vars::AntiHack::AntiAim::YawFake.Value == 10 || Vars::AntiHack::AntiAim::YawReal.Value == 10)
			{
				InputKeybind("Invert Key", Vars::AntiHack::AntiAim::InvertKey);
			}
			if (Vars::AntiHack::AntiAim::YawFake.Value == 14 || Vars::AntiHack::AntiAim::YawReal.Value == 14)
			{
				InputKeybind("Manual Key", Vars::AntiHack::AntiAim::ManualKey);
			}
			switch (Vars::AntiHack::AntiAim::YawFake.Value)
			{
				case 9: { WSlider("Custom fake yaw", &Vars::AntiHack::AntiAim::CustomFakeYaw.Value, -180, 180); break; }
				case 11:
				case 12:
				case 13: { WSlider("Fake Jitter Amt", &Vars::AntiHack::AntiAim::FakeJitter.Value, -180, 180); break; }
			}
			switch (Vars::AntiHack::AntiAim::YawReal.Value)
			{
				case 9: { WSlider("Custom Real yaw", &Vars::AntiHack::AntiAim::CustomRealYaw.Value, -180, 180); break; }
				case 11:
				case 12:
				case 13: { WSlider("Real Jitter Amt", &Vars::AntiHack::AntiAim::RealJitter.Value, -180, 180); break; }
			}
			MultiCombo({ "AntiOverlap", "Jitter Legs", "HidePitchOnShot", "Anti-Backstab" }, { &Vars::AntiHack::AntiAim::AntiOverlap.Value, &Vars::AntiHack::AntiAim::LegJitter.Value, &Vars::AntiHack::AntiAim::InvalidShootPitch.Value, &Vars::AntiHack::AntiAim::AntiBackstab.Value }, "Misc.");

			/* Section: Auto Peek */
			SectionTitle("Auto Peek");
			InputKeybind("Autopeek Key", Vars::Misc::CL_Move::AutoPeekKey); HelpMarker("Hold this key while peeking and use A/D to set the peek direction");
			WSlider("Max Distance", &Vars::Misc::CL_Move::AutoPeekDistance.Value, 50.f, 400.f, "%.0f"); HelpMarker("Maximum distance that auto peek can walk");
			WToggle("Free move", &Vars::Misc::CL_Move::AutoPeekFree.Value); HelpMarker("Allows you to move freely while peeking");
		} EndChild();

		EndTable();
	}
}

/* Tab: Misc */
void CMenu::MenuMisc()
{
	using namespace ImGui;
	if (BeginTable("MiscTable", 3))
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

			MultiFlags({ "Amsterdam", "Atlanta", "Mumbai", "Dubai", "Moses Lake", "Washington", "Frankfurt", "Tokyo (GNRT)", "Sao Paulo", "Hong Kong", "Virginia", "Johannesburg", "Los Angeles", "London", "Lima", "Luxembourg", "Chennai", "Madrid", "Manilla", "Oklahoma City", "Chicago", "Paris", "Santiago", "Seattle", "Singapore", "Stockholm", "Sydney", "Tokyo", "Vienna", "Warsaw" },
				{ DC_AMS,      DC_ATL,    DC_BOM,   DC_DXB,  DC_EAT,		 DC_MWH,	   DC_FRA,		DC_GNRT,		DC_GRU,		 DC_HKG,	  DC_IAD,	  DC_JNB,		  DC_LAX,		 DC_LHR,   DC_LIM, DC_LUX,		 DC_MAA,	DC_MAD,	  DC_MAN,    DC_OKC,		  DC_ORD,	 DC_PAR,  DC_SCL,     DC_SEA,	 DC_SGP,	  DC_STO,	   DC_SYD,   DC_TYO,  DC_VIE,	DC_WAW },
				&Vars::Misc::RegionsAllowed.Value,
				"Regions"
			);
			WCombo("Match accept notification", &Vars::Misc::InstantAccept.Value, { "Default", "Instant join", "Freeze timer" }); HelpMarker("Will skip the 10 second delay before joining a match or let you never join");
			WCombo("Auto casual queue", &Vars::Misc::AutoCasualQueue.Value, { "Off", "In menu", "Always" }); HelpMarker("Automatically starts queueuing for casual");

			SectionTitle("Exploits");
			WToggle("Pure bypass", &Vars::Misc::CheatsBypass.Value); HelpMarker("Allows you to use some sv_cheats commands(clientside)");
			WToggle("Ping reducer", &Vars::Misc::PingReducer.Value); HelpMarker("Reduces your ping on the scoreboard");
			if (Vars::Misc::PingReducer.Value)
			{
				WSlider("cl_cmdrate", &Vars::Misc::PingTarget.Value, 1, 66); HelpMarker("Lower values for lower leaderboard ping");
			}
			WToggle("Equip region unlock", &Vars::Visuals::EquipRegionUnlock.Value); HelpMarker("This doesn't let you add the equip regions back once you turn it on."); // why is this in visuals

			SectionTitle("Convar spoofer");
			WInputText("Convar", &Vars::Misc::ConvarName);
			WInputText("Value", &Vars::Misc::ConvarValue);
			if (Button("Send", ImVec2(GetWindowSize().x - 2 * GetStyle().WindowPadding.x, 20)))
			{
				CNetChannel* netChannel = I::EngineClient->GetNetChannelInfo();
				if (netChannel == nullptr) { return; }

				Utils::ConLog("Convar", tfm::format("Set %s to %s", Vars::Misc::ConvarName, Vars::Misc::ConvarValue).c_str(), { 255, 0, 255, 255 });
				NET_SetConVar cmd(Vars::Misc::ConvarName.c_str(), Vars::Misc::ConvarValue.c_str());
				netChannel->SendNetMsg(cmd);

				//Vars::Misc::ConvarName = "";
				//Vars::Misc::ConvarValue = "";
			}
		} EndChild();

		/* Column 3 */
		if (TableColumnChild("MiscCol3"))
		{
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

			SectionTitle("Utilities");
			const auto btnWidth = GetWindowSize().x - 2 * GetStyle().WindowPadding.x;
			if (Button("Full update", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("cl_fullupdate");
			if (Button("Reload HUD", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("hud_reloadscheme");
			if (Button("Restart sound", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("snd_restart");
			if (Button("Stop sound", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("stopsound");
			if (Button("Status", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("status");
			if (Button("Ping", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("ping");
			if (Button("Retry", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("retry");
			if (Button("Exit", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("exit");
			if (Button("Console", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("showconsole");
			if (Button("Demo playback", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("demoui");
			if (Button("Demo trackbar", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("demoui2");
			if (Button("Itemtest", ImVec2(btnWidth, 20)))
				I::EngineClient->ClientCmd_Unrestricted("itemtest");
			if (Button("Fix Chams", ImVec2(btnWidth, 20)))
			{
				F::DMEChams.CreateMaterials();
				F::Glow.CreateMaterials();
			}
			if (Button("Reveal bullet lines", ImVec2(btnWidth, 20)))
			{
				F::Visuals.RevealBulletLines();
			}
			if (Button("Reveal prediction lines", ImVec2(btnWidth, 20)))
			{
				F::Visuals.RevealSimLines();
			}
			if (Button("Reveal boxes", ImVec2(btnWidth, 20)))
			{
				F::Visuals.RevealBoxes();
			}
#ifdef DEBUG
			if (Button("Dump Classes", ImVec2(btnWidth, 20)))
			{
				F::Misc.DumpClassIDS();
			}
#endif
			//if (Button("CPrint", ImVec2(btnWidth, 20))){
			//	I::CenterPrint->Print((char*)"niggaz");
			//}


			if (!I::EngineClient->IsConnected())
			{
				if (Button("Unlock all achievements", ImVec2(btnWidth, 20)))
				{
					F::Misc.UnlockAchievements();
				}
			}
		} EndChild();

		EndTable();
	}
}
#pragma endregion

/* Settings Window */
void CMenu::SettingsWindow()
{
	using namespace ImGui;
	if (!ShowSettings) { return; }

	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(10, 10));

	if (Begin("Settings", &ShowSettings, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse))
	{
		if (ColorPicker("Menu accent", Vars::Menu::MenuAccent)) { LoadStyle(); } SameLine(); Text("Menu accent");
		if (Checkbox("Alternative Design", &Vars::Menu::ModernDesign)) { LoadStyle(); }

		WInputText("Cheat Name", &Vars::Menu::CheatName);
		WInputText("Chat Info Prefix", &Vars::Menu::CheatPrefix);

		SetNextItemWidth(100);
		InputKeybind("Extra Menu key", Vars::Menu::MenuKey, true, true);

		Dummy({ 0, 5 });

		if (Button("Open configs folder", ImVec2(200, 0)))
		{
			ShellExecuteA(NULL, NULL, g_CFG.GetConfigPath().c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		if (Button("Open visuals folder", ImVec2(200, 0)))
		{
			ShellExecuteA(NULL, NULL, g_CFG.GetVisualsPath().c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		Dummy({ 0, 5 });
		ImGui::PushFont(SectionFont);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });
		if (ImGui::BeginTable("ConfigTable", 2))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, TextLight.Value);
			if (ImGui::TabButton("General", CurrentConfigTab == ConfigTab::General))
			{
				CurrentConfigTab = ConfigTab::General;
			}

			if (ImGui::TabButton("Visuals", CurrentConfigTab == ConfigTab::Visuals))
			{
				CurrentConfigTab = ConfigTab::Visuals;
			}

			ImGui::PopStyleColor(1);
			ImGui::EndTable();
		}
		ImGui::PopStyleVar(2);
		ImGui::PopFont();

		static std::string selected;
		static std::string selectedvis;
		int nConfig = 0;

		if (CurrentConfigTab == ConfigTab::General)
		{
			for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetConfigPath()))
			{
				if (std::string(std::filesystem::path(entry).filename().string()).find(g_CFG.ConfigExtension) == std::string_view::npos)
				{
					continue;
				}
				nConfig++;
			}

			// Current config
			const std::string cfgText = "Loaded: " + g_CFG.GetCurrentConfig();
			Text(cfgText.c_str());

			// Config name field
			if (nConfig < 100)
			{
				std::string newConfigName = {};

				PushItemWidth(200);
				if (InputTextWithHint("###configname", "New config name", &newConfigName, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!std::filesystem::exists(g_CFG.GetConfigPath() + "\\" + newConfigName))
					{
						g_CFG.SaveConfig(newConfigName);
					}
				}
				PopItemWidth();
			}

			// Config list

			for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetConfigPath()))
			{
				if (std::string(std::filesystem::path(entry).filename().string()).find(g_CFG.ConfigExtension) == std::string_view::npos)
				{
					continue;
				}

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
					if (Button(configName.c_str(), ImVec2(200, 20)))
					{
						selected = configName;
					}
					PopStyleColor();

					// Save config button
					if (Button("Save", ImVec2(61, 20)))
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
					SameLine();
					if (Button("Load", ImVec2(61, 20)))
					{
						g_CFG.LoadConfig(selected);
						selected.clear();
						LoadStyle();
					}

					// Remove config button
					SameLine();
					if (Button("Remove", ImVec2(62, 20)))
					{
						OpenPopup("Remove config?");
					}

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
				else if (configName == g_CFG.GetCurrentConfig())
				{
					PushStyleColor(ImGuiCol_Button, GetStyle().Colors[ImGuiCol_ButtonActive]);
					std::string buttonText = "> " + configName + " <";
					if (Button(buttonText.c_str(), ImVec2(200, 20)))
					{
						selected = configName;
					}
					PopStyleColor();
				}
				else
				{
					if (Button(configName.c_str(), ImVec2(200, 20)))
					{
						selected = configName;
					}
				}
			}

		}
		else
		{
			for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetVisualsPath()))
			{
				if (std::string(std::filesystem::path(entry).filename().string()).find(g_CFG.ConfigExtension) == std::string_view::npos)
				{
					continue;
				}
				nConfig++;
			}

			// Current config
			const std::string cfgText = "Loaded: " + g_CFG.GetCurrentVisuals();
			Text(cfgText.c_str());

			// Config name field
			if (nConfig < 100)
			{
				std::string newConfigName = {};

				PushItemWidth(200);
				if (InputTextWithHint("###configname", "New config name", &newConfigName, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					if (!std::filesystem::exists(g_CFG.GetVisualsPath() + "\\" + newConfigName))
					{
						g_CFG.SaveVisual(newConfigName);
					}
				}
				PopItemWidth();
			}

			// Config list

			for (const auto& entry : std::filesystem::directory_iterator(g_CFG.GetVisualsPath()))
			{
				if (std::string(std::filesystem::path(entry).filename().string()).find(g_CFG.ConfigExtension) == std::string_view::npos)
				{
					continue;
				}

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
					if (Button(configName.c_str(), ImVec2(200, 20)))
					{
						selected = configName;
					}
					PopStyleColor();

					// Save config button
					if (Button("Save", ImVec2(61, 20)))
					{
						if (configName != g_CFG.GetCurrentVisuals())
						{
							OpenPopup("Save config?");
						}
						else
						{
							g_CFG.SaveVisual(selected);
							selected.clear();
						}
					}

					// Load config button
					SameLine();
					if (Button("Load", ImVec2(61, 20)))
					{
						g_CFG.LoadVisual(selected);
						selected.clear();
						LoadStyle();
					}

					// Remove config button
					SameLine();
					if (Button("Remove", ImVec2(62, 20)))
					{
						OpenPopup("Remove config?");
					}

					// Save config dialog
					if (BeginPopupModal("Save config?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
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
					if (BeginPopupModal("Remove config?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
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
				else if (configName == g_CFG.GetCurrentVisuals())
				{
					PushStyleColor(ImGuiCol_Button, GetStyle().Colors[ImGuiCol_ButtonActive]);
					std::string buttonText = "> " + configName + " <";
					if (Button(buttonText.c_str(), ImVec2(200, 20)))
					{
						selected = configName;
					}
					PopStyleColor();
				}
				else
				{
					if (Button(configName.c_str(), ImVec2(200, 20)))
					{
						selected = configName;
					}
				}
			}
		}

		End();
	}

	PopStyleVar(2);
}

/* Debug Menu */
void CMenu::DebugMenu()
{
	using namespace ImGui;
	if (!ShowDebugMenu) { return; }

	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
	PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(200, 200));

	if (Begin("Debug", &ShowDebugMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse))
	{
		const auto& pLocal = g_EntityCache.GetLocal();

		Checkbox("Show Debug info", &Vars::Debug::DebugInfo.Value);
		Checkbox("Allow secure servers", I::AllowSecureServers);

		bool* m_bPendingPingRefresh = reinterpret_cast<bool*>(I::TFGCClientSystem + 828);
		Checkbox("Pending Ping Refresh", m_bPendingPingRefresh);

		WToggle("Show server hitboxes###tpShowServer", &Vars::Visuals::ThirdPersonServerHitbox.Value); HelpMarker("Will show the server angles in thirdperson in localhost servers");

		// Particle tester
		if (CollapsingHeader("Particles"))
		{
			static std::string particleName = "ping_circle";

			InputText("Particle name", &particleName);
			if (Button("Dispatch") && pLocal != nullptr)
			{
				Particles::DispatchParticleEffect(particleName.c_str(), pLocal->GetAbsOrigin(), { });
			}
		}

		End();
	}

	PopStyleVar(2);
}

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
	if (!Vars::Menu::ShowKeybinds) { return; }

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
		drawOption("Double Tap", isActive(Vars::Misc::CL_Move::DTMode.Value != 3, Vars::Misc::CL_Move::DTMode.Value == 0, Vars::Misc::CL_Move::DoubletapKey.Value));
		drawOption("Anti Aim", Vars::AntiHack::AntiAim::Active.Value);
		drawOption("Fakelag", isActive(Vars::Misc::CL_Move::Fakelag.Value, Vars::Misc::CL_Move::FakelagOnKey.Value, Vars::Misc::CL_Move::FakelagKey.Value));
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

	// Toggle menu (default is 'insert' can be changed in menu)
	static KeyHelper menuKey{ &Vars::Menu::MenuKey.Value };
	if (menuKey.Pressed() || GetAsyncKeyState(VK_INSERT) & 0x1 || GetAsyncKeyState(VK_F3) & 0x1)
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
		AddDraggable("Doubletap", Vars::Misc::CL_Move::DTIndicator, Vars::Misc::CL_Move::Indicator.Value);
		AddDraggable("Crithack", Vars::CritHack::IndicatorPos, Vars::CritHack::Indicators.Value);
		AddDraggable("Spectators", Vars::Visuals::SpectatorPos, Vars::Visuals::SpectatorList.Value);
		AddDraggable("Conditions", Vars::Visuals::OnScreenConditions, Vars::Visuals::DrawOnScreenConditions.Value);
		AddDraggable("Ping", Vars::Visuals::OnScreenPing, Vars::Visuals::DrawOnScreenPing.Value);

		SettingsWindow();
		DebugMenu();
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
		ItemWidth = 120.f;

		// https://raais.github.io/ImStudio/
		Accent = ImGui::ColorToVec(Vars::Menu::MenuAccent);
		AccentDark = ImColor(Accent.Value.x * 0.8f, Accent.Value.y * 0.8f, Accent.Value.z * 0.8f, Accent.Value.w);

		auto& style = ImGui::GetStyle();
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f); // Center window title
		style.WindowMinSize = ImVec2(100, 100);
		style.WindowPadding = ImVec2(0, 0);
		style.WindowBorderSize = 1.f;
		style.ButtonTextAlign = ImVec2(0.5f, 0.4f); // Center button text
		style.FrameBorderSize = 1.f; // Old menu feeling
		style.FrameRounding = 0.f;
		style.ChildBorderSize = 1.f;
		style.ChildRounding = 0.f;
		style.GrabMinSize = 15.f;
		style.GrabRounding = 0.f;
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

		// Alternative Designs
		if (Vars::Menu::ModernDesign)
		{
			ItemWidth = 150.f;

			style.FrameBorderSize = 0.f;
			style.FrameRounding = 2.f;
			style.GrabRounding = 2.f;
		}
	}

	// Misc
	{
		//TitleGradient.ClearMarks();
		/*TitleGradient.AddMark(0.f, ImColor(0, 0, 0, 0));
		TitleGradient.AddMark(0.3f, ImColor(0, 0, 0, 0));
		TitleGradient.AddMark(0.5f, Accent);
		TitleGradient.AddMark(0.7f, ImColor(0, 0, 0, 0));
		TitleGradient.AddMark(1.f, ImColor(0, 0, 0, 0));*/
	}

	{
		MainGradient.ClearMarks();
		MainGradient.AddMark(0.f, ImColor(0, 0, 0, 0));
		MainGradient.AddMark(0.2f, ImColor(0, 0, 0, 0));
		MainGradient.AddMark(0.5f, Accent);
		MainGradient.AddMark(0.8f, ImColor(0, 0, 0, 0));
		MainGradient.AddMark(1.f, ImColor(0, 0, 0, 0));
	}

	{
		TabGradient.ClearMarks();
		TabGradient.AddMark(0.f, ImColor(0, 0, 0, 0));
		TabGradient.AddMark(0.2f, ImColor(0, 0, 0, 0));
		TabGradient.AddMark(0.5f, ImColor(255, 255, 255));
		TabGradient.AddMark(0.8f, ImColor(0, 0, 0, 0));
		TabGradient.AddMark(1.f, ImColor(0, 0, 0, 0));
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

		constexpr ImWchar fontRange[]{ 0x0020, 0x00FF,0x0400, 0x044F, 0 }; // Basic Latin, Latin Supplement and Cyrillic

		VerdanaSmall = io.Fonts->AddFontFromFileTTF(u8"C:\\Windows\\Fonts\\verdana.ttf", 12.0f, &fontConfig, fontRange);
		Verdana = io.Fonts->AddFontFromFileTTF(u8"C:\\Windows\\Fonts\\verdana.ttf", 14.0f, &fontConfig, fontRange);
		VerdanaBold = io.Fonts->AddFontFromFileTTF(u8"C:\\Windows\\Fonts\\verdanab.ttf", 14.0f, &fontConfig, fontRange);

		SectionFont = io.Fonts->AddFontFromFileTTF(u8"C:\\Windows\\Fonts\\verdana.ttf", 16.0f, &fontConfig, fontRange);
		TitleFont = io.Fonts->AddFontFromFileTTF(u8"C:\\Windows\\Fonts\\verdanab.ttf", 20.0f, &fontConfig, fontRange);

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