#include "Events.h"
#include "../../Features/Logs/Logs.h"
#include "../../Features/Resolver/Resolver.h"
#include "../../Features/PacketManip/AntiAim/AntiAim.h"
#include "../../Features/CheaterDetection/CheaterDetection.h"
#include "../../Features/Visuals/Visuals.h"
#include "../../Features/CritHack/CritHack.h"
#include "../../Features/Backtrack/Backtrack.h"
#include "../../Features/Misc/Misc.h"

void CEventListener::Setup(const std::deque<const char*>& deqEvents)
{
	if (deqEvents.empty())
		return;

	for (auto szEvent : deqEvents)
	{
		I::GameEventManager->AddListener(this, szEvent, false);

		if (!I::GameEventManager->FindListener(this, szEvent))
			throw std::runtime_error(std::format("failed to add listener: {}", szEvent));
	}
}

void CEventListener::Destroy()
{
	I::GameEventManager->RemoveListener(this);
}

void CEventListener::FireGameEvent(CGameEvent* pEvent)
{
	if (!pEvent || I::EngineClient->IsPlayingTimeDemo())
		return;

	auto pLocal = g_EntityCache.GetLocal();
	const auto uHash = FNV1A::Hash(pEvent->GetName());

	F::Logs.Event(pEvent, uHash, pLocal);
	F::CritHack.Event(pEvent, uHash, pLocal);
	F::Misc.Event(pEvent, uHash);
	switch (uHash)
	{
	case FNV1A::HashConst("player_hurt"):
		F::Resolver.OnPlayerHurt(pEvent);
		F::CheaterDetection.ReportDamage(pEvent);
		return;
	case FNV1A::HashConst("player_spawn"):
		F::Backtrack.SetLerp(pEvent);
		return;
	case FNV1A::HashConst("item_pickup"):
	{
		if (!Vars::Visuals::UI::PickupTimers.Value)
			return;

		const auto& pEntity = I::ClientEntityList->GetClientEntity(I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")));
		if (!pEntity)
			return;
		const auto itemName = pEvent->GetString("item");
		if (std::strstr(itemName, "medkit"))
			F::Visuals.PickupDatas.push_back({ 1, I::EngineClient->Time(), pEntity->GetAbsOrigin() });
		else if (std::strstr(itemName, "ammopack"))
			F::Visuals.PickupDatas.push_back({ 0, I::EngineClient->Time(), pEntity->GetAbsOrigin() });

		return;
	}
	case FNV1A::HashConst("revive_player_notify"):
	{
		if (!Vars::Misc::MannVsMachine::InstantRevive.Value)
			return;

		if (pEvent->GetInt("entindex") != I::EngineClient->GetLocalPlayer())
			return;
		
		auto kv = new KeyValues("MVM_Revive_Response");
		kv->SetInt("accepted", 1);
		I::EngineClient->ServerCmdKeyValues(kv);
	}
	}
}