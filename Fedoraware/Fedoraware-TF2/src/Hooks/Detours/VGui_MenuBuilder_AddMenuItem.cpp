#include "../Hooks.h"

//i stole this from mfed :)

namespace S
{
    MAKE_SIGNATURE(CVoiceStatus_IsPlayerBlocked_Call, CLIENT_DLL, "84 C0 BA ? ? ? ? 68", 0x0);
    MAKE_SIGNATURE(VGui_MenuBuilder_AddMenuItem_Call, CLIENT_DLL, "8B 0D ? ? ? ? 8D 55 ? 52 8D 55 ? 52 8B 01 FF 50 ? FF 75 ? 8B 8E ? ? ? ? FF 75 ? E8", 0x0);
}

static int PlayerIndex;
static uint32 CurrentFriendsID;

MAKE_HOOK(CVoiceStatus_IsPlayerBlocked, S::CVoiceStatus_IsPlayerBlocked(), bool, __fastcall,
    void* ecx, void* edx, int playerIndex)
{
    if (!Vars::Misc::ScoreboardPlayerlist.Value) { return Hook.Original<FN>()(ecx, edx, playerIndex); }

    auto retaddr = DWORD(_ReturnAddress());

    static auto CVoiceStatus_IsPlayerBlocked_Call = S::CVoiceStatus_IsPlayerBlocked_Call();
    if (retaddr == CVoiceStatus_IsPlayerBlocked_Call)
    {
        PlayerIndex = playerIndex;
    }

    return Hook.Original<FN>()(ecx, edx, playerIndex);
}

MAKE_HOOK(VGui_MenuBuilder_AddMenuItem, S::VGui_MenuBuilder_AddMenuItem(), void*, __fastcall,
    void* ecx, void* edx, const char* pszButtonText, const char* pszCommand, const char* pszCategoryName)
{
    if (!Vars::Misc::ScoreboardPlayerlist.Value) { return Hook.Original<FN>()(ecx, edx, pszButtonText, pszCommand, pszCategoryName); }

    auto retaddr = DWORD(_ReturnAddress());

    static auto VGui_MenuBuilder_AddMenuItem_Call = S::VGui_MenuBuilder_AddMenuItem_Call();
    if (retaddr == VGui_MenuBuilder_AddMenuItem_Call && PlayerIndex != -1)
    {
        auto ret = Hook.Original<FN>()(ecx, edx, pszButtonText, pszCommand, pszCategoryName);

        PlayerInfo_t pi;

        if (I::EngineClient->GetPlayerInfo(PlayerIndex, &pi))
        {
            CurrentFriendsID = pi.friendsID;

            bool ignored = G::PlayerPriority[pi.friendsID].Mode == 1;
            bool marked = G::PlayerPriority[pi.friendsID].Mode == 4;

            Hook.Original<FN>()(ecx, edx, ignored ? "Unignore" : "Ignore", "fedignore", "fed");
            Hook.Original<FN>()(ecx, edx, marked ? "Unmark" : "Mark as Cheater", "fedmark", "fed");
        }

        return ret;
    }

    return Hook.Original<FN>()(ecx, edx, pszButtonText, pszCommand, pszCategoryName);
}

MAKE_HOOK(CTFClientScoreBoardDialog_OnCommand, S::CTFClientScoreBoardDialog_OnCommand(), void, __fastcall,
    void* ecx, void* edx, const char* command)
{
    if (!Vars::Misc::ScoreboardPlayerlist.Value) { return Hook.Original<FN>()(ecx, edx, command); return; }

    if (FNV1A::Hash(command) == FNV1A::HashConst("fedignore"))
    {
        if (CurrentFriendsID)
        {
            G::SwitchIgnore(CurrentFriendsID);
        }
    }

    if (FNV1A::Hash(command) == FNV1A::HashConst("fedmark"))
    {
        if (CurrentFriendsID)
        {
            G::SwitchMark(CurrentFriendsID);
        }
    }

    Hook.Original<FN>()(ecx, edx, command);
}