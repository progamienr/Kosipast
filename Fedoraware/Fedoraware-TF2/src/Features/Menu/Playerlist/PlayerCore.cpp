#include "PlayerCore.h"

void CPlayerlistCore::Run()
{
	static Timer saveTimer{ };
	if (saveTimer.Run(1000))
	{
		LoadTags();
		SaveTags();
		LoadPlayers();
		SavePlayers();
	}
}

void CPlayerlistCore::LoadPlayers()
{
	if (!F::PlayerUtils.bLoadPlayers)
		return;

	try
	{
		if (std::filesystem::exists(g_CFG.GetConfigPath() + "\\Core\\Players.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(g_CFG.GetConfigPath() + "\\Core\\Players.json", readTree);
			G::PlayerTags.clear();

			for (auto& it : readTree)
			{
				uint32_t friendsID = std::stoi(it.first);

				for (const auto& [sTag, plTag] : F::PlayerUtils.vTags)
				{
					if (!plTag.Assignable)
						continue;

					if (auto _ = it.second.get_optional<int>(sTag)) {
						F::PlayerUtils.AddTag(friendsID, sTag, false);
					}
				}
			}
		}
		// support legacy format & convert over
		if (std::filesystem::exists(g_CFG.GetConfigPath() + "\\Core\\Playerlist.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(g_CFG.GetConfigPath() + "\\Core\\Playerlist.json", readTree);

			for (auto& it : readTree)
			{
				uint32_t friendsID = std::stoi(it.first);

				int iPriority = 2;
				if (auto getValue = it.second.get_optional<int>("Mode")) { iPriority = std::max(*getValue, 0); }

				if (iPriority == 4)
					F::PlayerUtils.AddTag(friendsID, "Cheater", false);
				if (iPriority == 1)
					F::PlayerUtils.AddTag(friendsID, "Ignored", false);
			}
		}

		F::PlayerUtils.bLoadPlayers = false;
	}
	catch (...) {}
}

void CPlayerlistCore::SavePlayers()
{
	if (!F::PlayerUtils.bSavePlayers)
		return;

	try
	{
		boost::property_tree::ptree writeTree;

		// Put map entries into ptree
		for (const auto& [friendsID, vTags] : G::PlayerTags)
		{
			// don't fill with pointless info
			if (!vTags.size())
				continue;

			boost::property_tree::ptree userTree;
			for (const auto& sTag : vTags)
				userTree.put(sTag, 0);

			writeTree.put_child(std::to_string(friendsID), userTree);
		}

		// Save the file
		write_json(g_CFG.GetConfigPath() + "\\Core\\Players.json", writeTree);

		F::PlayerUtils.bSavePlayers = false;
	}
	catch (...) {}
}

void CPlayerlistCore::LoadTags()
{
	if (!F::PlayerUtils.bLoadTags)
		return;

	try
	{
		if (std::filesystem::exists(g_CFG.GetConfigPath() + "\\Core\\Tags.json"))
		{
			boost::property_tree::ptree readTree;
			read_json(g_CFG.GetConfigPath() + "\\Core\\Tags.json", readTree);
			F::PlayerUtils.vTags = {
				{ "Default", { { 200, 200, 200, 255 }, 0, false, false, true } },
				{ "Ignored", { { 200, 200, 200, 255 }, -1, false, true, true } },
				{ "Cheater", { { 255, 100, 100, 255 }, 1, false, true, true } },
				{ "Friend", { { 100, 255, 100, 255 }, 0, true, false, true } }
			};

			for (auto& it : readTree)
			{
				std::string sTag = it.first;

				PriorityLabel plTag = {};
				if (const auto getChild = it.second.get_child_optional("Color")) { g_CFG.TreeToColor(*getChild, plTag.Color); }
				if (auto getValue = it.second.get_optional<int>("Priority")) { plTag.Priority = *getValue; }
				if (auto getValue = it.second.get_optional<bool>("Label")) { plTag.Label = *getValue; }

				F::PlayerUtils.vTags[sTag].Color = plTag.Color;
				F::PlayerUtils.vTags[sTag].Priority = plTag.Priority;
				F::PlayerUtils.vTags[sTag].Label = plTag.Label;
			}
		}

		F::PlayerUtils.bLoadTags = false;
	}
	catch (...) {}
}

void CPlayerlistCore::SaveTags()
{
	if (!F::PlayerUtils.bSaveTags)
		return;

	try
	{
		boost::property_tree::ptree writeTree;

		// Put map entries into ptree
		for (const auto& [sTag, plTag] : F::PlayerUtils.vTags)
		{
			boost::property_tree::ptree tagTree;
			tagTree.put_child("Color", g_CFG.ColorToTree(plTag.Color));
			tagTree.put("Priority", plTag.Priority);
			tagTree.put("Label", plTag.Label);

			writeTree.put_child(sTag, tagTree);
		}

		// Save the file
		write_json(g_CFG.GetConfigPath() + "\\Core\\Tags.json", writeTree);

		F::PlayerUtils.bSaveTags = false;
	}
	catch (...) {}
}