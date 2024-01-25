#include "PlayerUtils.h"
#include "../../Vars.h"
#include "../../Color.h"
#include "../../Logs/Logs.h"

uint32_t GetFriendsID(int iIndex)
{
	PlayerInfo_t pi{};
	if (I::EngineClient->GetPlayerInfo(iIndex, &pi))
		return pi.friendsID;
	return 0;
}

bool CPlayerlistUtils::GetTag(std::string sTag, PriorityLabel* plTag)
{
	if (sTag == "")
		return false;

	const auto find = vTags.find(sTag);
	if (find != vTags.end())
	{
		*plTag = (*find).second;
		return true;
	}

	return false;
}



void CPlayerlistUtils::AddTag(uint32_t friendsID, std::string sTag, bool bSave, std::string sName)
{
	if (!friendsID)
		return;

	if (!HasTag(friendsID, sTag))
	{
		G::PlayerTags[friendsID].push_back(sTag);
		bSavePlayers = bSave;
		if (sName != "")
		{
			PriorityLabel plTag;
			if (GetTag(sTag, &plTag))
				F::Logs.TagsChanged(sName, "Added", plTag.Color.to_hex_alpha(), sTag);
		}
	}
}
void CPlayerlistUtils::AddTag(int iIndex, std::string sTag, bool bSave, std::string sName)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		AddTag(friendsID, sTag, bSave, sName);
}

void CPlayerlistUtils::RemoveTag(uint32_t friendsID, std::string sTag, bool bSave, std::string sName)
{
	if (!friendsID)
		return;

	auto& vTags = G::PlayerTags[friendsID];
	for (auto it = vTags.begin(); it != vTags.end(); it++)
	{
		if (*it == sTag)
		{
			vTags.erase(it);
			bSavePlayers = bSave;
			if (sName != "")
			{
				PriorityLabel plTag;
				if (GetTag(sTag, &plTag))
					F::Logs.TagsChanged(sName, "Removed", plTag.Color.to_hex_alpha(), sTag);
			}
			break;
		}
	}
}
void CPlayerlistUtils::RemoveTag(int iIndex, std::string sTag, bool bSave, std::string sName)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		RemoveTag(friendsID, sTag, bSave, sName);
}

bool CPlayerlistUtils::HasTags(uint32_t friendsID)
{
	if (!friendsID)
		return false;

	return G::PlayerTags[friendsID].size();
}
bool CPlayerlistUtils::HasTags(int iIndex)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return HasTags(friendsID);
	return false;
}

bool CPlayerlistUtils::HasTag(uint32_t friendsID, std::string sTag)
{
	if (!friendsID)
		return false;

	for (const auto& _sTag : G::PlayerTags[friendsID])
	{
		if (sTag == _sTag)
			return true;
	}
	return false;
}
bool CPlayerlistUtils::HasTag(int iIndex, std::string sTag)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return HasTag(friendsID, sTag);
	return false;
}



int CPlayerlistUtils::GetPriority(uint32_t friendsID)
{
	const int iDefault = vTags["Default"].Priority;
	if (!friendsID)
		return iDefault;

	if (HasTag(friendsID, "Ignored"))
		return vTags["Ignored"].Priority;

	std::vector<int> vPriorities;
	for (const auto& sTag : G::PlayerTags[friendsID])
	{
		PriorityLabel plTag;
		if (F::PlayerUtils.GetTag(sTag, &plTag) && !plTag.Label)
			vPriorities.push_back(plTag.Priority);
	}
	if (Utils::IsSteamFriend(friendsID))
	{
		auto& plTag = vTags["Friend"];
		if (!plTag.Label)
			vPriorities.push_back(plTag.Priority);
	}

	if (vPriorities.size())
	{
		std::sort(vPriorities.begin(), vPriorities.end(), std::greater<int>());
		return *vPriorities.begin();
	}
	return iDefault;
}
int CPlayerlistUtils::GetPriority(int iIndex)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return GetPriority(friendsID);
	return vTags["Default"].Priority;
}

bool CPlayerlistUtils::GetSignificantTag(uint32_t friendsID, std::string* sTag, PriorityLabel* plTag, int iMode)
{
	if (!friendsID)
		return false;

	std::vector<std::pair<std::string, PriorityLabel>> vLabels;
	if (!iMode || iMode == 1)
	{
		if (HasTag(friendsID, "Ignored"))
		{
			*sTag = "Ignored";
			*plTag = vTags["Ignored"];
			return true;
		}

		for (const auto& _sTag : G::PlayerTags[friendsID])
		{
			PriorityLabel _plTag;
			if (F::PlayerUtils.GetTag(_sTag, &_plTag) && !_plTag.Label)
				vLabels.push_back({ _sTag, _plTag });
		}
		if (Utils::IsSteamFriend(friendsID))
		{
			auto& _plTag = vTags["Friend"];
			if (!_plTag.Label)
				vLabels.push_back({ "Friend", _plTag });
		}
	}
	if ((!iMode || iMode == 2) && !vLabels.size())
	{
		for (const auto& _sTag : G::PlayerTags[friendsID])
		{
			PriorityLabel _plTag;
			if (F::PlayerUtils.GetTag(_sTag, &_plTag) && _plTag.Label)
				vLabels.push_back({ _sTag, _plTag });
		}
		if (Utils::IsSteamFriend(friendsID))
		{
			auto& _plTag = vTags["Friend"];
			if (_plTag.Label)
				vLabels.push_back({ "Friend", _plTag });
		}
	}
	if (!vLabels.size())
		return false;

	std::sort(vLabels.begin(), vLabels.end(), [&](const auto& a, const auto& b) -> bool
		{
			// sort by priority if unequal
			if (a.second.Priority != b.second.Priority)
				return a.second.Priority > b.second.Priority;

			return a.first < b.first;
		});
	*sTag = (*vLabels.begin()).first;
	*plTag = (*vLabels.begin()).second;
	return true;
}
bool CPlayerlistUtils::GetSignificantTag(int iIndex, std::string* sTag, PriorityLabel* plTag, int iMode)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return GetSignificantTag(friendsID, sTag, plTag, iMode);
	return false;
}

bool CPlayerlistUtils::IsIgnored(uint32_t friendsID)
{
	if (!friendsID)
		return false;

	const int iPriority = GetPriority(friendsID);
	const int iIgnored = vTags["Ignored"].Priority;
	return iPriority <= iIgnored;
}
bool CPlayerlistUtils::IsIgnored(int iIndex)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return IsIgnored(friendsID);
	return false;
}

bool CPlayerlistUtils::IsFriend(int iIndex)
{
	if (const uint32_t friendsID = GetFriendsID(iIndex))
		return Utils::IsSteamFriend(friendsID);
	return false;
}



void CPlayerlistUtils::UpdatePlayers()
{
	static Timer updateTimer{};
	if (updateTimer.Run(1000))
	{
		std::lock_guard lock(mutex);
		vPlayerCache.clear();

		const auto& pr = g_EntityCache.GetPR();
		if (!pr)
			return;

		for (int i = 1; i <= I::GlobalVars->maxclients; i++)
		{
			if (!pr->GetValid(i) || !pr->GetConnected(i))
				continue;

			bool bFake = true, bFriend = false;
			PlayerInfo_t pi{};
			if (I::EngineClient->GetPlayerInfo(i, &pi))
			{
				bFake = pi.fakeplayer;
				bFriend = Utils::IsSteamFriend(pi.friendsID);
			}

			vPlayerCache.push_back({
				pr->GetPlayerName(i),
				pr->GetAccountID(i),
				pr->GetUserID(i),
				pr->GetTeam(i),
				pr->GetClass(i),
				pr->IsAlive(i),
				i == I::EngineClient->GetLocalPlayer(),
				bFriend,
				bFake
			});
		}
	}
}


/*
void CPlayerlistUtils::Render()
{
	const auto winSize = ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
	auto winPos = ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);

	// Check if we are in-game
	if (!I::EngineClient->IsInGame())
	{
		ImGui::Dummy({ 0, 50 });
		static ImVec2 fontSize = ImGui::CalcTextSize("Not ingame");
		ImGui::SameLine(ImGui::GetWindowSize().x / 2 - fontSize.x + (fontSize.x / 2));
		ImGui::Text("Not ingame");
	}
	else
	{
		const int columnCount = Vars::AntiHack::Resolver::Resolver.Value ? 6 : 4;
		if (ImGui::BeginTable("Playerlist", columnCount,
			ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable))
		{
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Class");
			ImGui::TableSetupColumn("Health");
			ImGui::TableSetupColumn("Priority");
			if (Vars::AntiHack::Resolver::Resolver.Value)
			{
				ImGui::TableSetupColumn("Pitch");
				ImGui::TableSetupColumn("Yaw");
			}
			ImGui::TableHeadersRow();

			std::lock_guard lock(mutex);

			for (const auto& [Team, Player] : PlayerCache)
			{
				ImGui::TableNextRow();

				ImGui::PushID(ImGui::GetCurrentTable()->CurrentRow);

				for (int column = 0; column < columnCount; column++)
				{
					ImGui::TableSetColumnIndex(column);

					//// don't show priority or resolver for bots
					//if (Player.FakePlayer && column > 2)
					//{
					//	continue;
					//}

					switch (column)
					{
					case 0: // Name
						ImGui::PushStyleColor(ImGuiCol_Text, ImColor(Player.Color.r, Player.Color.g, Player.Color.b, Player.Alive ? Player.Color.a : 127).Value);
						ImGui::Text("%s", Player.Name ? Player.Name : "N/A");
						ImGui::PopStyleColor();
						break;
					case 1: // Class
						ImGui::Text("%s", Utils::GetClassByIndex(Player.Class));
						break;
					case 2: // Health
						ImGui::Text("%i/%i", Player.Health, Player.MaxHealth);
						break;
					case 3: // Priority
						if (ImGui::Selectable(priorityModes[G::PlayerPriority[Player.FriendsID].Mode]))
							ImGui::OpenPopup("priority_popup");

						if (ImGui::BeginPopup("priority_popup"))
						{
							for (int i = 0; i < IM_ARRAYSIZE(priorityModes); i++)
							{
								if (ImGui::MenuItem(priorityModes[i]))
								{
									G::PlayerPriority[Player.FriendsID].Mode = i;
									ShouldSave = true;
								}
							}

							ImGui::EndPopup();
						}
						break;
					case 4: // Pitch resolver
						if (ImGui::Selectable(resolveListPitch[F::Resolver.mResolverMode[Player.FriendsID].first]))
							ImGui::OpenPopup("pitch_popup");

						if (ImGui::BeginPopup("pitch_popup"))
						{
							for (int i = 0; i < IM_ARRAYSIZE(resolveListPitch); i++)
							{
								if (ImGui::MenuItem(resolveListPitch[i]))
								{
									F::Resolver.mResolverMode[Player.FriendsID].first = i;
								}
							}

							ImGui::EndPopup();
						}
						break;
					case 5: // Yaw resolver
						ImGui::PushID(1); // in case of m_Yaw = m_Pitch
						if (ImGui::Selectable(resolveListYaw[F::Resolver.mResolverMode[Player.FriendsID].second]))
							ImGui::OpenPopup("yaw_popup");

						if (ImGui::BeginPopup("yaw_popup"))
						{
							for (int i = 0; i < IM_ARRAYSIZE(resolveListYaw); i++)
							{
								if (ImGui::MenuItem(resolveListYaw[i]))
								{
									F::Resolver.mResolverMode[Player.FriendsID].second = i;
								}
							}

							ImGui::EndPopup();
						}
						ImGui::PopID();

						break;
					}
				}

				ImGui::SameLine();
				ImGui::Selectable("##contextmenu", false, ImGuiSelectableFlags_SpanAllColumns);

				if (!Player.FakePlayer && ImGui::BeginPopupContextItem())
				{
					if (ImGui::Selectable("Profile"))
						g_SteamInterfaces.Friends->ActivateGameOverlayToUser("steamid", CSteamID(0x0110000100000000ULL + Player.FriendsID));

					if (ImGui::Selectable("Votekick"))
						I::EngineClient->ClientCmd_Unrestricted(std::format("callvote kick {}", Player.UserID).c_str());

					ImGui::EndPopup();
				}

				ImGui::PopID();
			}

			ImGui::EndTable();
		}
	}
}
*/