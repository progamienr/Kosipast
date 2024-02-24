#pragma once
#include "../SDK/SDK.h"

#include <typeinfo>

// forward declartion of ConfigVar
template <class T>
class ConfigVar;

class CVarBase
{
public:
	size_t m_iType;
	std::string m_sName;
	int m_iFlags; // TODO: Use group enum?

	// getter for ConfigVar
	template <class T>
	ConfigVar<T>* GetVar()
	{
		if (typeid(T).hash_code() != m_iType)
			return nullptr;

		return reinterpret_cast<ConfigVar<T>*>(this);
	}
};

template <class T>
class ConfigVar : public CVarBase
{
public:
	T Default; // for future use ;)
	T Value;
	ConfigVar(T value, std::string name, int iFlags = 0);
};

inline std::vector<CVarBase*> g_Vars;

template<class T>
inline ConfigVar<T>::ConfigVar(T value, std::string name, int iFlags)
{
	Default = value;
	Value = value;
	m_iType = typeid(T).hash_code();
	m_sName = name;
	g_Vars.push_back(this);
	m_iFlags = iFlags;
}

// These are dumb workarounds to make this compatible with the old system... Hope this works :)
#define NAMESPACE_BEGIN(name) \
	namespace name { \
		inline std::string GetNamespace() { return "Vars::" + std::string(#name) + "::"; } \
		inline std::string GetSubname() { return ""; } \

#define SUBNAMESPACE_BEGIN(name) \
	namespace name { \
		inline std::string GetSubname() { return std::string(#name) + "::"; } \

// u dont actually have to put the name here, or even use it, but it makes it easier to read
#define NAMESPACE_END(name) \
	}
#define SUBNAMESPACE_END(name) \
	}

#define CVar(name, value, ...) \
	inline ConfigVar<decltype(value)> name{ value, GetNamespace() + GetSubname() + std::string(#name), __VA_ARGS__ }; \

#define VISUAL 1
#define NOSAVE 2

namespace Vars
{
	NAMESPACE_BEGIN(Menu)
		CVar(CheatName, std::string("Fedoraware"), VISUAL)
		CVar(CheatPrefix, std::string("[FeD]"), VISUAL)
		CVar(MenuKey, 0)

		CVar(ShowKeybinds, false)
		CVar(KeybindsDisplay, DragBox_t())
		CVar(ShowKBChanges, false)

		CVar(Indicators, 0b00000)
		//CVar(SpectatorAvatars, false, VISUAL)

		CVar(TicksDisplay, DragBox_t())
		CVar(CritsDisplay, DragBox_t())
		CVar(SpectatorsDisplay, DragBox_t())
		CVar(PingDisplay, DragBox_t())
		CVar(ConditionsDisplay, DragBox_t())

		CVar(DPI, 1.f)

		SUBNAMESPACE_BEGIN(Theme) // possibly reduce the amount of theme vars
			CVar(Accent, Color_t(255, 101, 101, 255), VISUAL)
			CVar(AccentLight, Color_t(255, 111, 111, 255), VISUAL)
			CVar(Background, Color_t(23, 23, 23, 250), VISUAL)
			CVar(Foreground, Color_t(11, 11, 11, 250), VISUAL)
			CVar(Foremost, Color_t(23, 23, 23, 250), VISUAL)
			CVar(ForemostLight, Color_t(25, 25, 25, 250), VISUAL)
			CVar(Inactive, Color_t(150, 150, 150, 255), VISUAL)
			CVar(Active, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(Theme)
	NAMESPACE_END(Menu)

	NAMESPACE_BEGIN(Aimbot)
		SUBNAMESPACE_BEGIN(Global)
			CVar(Active, false)
			CVar(AutoShoot, false)
			CVar(AimKey, 0x0)
			CVar(AimAt, 0b0000001) // { Bombs, NPCs, Stickies, Teleporter, Dispenser, Sentry, Players }
			CVar(IgnoreOptions, 0b00000000) // { Disguised, Unsimulated, Vaccinator, Taunting, Friends, Dead Ringer, Cloaked, Invulnerable }
			CVar(IgnoreCloakPercentage, 100)
			CVar(MaxTargets, 2)
			CVar(TickTolerance, 7)
		SUBNAMESPACE_END(Global)

		SUBNAMESPACE_BEGIN(Hitscan)
			CVar(Active, false)
			CVar(AimFOV, 25.f)
			CVar(AimMethod, 2) // 0 - Normal, 1 - Smooth, 2 - Silent
			CVar(SortMethod, 0) // 0 - FOV, 1 - Distance
			CVar(Hitboxes, 0b00111) // { Legs, Arms, Body, Pelvis, Head }
			CVar(Modifiers, 0) // { Extinguish team, Bodyaim if lethal, Auto scope, Scoped only, Wait for charge, Wait for heatshot, Tapfire }
			CVar(SmoothingAmount, 25)
			CVar(PointScale, 0)
			CVar(TapFireDist, 1000.f)
		SUBNAMESPACE_END(HITSCAN)

		SUBNAMESPACE_BEGIN(Projectile)
			CVar(Active, false)
			CVar(AimFOV, 25.f)
			CVar(AimMethod, 2) // 0 - Normal, 1 - Smooth, 2 - Silent
			CVar(SortMethod, 0) // 0 - FOV, 1 - Distance
			CVar(StrafePrediction, 0b11) // { Ground, Air }
			CVar(SmoothingAmount, 25)
			CVar(StrafePredictionHitchance, 0)
			CVar(PredictionTime, 2.0f)
			CVar(AutoRelease, 0)
			CVar(SplashPrediction, false) // not currently implemented
			CVar(ChargeLooseCannon, false)
			CVar(NoSpread, false)

			CVar(iGroundSamples, 5, NOSAVE) // debug
			CVar(iAirSamples, 5, NOSAVE) // debug
			CVar(VerticalShift, 5.f, NOSAVE) // debug
			CVar(LatOff, -2.f, NOSAVE) // debug
			CVar(PhyOff, -2.f, NOSAVE) // debug
			CVar(HullInc, 0.f, NOSAVE) // debug
			CVar(DragOverride, 0.f, NOSAVE) // debug
			CVar(TimeOverride, 0.f, NOSAVE) // debug
			CVar(HuntermanMode, 4, NOSAVE) // debug
			CVar(HuntermanShift, 2.f, NOSAVE) // debug
			CVar(HuntermanLerp, 50.f, NOSAVE) // debug
		SUBNAMESPACE_END(Projectile)

		SUBNAMESPACE_BEGIN(Melee)
			CVar(Active, false)
			CVar(AimFOV, 180.f)
			CVar(AimMethod, 2) // 0 - Normal, 1 - Smooth, 2 - Silent
			CVar(SortMethod, 1) // 0 - FOV, 1 - Distance
			CVar(SmoothingAmount, 25)
			CVar(AutoBackstab, true)
			CVar(IgnoreRazorback, true)
			CVar(SwingPrediction, false)
			CVar(WhipTeam, false)

			CVar(SwingTicks, 13, NOSAVE) // debug
		SUBNAMESPACE_END(Melee)
	NAMESPACE_END(AIMBOT)
	
	NAMESPACE_BEGIN(CritHack)
		CVar(Active, false)
		CVar(AvoidRandom, false)
		CVar(AlwaysMelee, false)
		CVar(CritKey, 0x0)
	NAMESPACE_END(CritHack)

	NAMESPACE_BEGIN(Backtrack)
		CVar(Enabled, false)
		CVar(Method, 0)
		CVar(Latency, 0)
		CVar(Interp, 0)
		CVar(Window, 185)

		CVar(Offset, 0, NOSAVE) // debug
	NAMESPACE_END(Backtrack)

	NAMESPACE_BEGIN(CL_Move)
		SUBNAMESPACE_BEGIN(DoubleTap)
			CVar(Enabled, false)
			CVar(TickLimit, 21)
			CVar(WarpRate, 21)
			CVar(PassiveRecharge, 0)
			CVar(Mode, 1) // 0 - Always, 1 - Hold, 2 - Toggle
			CVar(DoubletapKey, 0x0)
			CVar(RechargeKey, 0x0)
			CVar(TeleportKey, 0x0)
			CVar(Options, 0b00101) // { Recharge While Dead, Auto Recharge, Auto retain, Avoid airborne, Anti-warp }
		SUBNAMESPACE_END(DoubleTap)

		SUBNAMESPACE_BEGIN(FakeLag)
			CVar(Enabled, false)
			CVar(Mode, 1) // 0 - Always, 1 - Hold, 2 - Toggle
			CVar(Key, 0x0)
			CVar(Type, 0) // 0 - plain, 1 - random, 2 - adaptive
			CVar(Value, 12)
			CVar(Min, 10)
			CVar(Max, 14)
			CVar(Options, 0b000) // { While Airborne, While Unducking, While Moving }
			CVar(UnchokeOnAttack, true)
			CVar(RetainBlastJump, false)
		SUBNAMESPACE_END(FakeLag)

		CVar(SpeedEnabled, false)
		CVar(SpeedFactor, 1)

		CVar(AutoPeekKey, 0x0)
	NAMESPACE_END(CL_Move)

	NAMESPACE_BEGIN(AntiHack)
		SUBNAMESPACE_BEGIN(AntiAim)
			CVar(Active, false)
			CVar(ToggleKey, 0x0)
			CVar(PitchReal, 0) // 0 - None, 1 - Zero, 2 - Up, 3 - Down, 4 - Zero
			CVar(PitchFake, 0) // 0 - None, 1 - Zero, 2 - Up, 3 - Down
			CVar(YawReal, 0) // 0 - None, 1 - Forward, 2 - Left, 3 - Right, 4 - Backwards, 5 - Spin, 6 - Edge
			CVar(YawFake, 0) // 0 - None, 1 - Forward, 2 - Left, 3 - Right, 4 - Backwards, 5 - Spin, 6 - Edge
			CVar(RealYawMode, 0) // 0 - View, 1 - Target
			CVar(FakeYawMode, 0) // 0 - View, 1 - Target
			CVar(RealYawOffset, 0)
			CVar(FakeYawOffset, 0)
			CVar(SpinSpeed, 15.f)
			CVar(AntiOverlap, false)
			CVar(InvalidShootPitch, false)
		SUBNAMESPACE_END(AntiAim)

		SUBNAMESPACE_BEGIN(Resolver)
			CVar(Resolver, false)
			CVar(AutoResolveCheaters, false)
			CVar(IgnoreAirborne, false)
		SUBNAMESPACE_END(Resolver)
	NAMESPACE_END(AntiHack)

	NAMESPACE_BEGIN(CheaterDetection)
		CVar(Enabled, false)
		CVar(Methods, 0b111111100)			// { Duck Speed, Aimbot, OOB Angles, Aim Flicking, Bunnyhopping, Packet Choking, Simtime Changes, Score, Accuracy }
		CVar(Protections, 0b111)			// { Timing Out, Lagging Client, Double Scans }
		CVar(SuspicionGate, 10)				// the amount of infractions prior to marking someone as a cheater
		CVar(PacketManipGate, 14)			// the avg choke for someone to receive and infraction for packet choking
		CVar(BHopMaxDelay, 1)				// max groundticks used when detecting a bunny hop.
		CVar(BHopDetectionsRequired, 5)		// how many times must some be seen bunny hopping to receive an infraction
		CVar(ScoreMultiplier, 2.f)			// multiply the avg score/s by this to receive the maximum amount
		CVar(MinimumFlickDistance, 20.f)	// min mouse flick size to suspect someone of angle cheats.
		CVar(MaximumNoise, 5.f)				// max mouse noise prior to a flick to mark somebody
		CVar(MinimumAimbotFoV, 3.f)			// scaled with how many ticks a player has choked up to ->
		CVar(MaxScaledAimbotFoV, 20.f)		// self explanatory
	NAMESPACE_END(CheaterDetection)

	NAMESPACE_BEGIN(Auto)
		SUBNAMESPACE_BEGIN(Global)
			CVar(Active, false)
			CVar(AutoKey, 0x0)
			CVar(IgnoreOptions, 0b00000) // { Disguised, Unsimulated, Players, Taunting, Cloaked, Invulnerable }
		SUBNAMESPACE_END(Global)

		SUBNAMESPACE_BEGIN(Uber)
			CVar(Active, false)
			CVar(OnlyFriends, false)
			CVar(PopLocal, false)
			CVar(AutoVaccinator, 0b111) // { Fire, Blast, Bullet }
			CVar(ReactClasses, 0b000000000)
			CVar(HealthLeft, 35.0f)
			CVar(ReactFOV, 25)
			CVar(VoiceCommand, false)
		SUBNAMESPACE_END(Uber)

		SUBNAMESPACE_BEGIN(Jump)
			CVar(JumpKey, 0x0)
			CVar(CTapKey, 0x0)

			CVar(TimingOffset, -1, NOSAVE) // debug, -1 for more consistency
			CVar(ApplyAbove, 2, NOSAVE) // debug
		SUBNAMESPACE_END(Jump)

		SUBNAMESPACE_BEGIN(Detonate)
			CVar(Active, false)
			CVar(DetonateTargets, 0b0000001) // { Stickies, Bombs, NPCs, Teleporter, Dispenser, Sentry, Players }
			CVar(Stickies, false)
			CVar(Flares, false)
			CVar(RadiusScale, 90)
		SUBNAMESPACE_END(Detonate)

		SUBNAMESPACE_BEGIN(Airblast)
			CVar(Active, false)
			CVar(Rage, false)
			CVar(Silent, false)
			CVar(ExtinguishPlayers, false)
			CVar(DisableOnAttack, false)
			CVar(Fov, 60)
		SUBNAMESPACE_END(Blast)
	NAMESPACE_END(Triggerbot)

	NAMESPACE_BEGIN(ESP)
		CVar(Draw, 0, VISUAL)
		CVar(Player, 0, VISUAL)
		CVar(Building, 0, VISUAL)

		CVar(ActiveAlpha, 255, VISUAL)
		CVar(DormantAlpha, 50, VISUAL)
		CVar(DormantPriority, false, VISUAL)
		CVar(DormantTime, 1.f, VISUAL)
	NAMESPACE_END(ESP)

	NAMESPACE_BEGIN(Chams)
		SUBNAMESPACE_BEGIN(Friendly)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)

			CVar(Chams, Chams_t(), VISUAL)
		SUBNAMESPACE_END(Friendly)

		SUBNAMESPACE_BEGIN(Enemy)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)

			CVar(Chams, Chams_t(), VISUAL)
		SUBNAMESPACE_END(Enemy)

		SUBNAMESPACE_BEGIN(World)
			CVar(NPCs, false, VISUAL)
			CVar(Pickups, false, VISUAL)
			CVar(Bombs, false, VISUAL)

			CVar(Chams, Chams_t(), VISUAL)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(Backtrack)
			CVar(Active, false, VISUAL)
			CVar(Draw, 0, VISUAL)

			CVar(Chams, Chams_t(), VISUAL)
		SUBNAMESPACE_END(Backtrack)

		SUBNAMESPACE_BEGIN(FakeAngle)
			CVar(Active, false, VISUAL)

			CVar(Chams, Chams_t(), VISUAL)
		SUBNAMESPACE_END(FakeAngle)

		SUBNAMESPACE_BEGIN(Viewmodel)
			CVar(Weapon, false, VISUAL)
			CVar(Hands, false, VISUAL)

			CVar(Chams, Chams_t(), VISUAL)
		SUBNAMESPACE_END(Viewmodel)
	NAMESPACE_END(Chams)

	NAMESPACE_BEGIN(Glow)
		SUBNAMESPACE_BEGIN(Friendly)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)

			CVar(Glow, Glow_t(), VISUAL)
		SUBNAMESPACE_END(Friendly)

		SUBNAMESPACE_BEGIN(Enemy)
			CVar(Players, false, VISUAL)
			CVar(Buildings, false, VISUAL)
			CVar(Ragdolls, false, VISUAL)
			CVar(Projectiles, false, VISUAL)

			CVar(Glow, Glow_t(), VISUAL)
		SUBNAMESPACE_END(Enemy)

		SUBNAMESPACE_BEGIN(World)
			CVar(NPCs, false, VISUAL)
			CVar(Pickups, false, VISUAL)
			CVar(Bombs, false, VISUAL)

			CVar(Glow, Glow_t(), VISUAL)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(Backtrack)
			CVar(Active, false, VISUAL)
			CVar(Draw, 0, VISUAL)

			CVar(Glow, Glow_t(), VISUAL)
		SUBNAMESPACE_END(Backtrack)

		SUBNAMESPACE_BEGIN(FakeAngle)
			CVar(Active, false, VISUAL)

			CVar(Glow, Glow_t(), VISUAL)
		SUBNAMESPACE_END(FakeAngle)

		SUBNAMESPACE_BEGIN(Viewmodel)
			CVar(Weapon, false, VISUAL)
			CVar(Hands, false, VISUAL)

			CVar(Glow, Glow_t(), VISUAL)
		SUBNAMESPACE_END(Viewmodel)
	NAMESPACE_END(GLOW)

	NAMESPACE_BEGIN(Visuals)
		CVar(RemoveScope, false, VISUAL)
		CVar(RemoveInterpolation, false)
		CVar(RemoveDisguises, false, VISUAL)
		CVar(RemoveScreenOverlays, false, VISUAL)
		CVar(RemoveTaunts, false, VISUAL)
		CVar(RemoveScreenEffects, false, VISUAL)
		CVar(RemovePunch, false, VISUAL)
		CVar(PreventForcedAngles, false, VISUAL)
		CVar(RemoveMOTD, false, VISUAL)
		CVar(RemoveConvarQueries, false, VISUAL)
		CVar(RemovePostProcessing, false, VISUAL)
		CVar(RemoveDSP, false, VISUAL)
			
		CVar(FieldOfView, 0, VISUAL)
		CVar(ZoomFieldOfView, 0, VISUAL)
		CVar(RevealScoreboard, false, VISUAL)
		CVar(ScoreboardPlayerlist, false)
		CVar(CleanScreenshots, true)
		CVar(ScoreboardColors, false, VISUAL)
		CVar(SniperSightlines, false, VISUAL)
		CVar(PickupTimers, false, VISUAL)

		CVar(CrosshairAimPos, false, VISUAL)
		CVar(AimbotViewmodel, false, VISUAL)
		CVar(VMOffsetX, 0, VISUAL)
		CVar(VMOffsetY, 0, VISUAL)
		CVar(VMOffsetZ, 0, VISUAL)
		CVar(VMRoll, 0, VISUAL)
		CVar(ViewmodelSway, false, VISUAL)
		CVar(ViewmodelSwayScale, 5.f, VISUAL)
		CVar(ViewmodelSwayInterp, 0.05f, VISUAL)

		SUBNAMESPACE_BEGIN(Tracers)
			CVar(ParticleTracer, std::string("Off"), VISUAL)
			CVar(ParticleTracerCrits, std::string("Off"), VISUAL)
		SUBNAMESPACE_END(Tracers)

		SUBNAMESPACE_BEGIN(Ragdolls)
			CVar(NoRagdolls, false, VISUAL)
			CVar(NoGib, false, VISUAL)
			CVar(Active, false, VISUAL)
			CVar(EnemyOnly, false, VISUAL)
			CVar(Effects, 0, VISUAL)
			CVar(Type, 0, VISUAL)
			CVar(Force, 1.f, VISUAL)
			CVar(ForceHorizontal, 1.f, VISUAL)
			CVar(ForceVertical, 1.f, VISUAL)
		SUBNAMESPACE_END(RagdollEffects)

		CVar(BulletTracer, false, VISUAL)

		CVar(SimLines, false, VISUAL)
		CVar(TimedLines, false, VISUAL)
		CVar(SimSeperators, false, VISUAL)
		CVar(SeperatorLength, 12, VISUAL)
		CVar(SeperatorSpacing, 6, VISUAL)
		CVar(ProjectileTrajectory, false, VISUAL)
		CVar(ProjectileCamera, false, VISUAL)
		CVar(ProjectileWindow, WindowBox_t())
		CVar(TrajectoryOnShot, false, VISUAL)
		CVar(SwingLines, false, VISUAL)

		CVar(PTOverwrite, false, NOSAVE) // debug
		CVar(PTOffX, 16.f, NOSAVE) // debug
		CVar(PTOffY, 8.f, NOSAVE) // debug
		CVar(PTOffZ, -6.f, NOSAVE) // debug
		CVar(PTPipes, true, NOSAVE) // debug
		CVar(PTHull, 5.f, NOSAVE) // debug
		CVar(PTSpeed, 1200.f, NOSAVE) // debug
		CVar(PTGravity, 1.f, NOSAVE) // debug
		CVar(PTNoSpin, false, NOSAVE) // debug
		CVar(PTLifeTime, 2.2f, NOSAVE) // debug
		CVar(PTUpVelocity, 200.f, NOSAVE) // debug
		CVar(PTAngVelocityX, 600.f, NOSAVE) // debug
		CVar(PTAngVelocityY, -1200.f, NOSAVE) // debug
		CVar(PTAngVelocityZ, 0.f, NOSAVE) // debug
		CVar(PTDrag, 1.f, NOSAVE) // debug
		CVar(PTDragBasisX, 0.003902f, NOSAVE) // debug
		CVar(PTDragBasisY, 0.009962f, NOSAVE) // debug
		CVar(PTDragBasisZ, 0.009962f, NOSAVE) // debug
		CVar(PTAngDragBasisX, 0.003618f, NOSAVE) // debug
		CVar(PTAngDragBasisY, 0.001514f, NOSAVE) // debug
		CVar(PTAngDragBasisZ, 0.001514f, NOSAVE) // debug
		CVar(MaxVelocity, k_flMaxVelocity, NOSAVE) // debug
		CVar(MaxAngularVelocity, k_flMaxAngularVelocity, NOSAVE) // debug

		CVar(ShowHitboxes, false, VISUAL)

		SUBNAMESPACE_BEGIN(ThirdPerson)
			CVar(Active, false, VISUAL)
			CVar(Key, 0x0, VISUAL)
			CVar(Distance, 200.f, VISUAL)
			CVar(Right, 0.f, VISUAL)
			CVar(Up, 0.f, VISUAL)
			CVar(Crosshair, false, VISUAL)
		SUBNAMESPACE_END(ThirdPerson)

		SUBNAMESPACE_BEGIN(Arrows)
			CVar(Active, false, VISUAL)
			CVar(Offset, 25, VISUAL)
			CVar(MaxDist, 1000.f, VISUAL)
		SUBNAMESPACE_END(Arrows)

		SUBNAMESPACE_BEGIN(World)
			CVar(Modulations, 0b00000, VISUAL) // { Fog, Particle, Prop, Sky, World }
			CVar(SkyboxChanger, std::string("Off"), VISUAL)
			CVar(PropWireframe, false, VISUAL)
			CVar(WorldTexture, std::string("Default"), VISUAL)
			CVar(NearPropFade, false, VISUAL)
			CVar(NoPropFade, false, VISUAL)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(Beans)
			CVar(Active, false, VISUAL)
			CVar(Rainbow, false, VISUAL)
			CVar(BeamColor, Color_t(255, 255, 255, 255), VISUAL)
			CVar(UseCustomModel, false, VISUAL)
			CVar(Model, std::string("sprites/physbeam.vmt"), VISUAL)
			CVar(Life, 2.f, VISUAL)
			CVar(Width, 2.f, VISUAL)
			CVar(EndWidth, 2.f, VISUAL)
			CVar(FadeLength, 10.f, VISUAL)
			CVar(Amplitude, 2.f, VISUAL)
			CVar(Brightness, 255.f, VISUAL)
			CVar(Speed, 0.2f, VISUAL)
			CVar(Flags, 65792, VISUAL)
			CVar(Segments, 2, VISUAL)
		SUBNAMESPACE_END(Beans)
	NAMESPACE_END(Visuals)

	NAMESPACE_BEGIN(Radar)
		SUBNAMESPACE_BEGIN(Main)
			CVar(Active, false, VISUAL)
			CVar(Window, WindowBox_t())
			CVar(Range, 1500, VISUAL)
			CVar(BackAlpha, 128, VISUAL)
			CVar(LineAlpha, 255, VISUAL)
			CVar(AlwaysDraw, true, VISUAL)
		SUBNAMESPACE_END(Main)

		SUBNAMESPACE_BEGIN(Players)
			CVar(Active, false, VISUAL)
			CVar(IconType, 1, VISUAL) // 0 - Scoreboard, 1 - Portraits, 2 - Avatars
			CVar(BackGroundType, 2, VISUAL) // 0 - Off, 1 - Rect, 2 - Texture
			CVar(Outline, false, VISUAL)
			CVar(IgnoreTeam, 2, VISUAL) // 0 - Off, 1 - All, 2 - Keep Friends
			CVar(IgnoreCloaked, 0, VISUAL) // 0 - Off, 1 - All, 2 - Enemies Only
			CVar(IconSize, 24, VISUAL)
			CVar(Health, false, VISUAL)
			CVar(Height, false, VISUAL)
		SUBNAMESPACE_END(Players)

		SUBNAMESPACE_BEGIN(Buildings)
			CVar(Active, false, VISUAL)
			CVar(IgnoreTeam, false, VISUAL)
			CVar(Outline, false, VISUAL)
			CVar(Health, false, VISUAL)
			CVar(IconSize, 18, VISUAL)
		SUBNAMESPACE_END(Buildings)

		SUBNAMESPACE_BEGIN(World)
			CVar(Active, false, VISUAL)
			CVar(Health, false, VISUAL)
			CVar(Ammo, false, VISUAL)
			CVar(IconSize, 14, VISUAL)
		SUBNAMESPACE_END(World)
	NAMESPACE_END(Radar)

	NAMESPACE_BEGIN(Misc)
		CVar(AutoStrafe, 0)
		CVar(DirectionalOnlyOnMove, true)
		CVar(DirectionalOnlyOnSpace, false)
		CVar(AutoJump, false)
		CVar(AutoJumpbug, false)
		CVar(FastStop, false)
		CVar(FastAccel, false)
		CVar(FastStrafe, false)
		CVar(NoPush, false)
		CVar(CrouchSpeed, false)

		CVar(CheatsBypass, false)
		CVar(BypassPure, false)
		CVar(PingReducer, false)
		CVar(PingTarget, 1)
		CVar(EquipRegionUnlock, false)
		CVar(ConvarName, std::string())
		CVar(ConvarValue, std::string())

		CVar(AntiBackstab, false)
		CVar(AntiAFK, false)
		CVar(AntiAutobalance, false)
		CVar(AutoAcceptItemDrops, false)
		CVar(TauntControl, false)
		CVar(KartControl, false)
		CVar(BackpackExpander, true)

		CVar(SoundBlock, 0)
		CVar(GiantWeaponSounds, false)

		CVar(InstantRespawn, false)
		CVar(InstantRevive, false)

		CVar(NetworkFix, false)
		CVar(PredictionFix, false)
		CVar(PredictionErrorJitterFix, false)
		CVar(SetupBonesOptimization, false)

		CVar(ForceRegions, 0)
		CVar(FreezeQueue, false)
		CVar(AutoCasualQueue, 0)

		CVar(ChatTags, false)

		SUBNAMESPACE_BEGIN(Steam)
			CVar(EnableRPC, false)
			CVar(MatchGroup, 0) // 0 - Special Event; 1 - MvM Mann Up; 2 - Competitive; 3 - Casual; 4 - MvM Boot Camp;
			CVar(OverrideMenu, false) // Override matchgroup when in main menu
			CVar(MapText, std::string("Fedoraware"))
			CVar(GroupSize, 1337)
		SUBNAMESPACE_END(Steam)
	NAMESPACE_END(Misc)

	NAMESPACE_BEGIN(Colors)
		CVar(FOVCircle, Color_t(255, 255, 255, 100), VISUAL)
		CVar(Relative, false, VISUAL)
		CVar(TeamRed, Color_t(225, 60, 60, 255), VISUAL)
		CVar(TeamBlu, Color_t(75, 175, 225, 255), VISUAL)
		CVar(Enemy, Color_t(225, 60, 60, 255), VISUAL)
		CVar(Team, Color_t(75, 175, 225, 255), VISUAL)
		CVar(Local, Color_t(255, 255, 255, 255), VISUAL)
		CVar(Target, Color_t(255, 0, 0, 255), VISUAL)
		CVar(Invulnerable, Color_t(125, 100, 175, 255), VISUAL)
		CVar(Cloak, Color_t(150, 175, 210, 255), VISUAL)
		CVar(Overheal, Color_t(75, 175, 255, 255), VISUAL)
		CVar(HealthBar, Gradient_t({ 255, 0, 0, 255 }, { 0, 200, 125, 255 }), VISUAL)
		CVar(UberBar, Color_t( 127, 255, 255, 255 ), VISUAL)
		CVar(Health, Color_t(0, 225, 75, 255), VISUAL)
		CVar(Ammo, Color_t(175, 175, 175, 255), VISUAL)
		CVar(NPC, Color_t(255, 255, 255, 255), VISUAL)
		CVar(Bomb, Color_t(255, 75, 0, 255), VISUAL)
		CVar(Spellbook, Color_t(100, 0, 255, 255), VISUAL)
		CVar(Gargoyle, Color_t(0, 150, 75, 255), VISUAL)

		CVar(WorldModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(SkyModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(PropModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ParticleModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(FogModulation, Color_t(255, 255, 255, 255), VISUAL)

		CVar(BulletTracer, Color_t(255, 255, 255, 255), VISUAL)
		CVar(PredictionColor, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ProjectileColor, Color_t(255, 100, 100, 255), VISUAL)
		CVar(ClippedColor, Color_t(255, 255, 255, 0), VISUAL)
		CVar(HitboxEdge, Color_t(255, 255, 255, 255), VISUAL)
		CVar(HitboxFace, Color_t(255, 255, 255, 0), VISUAL)
	NAMESPACE_END(Colors)

	NAMESPACE_BEGIN(Logging)
		CVar(Logs, 0b0011) // { Damage, Class Changes, Vote cast, Vote start, Cheat Detection, Tags }
		// LogTo // { Console, Party, Chat, Toasts }
		CVar(Lifetime, 5.f, VISUAL)

		SUBNAMESPACE_BEGIN(VoteStart)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(VoteStart)

		SUBNAMESPACE_BEGIN(VoteCast)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(VoteCast)

		SUBNAMESPACE_BEGIN(ClassChange)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(ClassChange)

		SUBNAMESPACE_BEGIN(Damage)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(Damage)

		SUBNAMESPACE_BEGIN(CheatDetection)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(CheatDetection)

		SUBNAMESPACE_BEGIN(Tags)
			CVar(LogTo, 0b0001)
		SUBNAMESPACE_END(Tags)
	NAMESPACE_END(Logging)

	NAMESPACE_BEGIN(Debug)
		CVar(Info, false, NOSAVE)
		CVar(Logging, false, NOSAVE)
		CVar(ServerHitbox, false, NOSAVE)
		CVar(AntiAimLines, false, NOSAVE)
	NAMESPACE_END(Debug)

	NAMESPACE_BEGIN(Fonts)
		SUBNAMESPACE_BEGIN(FONT_ESP)
			CVar(szName, std::string("Tahoma"), VISUAL)
			CVar(nTall, 12, VISUAL)
			CVar(nWeight, 0, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_ESP)

		SUBNAMESPACE_BEGIN(FONT_NAME)
			CVar(szName, std::string("Tahoma"), VISUAL)
			CVar(nTall, 12, VISUAL)
			CVar(nWeight, 0, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_NAME)

		SUBNAMESPACE_BEGIN(FONT_INDICATORS)
			CVar(szName, std::string("Tahoma"), VISUAL)
			CVar(nTall, 13, VISUAL)
			CVar(nWeight, -1, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_INDICATORS)
	NAMESPACE_END(Fonts)
}