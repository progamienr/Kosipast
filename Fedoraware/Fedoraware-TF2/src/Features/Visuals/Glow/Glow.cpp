#include "Glow.h"

#include "../../Vars.h"
#include "../../Color.h"

#include "../Materials/Materials.h"

void CGlow::Init()
{
	m_pRtFullFrame = I::MaterialSystem->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
	m_pRtFullFrame->IncrementReferenceCount();

	m_pRenderBuffer1 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
		"glow_buffer_1",
		m_pRtFullFrame->GetActualWidth(),
		m_pRtFullFrame->GetActualHeight(),
		RT_SIZE_LITERAL,
		IMAGE_FORMAT_RGB888,
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
		CREATERENDERTARGETFLAGS_HDR
	);
	m_pRenderBuffer1->IncrementReferenceCount();

	m_pRenderBuffer2 = I::MaterialSystem->CreateNamedRenderTargetTextureEx(
		"glow_buffer_2",
		m_pRtFullFrame->GetActualWidth(),
		m_pRtFullFrame->GetActualHeight(),
		RT_SIZE_LITERAL,
		IMAGE_FORMAT_RGB888,
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
		CREATERENDERTARGETFLAGS_HDR
	);
	m_pRenderBuffer2->IncrementReferenceCount();
}



void CGlow::SetScale(int nScale, bool bReset = false)
{
	auto m_pMatBlurY = F::Materials.GetMaterial("BlurY");

	static IMaterialVar* pVar = nullptr;
	static bool bFound = false;

	if (bReset) { pVar = nullptr; bFound = false; return; }

	if (!bFound && m_pMatBlurY)
		pVar = m_pMatBlurY->FindVar("$bloomamount", &bFound);
	else if (pVar)
		pVar->SetFloatValue(float(nScale) / 2.f);
}

void CGlow::DrawModel(CBaseEntity* pEntity, bool bModel, Color_t cColor)
{
	if (!pEntity->ShouldDraw())
		return;

	m_bRendering = true;
	if (bModel)
	{
		m_vecGlowEntities.push_back({ pEntity, cColor });
		I::RenderView->SetBlend(0.f);
	}
	//else
	//	m_bDrawingGlow = true;

	const float flOldInvisibility = pEntity->IsPlayer() ? pEntity->m_flInvisibility() : -1.0f;
	if (flOldInvisibility > 0.99f)
		pEntity->m_flInvisibility() = 0.f;

	pEntity->DrawModel(bModel ? STUDIO_RENDER : (STUDIO_RENDER | STUDIO_NOSHADOWS));

	if (flOldInvisibility > 0.99f)
		pEntity->m_flInvisibility() = flOldInvisibility;

	m_bRendering = false;
	if (bModel)
		I::RenderView->SetBlend(1.f);
	//else
	//	m_bDrawingGlow = false;
}

void CGlow::Render()
{
	if (!m_vecGlowEntities.empty())
		m_vecGlowEntities.clear();

	if (!Vars::Glow::Main::Active.Value)
		return;

	const int w = g_ScreenSize.w, h = g_ScreenSize.h;
	if (I::EngineVGui->IsGameUIVisible() || w < 1 || h < 1 || w > 4096 || h > 2160)
		return;

	IMatRenderContext* pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return;

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal)
		return;

	auto m_pMatGlowColor = F::Materials.GetMaterial("GlowColor");
	auto m_pMatBlurX = F::Materials.GetMaterial("BlurX");
	auto m_pMatBlurY = F::Materials.GetMaterial("BlurY");
	auto m_pMatHaloAddToScreen = F::Materials.GetMaterial("HaloAddToScreen");
	if (!m_pMatGlowColor || !m_pMatBlurX || !m_pMatBlurY || !m_pMatHaloAddToScreen)
		return F::Materials.ReloadMaterials();



	SetScale(Vars::Glow::Main::Scale.Value);

	ShaderStencilState_t StencilStateDisable = {};
	StencilStateDisable.m_bEnable = false;

	float flOriginalColor[3] = {};
	I::RenderView->GetColorModulation(flOriginalColor);
	float flOriginalBlend = I::RenderView->GetBlend();

	{
		ShaderStencilState_t StencilState = {};
		StencilState.m_bEnable = true;
		StencilState.m_nReferenceValue = 1;
		StencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		StencilState.m_PassOp = STENCILOPERATION_REPLACE;
		StencilState.m_FailOp = STENCILOPERATION_KEEP;
		StencilState.m_ZFailOp = STENCILOPERATION_REPLACE;
		StencilState.SetStencilState(pRenderContext);
	}

	I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
	I::RenderView->SetBlend(1.0f);



	if (Vars::Glow::Players::Active.Value)
	{
		for (const auto& pPlayer : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
		{
			if (!pPlayer->IsAlive() || pPlayer->IsAGhost() || pPlayer->GetDormant())
				continue;

			bool bIsLocal = pPlayer->GetIndex() == I::EngineClient->GetLocalPlayer();
			if (!bIsLocal)
			{
				switch (Vars::Glow::Players::IgnoreTeammates.Value)
				{
				case 0: break;
				case 1:
					if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum())
						continue;
					break;
				case 2:
					if (pPlayer->m_iTeamNum() == pLocal->m_iTeamNum() && !g_EntityCache.IsFriend(pPlayer->GetIndex()))
						continue;
					break;
				}
			}
			else if (!Vars::Glow::Players::ShowLocal.Value)
				continue;

			if (!Utils::IsOnScreen(pLocal, pPlayer))
				continue;

			Color_t DrawColor = DrawColor = GetEntityDrawColor(pPlayer, Vars::ESP::Main::EnableTeamEnemyColors.Value);

			DrawModel(pPlayer, true, DrawColor);
			
			CBaseEntity* pAttachment = pPlayer->FirstMoveChild();
			for (int n = 0; n < 32; n++)
			{
				if (!pAttachment)
					break;
					
				if (pAttachment->IsWearable())
					DrawModel(pAttachment, true, DrawColor);

				pAttachment = pAttachment->NextMovePeer();
			}
			
			if (const auto& pWeapon = pPlayer->GetActiveWeapon())
				DrawModel(pWeapon, true, DrawColor);
		}
	}

	if (Vars::Glow::Buildings::Active.Value)
	{
		for (const auto& Building : g_EntityCache.GetGroup(EGroupType::BUILDINGS_ALL))
		{
			const auto& pBuilding = reinterpret_cast<CBaseObject*>(Building);
			if (!Building->IsAlive())
				continue;

			if (Vars::Glow::Buildings::IgnoreTeammates.Value && Building->m_iTeamNum() == pLocal->m_iTeamNum())
				continue;

			if (!Utils::IsOnScreen(pLocal, Building))
				continue;

			Color_t DrawColor = GetEntityDrawColor(Building, Vars::ESP::Main::EnableTeamEnemyColors.Value);

			DrawModel(Building, true, DrawColor);
		}
	}

	if (Vars::Glow::World::Active.Value)
	{
		if (Vars::Glow::World::Health.Value)
		{
			for (const auto& Health : g_EntityCache.GetGroup(EGroupType::WORLD_HEALTH))
			{
				if (!Utils::IsOnScreen(pLocal, Health))
					continue;

				DrawModel(Health, true, Vars::Colors::Health.Value);
			}
		}

		if (Vars::Glow::World::Ammo.Value)
		{
			for (const auto& Ammo : g_EntityCache.GetGroup(EGroupType::WORLD_AMMO))
			{
				if (!Utils::IsOnScreen(pLocal, Ammo))
					continue;

				DrawModel(Ammo, true, Vars::Colors::Ammo.Value);
			}
		}

		if (Vars::Glow::World::Projectiles.Value)
		{
			for (const auto& Projectile : g_EntityCache.GetGroup(EGroupType::WORLD_PROJECTILES))
			{
				if (*reinterpret_cast<byte*>(Projectile + 0x7C) & EF_NODRAW)
					continue;

				int nTeam = Projectile->m_iTeamNum();

				if (Vars::Glow::World::Projectiles.Value == 2 && nTeam == pLocal->m_iTeamNum())
					continue;

				if (!Utils::IsOnScreen(pLocal, Projectile))
					continue;

				DrawModel(Projectile, true, GetTeamColor(nTeam, Vars::ESP::Main::EnableTeamEnemyColors.Value));
			}
		}

		if (Vars::Glow::World::NPCs.Value)
		{
			for (const auto& NPC : g_EntityCache.GetGroup(EGroupType::WORLD_NPC))
			{
				if (!Utils::IsOnScreen(pLocal, NPC))
					continue;

				DrawModel(NPC, true, GetEntityDrawColor(NPC, false));
			}
		}

		if (Vars::Glow::World::Bombs.Value)
		{
			for (const auto& Bomb : g_EntityCache.GetGroup(EGroupType::WORLD_BOMBS))
			{
				if (!Utils::IsOnScreen(pLocal, Bomb))
					continue;

				DrawModel(Bomb, true, Vars::Colors::Bomb.Value);
			}
		}

		if (Vars::Glow::World::Spellbook.Value)
		{
			for (const auto& Book : g_EntityCache.GetGroup(EGroupType::WORLD_SPELLBOOK))
			{
				if (!Utils::IsOnScreen(pLocal, Book))
					continue;

				DrawModel(Book, true, Vars::Colors::Spellbook.Value);
			}
		}

		if (Vars::Glow::World::Gargoyle.Value)
		{
			for (const auto& Gargy : g_EntityCache.GetGroup(EGroupType::WORLD_GARGOYLE))
			{
				if (!Utils::IsOnScreen(pLocal, Gargy))
					continue;

				DrawModel(Gargy, true, Vars::Colors::Gargoyle.Value);
			}
		}
	}

	StencilStateDisable.SetStencilState(pRenderContext);



	if (m_vecGlowEntities.empty())
		return;

	I::ModelRender->ForcedMaterialOverride(m_pMatGlowColor);

	pRenderContext->PushRenderTargetAndViewport();
	{
		pRenderContext->SetRenderTarget(m_pRenderBuffer1);
		pRenderContext->Viewport(0, 0, w, h);
		pRenderContext->ClearColor4ub(0, 0, 0, 0);
		pRenderContext->ClearBuffers(true, false, false);

		for (const auto& GlowEntity : m_vecGlowEntities)
		{
			I::RenderView->SetColorModulation( Color::TOFLOAT(GlowEntity.m_Color.r), Color::TOFLOAT(GlowEntity.m_Color.g), Color::TOFLOAT(GlowEntity.m_Color.b) );
			I::RenderView->SetBlend(Color::TOFLOAT(GlowEntity.m_Color.a));
			DrawModel(GlowEntity.m_pEntity);
		}

		StencilStateDisable.SetStencilState(pRenderContext);
	}
	pRenderContext->PopRenderTargetAndViewport();

	if (Vars::Glow::Main::Type.Value == 0)
	{
		pRenderContext->PushRenderTargetAndViewport();
		{
			pRenderContext->Viewport(0, 0, w, h);
			pRenderContext->SetRenderTarget(m_pRenderBuffer2);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatBlurX, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
			pRenderContext->SetRenderTarget(m_pRenderBuffer1);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatBlurY, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		}
		pRenderContext->PopRenderTargetAndViewport();
	}

	{
		ShaderStencilState_t StencilState = {};
		StencilState.m_bEnable = true;
		StencilState.m_nWriteMask = 0x0;
		StencilState.m_nTestMask = 0xFF;
		StencilState.m_nReferenceValue = 0;
		StencilState.m_CompareFunc = STENCILCOMPARISONFUNCTION_EQUAL;
		StencilState.m_PassOp = STENCILOPERATION_KEEP;
		StencilState.m_FailOp = STENCILOPERATION_KEEP;
		StencilState.m_ZFailOp = STENCILOPERATION_KEEP;
		StencilState.SetStencilState(pRenderContext);
	}

	switch (Vars::Glow::Main::Type.Value)
	{
	case 0:
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		break;
	case 1: {
		int side = int(float(Vars::Glow::Main::Scale.Value) / 2 + 0.5f);
		int corner = int(float(Vars::Glow::Main::Scale.Value) / 2);
		if (corner)
		{
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -corner, -corner, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, corner, corner, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, corner, -corner, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
			pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -corner, corner, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		}
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, -side, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, -side, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, 0, side, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		pRenderContext->DrawScreenSpaceRectangle(m_pMatHaloAddToScreen, side, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		break;
	}
	}
	StencilStateDisable.SetStencilState(pRenderContext);

	I::ModelRender->ForcedMaterialOverride(nullptr);
	I::RenderView->SetColorModulation(flOriginalColor);
	I::RenderView->SetBlend(flOriginalBlend);
}