#include "../Hooks.h"
#include "../../Features/Visuals/Materials/Materials.h"

MAKE_HOOK(CMaterial_Uncache, S::CMaterial_Uncache(), void, __fastcall,
	IMaterial* ecx, void* edx, bool bPreserveVars)
{
	if (ecx)
	{
		if (std::ranges::find(F::Materials.v_MatList, ecx) != F::Materials.v_MatList.end())
			return;
	}

	Hook.Original<FN>()(ecx, edx, bPreserveVars);
}