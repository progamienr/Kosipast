#include "Chams.h"

#include "../../Vars.h"
#include "../../../Hooks/Hooks.h"
#include "../../../Hooks/HookManager.h"
#include "../Materials/Materials.h"
#include "../FakeAngleManager/FakeAng.h"

bool CDMEChams::ShouldRun()
{
	if (!Vars::Chams::Active.Value || I::EngineVGui->IsGameUIVisible())
		return false;

	return true;
}

int GetType(int EntIndex)
{
	CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(EntIndex);
	if (!pEntity)
		return 0;

	switch (pEntity->GetClassID())
	{
	case ETFClassID::CTFViewModel:
		return 1;
	case ETFClassID::CBasePlayer:
	case ETFClassID::CTFPlayer:
		return 2;
	case ETFClassID::CRagdollPropAttached:
	case ETFClassID::CRagdollProp:
	case ETFClassID::CTFRagdoll:
		return 3;
	case ETFClassID::CTFWearable:
		return 4;
	case ETFClassID::CTFAmmoPack:
		return 6;
	case ETFClassID::CBaseAnimating:
	{
		const auto szName = pEntity->GetModelName();
		if (Hash::IsAmmo(szName))
			return 6;
		if (Hash::IsHealth(szName))
			return 7;
		break;
	}
	case ETFClassID::CObjectSentrygun:
	case ETFClassID::CObjectDispenser:
	case ETFClassID::CObjectTeleporter:
		return 8;
	case ETFClassID::CTFProjectile_Rocket:
	case ETFClassID::CTFGrenadePipebombProjectile:
	case ETFClassID::CTFProjectile_Jar:
	case ETFClassID::CTFProjectile_JarGas:
	case ETFClassID::CTFProjectile_JarMilk:
	case ETFClassID::CTFProjectile_Arrow:
	case ETFClassID::CTFProjectile_SentryRocket:
	case ETFClassID::CTFProjectile_Flare:
	case ETFClassID::CTFProjectile_Cleaver:
	case ETFClassID::CTFProjectile_EnergyBall:
	case ETFClassID::CTFProjectile_HealingBolt:
	case ETFClassID::CTFProjectile_ThrowableBreadMonster:
		return 9;
	case ETFClassID::CBaseDoor:
		return 10;
	}

	CBaseCombatWeapon* pWeapon = reinterpret_cast<CBaseCombatWeapon*>(pEntity);
	if (pWeapon)
		return 5;

	return -1;
}

Chams_t GetPlayerChams(CBaseEntity* pEntity)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (pEntity && pLocal)
	{
		if (pEntity->GetIndex() == G::CurrentTargetIdx && Vars::Chams::Players::Target.Value.ChamsActive)
			return Vars::Chams::Players::Target.Value;
		if (pEntity == pLocal)
			return Vars::Chams::Players::Local.Value;
		if (g_EntityCache.IsFriend(pEntity->GetIndex()) && Vars::Chams::Players::Friend.Value.ChamsActive)
			return Vars::Chams::Players::Friend.Value;
		if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
			return Vars::Chams::Players::Enemy.Value;
		if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
			return Vars::Chams::Players::Team.Value;
	}

	return Chams_t();
}

Chams_t GetBuildingChams(CBaseEntity* pEntity)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (pEntity && pLocal)
	{
		if (pEntity->GetIndex() == G::CurrentTargetIdx && Vars::Chams::Buildings::Target.Value.ChamsActive)
			return Vars::Chams::Buildings::Target.Value;
		if (pEntity->GetIndex() == pLocal->GetIndex())
			return Vars::Chams::Buildings::Local.Value;
		if (g_EntityCache.IsFriend(pEntity->GetIndex()) && Vars::Chams::Buildings::Friend.Value.ChamsActive)
			return Vars::Chams::Buildings::Friend.Value;
		if (pEntity->m_iTeamNum() != pLocal->m_iTeamNum())
			return Vars::Chams::Buildings::Enemy.Value;
		if (pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
			return Vars::Chams::Buildings::Team.Value;
	}

	return Chams_t();
}

Chams_t GetChamsType(int nIndex, CBaseEntity* pEntity = nullptr)
{
	switch (nIndex)
	{
	case 0:
		Vars::Chams::Players::Weapon.Value.ChamsActive = true;
		Vars::Chams::Players::Weapon.Value.IgnoreZ = true;
		return Vars::Chams::Players::Weapon.Value;
	case 1:
		Vars::Chams::Players::Hands.Value.ChamsActive = true;
		Vars::Chams::Players::Hands.Value.IgnoreZ = true;
		return Vars::Chams::Players::Hands.Value;
	case 2:
		return GetPlayerChams(pEntity);
	case 3:
		return Vars::Chams::Players::Ragdoll.Value;
	case 4:
		if (!pEntity)
			return Chams_t();
		if (CBaseEntity* pOwner = I::ClientEntityList->GetClientEntityFromHandle(pEntity->m_hOwnerEntity()))
			return GetPlayerChams(pOwner);
		return Chams_t();
	case 5:
		if (!pEntity)
			return Chams_t();
		if (CBaseEntity* pOwner = I::ClientEntityList->GetClientEntityFromHandle(pEntity->m_hOwnerEntity()))
			return GetPlayerChams(pOwner);
		return Chams_t();
	case 6:
		return Vars::Chams::World::Ammo.Value;
	case 7:
		return Vars::Chams::World::Health.Value;
	case 8:
	{
		if (!pEntity)
			return Chams_t();
		const auto& pBuilding = reinterpret_cast<CBaseObject*>(pEntity);
		if (!pBuilding || !(!pBuilding->GetCarried() && pBuilding->GetConstructed()))
			return Chams_t();
		if (CBaseEntity* pOwner = pBuilding->GetOwner())
			return GetBuildingChams(pOwner);
		else if (int teamNum = pEntity->m_iTeamNum())
		{	// if we don't have an owner, we need to do this, or else spawned buildings that do have a team will return no cham struct.
			CBaseEntity* pLocal = g_EntityCache.GetLocal();
			if (pLocal)
				return (teamNum = pLocal->m_iTeamNum()) ? Vars::Chams::Buildings::Team.Value : Vars::Chams::Buildings::Enemy.Value;
		}
		return Chams_t();
	}
	case 9:
		if (!pEntity)
			return Chams_t();
		if (CBaseEntity* pOwner = I::ClientEntityList->GetClientEntityFromHandle(pEntity->m_hThrower()))
			return GetPlayerChams(pOwner);
		return Chams_t();
	}

	return Chams_t();
}

void CDMEChams::RenderFakeAng(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	const auto ModelRender_DrawModelExecute = g_HookManager.GetMapHooks()["ModelRender_DrawModelExecute"];
	const auto& pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!ModelRender_DrawModelExecute || !pRenderContext)
		return;



	Chams_t chams = Vars::Chams::Players::FakeAng.Value;
	if (!chams.ChamsActive || !F::FakeAng.DrawChams)
		return;

	auto baseMaterial = F::Materials.GetMaterial(chams.Material), overlayMaterial = F::Materials.GetMaterial(chams.OverlayMaterial);
	pRenderContext->DepthRange(0.0f, chams.IgnoreZ ? 0.2f : 1.f);
	F::Materials.SetColor(baseMaterial, chams.Color);
	if (baseMaterial)
		I::ModelRender->ForcedMaterialOverride(baseMaterial);
	ModelRender_DrawModelExecute->Original<void(__thiscall*)(CModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, reinterpret_cast<matrix3x4*>(&F::FakeAng.BoneMatrix));
	if (overlayMaterial)
	{
		F::Materials.SetColor(overlayMaterial, chams.OverlayColor);
		I::ModelRender->ForcedMaterialOverride(overlayMaterial);

		ModelRender_DrawModelExecute->Original<void(__thiscall*)(CModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, reinterpret_cast<matrix3x4*>(&F::FakeAng.BoneMatrix));
	}

	pRenderContext->DepthRange(0.0f, 1.f);
	I::ModelRender->ForcedMaterialOverride(nullptr);
	I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);

	I::RenderView->SetBlend(1.0f);
	return;
}

bool CDMEChams::Render(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	m_bRendering = false;
	if (!ShouldRun())
		return false;

	const auto ModelRender_DrawModelExecute = g_HookManager.GetMapHooks()["ModelRender_DrawModelExecute"];
	const auto& pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!ModelRender_DrawModelExecute || !pRenderContext)
		return false;

	CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(pInfo.m_nEntIndex);
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	//if (pEntity && !pEntity->ShouldDraw())
	//	return false;



	const int drawType = GetType(pInfo.m_nEntIndex);
	if (drawType == -1)
		return false;

	if (drawType == 0)
	{	// filter weapon draws
		std::string_view szModelName = I::ModelInfoClient->GetModelName(pInfo.m_pModel);
		if (!(szModelName.find("weapon") != std::string_view::npos
			&& szModelName.find("arrow") == std::string_view::npos
			&& szModelName.find("w_syringe") == std::string_view::npos
			&& szModelName.find("nail") == std::string_view::npos
			&& szModelName.find("shell") == std::string_view::npos
			&& szModelName.find("parachute") == std::string_view::npos
			&& szModelName.find("buffbanner") == std::string_view::npos
			&& szModelName.find("shogun_warbanner") == std::string_view::npos
			&& szModelName.find("targe") == std::string_view::npos // same as world model, can't filter
			&& szModelName.find("shield") == std::string_view::npos // same as world model, can't filter
			&& szModelName.find("repair_claw") == std::string_view::npos))
		{
			return false;
		}
	}

	if (drawType == 3)
	{	// don't interfere with ragdolls
		if (Vars::Visuals::Ragdolls::Type.Value)
		{
			if (Vars::Visuals::Ragdolls::EnemyOnly.Value && pEntity && pLocal && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
				return false;
			else
				return false;
		}
	}

	if (pEntity == pLocal)
		RenderFakeAng(pState, pInfo, pBoneToWorld);



	Chams_t chams = GetChamsType(drawType, pEntity);
	if (!chams.ChamsActive)
		return false;

	m_bRendering = true;

	auto baseMaterial = F::Materials.GetMaterial(chams.Material), overlayMaterial = F::Materials.GetMaterial(chams.OverlayMaterial);
	pRenderContext->DepthRange(0.0f, chams.IgnoreZ ? 0.2f : 1.f);
	F::Materials.SetColor(baseMaterial, chams.Color);
	if (baseMaterial)
		I::ModelRender->ForcedMaterialOverride(baseMaterial);
	ModelRender_DrawModelExecute->Original<void(__thiscall*)(CModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);
	if (overlayMaterial)
	{
		F::Materials.SetColor(overlayMaterial, chams.OverlayColor);
		I::ModelRender->ForcedMaterialOverride(overlayMaterial);

		ModelRender_DrawModelExecute->Original<void(__thiscall*)(CModelRender*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4*)>()(I::ModelRender, pState, pInfo, pBoneToWorld);
	}

	pRenderContext->DepthRange(0.0f, 1.f);
	I::RenderView->SetColorModulation(1.0f, 1.0f, 1.0f);
	I::RenderView->SetBlend(1.0f);
	I::ModelRender->ForcedMaterialOverride(nullptr);

	m_bRendering = false;

	return true;
}