#include "../Hooks.h"

#include "../../Features/Visuals/Chams/Chams.h"
#include "../../Features/Visuals/Materials/Materials.h"
#include "../../Features/Visuals/Glow/Glow.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/Visuals/FakeAngleManager/FakeAng.h"

void DrawBT(void* ecx, void* edx, CBaseEntity* pEntity, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld);

MAKE_HOOK(ModelRender_DrawModelExecute, Utils::GetVFuncPtr(I::ModelRender, 19), void, __fastcall,
	void* ecx, void* edx, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (Vars::Visuals::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return Hook.Original<FN>()(ecx, edx, pState, pInfo, pBoneToWorld);

	DrawBT(ecx, edx, I::ClientEntityList->GetClientEntity(pInfo.m_nEntIndex), pState, pInfo, pBoneToWorld);

	if (!F::Glow.m_bRendering && F::Chams.Render(pState, pInfo, pBoneToWorld))
		return;

	Hook.Original<FN>()(ecx, edx, pState, pInfo, pBoneToWorld);
}

void DrawBT(void* ecx, void* edx, CBaseEntity* pEntity, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	auto OriginalFn = Hooks::ModelRender_DrawModelExecute::Hook.Original<Hooks::ModelRender_DrawModelExecute::FN>();
	const auto& pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return;

	if (!Vars::Backtrack::Enabled.Value)
		return;

	if (F::Glow.m_bRendering/* || F::Chams.m_bRendering*/)
		return;

	if (!pEntity || pEntity->GetClassID() != ETFClassID::CTFPlayer || !pEntity->IsAlive())
		return;

	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();
	if (!pLocal || !pWeapon)
		return;
	if (G::CurWeaponType == EWeaponType::PROJECTILE)
		return;
	if (pEntity == pLocal ||
		pWeapon->m_iItemDefinitionIndex() != Soldier_t_TheDisciplinaryAction && pWeapon->GetWeaponID() != TF_WEAPON_MEDIGUN && pEntity->m_iTeamNum() == pLocal->m_iTeamNum() ||
		pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN && pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
		return;



	Chams_t chams = Vars::Chams::Players::Backtrack.Value;
	if (!chams.ChamsActive)
		return;

	auto baseMaterial = F::Materials.GetMaterial(chams.Material), overlayMaterial = F::Materials.GetMaterial(chams.OverlayMaterial);

	#define drawModel(matrix)\
	{\
		pRenderContext->DepthRange(0.0f, chams.IgnoreZ ? 0.2f : 1.f);\
		F::Materials.SetColor(baseMaterial, chams.Color);\
		if (baseMaterial)\
			I::ModelRender->ForcedMaterialOverride(baseMaterial);\
		OriginalFn(ecx, edx, pState, pInfo, matrix);\
		if (overlayMaterial)\
		{\
			F::Materials.SetColor(overlayMaterial, chams.OverlayColor);\
			I::ModelRender->ForcedMaterialOverride(overlayMaterial);\
			OriginalFn(ecx, edx, pState, pInfo, matrix);\
		}\
	};

	if (Vars::Backtrack::LastOnly.Value)
	{
		std::optional<TickRecord> vLastRec = F::Backtrack.GetLastRecord(pEntity);
		if (vLastRec && pEntity->GetAbsOrigin().DistTo(vLastRec->vOrigin) >= 0.1f)
			drawModel((matrix3x4*)(&vLastRec->BoneMatrix));
	}
	else
	{
		const auto& vRecords = F::Backtrack.GetRecords(pEntity);
		if (vRecords && !vRecords->empty())
		{
			for (auto& record : *vRecords)
			{
				if (!F::Backtrack.WithinRewind(record))
					continue;
				if (pEntity->GetAbsOrigin().DistTo(record.vOrigin) <= 0.1f)
					continue;

				drawModel((matrix3x4*)(&record.BoneMatrix));
			}
		}
	}

	pRenderContext->DepthRange(0.0f, 1.0f);
	I::ModelRender->ForcedMaterialOverride(nullptr);
	I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
	I::RenderView->SetBlend(1.0f);
}