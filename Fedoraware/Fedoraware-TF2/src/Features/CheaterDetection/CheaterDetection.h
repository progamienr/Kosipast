#pragma once
#include "../Feature.h"
#include "../Backtrack/Backtrack.h"

struct PlayerInfo
{
	bool bShouldScan = false;
	uint32_t friendsID = 0;
	std::string sName = "";

	int iDetections = 0;

	std::deque<int> vChokes = {};					// store last 3 choke counts
	bool bChoke = false;							// infract the user for choking?
	std::deque<std::pair<Vec3, bool>> vAngles = {};	// store last 3 angles & if damage was dealt
	bool bDamage = false;
	int iDuckSpeed = 0;								// how many times in a row a user has been detected for duck speed
};

class CCheaterDetection
{
	bool ShouldScan();

	bool InvalidPitch(CBaseEntity* pEntity);
	bool IsChoking(CBaseEntity* pEntity);
	bool IsFlicking(CBaseEntity* pEntity);
	bool IsDuckSpeed(CBaseEntity* pEntity);

	void Infract(CBaseEntity* pEntity, std::string sReason);

	std::unordered_map<CBaseEntity*, PlayerInfo> mData;

public:
	void Run();
	void Fill();

	void ReportChoke(CBaseEntity* pEntity, int iChoke);
	void ReportDamage(CGameEvent* pEvent);
	void Reset();
};

ADD_FEATURE(CCheaterDetection, CheaterDetection)