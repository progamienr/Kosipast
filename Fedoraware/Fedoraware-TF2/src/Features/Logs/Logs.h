#pragma once

#include "../Feature.h"

class CLogs
{
	void OutputInfo(int flags, std::string name, std::string string, std::string cstring);

	void TagsOnJoin(std::string name, uint32_t friendsID);
	bool bTagsOnJoin = false;

public:
	void Event(CGameEvent* pEvent, FNV1A_t uNameHash);
	void UserMessage(UserMessageType type, bf_read& msgData);
	void CheatDetection(std::string name, std::string action, std::string reason);
	void TagsChanged(std::string name, std::string action, std::string color, std::string tag);

	std::unordered_map<int, bool> KnownBots;
};

ADD_FEATURE(CLogs, Logs)