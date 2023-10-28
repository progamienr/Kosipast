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
		CVar(ShowPlayerlist, false)
		CVar(ShowKeybinds, false)
		CVar(MenuKey, 0)

		SUBNAMESPACE_BEGIN(Theme)
			CVar(Accent, Color_t(255, 101, 101, 255), VISUAL)
		SUBNAMESPACE_END(Theme)
	NAMESPACE_END(Menu)
	
	NAMESPACE_BEGIN(CritHack)
		CVar(Active, false)
		CVar(Indicators, false, VISUAL)
		CVar(IndicatorPos, DragBox_t(), VISUAL)
		CVar(AvoidRandom, false)
		CVar(AlwaysMelee, false)
		CVar(CritKey, VK_SHIFT)
	NAMESPACE_END(CritHack)

	NAMESPACE_BEGIN(Backtrack)
		CVar(Enabled, false)
		CVar(Method, 0)
		CVar(Latency, 0)
		CVar(Interp, 0)
		CVar(Window, 185)
		CVar(UnchokePrediction, false) // seems iffy
		CVar(PassthroughOffset, -1, NOSAVE) // debug
		CVar(TicksetOffset, 0, NOSAVE) // debug
		CVar(ChokePassMod, 1, NOSAVE) // debug
		CVar(ChokeSetMod, 0, NOSAVE) // debug
		CVar(NWindowSub, 0, NOSAVE) // debug
		CVar(OWindowSub, 0, NOSAVE) // debug

		SUBNAMESPACE_BEGIN(BtChams)
			CVar(Enabled, false, VISUAL)
			CVar(LastOnly, false, VISUAL)
			CVar(EnemyOnly, false, VISUAL)
			CVar(IgnoreZ, false, VISUAL)
			CVar(Material, 1, VISUAL)
			CVar(Custom, std::string("None"), VISUAL)
			CVar(Overlay, 1, VISUAL)
			CVar(BacktrackColor, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(BtChams)
	NAMESPACE_END(Backtrack)

	NAMESPACE_BEGIN(Aimbot)
		SUBNAMESPACE_BEGIN(Global)
			CVar(Active, false)
			CVar(AimKey, VK_XBUTTON1)
			CVar(AutoShoot, false)
			CVar(AimAt, 0b0000001) //players, sentry, dispenser, teleporter, stickies, npcs, bombs
			CVar(IgnoreOptions, 0b0000000) //disguised, fakelagging players, vaccinator, taunting, friends, deadringer,cloaked, invul
			CVar(TickTolerance, 7)
			CVar(MaxTargets, 2)
			CVar(IgnoreCloakPercentage, 100) // if player cloak percent > ignore threshold, ignore them 
			CVar(BAimLethal, false) // This is in global cause i remmebered hunterman exists
			CVar(ShowHitboxes, false, VISUAL)
		SUBNAMESPACE_END(Global)

		SUBNAMESPACE_BEGIN(Hitscan)
			CVar(Active, false)
			CVar(AimFOV, 25.f)
			CVar(SortMethod, 0) //0 - FOV,		1 - Distance
			CVar(SmoothingAmount, 25)
			CVar(AimMethod, 2) //0 - Normal,	1 - Smooth, 2 - Silent
			CVar(Hitboxes, 0b00111) // {legs, arms, body, pelvis, head}
			CVar(PointScale, 0.f)
			CVar(TapFire, 0) //0 - Off, 1 - Distance, 2 - Always
			CVar(TapFireDist, 1000.f)
			CVar(ScanBuildings, false)
			CVar(WaitForHeadshot, false)
			CVar(WaitForCharge, false)
			CVar(ScopedOnly, false)
			CVar(AutoScope, false)
			CVar(ExtinguishTeam, false)
		SUBNAMESPACE_END(HITSCAN)

		SUBNAMESPACE_BEGIN(Projectile)
			CVar(Active, false)
			CVar(AimFOV, 25.f)
			CVar(SortMethod, 0) //0 - FOV,		1 - Distance
			CVar(AimMethod, 2) //0 - Normal,	1 - Smooth, 2 - Silent
			CVar(SmoothingAmount, 25)
			CVar(SplashPrediction, false)
			CVar(PredictionTime, 2.0f)
			CVar(NoSpread, false)
			CVar(AutoRelease, false)
			CVar(AutoReleaseAt, 0.6f)
			CVar(ChargeLooseCannon, false)
			CVar(StrafePredictionGround, false)
			CVar(StrafePredictionAir, false)
			CVar(StrafePredictionMaxDistance, 1000)
			CVar(StrafePredictionMinDifference, 10)
			CVar(StrafePredictionHitchance, 0.f)
			CVar(iSamples, 10, NOSAVE) // debug
			CVar(VerticalShift, 5.f, NOSAVE) // debug
			CVar(LatOff, 0.f, NOSAVE) // debug
			CVar(PhyOff, 0.f, NOSAVE) // debug
			CVar(HuntermanMode, 0, NOSAVE) // debug
			CVar(HuntermanShift, 2.f, NOSAVE) // debug
		SUBNAMESPACE_END(Projectile)

		SUBNAMESPACE_BEGIN(Melee)
			CVar(Active, false)
			CVar(AimFOV, 180.f)
			CVar(SortMethod, 1) //0 - FOV, 1 - Distance
			CVar(AimMethod, 2) //0 - Normal,	1 - Smooth, 2 - Silent
			CVar(SmoothingAmount, 25)
			CVar(PredictSwing, false)
			CVar(AutoBackstab, true)
			CVar(IgnoreRazorback, true)
			CVar(WhipTeam, false)
			CVar(SwingTicks, 13, NOSAVE) // debug
		SUBNAMESPACE_END(Melee)
	NAMESPACE_END(AIMBOT)

	NAMESPACE_BEGIN(Triggerbot)
		SUBNAMESPACE_BEGIN(Global)
			CVar(Active, false)
			CVar(TriggerKey, VK_XBUTTON2)
			CVar(IgnoreOptions, 0b00000) //disguised, fakelagging players, taunting, friends, cloaked, invul
		SUBNAMESPACE_END(Global)

		SUBNAMESPACE_BEGIN(Detonate)
			CVar(Active, false)
			CVar(DetonateTargets, 0b00001)
			CVar(Stickies, false)
			CVar(Flares, false)
			CVar(RadiusScale, 0.9f)
		SUBNAMESPACE_END(Detonate)

		SUBNAMESPACE_BEGIN(Blast)
			CVar(Active, false)
			CVar(Rage, false)
			CVar(Silent, false)
			CVar(ExtinguishPlayers, false)
			CVar(DisableOnAttack, false)
			CVar(Fov, 60)
		SUBNAMESPACE_END(Blast)

		SUBNAMESPACE_BEGIN(Uber)
			CVar(Active, false)
			CVar(OnlyFriends, false)
			CVar(PopLocal, false)
			CVar(AutoVacc, false)
			CVar(HealthLeft, 35.0f)
			CVar(VoiceCommand, false)
			CVar(ReactFoV, 25)
			CVar(BulletRes, true)
			CVar(BlastRes, true)
			CVar(FireRes, true)
			CVar(ReactClasses, 0b000000000)	//	this is intuitive
		SUBNAMESPACE_END(Uber)

		SUBNAMESPACE_BEGIN(Jump) // more auto than trigger idc
			CVar(JumpKey, 0x0)
			CVar(CTapKey, 0x0)
		SUBNAMESPACE_END(Jump)
	NAMESPACE_END(Triggerbot)

	NAMESPACE_BEGIN(ESP)
		SUBNAMESPACE_BEGIN(Main)
			CVar(Active, false, VISUAL)
			CVar(EnableTeamEnemyColors, false, VISUAL)
			CVar(DormantSoundESP, false, VISUAL)
			CVar(DormantTime, 1.f, VISUAL)
		SUBNAMESPACE_END(Main)

		SUBNAMESPACE_BEGIN(Players)
			CVar(Active, false, VISUAL)
			CVar(IgnoreLocal, true, VISUAL)
			CVar(IgnoreTeam, true, VISUAL)
			CVar(IgnoreFriends, false, VISUAL)
			CVar(IgnoreCloaked, false, VISUAL)
			CVar(Name, false, VISUAL)
			CVar(HealthBar, false, VISUAL)
			CVar(HealthText, false, VISUAL)
			CVar(UberBar, false, VISUAL)
			CVar(UberText, false, VISUAL)
			CVar(ClassIcon, false, VISUAL)
			CVar(ClassText, false, VISUAL)
			CVar(WeaponIcon, false, VISUAL)
			CVar(WeaponText, false, VISUAL)
			CVar(Distance, false, VISUAL)
			CVar(Box, false, VISUAL)
			CVar(Bones, false, VISUAL)
			CVar(PriorityText, false, VISUAL)
			
			SUBNAMESPACE_BEGIN(Conditions)
				CVar(Buffs, true, VISUAL)
				CVar(Debuffs, true, VISUAL)
				CVar(Other, false, VISUAL)
				CVar(LagComp, false, VISUAL)
				CVar(Ping, true, VISUAL)
				CVar(KD, false, VISUAL)
			SUBNAMESPACE_END(Conditions)

			CVar(Alpha, 1.0f, VISUAL)
		SUBNAMESPACE_END(Players)

		SUBNAMESPACE_BEGIN(Buildings)
			CVar(Active, false, VISUAL)
			CVar(IgnoreTeam, false, VISUAL)
			CVar(Name, false, VISUAL)
			CVar(HealthBar, false, VISUAL)
			CVar(HealthText, false, VISUAL)
			CVar(Distance, false, VISUAL)
			CVar(Box, false, VISUAL)
			CVar(Owner, false, VISUAL)
			CVar(Level, false, VISUAL)
			CVar(Condition, false, VISUAL)
			CVar(Alpha, 1.0f, VISUAL)
		SUBNAMESPACE_END(Buildings)

		SUBNAMESPACE_BEGIN(World)
			CVar(Active, false, VISUAL)
			CVar(Health, false, VISUAL)
			CVar(Ammo, false, VISUAL)
			CVar(NPC, false, VISUAL)
			CVar(Bomb, false, VISUAL)
			CVar(Spellbook, false, VISUAL)
			CVar(Gargoyle, false, VISUAL)
			CVar(Alpha, 1.0f, VISUAL)
		SUBNAMESPACE_END(World)
	NAMESPACE_END(ESP)

	NAMESPACE_BEGIN(Chams)
		CVar(Active, false, VISUAL)

		SUBNAMESPACE_BEGIN(Players)
			CVar(Active, false, VISUAL)
			CVar(Wearables, false, VISUAL)
			CVar(Weapons, false, VISUAL)
			CVar(EnemyOnly, true, VISUAL)

			// {ignorez, material, overlay type (0 = off), active}
			CVar(Local, Chams_t(), VISUAL)
			CVar(FakeAng, Chams_t(), VISUAL)
			CVar(Friend, Chams_t(), VISUAL)
			CVar(Enemy, Chams_t(), VISUAL)
			CVar(Team, Chams_t(), VISUAL)
			CVar(Target, Chams_t(), VISUAL)
			CVar(Ragdoll, Chams_t(), VISUAL)
		SUBNAMESPACE_END(Players)

		SUBNAMESPACE_BEGIN(Buildings)
			CVar(Local, Chams_t(), VISUAL)
			CVar(Friend, Chams_t(), VISUAL)
			CVar(Enemy, Chams_t(), VISUAL)
			CVar(Team, Chams_t(), VISUAL)
			CVar(Target, Chams_t(), VISUAL)

			CVar(Active, false, VISUAL)
			CVar(Material, 3, VISUAL) //0 - None, 1 - Shaded, 2 - Shiny, 3 - Flat
			CVar(IgnoreZ, false, VISUAL)
			CVar(EnemyOnly, true, VISUAL)
		SUBNAMESPACE_END(Buildings)

		SUBNAMESPACE_BEGIN(World)
			CVar(Active, false, VISUAL)
			CVar(Projectilez, 2, VISUAL)
			CVar(Health, Chams_t(), VISUAL)
			CVar(Ammo, Chams_t(), VISUAL)

			SUBNAMESPACE_BEGIN(Projectiles)
				CVar(Enemy, Chams_t())
				CVar(Team, Chams_t())
				CVar(Projectiles, Chams_t())
			SUBNAMESPACE_END(Projectiles)
		SUBNAMESPACE_END(World)

		SUBNAMESPACE_BEGIN(DME)
			CVar(Hands, Chams_t(1, 0, 0, true), VISUAL)
			CVar(Weapon, Chams_t(1, 0, 0, true), VISUAL)

			CVar(HandsGlowOverlay, 0, VISUAL) // 0 - Off,  1 - Fresnel Glow, 2 - Wireframe Glow
			CVar(HandsProxySkin, 0, VISUAL)
			CVar(HandsGlowAmount, 1, VISUAL)
			CVar(HandsProxyWF, false, VISUAL)
			CVar(HandsRainbow, false, VISUAL)
			CVar(HandsOverlayRainbow, false, VISUAL)
			CVar(HandsOverlayPulse, false, VISUAL)

			CVar(WeaponGlowOverlay, 0, VISUAL) // 0 - Off,  1 - Fresnel Glow, 2 - Wireframe Glow
			CVar(WeaponsProxySkin, 0, VISUAL)
			CVar(WeaponGlowAmount, 1, VISUAL)
			CVar(WeaponsProxyWF, false, VISUAL)
			CVar(WeaponRainbow, false, VISUAL)
			CVar(WeaponOverlayRainbow, false, VISUAL)
			CVar(WeaponOverlayPulse, false, VISUAL)
		SUBNAMESPACE_END(DME)
	NAMESPACE_END(Chams)

	NAMESPACE_BEGIN(Glow)
		SUBNAMESPACE_BEGIN(Main)
			CVar(Active, false, VISUAL)
			CVar(Type, 0, VISUAL)	//	blur, stencil
			CVar(Scale, 1, VISUAL)
		SUBNAMESPACE_END(Main)

		SUBNAMESPACE_BEGIN(Players)
			CVar(Active, false, VISUAL)
			CVar(ShowLocal, false, VISUAL)
			CVar(LocalRainbow, false, VISUAL)
			CVar(IgnoreTeammates, 2, VISUAL) //0 - OFF, 1 - All, 2 - Keep Friends
			CVar(Wearables, false, VISUAL)
			CVar(Weapons, false, VISUAL)
			CVar(Alpha, 1.0f, VISUAL)
			CVar(Color, 0, VISUAL) //0 - Team, 1 - Health
		SUBNAMESPACE_END(Players)

		SUBNAMESPACE_BEGIN(Buildings)
			CVar(Active, false, VISUAL)
			CVar(IgnoreTeammates, false, VISUAL)
			CVar(Alpha, 1.0f, VISUAL)
			CVar(Color, 0, VISUAL) //0 - Team, 1 - Health
		SUBNAMESPACE_END(Buildings)

		SUBNAMESPACE_BEGIN(World)
			CVar(Active, false, VISUAL)
			CVar(Health, false, VISUAL)
			CVar(Ammo, false, VISUAL)
			CVar(NPCs, false, VISUAL)
			CVar(Bombs, false, VISUAL)
			CVar(Spellbook, false, VISUAL)
			CVar(Gargoyle, false, VISUAL)
			CVar(Projectiles, 1, VISUAL) //0 - Off, 1 - All, 2 - Enemy Only
			CVar(Alpha, 1.0f, VISUAL)
		SUBNAMESPACE_END(World)
	NAMESPACE_END(GLOW)

	NAMESPACE_BEGIN(Radar)
		SUBNAMESPACE_BEGIN(Main)
			CVar(Active, false, VISUAL)
			CVar(BackAlpha, 128, VISUAL)
			CVar(LineAlpha, 255, VISUAL)
			CVar(Range, 1500, VISUAL)
			CVar(NoTitleGradient, false, VISUAL)
		SUBNAMESPACE_END(Main)

		SUBNAMESPACE_BEGIN(Players)
			CVar(Active, false, VISUAL)
			CVar(IconType, 1, VISUAL) //0 - Scoreboard, 1 - Portraits, 2 - Avatars
			CVar(BackGroundType, 2, VISUAL) //0 - Off, 1 - Rect, 2 - Texture
			CVar(Outline, false, VISUAL)
			CVar(IgnoreTeam, 2, VISUAL) //0 - Off, 1 - All, 2 - Keep Friends
			CVar(IgnoreCloaked, 0, VISUAL) //0 - Off, 1 - All, 2 - Enemies Only
			CVar(Health, false, VISUAL)
			CVar(IconSize, 24, VISUAL)
			CVar(Height, false, VISUAL)
		SUBNAMESPACE_END(Players)

		SUBNAMESPACE_BEGIN(Buildings)
			CVar(Active, false, VISUAL)
			CVar(Outline, false, VISUAL)
			CVar(IgnoreTeam, false, VISUAL)
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

	NAMESPACE_BEGIN(Visuals)
		CVar(RevealScoreboard, false, VISUAL)
		CVar(ScoreboardColors, false, VISUAL)
		CVar(CleanScreenshots, true, VISUAL)
		CVar(RemoveDisguises, false, VISUAL)
		CVar(RemoveTaunts, false, VISUAL)
		CVar(DrawOnScreenConditions, false, VISUAL)
		CVar(DrawOnScreenPing, false, VISUAL)
		CVar(OnScreenConditions, DragBox_t(), VISUAL)
		CVar(OnScreenPing, DragBox_t(), VISUAL)
		CVar(FieldOfView, 90, VISUAL)
		CVar(ZoomFieldOfView, 30, VISUAL)
		CVar(RemoveScope, false, VISUAL)
		CVar(RemoveRagdolls, false, VISUAL)
		CVar(RemoveMOTD, false, VISUAL)
		CVar(RemoveScreenEffects, false, VISUAL)
		CVar(RemoveScreenOverlays, false, VISUAL)
		CVar(RemoveConvarQueries, false, VISUAL)
		CVar(RemoveDSP, false, VISUAL)
		CVar(PreventForcedAngles, false, VISUAL)
		CVar(SniperSightlines, false, VISUAL)
		CVar(PickupTimers, false, VISUAL)
		CVar(RemoveZoom, false, VISUAL)
		CVar(RemovePunch, false, VISUAL)
		CVar(CrosshairAimPos, false, VISUAL)
		CVar(ChatInfoText, false, VISUAL)
		CVar(ChatInfoChat, false, VISUAL)
		CVar(SpectatorList, false, VISUAL)
		CVar(SpectatorAvatars, false, VISUAL)
		CVar(SpectatorPos, DragBox_t(), VISUAL);

		SUBNAMESPACE_BEGIN(ThirdPerson)
			CVar(Active, false, VISUAL)
			CVar(Key, VK_B, VISUAL)
			CVar(Distance, 200.f, VISUAL)
			CVar(Right, 0.f, VISUAL)
			CVar(Up, 0.f, VISUAL)
			CVar(Crosshair, false, VISUAL)
		SUBNAMESPACE_END(ThirdPerson)

		SUBNAMESPACE_BEGIN(World)
			CVar(WorldModulation, false, VISUAL)
			CVar(SkyModulation, false, VISUAL)
			CVar(PropModulation, false, VISUAL)
			CVar(ParticleModulation, false, VISUAL)
			CVar(PropWireframe, false, VISUAL)
			CVar(OverrideTextures, false, VISUAL)
			CVar(SkyboxChanger, false, VISUAL)
			CVar(SkyboxNum, 0, VISUAL)
			CVar(SkyboxName, std::string("mr_04"), VISUAL)
		SUBNAMESPACE_END(World)
		
		CVar(AimbotViewmodel, false, VISUAL)
		CVar(ViewmodelSway, false, VISUAL)
		CVar(ViewmodelSwayScale, 5.f, VISUAL)
		CVar(ViewmodelSwayInterp, 0.05f, VISUAL)

		CVar(BulletTracer, false, VISUAL)

		CVar(SimLines, false, VISUAL)
		CVar(TimedLines, false, VISUAL)
		CVar(SimSeperators, false, VISUAL)
		CVar(SeperatorLength, 12, VISUAL)
		CVar(SeperatorSpacing, 6, VISUAL)
		CVar(SwingLines, false, VISUAL)
		CVar(ProjectileTrajectory, false, VISUAL)
		CVar(PTOverwrite, false, NOSAVE) // debug
		CVar(PTType, 0, NOSAVE) // debug
		CVar(PTOffX, 23.5f, NOSAVE) // debug
		CVar(PTOffY, 12.f, NOSAVE) // debug
		CVar(PTOffZ, -3.f, NOSAVE) // debug
		CVar(PTPipes, false, NOSAVE) // debug
		CVar(PTHull, 1.f, NOSAVE) // debug
		CVar(PTSpeed, 1000.f, NOSAVE) // debug
		CVar(PTGravity, 0.f, NOSAVE) // debug
		CVar(PTNoSpin, true, NOSAVE) // debug
		CVar(PTLifeTime, 10.f, NOSAVE) // debug
		CVar(PTUpVelocity, 0.f, NOSAVE) // debug
		CVar(PTAngVelocityX, 0.f, NOSAVE) // debug
		CVar(PTAngVelocityY, 0.f, NOSAVE) // debug
		CVar(PTAngVelocityZ, 0.f, NOSAVE) // debug
		CVar(PTDrag, 0.f, NOSAVE) // debug
		CVar(PTDragBasisX, 0.f, NOSAVE) // debug
		CVar(PTDragBasisY, 0.f, NOSAVE) // debug
		CVar(PTDragBasisZ, 0.f, NOSAVE) // debug
		CVar(PTAngDragBasisX, 0.f, NOSAVE) // debug
		CVar(PTAngDragBasisY, 0.f, NOSAVE) // debug
		CVar(PTAngDragBasisZ, 0.f, NOSAVE) // debug

		CVar(DoPostProcessing, false, VISUAL)

		SUBNAMESPACE_BEGIN(Tracers)
			CVar(ParticleTracer, 0, VISUAL)
			CVar(ParticleTracerCrits, 0, VISUAL)
			CVar(ParticleName, std::string("merasmus_zap_beam01"), VISUAL)
			CVar(ParticleNameCrits, std::string("merasmus_zap_beam01"), VISUAL)
		SUBNAMESPACE_END(Tracers)

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

		SUBNAMESPACE_BEGIN(Arrows)
			CVar(Active, false, VISUAL)
			CVar(Offset, 25, VISUAL)
			CVar(MaxDist, 1000.f, VISUAL)
		SUBNAMESPACE_END(Arrows)

		CVar(VMOffX, 0, VISUAL)
		CVar(VMOffY, 0, VISUAL)
		CVar(VMOffZ, 0, VISUAL)
		CVar(VMOffsets, Vec3(), VISUAL)
		CVar(VMRoll, 0, VISUAL)

		CVar(EquipRegionUnlock, false, VISUAL)

		CVar(NoStaticPropFade, false, VISUAL)

		SUBNAMESPACE_BEGIN(RagdollEffects)
			CVar(NoGib, false, VISUAL)
			CVar(EnemyOnly, false, VISUAL)
			CVar(Burning, false, VISUAL)
			CVar(Electrocuted, false, VISUAL)
			CVar(BecomeAsh, false, VISUAL)
			CVar(Dissolve, false, VISUAL)
			CVar(RagdollType, 0, VISUAL)

			CVar(SeparateVectors, false, VISUAL)
			CVar(RagdollForce, 1.f, VISUAL)
			CVar(RagdollForceForwards, 1.f, VISUAL)
			CVar(RagdollForceSides, 1.f, VISUAL)
			CVar(RagdollForceUp, 1.f, VISUAL)
		SUBNAMESPACE_END(RagdollEffects)
	NAMESPACE_END(Visuals)

	NAMESPACE_BEGIN(CL_Move)
		SUBNAMESPACE_BEGIN(DoubleTap)
			CVar(Enabled, false)

			CVar(TickLimit, 21)
			CVar(WarpRate, 21)
			CVar(PassiveRecharge, 0)

			CVar(Mode, 1) // 0 - Always, 1 - Hold, 2 - Toggle (only for doubletap)
			CVar(DoubletapKey, 0x56)
			CVar(RechargeKey, 0x48)
			CVar(TeleportKey, 0x52)

			CVar(WaitReady, false)
			CVar(AntiWarp, false)
			CVar(NotInAir, true)
			CVar(AutoRetain, true)
			CVar(AutoRecharge, false)
			CVar(RechargeWhileDead, false)
			CVar(SafeTick, false)
			CVar(SafeTickAirOverride, false)

			CVar(Indicator, false, VISUAL)
			CVar(Position, DragBox_t(), VISUAL)
		SUBNAMESPACE_END(DoubleTap)

		CVar(SpeedEnabled, false)
		CVar(SpeedFactor, 1)

		SUBNAMESPACE_BEGIN(FakeLag)
			CVar(Enabled, false)

			CVar(Key, 0x54)
			CVar(Mode, 1) // 0 - Always, 1 - Hold, 2 - Toggle

			CVar(Type, 0) // 0 - plain, 1 - random
			CVar(Min, 1)
			CVar(Max, 1)
			CVar(Value, 1)

			CVar(WhileMoving, false)
			CVar(WhileUnducking, false)
			CVar(WhileGrounded, false)

			CVar(UnchokeOnAttack, false)
			CVar(RetainBlastJump, false)
		SUBNAMESPACE_END(FakeLag)

		CVar(AutoPeekKey, 0)
		CVar(AutoPeekDistance, 200.f)
		CVar(AutoPeekFree, false)

		SUBNAMESPACE_BEGIN(FLGChams)
			CVar(Enabled, false, VISUAL)
			CVar(Material, 1, VISUAL)
			CVar(FakelagColor, Color_t(255, 255, 255, 255), VISUAL)
		SUBNAMESPACE_END(FLGChams)
	NAMESPACE_END(CL_Move)

	NAMESPACE_BEGIN(Misc)
		CVar(FastDeltaStrafe, false)
		CVar(AccurateMovement, 0)
		CVar(AutoJump, false)
		CVar(AutoStrafe, 0)
		CVar(DirectionalOnlyOnMove, true)
		CVar(DirectionalOnlyOnSpace, false)
		CVar(Directional, false)
		CVar(TauntSlide, false)
		CVar(BypassPure, false)
		CVar(DisableInterpolation, false)
		CVar(NoPush, 0)	//	0 off, 1 on, 2 on while not afk, 3 semi while afk
		CVar(AntiAFK, false)
		CVar(VotingOptions, 0b000100) // 000100 {verbose, autovote, party, chat, console, text}
		CVar(CheatsBypass, false)
		CVar(CrouchSpeed, false)
		CVar(FastAccel, false)
		CVar(FakeAccelAngle, false)
		CVar(AntiAutobalance, false)
		CVar(MVMRes, false)
		CVar(PingReducer, false)
		CVar(PingTarget, 1)
		CVar(ConvarName, std::string())
		CVar(ConvarValue, std::string())
		CVar(SoundBlock, 0)
		CVar(ChatFlags, false)
		CVar(AutoAcceptItemDrops, false)
		CVar(RegionChanger, false)
		CVar(RegionsAllowed, 0)
		CVar(AutoCasualQueue, 0)
		CVar(FreezeQueue, false)
		CVar(ScoreboardPlayerlist, false)

		SUBNAMESPACE_BEGIN(CheaterDetection)
			CVar(Enabled, false)
			CVar(Methods, 0b111111100)			//	Duckspeed, Aimbot, OOB pitch, angles, bhop, fakelag, simtime, high score, high accuracy
			CVar(Protections, 0b111)			//	double scans, lagging client, timing out
			CVar(SuspicionGate, 10)				//	the amount of infractions prior to marking someone as a cheater
			CVar(PacketManipGate, 14)			//	the avg choke for someone to receive and infraction for packet choking
			CVar(BHopMaxDelay, 1)				//	max groundticks used when detecting a bunny hop.
			CVar(BHopDetectionsRequired, 5)		//	how many times must some be seen bunny hopping to receive an infraction
			CVar(ScoreMultiplier, 2.f)		//	multiply the avg score/s by this to receive the maximum amount
			CVar(MinimumFlickDistance, 20.f)	//	min mouse flick size to suspect someone of angle cheats.
			CVar(MaximumNoise, 5.f)			//	max mouse noise prior to a flick to mark somebody
			CVar(MinimumAimbotFoV, 3.f)		//	scaled with how many ticks a player has choked up to ->
			CVar(MaxScaledAimbotFoV, 20.f)	//	self explanatory
		SUBNAMESPACE_END(CheaterDetection)

		SUBNAMESPACE_BEGIN(Steam)
			CVar(EnableRPC, false)
			CVar(MatchGroup, 0) // 0 - Special Event; 1 - MvM Mann Up; 2 - Competitive; 3 - Casual; 4 - MvM Boot Camp;
			CVar(OverrideMenu, false) // Override matchgroup when in main menu
			CVar(MapText, 1) // 0 - Fedoraware; 1 - CUM.clab; 2 - Meowhook.club; 3 - rathook.cc; 4 - NNitro.tf; 5 - custom;
			CVar(GroupSize, 1337)
			CVar(CustomText, std::string("M-FeD is gay"))
		SUBNAMESPACE_END(Steam)
	NAMESPACE_END(Misc)

	NAMESPACE_BEGIN(AntiHack)
		SUBNAMESPACE_BEGIN(AntiAim)
			CVar(Active, false)
			CVar(ToggleKey, 0)

			CVar(PitchReal, 0) //0 - None, 1 - Zero, 2 - Up, 3 - Down, 4 - Zero
			CVar(PitchFake, 0) //0 - None, 1 - Zero, 2 - Up, 3 - Down

			CVar(YawReal, 0) //0 - None, 1 - Forward, 2 - Left, 3 - Right, 4 - Backwards, 5 - Spin, 6 - Edge
			CVar(YawFake, 0) //0 - None, 1 - Forward, 2 - Left, 3 - Right, 4 - Backwards, 5 - Spin, 6 - Edge

			CVar(RealYawMode, 0)
			CVar(RealYawOffset, 0.f)
			CVar(FakeYawMode, 0)
			CVar(FakeYawOffset, 0.f)

			CVar(SpinSpeed, 15.f)
			CVar(AntiOverlap, false)
			CVar(AntiBackstab, false)
			CVar(LegJitter, false) // frick u fourteen
			CVar(InvalidShootPitch, false) // i dont know what to name this its TRASH
		SUBNAMESPACE_END(AntiAim)

		SUBNAMESPACE_BEGIN(Resolver)
			CVar(Resolver, false)
			CVar(AutoResolveCheaters, false)
			CVar(IgnoreAirborne, false)
		SUBNAMESPACE_END(Resolver)
	NAMESPACE_END(AntiHack)

	NAMESPACE_BEGIN(Colors)
		CVar(FOVCircle, Color_t(255, 255, 255, 100), VISUAL)
		CVar(TeamRed, Color_t(225, 60, 60, 255), VISUAL)
		CVar(TeamBlu, Color_t(75, 175, 225, 255), VISUAL)
		CVar(Enemy, Color_t(225, 60, 60, 255), VISUAL)
		CVar(Team, Color_t(75, 175, 225, 255), VISUAL)
		CVar(Local, Color_t(255, 255, 255, 255), VISUAL)
		CVar(Friend, Color_t(50, 255, 150, 255), VISUAL)
		CVar(Ignored, Color_t(25, 200, 100, 255), VISUAL)
		CVar(Target, Color_t(255, 0, 0, 255), VISUAL)
		CVar(Invulnerable, Color_t(125, 100, 175, 255), VISUAL)
		CVar(Cloak, Color_t(150, 175, 210, 255), VISUAL)
		CVar(Overheal, Color_t(75, 175, 255, 255), VISUAL)
		CVar(HealthBar, Gradient_t({ 255, 0, 0, 255 }, { 0, 200, 125, 255 }), VISUAL)
		CVar(UberBar, Color_t( 127, 255, 255, 255 ), VISUAL)
		CVar(Weapon, Color_t(255, 255, 255, 255), VISUAL)
		CVar(Health, Color_t(0, 225, 75, 255), VISUAL)
		CVar(Ammo, Color_t(175, 175, 175, 255), VISUAL)
		CVar(NPC, Color_t(255, 255, 255, 255), VISUAL)
		CVar(Bomb, Color_t(255, 75, 0, 255), VISUAL)
		CVar(Spellbook, Color_t(100, 0, 255, 255), VISUAL)
		CVar(Gargoyle, Color_t(0, 150, 75, 255), VISUAL)

		CVar(WorldModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(SkyModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(StaticPropModulation, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ParticleModulation, Color_t(255, 255, 255, 255), VISUAL)

		CVar(DtOutline, Color_t(30, 30, 30, 180), VISUAL)

		CVar(BulletTracer, Color_t(255, 255, 255, 255), VISUAL)
		CVar(PredictionColor, Color_t(255, 255, 255, 255), VISUAL)
		CVar(ProjectileColor, Color_t(255, 100, 100, 255), VISUAL)
		CVar(HitboxEdge, Color_t(255, 255, 255, 255), VISUAL)
		CVar(HitboxFace, Color_t(255, 255, 255, 0), VISUAL)
	NAMESPACE_END(Colors)

	NAMESPACE_BEGIN(Fonts)
		SUBNAMESPACE_BEGIN(FONT_ESP)
			CVar(szName, std::string("Small Fonts"), VISUAL)
			CVar(nTall, 12, VISUAL)
			CVar(nWeight, 0, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_ESP)

		SUBNAMESPACE_BEGIN(FONT_ESP_NAME)
			CVar(szName, std::string("Small Fonts"), VISUAL)
			CVar(nTall, 12, VISUAL)
			CVar(nWeight, 0, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_ESP_NAME)

		SUBNAMESPACE_BEGIN(FONT_ESP_COND)
			CVar(szName, std::string("Small Fonts"), VISUAL)
			CVar(nTall, 12, VISUAL)
			CVar(nWeight, 0, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_ESP_COND)

		SUBNAMESPACE_BEGIN(FONT_ESP_PICKUPS)
			CVar(szName, std::string("Small Fonts"), VISUAL)
			CVar(nTall, 12, VISUAL)
			CVar(nWeight, 0, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_ESP_PICKUPS)

		SUBNAMESPACE_BEGIN(FONT_MENU)
			CVar(szName, std::string("ProggyClean"), VISUAL)
			CVar(nTall, 13, VISUAL)
			CVar(nWeight, -1, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_MENU)

		SUBNAMESPACE_BEGIN(FONT_INDICATORS)
			CVar(szName, std::string("ProggyClean"), VISUAL)
			CVar(nTall, 13, VISUAL)
			CVar(nWeight, -1, VISUAL)
			CVar(nFlags, 512, VISUAL)
		SUBNAMESPACE_END(FONT_INDICATORS)
	NAMESPACE_END(Fonts)

	NAMESPACE_BEGIN(Logging)
		CVar(Logs, 0b001) // { Damage, Class Changes, Votes }
		CVar(LogTo, 0b0001) // { Console, Party, Chat, Toasts }


		SUBNAMESPACE_BEGIN(Notification)
			CVar(Background, Color_t(30, 30, 30, 255), VISUAL)
			CVar(Outline, Color_t(255, 101, 101, 255), VISUAL)
			CVar(Text, Color_t(255, 255, 255, 255), VISUAL)
			CVar(Lifetime, 5.f, VISUAL)
		SUBNAMESPACE_END(Notification)
	NAMESPACE_END(Logging)

	NAMESPACE_BEGIN(Debug)
		CVar(Info, false, NOSAVE)
		CVar(Logging, false, NOSAVE)
		CVar(ServerHitbox, false, NOSAVE)
		CVar(AntiAimLines, false, NOSAVE)
	NAMESPACE_END(Debug)
}