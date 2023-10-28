#include "../Hooks.h"

#include "../../SDK/Main/CEconNotification.h"

MAKE_HOOK(NotificationQueue_Add, S::NotificationQueue_Add(), int, __cdecl,
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