#include "AutoGlobal.h"

#include "../../Vars.h"
#include "../../Menu/Playerlist/PlayerUtils.h"

bool CAutoGlobal::IsKeyDown()
{
	switch (Vars::Auto::Global::AutoKey.Value)
	{
		case 0x0: return true;
		default: return F::KeyHandler.Down(Vars::Auto::Global::AutoKey.Value);
	}
}

bool CAutoGlobal::ShouldIgnore(CBaseEntity* pTarget)
{
	PlayerInfo_t pi{};
	if (!pTarget) return true;
	if (pTarget->GetDormant()) return true;
	if (!I::EngineClient->GetPlayerInfo(pTarget->GetIndex(), &pi)) return true;
	if (F::PlayerUtils.IsIgnored(pi.friendsID)) return true;
	if (Vars::Auto::Global::IgnoreOptions.Value & (1 << 5) && pTarget->IsPlayer() && pTarget->IsDisguised()) return true;
	if (Vars::Auto::Global::IgnoreOptions.Value & (1 << 4) && pTarget->IsPlayer() && (pTarget->m_flSimulationTime() == pTarget->m_flOldSimulationTime())) return true;
	if (Vars::Auto::Global::IgnoreOptions.Value & (1 << 3) && pTarget->IsTaunting()) return true;
	if (Vars::Auto::Global::IgnoreOptions.Value & (1 << 2) && g_EntityCache.IsFriend(pTarget->GetIndex())) return true;
	if (Vars::Auto::Global::IgnoreOptions.Value & (1 << 1) && pTarget->IsInvisible()) return true;
	if (Vars::Auto::Global::IgnoreOptions.Value & (1 << 0) && pTarget->IsInvulnerable()) return true;

	return false;
}
