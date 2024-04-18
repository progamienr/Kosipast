#pragma once
#include <bitset>

#include "../BaseEntity/BaseEntity.h"
#include "../BaseCombatWeapon/BaseCombatWeapon.h"
#include "../PlayerResource/PlayerResource.h"

enum struct EGroupType
{
	PLAYERS_ALL, PLAYERS_ENEMIES, PLAYERS_TEAMMATES,
	BUILDINGS_ALL, BUILDINGS_ENEMIES, BUILDINGS_TEAMMATES,
	WORLD_HEALTH, WORLD_AMMO, WORLD_PROJECTILES, WORLD_NPC, WORLD_BOMBS, WORLD_MONEY, WORLD_SPELLBOOK, WORLD_GARGOYLE,
	LOCAL_STICKIES, LOCAL_FLARES, MISC_DOTS
};

class CEntityCache
{
	CBaseEntity* m_pLocal = nullptr;
	CBaseCombatWeapon* m_pLocalWeapon = nullptr;
	CBaseEntity* m_pObservedTarget = nullptr;
	CTFPlayerResource* m_pPlayerResource = nullptr;

	std::unordered_map<EGroupType, std::vector<CBaseEntity*>> m_mGroups = {};
	std::unordered_map<uint32_t, int> m_mIDIndex = {};
	std::bitset<128> m_bFriends;
	void UpdateFriends();

public:
	void Fill();
	void Clear();

	CBaseEntity* GetLocal() { return m_pLocal; }
	CBaseCombatWeapon* GetWeapon() { return m_pLocalWeapon; }
	CBaseEntity* GetObservedTarget() { return m_pObservedTarget; }
	CTFPlayerResource* GetPR() { return m_pPlayerResource; }

	const std::vector<CBaseEntity*>& GetGroup(const EGroupType& Group) { return m_mGroups[Group]; }
	bool IsSteamFriend(int entIdx) { return m_bFriends[entIdx]; }
	bool IsSteamFriend(uint32_t friendsID) { return m_bFriends[m_mIDIndex[friendsID]]; }
};

inline CEntityCache g_EntityCache;