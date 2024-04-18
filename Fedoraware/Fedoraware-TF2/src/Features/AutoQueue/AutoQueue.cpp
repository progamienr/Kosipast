#include "AutoQueue.h"

void CAutoQueue::Run()
{
	if (Vars::Misc::Queueing::AutoCasualQueue.Value)
	{
		const bool bInQueueForMatchGroup = I::TFPartyClient->BInQueueForMatchGroup(k_eTFMatchGroup_Casual_Default);
		if (!bInQueueForMatchGroup)
		{
			I::TFPartyClient->LoadSavedCasualCriteria();
			I::TFPartyClient->RequestQueueForMatch(k_eTFMatchGroup_Casual_Default);
		}
	}
}
