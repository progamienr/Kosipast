#include "../Hooks.h"

#include "../../Features/Visuals/Chams/Chams.h"
#include "../../Features/Visuals/Chams/DMEChams.h"
#include "../../Features/Visuals/Glow/Glow.h"

MAKE_HOOK(ModelRender_ForcedMaterialOverride, Utils::GetVFuncPtr(I::ModelRender, 1), void, __fastcall,
	void* ecx, void* edx, IMaterial* mat, EOverrideType type)
{
	if (!F::DMEChams.m_bRendering)
	{
		if (F::Glow.m_bRendering && !F::Glow.IsGlowMaterial(mat) || F::Chams.m_bRendering && !F::Chams.IsChamsMaterial(mat))
		{
			return;
		}
	}

	Hook.Original<FN>()(ecx, edx, mat, type);
}