#pragma once
#include "Vars.h"
//TODO: move this lol

__inline Color_t GetTeamColor(int iTeam, bool bOther)
{
	if (bOther)
	{
		if (const auto& pLocal = g_EntityCache.GetLocal())
		{
			if (pLocal->IsInValidTeam() && (iTeam == 2 || iTeam == 3))
				return pLocal->m_iTeamNum() == iTeam ? Vars::Colors::Team.Value : Vars::Colors::Enemy.Value;
		}
	}
	else {
		switch (iTeam)
		{
		case 2: return Vars::Colors::TeamRed.Value;
		case 3: return Vars::Colors::TeamBlu.Value;
		}
	}

	return { 255, 255, 255, 255 };
}

__inline Color_t GetEntityDrawColor(CBaseEntity* pEntity, bool enableOtherColors)
{
	Color_t out = GetTeamColor(pEntity->m_iTeamNum(), enableOtherColors);
	PlayerInfo_t info{}; I::EngineClient->GetPlayerInfo(pEntity->GetIndex(), &info);

	if (pEntity->IsPlayer())
	{
		if (g_EntityCache.GetLocal()->GetIndex() == pEntity->GetIndex())
			out = Vars::Colors::Local.Value;
		else if (g_EntityCache.IsFriend(pEntity->GetIndex()) || pEntity == g_EntityCache.GetLocal())
			out = Vars::Colors::Friend.Value;
		else if (G::IsIgnored(info.friendsID))
			out = Vars::Colors::Ignored.Value;
		else if (pEntity->IsCloaked())
			out = Vars::Colors::Cloak.Value;
		else if (pEntity->IsInvulnerable())
			out = Vars::Colors::Invulnerable.Value;
	}

	if (pEntity->GetIndex() == G::CurrentTargetIdx)
		out = Vars::Colors::Target.Value;

	return out;
}

__inline Color_t GetHealthColor(int nHP, int nMAXHP)
{
	nHP = std::max(0, std::min(nHP, nMAXHP));
	int r = std::min((510 * (nMAXHP - nHP)) / nMAXHP, 200);
	int g = std::min((510 * nHP) / nMAXHP, 200);
	return { static_cast<byte>(r), static_cast<byte>(g), 0, 255 };
}