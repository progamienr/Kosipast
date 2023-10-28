#include "../Hooks.h"
#include "../../Features/Color.h"

namespace S
{
    MAKE_SIGNATURE(C_TFPlayer_Resource_Call, CLIENT_DLL, "33 C9 89 45 ? 89 4D ? 85 C0 75", 0x0);
}

int iCurPlayer;
unsigned char _color[4];

__inline Color_t GetScoreboardColor(CBaseEntity* pEntity, bool enableOtherColors)
{
    Color_t out = { 0, 0, 0, 0 };
    PlayerInfo_t info{}; I::EngineClient->GetPlayerInfo(pEntity->GetIndex(), &info);

    const auto& pLocal = g_EntityCache.GetLocal();

    if (pEntity->IsPlayer() && pLocal)
    {
        if (pLocal->GetIndex() == pEntity->GetIndex())
            out = Vars::Colors::Local.Value;
        else if (g_EntityCache.IsFriend(pEntity->GetIndex()) || pEntity == pLocal)
            out = Vars::Colors::Friend.Value;
        else if (G::IsIgnored(info.friendsID))
            out = Vars::Colors::Ignored.Value;
    }
    return out;
}

MAKE_HOOK(C_TFPlayer_Resource_GetPlayerConnectionState, S::C_TFPlayer_Resource_GetPlayerConnectionState(), MM_PlayerConnectionState_t, __fastcall,
    void* ecx, void* edx, int iIndex)
{
    const auto result = Hook.Original<FN>()(ecx, edx, iIndex);

    if (result != MM_WAITING_FOR_PLAYER)
    {
        auto retaddr = reinterpret_cast<DWORD>(_ReturnAddress());
        static auto call = S::C_TFPlayer_Resource_Call();
        if (retaddr != call)
            return result;

        iCurPlayer = iIndex;
    }
    else
    {
        iCurPlayer = 0;
    }

    return result;
}

MAKE_HOOK(C_PlayerResource_GetTeamColor, S::C_PlayerResource_GetTeamColor(), unsigned char*, __fastcall,
    void* ecx, void* edx, int index)
{
    if (index < 0 || index > I::EngineClient->GetMaxClients() || !iCurPlayer || !Vars::Visuals::ScoreboardColors.Value ||
        Vars::Visuals::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
    {
        return Hook.Original<FN>()(ecx, edx, index);
    }

    CBaseEntity* ent = I::ClientEntityList->GetClientEntity(iCurPlayer);
    if (!ent || !ent->IsAlive() || !ent->IsPlayer())
        return Hook.Original<FN>()(ecx, edx, index);

    const Color_t cReturn = GetScoreboardColor(ent, Vars::ESP::Main::EnableTeamEnemyColors.Value);
    if (!cReturn.a)
        return Hook.Original<FN>()(ecx, edx, index);

    _color[0] = cReturn.r; _color[1] = cReturn.g; _color[2] = cReturn.b; _color[3] = 255;    //  force alpha
    return _color;
}