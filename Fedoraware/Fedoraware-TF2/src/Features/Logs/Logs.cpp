#include "Logs.h"

#include "../Vars.h"
#include "../Misc/Notifications/Notifications.h"
#include "../../Features/Visuals/Visuals.h"
#include "../Menu/Playerlist/PlayerUtils.h"

static std::string white({ '\x7', 'F', 'F', 'F', 'F', 'F', 'F' }); //FFFFFF
static std::string red({ '\x7', 'F', 'F', '3', 'A', '3', 'A' }); //FF3A3A
static std::string green({ '\x7', '3', 'A', 'F', 'F', '4', 'D' }); //3AFF4D
static std::string blue({ '\x7', '0', 'D', '9', '2', 'F', 'F' }); //0D92FF
static std::string yellow({ '\x7', 'C', '8', 'A', '9', '0', '0' }); //C8A900
static std::string gsred({ '\x7', '7', '5', '7', '5', '7', '5' }); //757575
static std::string gsgreen({ '\x7', 'B', '0', 'B', '0', 'B', '0' }); //b0b0b0
static std::string gsblue({ '\x7', '7', '6', '7', '6', '7', '6' }); //767676
static std::string gsyellow({ '\x7', 'C', 'A', 'C', 'A', 'C', 'A' }); //CACACA

void CLogs::OutputInfo(int flags, std::string name, std::string string, std::string cstring)
{
	if (flags & 1 << 0)
		F::Notifications.Add(string);
	if (flags & 1 << 1)
		I::ClientModeShared->m_pChatElement->ChatPrintf(0, std::format("{}{}\x1 {}", Vars::Menu::Theme::Accent.Value.to_hex(), Vars::Menu::CheatPrefix.Value, cstring).c_str());
	if (flags & 1 << 2)
		I::EngineClient->ClientCmd_Unrestricted(std::format("tf_party_chat \"{}\"", string).c_str());
	if (flags & 1 << 3)
		Utils::ConLog(name.c_str(), string.c_str(), Vars::Menu::Theme::Accent.Value);
}

// Event info
void CLogs::Event(CGameEvent* pEvent, const FNV1A_t uNameHash)
{
	if (uNameHash == FNV1A::HashConst("game_newmap"))
	{
		bTagsOnJoin = true;
		return;
	}

	if (!I::EngineClient->IsConnected() || !I::EngineClient->IsInGame())
		return;

	const auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal)
		return;

	// Voting
	if (uNameHash == FNV1A::HashConst("vote_cast"))
	{
		if (!(Vars::Logging::Logs.Value & 1 << 1))
			return;

		const int iIndex = pEvent->GetInt("entityid");
		const auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
		if (!pEntity || !pEntity->IsPlayer())
			return;

		const bool bVotedYes = pEvent->GetInt("vote_option") == 0;
		const bool bSameTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();

		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(iIndex, &pi))
			return;

		std::string string = std::format("{}{} voted {}", (bSameTeam ? "" : "[Enemy] "), (pi.name), (bVotedYes ? "Yes" : "No"));
		std::string cstring = std::format("{}{}{}\x1 voted {}{}", (bSameTeam ? "" : "[Enemy] "), (yellow), (pi.name), (bVotedYes ? green : red), (bVotedYes ? "Yes" : "No"));
		OutputInfo(Vars::Logging::VoteCast::LogTo.Value, "Vote Cast", string, cstring);
		return;
	}

	// Class change
	if (uNameHash == FNV1A::HashConst("player_changeclass"))
	{
		if (!(Vars::Logging::Logs.Value & 1 << 2))
			return;

		const int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		const auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
		if (!pEntity || iIndex == pLocal->GetIndex())
			return;

		const bool bSameTeam = pEntity->m_iTeamNum() == pLocal->m_iTeamNum();

		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(iIndex, &pi) || pi.fakeplayer)
			return; // dont spam chat by giving class changes for bots

		std::string string = std::format("{}{} changed class to {}", (bSameTeam ? "" : "[Enemy] "), (pi.name), (Utils::GetClassByIndex(pEvent->GetInt("class"))));
		std::string cstring = std::format("{}{}{}\x1 changed class to {}{}", (bSameTeam ? "" : "[Enemy] "), (yellow), (pi.name), (yellow), (Utils::GetClassByIndex(pEvent->GetInt("class"))));
		OutputInfo(Vars::Logging::ClassChange::LogTo.Value, "Class Change", string, cstring);
		return;
	}

	// Damage
	if (uNameHash == FNV1A::HashConst("player_hurt"))
	{
		if (!(Vars::Logging::Logs.Value & 1 << 3))
			return;

		const int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		const auto pEntity = I::ClientEntityList->GetClientEntity(iIndex);
		if (!pEntity || iIndex == pLocal->GetIndex())
			return;

		const int nAttacker = pEvent->GetInt("attacker");
		const int nHealth = pEvent->GetInt("health");
		const int nDamage = pEvent->GetInt("damageamount");
		const bool bCrit = pEvent->GetBool("crit");
		const int maxHealth = pEntity->GetMaxHealth();

		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(I::EngineClient->GetLocalPlayer(), &pi) || nAttacker != pi.userID ||
			!I::EngineClient->GetPlayerInfo(iIndex, &pi))
			return;

		std::string string = std::format("You hit {} for {} damage{}({} / {})", (pi.name), (nDamage), (bCrit ? " (crit) " : " "), (nHealth), (maxHealth));
		std::string cstring = std::format("You hit {}{}\x1 for {}{} damage{}{}({} / {})", (yellow), (pi.name), (red), (nDamage), (bCrit ? " (crit) " : " "), (yellow), (nHealth), (maxHealth));
		OutputInfo(Vars::Logging::Damage::LogTo.Value, "Damage", string, cstring);
		return;
	}

	// Tags (player join)
	if (uNameHash == FNV1A::HashConst("player_activate"))
	{
		if (!(Vars::Logging::Logs.Value & 1 << 5) || bTagsOnJoin)
			return;

		const int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		if (iIndex == pLocal->GetIndex())
			return;

		PlayerInfo_t pi{};
		if (!I::EngineClient->GetPlayerInfo(iIndex, &pi) || pi.fakeplayer)
			return;

		TagsOnJoin(pi.name, pi.friendsID);
		return;
	}
	// Tags (local join)
	if (uNameHash == FNV1A::HashConst("player_spawn"))
	{
		if (!(Vars::Logging::Logs.Value & 1 << 5) || !bTagsOnJoin)
			return;

		const int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		if (iIndex != pLocal->GetIndex())
			return;

		bTagsOnJoin = false;
		for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
		{
			PlayerInfo_t pi{};
			if (n == pLocal->GetIndex() || !I::EngineClient->GetPlayerInfo(n, &pi) || pi.fakeplayer)
				continue;

			TagsOnJoin(pi.name, pi.friendsID);
		}
	}
}

// Vote start
void CLogs::UserMessage(UserMessageType type, bf_read& msgData)
{
	if (type != VoteStart || !(Vars::Logging::Logs.Value & 1 << 0))
		return;

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal)
		return;

	const int team = msgData.ReadByte();
	const int voteID = msgData.ReadLong();
	const int caller = msgData.ReadByte();
	char reason[64], voteTarget[64];
	msgData.ReadString(reason, 64);
	msgData.ReadString(voteTarget, 64);
	const int target = static_cast<unsigned char>(msgData.ReadByte()) >> 1;
	const bool bSameTeam = team == pLocal->m_iTeamNum();

	PlayerInfo_t piTarget{}, piCaller{};
	if (!caller || !target || !I::EngineClient->GetPlayerInfo(target, &piTarget) || !I::EngineClient->GetPlayerInfo(caller, &piCaller))
		return;

	std::string string = std::format("{}{} called a vote on {}", (bSameTeam ? "" : "[Enemy] "), (piCaller.name), (piTarget.name));
	std::string cstring = std::format("{}{}{}\x1 called a vote on {}{}", (bSameTeam ? "" : "[Enemy] "), (yellow), (piCaller.name), (yellow), (piTarget.name));
	OutputInfo(Vars::Logging::VoteStart::LogTo.Value, "Vote Start", string, cstring);
}

// Cheat detection
void CLogs::CheatDetection(std::string name, std::string action, std::string reason)
{
	if (!(Vars::Logging::Logs.Value & 1 << 4))
		return;

	std::string string = std::format("{} {} for {}", (name), (action), (reason));
	std::string cstring = std::format("{}{}\x1 {} for {}{}", (yellow), (name), (action), (yellow), (reason));
	OutputInfo(Vars::Logging::CheatDetection::LogTo.Value, "Cheat Detection", string, cstring);
}

// Tags
void CLogs::TagsOnJoin(std::string name, uint32_t friendsID)
{
	const auto& vTags = G::PlayerTags[friendsID];
	std::vector<std::pair<std::string, std::string>> vColorsTags = {};
	for (auto& sTag : vTags)
	{
		PriorityLabel lbTag;
		if (!F::PlayerUtils.GetTag(sTag, &lbTag))
			continue;
		vColorsTags.push_back({ lbTag.Color.to_hex_alpha(), sTag });
	}

	std::string tagtext, ctagtext;
	switch (vColorsTags.size())
	{
	case 0: return;
	case 1:
	{
		auto& pColorTag = *vColorsTags.begin();
		tagtext = pColorTag.second;
		ctagtext = std::format("{}{}", pColorTag.first, pColorTag.second);
		break;
	}
	case 2:
	{
		auto& pColorTag1 = *vColorsTags.begin(), &pColorTag2 = *(vColorsTags.begin() + 1);
		tagtext = std::format("{} and ", pColorTag1.second, pColorTag2.second);
		ctagtext = std::format("{}{}\x1 and {}{}", pColorTag1.first, pColorTag1.second, pColorTag2.first, pColorTag2.second);
		break;
	}
	default:
	{
		for (auto it = vColorsTags.begin(); it != vColorsTags.end(); it++)
		{
			auto& pColorTag = *it;
			if (it + 1 != vColorsTags.end())
			{
				tagtext += std::format("{}, ", pColorTag.second);
				ctagtext += std::format("{}{}\x1, ", pColorTag.first, pColorTag.second);
			}
			else
			{
				tagtext += std::format("and {}", pColorTag.second);
				ctagtext += std::format("and {}{}", pColorTag.first, pColorTag.second);
			}
		}
	}
	}

	std::string string = std::format("{} has the {} {}", (name), (vColorsTags.size() == 1 ? "tag" : "tags"), (tagtext));
	std::string cstring = std::format("{}{}\x1 has the {} {}", (yellow), (name), (vColorsTags.size() == 1 ? "tag" : "tags"), (ctagtext));
	OutputInfo(Vars::Logging::Tags::LogTo.Value, "Tags", string, cstring);
}
void CLogs::TagsChanged(std::string name, std::string action, std::string color, std::string tag)
{
	if (!(Vars::Logging::Logs.Value & 1 << 5))
		return;

	std::string string = std::format("{} tag {} {} {}", (action), (tag), (action == "Added" ? "to" : "from"), (name));
	std::string cstring = std::format("{} tag {}{}\x1 {} {}{}", (action), (color), (tag), (action == "Added" ? "to" : "from"), (yellow), (name));
	OutputInfo(Vars::Logging::Tags::LogTo.Value, "Tags", string, cstring);
}