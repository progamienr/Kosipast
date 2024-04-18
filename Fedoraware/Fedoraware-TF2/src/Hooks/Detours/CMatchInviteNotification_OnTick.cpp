#include "../Hooks.h"

MAKE_HOOK(CMatchInviteNotification_OnTick, S::CMatchInviteNotification_OnTick(), void, __fastcall,
	void* ecx, void* edx)
{
	static auto CMatchInviteNotification_AcceptMatch = reinterpret_cast<void(__thiscall*)(void*)>(S::CMatchInviteNotification_AcceptMatch());

	if (Vars::Misc::Queueing::FreezeQueue.Value)
		*reinterpret_cast<double*>(reinterpret_cast<DWORD>(ecx) + 424) = 0.0;

	Hook.Original<FN>()(ecx, edx);
}