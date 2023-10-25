#include "../Hooks.h"

#include "../../SDK/Main/CEconNotification.h"

MAKE_HOOK(NotificationQueue_Add, g_Pattern.Find(L"client.dll", L"55 8B EC 8B 0D ? ? ? ? 56 57 8B 01 8B 40 68 FF D0 8B 75 08 84 C0 75 09 80 BE ? ? ? ? ? 75 29"), int, __cdecl,
		  CEconNotification* pNotification)
{
	auto OnNotification = [](CEconNotification* pNotify)
	{
		if (!Vars::Misc::AutoAcceptItemDrops.Value)
			G::NotificationVector.clear();

		if (G::NotificationVector.empty())
			return;

		for (const auto& notification : G::NotificationVector)
		{
			if (notification == pNotify)
			{
				pNotify->Accept();
				pNotify->Trigger();
				pNotify->UpdateTick();
				pNotify->MarkForDeletion();
				G::NotificationVector.clear();
				return;
			}
		}
	};

	OnNotification(pNotification);
	return Hook.Original<FN>()(pNotification);
}