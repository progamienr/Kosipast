#include "../Hooks.h"
#include "../../Features/Visuals/Chams/DMEChams.h"

MAKE_HOOK(CBaseClient_Disconnect, S::CBaseClient_Disconnect(), void, __fastcall,
	void* ecx, void* edx, const char* fmt, ...)
{
	F::DMEChams.DeleteMaterials();
	return Hook.Original<FN>()(ecx, edx, fmt);
}