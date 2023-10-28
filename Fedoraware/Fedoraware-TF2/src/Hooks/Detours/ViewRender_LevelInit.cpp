#include "../Hooks.h"

#include "../../Features/Visuals/Visuals.h"
#include "../../Features/Visuals/Chams/DMEChams.h"
#include "../../Features/Visuals/Glow/Glow.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/TickHandler/TickHandler.h"
#include "../../Features/AntiHack/CheaterDetection.h"

#include "../../Features/Misc/Misc.h"

MAKE_HOOK(ViewRender_LevelInit, Utils::GetVFuncPtr(I::ViewRender, 1), void, __fastcall,
	void* ecx, void* edx)
{
	F::Visuals.StoreMaterialHandles();
	F::Visuals.OverrideWorldTextures();
	
	F::DMEChams.CreateMaterials();
	F::Glow.CreateMaterials();

	F::Backtrack.Restart();
	F::Ticks.Reset();
	F::BadActors.OnLoad();

	G::NextSafeTick = 0;

	Hook.Original<FN>()(ecx, edx);

	F::Visuals.ModulateWorld();
}