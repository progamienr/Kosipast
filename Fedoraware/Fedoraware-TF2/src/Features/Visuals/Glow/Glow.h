#pragma once

#include "../../Feature.h"

class CGlow
{
	void SetScale(int nScale, bool bReset);
	void DrawModel(CBaseEntity* pEntity, bool bModel = false, Color_t cColor = {});

	ITexture* m_pRtFullFrame;
	ITexture* m_pRenderBuffer1;
	ITexture* m_pRenderBuffer2;

	struct GlowEnt_t
	{
		CBaseEntity* m_pEntity;
		Color_t m_Color;
	};

	std::vector<GlowEnt_t> m_vecGlowEntities;

public:
	void Init();
	void Render();

	//bool m_bDrawingGlow;
	bool m_bRendering;
};

ADD_FEATURE(CGlow, Glow)