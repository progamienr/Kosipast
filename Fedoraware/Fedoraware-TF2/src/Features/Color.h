#pragma once
#include "Vars.h"
#include "Menu/Playerlist/PlayerUtils.h"

__inline Color_t GetTeamColor(int iLocalTeam, int iTargetTeam, bool bOther)
{
	if (bOther)
		return iLocalTeam == iTargetTeam ? Vars::Colors::Team.Value : Vars::Colors::Enemy.Value;
	else
	{
		switch (iTargetTeam)
		{
		case 2: return Vars::Colors::TeamRed.Value;
		case 3: return Vars::Colors::TeamBlu.Value;
		}
	}

	return { 255, 255, 255, 255 };
}

__inline Color_t GetEntityDrawColor(CBaseEntity* pLocal, CBaseEntity* pEntity, bool enableOtherColors)
{
	Color_t out = GetTeamColor(pLocal->m_iTeamNum(), pEntity->m_iTeamNum(), enableOtherColors);

	if (pEntity->IsPlayer())
	{
		PlayerInfo_t pi{}; bool bTagColor = false; Color_t cTagColor;
		if (I::EngineClient->GetPlayerInfo(pEntity->GetIndex(), &pi))
		{
			std::string _; PriorityLabel_t plTag;
			if (bTagColor = F::PlayerUtils.GetSignificantTag(pi.friendsID, &_, &plTag))
				cTagColor = plTag.Color;
		}

		if (pLocal == pEntity)
			out = Vars::Colors::Local.Value;
		else if (g_EntityCache.IsSteamFriend(pEntity->GetIndex()))
			out = F::PlayerUtils.mTags["Friend"].Color;
		else if (bTagColor)
			out = cTagColor;
		else if (pEntity->IsCloaked())
			out = Vars::Colors::Cloak.Value;
		else if (pEntity->IsInvulnerable())
			out = Vars::Colors::Invulnerable.Value;
	}

	if (pEntity->GetIndex() == G::CurrentTarget.first && abs(G::CurrentTarget.second - I::GlobalVars->tickcount) < 32)
		out = Vars::Colors::Target.Value;

	return out;
}