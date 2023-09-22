#pragma once
#include "../../SDK/SDK.h"

#include <map>

struct Sightline_t
{
	Vec3 m_vStart = { 0,0,0 };
	Vec3 m_vEnd = { 0,0,0 };
	Color_t m_Color = { 0,0,0,0 };
	bool m_bDraw = false;
};

struct BulletTracer_t
{
	Vec3 m_vStartPos;
	Vec3 m_vEndPos;
	Color_t m_Color;
	float m_flTimeCreated;
};

class CVisuals
{
private:
	int m_nHudZoom = 0;
	int m_nHudMotd = 0;
	IMaterial* m_pMatDev;

public:
	bool RemoveScope(int nPanel);
	void FOV(CViewSetup* pView);
	void ThirdPerson(CViewSetup* pView);
	void ModulateWorld();
	void RestoreWorldModulation();
	void OverrideWorldTextures();
	void PickupTimers();
	void DrawHitbox(matrix3x4 bones[128], CBaseEntity* pEntity);
	void DrawHitbox(CBaseEntity* pTarget, Vec3 vOrigin, float flTime);

	void DrawOnScreenConditions(CBaseEntity* pLocal);
	void DrawOnScreenPing(CBaseEntity* pLocal);
	void SkyboxChanger();
	void BulletTrace(CBaseEntity* pEntity, Color_t color);
	void ProjectileTrace();
	void DrawAimbotFOV(CBaseEntity* pLocal);
	void DrawDebugInfo(CBaseEntity* pLocal);
	void DrawAntiAim(CBaseEntity* pLocal);
	void DrawTickbaseText();
	void DrawTickbaseBars();
	void DrawBulletLines();
	void ClearBulletLines();
	void RevealBulletLines();
	void DrawSimLine(std::deque<std::pair<Vec3, Vec3>>& Line, Color_t Color, bool bSeparators = false);
	void DrawSimLines();
	void RevealSimLines();
	void DrawBoxes();
	void RevealBoxes();
	void ManualNetwork(const StartSoundParams_t& params); // Credits: reestart
	void RenderLine(const Vec3& v1, const Vec3& v2, Color_t c, bool bZBuffer);
	void RenderBox(const Vec3& vPos, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vOrientation, Color_t cEdge, Color_t cFace);
	void DrawSightlines();
	void FillSightlines();
	void AddBulletTracer(const Vec3& vFrom, const Vec3& vTo, const Color_t& clr);
	void PruneBulletTracers();
	void DrawBulletTracers();
	void DrawProjectileTracer(CBaseEntity* pLocal, const Vec3& position);
	void DrawServerHitboxes();

	std::vector<BulletTracer_t> m_vecBulletTracers;

	float arrowUp = 0.f;
	float arrowRight = 0.f;

	std::array<Sightline_t, 64> m_SightLines;

	struct PickupData
	{
		int Type = 0;
		float Time = 0.f;
		Vec3 Location;
	};
	std::vector<PickupData> PickupDatas;

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