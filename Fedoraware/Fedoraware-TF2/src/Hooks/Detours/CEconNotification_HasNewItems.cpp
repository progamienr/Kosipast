#include "../Hooks.h"

#include "../../SDK/Main/CEconNotification.h"

MAKE_HOOK(CEconNotification_HasNewItems, S::CEconNotification_HasNewItems(), CEconNotification*, __fastcall,
	void* ecx, void* edx)
{
	auto vRet = Hook.Original<FN>()(ecx, edx);

	auto pNotification = static_cast<CEconNotification*>(vRet);
	G::NotificationVector.push_back(pNotification);

	return vRet;
}