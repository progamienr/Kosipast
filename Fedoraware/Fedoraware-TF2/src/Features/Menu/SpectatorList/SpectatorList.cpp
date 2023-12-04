#include "SpectatorList.h"

#include "../../Vars.h"
#include "../Menu.h"

bool CSpectatorList::GetSpectators(CBaseEntity* pLocal)
{
	G::FirstSpectated = G::ThirdSpectated = false;

	Spectators.clear();

	for (const auto& pTeammate : g_EntityCache.GetGroup(EGroupType::PLAYERS_TEAMMATES))
	{
		CBaseEntity* pObservedPlayer = I::ClientEntityList->GetClientEntityFromHandle(pTeammate->m_hObserverTarget());

		if (pTeammate && !pTeammate->IsAlive() && pObservedPlayer == pLocal)
		{
			std::wstring szMode;
			switch (pTeammate->m_iObserverMode())
			{
				case OBS_MODE_FIRSTPERSON:
				{
					szMode = L"1st";
					G::FirstSpectated = true;
					break;
				}
				case OBS_MODE_THIRDPERSON:
				{
					szMode = L"3rd";
					G::ThirdSpectated = true;
					break;
				}
				default: 
					continue;
			}

			int respawnIn = 0; float respawnTime = 0;
			if (CTFPlayerResource* pResource = g_EntityCache.GetPR())
			{
				respawnTime = pResource->GetNextRespawnTime(pTeammate->GetIndex());
				respawnIn = std::max(int(respawnTime - I::GlobalVars->curtime), 0);
			}
			bool respawnTimeIncreased = false;
			// theoretically the respawn times could change without extendfreeze but oh well
			if (RespawnCache.find(pTeammate->GetIndex()) == RespawnCache.end())
			{
				RespawnCache[pTeammate->GetIndex()] = respawnTime;
			}
			if (RespawnCache[pTeammate->GetIndex()] + 4.9f < respawnTime)
			{
				respawnTimeIncreased = true;
				RespawnCache[pTeammate->GetIndex()] = -1.f;
			}

			PlayerInfo_t playerInfo{ };
			if (I::EngineClient->GetPlayerInfo(pTeammate->GetIndex(), &playerInfo))
			{
				Spectators.push_back({
					Utils::ConvertUtf8ToWide(playerInfo.name), szMode, respawnIn, respawnTimeIncreased, g_EntityCache.IsFriend(pTeammate->GetIndex()),
					pTeammate->m_iTeamNum(), pTeammate->GetIndex()
				});
			}
		}
		else
		{
			auto iter = RespawnCache.find(pTeammate->GetIndex());
			if (iter != RespawnCache.end())
				RespawnCache.erase(iter);
		}
	}

	return !Spectators.empty();
}

void CSpectatorList::Run()
{
	if (!(Vars::Menu::Indicators.Value & (1 << 2)))
	{
		RespawnCache.clear();
		return;
	}

	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		if (!pLocal->IsAlive() || !GetSpectators(pLocal)) { return; }

		int x = Vars::Menu::SpectatorsDisplay.Value.x;
		int iconOffset = 0;
		int y = Vars::Menu::SpectatorsDisplay.Value.y + 8;

		EStringAlign align = ALIGN_CENTERHORIZONTAL;
		if (x <= 100)
		{
			iconOffset = 36;
			x += 8;
			align = ALIGN_DEFAULT;
		}
		else if (x >= g_ScreenSize.w - 200)
		{
			x += 92;
			align = ALIGN_REVERSE;
		}
		else
		{
			iconOffset = 16;
			x += 50;
		}

		//if (!Vars::Menu::SpectatorAvatars.Value)
			iconOffset = 0;

		g_Draw.String(g_Draw.GetFont(FONT_INDICATORS), x, y, Vars::Menu::Theme::Accent.Value, align, L"Spectating You:");

		for (const auto& Spectator : Spectators)
		{
			y += g_Draw.GetFont(FONT_INDICATORS).nTall;

			/*
			if (Vars::Visuals::SpectatorAvatars.Value)
			{
				int w, h;

				I::VGuiSurface->GetTextSize(g_Draw.GetFont(FONT_MENU).dwFont,
					(Spectator.Name + Spectator.Mode + std::to_wstring(Spectator.RespawnIn) + std::wstring{L" -  (respawn s)"}).c_str(), w, h);
				switch (align)
				{
				case ALIGN_DEFAULT: { w = 0; break; }
				case ALIGN_CENTERHORIZONTAL: { w /= 2; break; }
				}

				PlayerInfo_t pi{};
				if (!I::EngineClient->GetPlayerInfo(Spectator.Index, &pi)) { continue; }

				g_Draw.Avatar(x - w - (36 - iconOffset), y, 24, 24, pi.friendsID);
				// center - half the width of the string
				y += 6;
			}
			*/

			Color_t color = { 255, 255, 255, 255 };
			if (Spectator.Mode == std::wstring{L"1st"})
				color = { 255, 200, 127, 255 };
			if (Spectator.RespawnTimeIncreased)
				color = { 255, 100, 100, 255 };
			if (Spectator.IsFriend)
				color = { 200, 255, 200, 255 };
			g_Draw.String(g_Draw.GetFont(FONT_INDICATORS), x + iconOffset, y, color, align,
				L"%ls - %ls (respawn %ds)", Spectator.Name.data(), Spectator.Mode.data(), Spectator.RespawnIn);
		}
	}
}
