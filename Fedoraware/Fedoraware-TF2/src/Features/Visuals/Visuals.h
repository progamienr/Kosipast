#pragma once
#include "../Feature.h"

#include <map>

struct Sightline_t
{
	Vec3 m_vStart = { 0,0,0 };
	Vec3 m_vEnd = { 0,0,0 };
	Color_t m_Color = { 0,0,0,0 };
	bool m_bDraw = false;
};

class CVisuals
{
private:
	int m_nHudZoom = 0;

public:
	void DrawAimbotFOV(CBaseEntity* pLocal);
	void DrawTickbaseText();
	void DrawTickbaseBars();
	void DrawOnScreenConditions(CBaseEntity* pLocal);
	void DrawOnScreenPing(CBaseEntity* pLocal);
	void ProjectileTrace(const bool bQuick = true);
	void DrawAntiAim(CBaseEntity* pLocal);
	void DrawDebugInfo(CBaseEntity* pLocal);

	void DrawHitbox(matrix3x4 bones[128], CBaseEntity* pEntity);
	void DrawHitbox(CBaseEntity* pTarget, Vec3 vOrigin, float flTime);
	void DrawBulletLines();
	void DrawSimLine(std::deque<std::pair<Vec3, Vec3>>& Line, Color_t Color, bool bSeparators = false, bool bZBuffer = false);
	void DrawSimLines();
	void DrawBoxes();
	void RevealSimLines();
	void RevealBulletLines();
	void RevealBoxes();
	void ClearBulletLines();
	void DrawServerHitboxes();
	void RenderLine(const Vec3& v1, const Vec3& v2, Color_t c, bool bZBuffer = false);
	void RenderBox(const Vec3& vPos, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vOrientation, Color_t cEdge, Color_t cFace, bool bZBuffer = false);

	void FOV(CViewSetup* pView);
	void ThirdPerson(CViewSetup* pView);
	bool RemoveScope(int nPanel);
	void DrawSightlines();
	void FillSightlines();
	void PickupTimers();
	void ManualNetwork(const StartSoundParams_t& params); // Credits: reestart

	std::array<Sightline_t, 64> m_SightLines;

	struct PickupData
	{
		int Type = 0;
		float Time = 0.f;
		Vec3 Location;
	};
	std::vector<PickupData> PickupDatas;

	void OverrideWorldTextures();
	void ModulateWorld();
	void SkyboxChanger();
	void RestoreWorldModulation();

	struct MaterialHandleData
	{
		enum class EMatGroupType
		{
			GROUP_OTHER,
			GROUP_WORLD,
			GROUP_SKY
		};

		MaterialHandle_t Handle;
		IMaterial* Material;
		std::string_view Group;
		std::string_view Name;
		EMatGroupType	 GroupType;
		bool			 ShouldOverrideTextures;
	};
	std::vector<MaterialHandleData> MaterialHandleDatas;

	void StoreMaterialHandles();
	void ClearMaterialHandles();
};

ADD_FEATURE(CVisuals, Visuals)