#include "CheaterDetection.h"

#include "../Menu/Playerlist/PlayerUtils.h"
#include "../Logs/Logs.h"

bool CCheaterDetection::ShouldScan()
{
	if (!Vars::CheaterDetection::Methods.Value || I::EngineClient->IsPlayingTimeDemo())
		return false;

	static float flOldTime = I::GlobalVars->curtime;
	const float flCurTime = I::GlobalVars->curtime;
	const bool bShouldSkip = TIME_TO_TICKS(flCurTime - flOldTime) != 1;
	flOldTime = flCurTime;
	if (bShouldSkip)
		return false;

	auto pNetChan = I::EngineClient->GetNetChannelInfo();
	if (pNetChan && (pNetChan->GetTimeSinceLastReceived() > TICK_INTERVAL * 2 || pNetChan->IsTimingOut()))
		return false;

	return true;
}

bool CCheaterDetection::InvalidPitch(CBaseEntity* pEntity)
{
	return Vars::CheaterDetection::Methods.Value & (1 << 0) && fabsf(pEntity->m_angEyeAnglesX()) > 89.9f;
}

bool CCheaterDetection::IsChoking(CBaseEntity* pEntity)
{
	const bool bReturn = mData[pEntity].bChoke;
	mData[pEntity].bChoke = false;

	return Vars::CheaterDetection::Methods.Value & (1 << 1) && bReturn;
}

bool CCheaterDetection::IsFlicking(CBaseEntity* pEntity) // this is aggravating
{
	auto& vAngles = mData[pEntity].vAngles;
	if (!(Vars::CheaterDetection::Methods.Value & (1 << 2)))
	{
		vAngles.clear();
		return false;
	}

	if (vAngles.size() != 3 || !vAngles[0].second && !vAngles[1].second && !vAngles[2].second)
		return false;

	if (Math::CalcFov(vAngles[0].first, vAngles[1].first) < Vars::CheaterDetection::MinimumFlick.Value)
		return false;

	if (Math::CalcFov(vAngles[0].first, vAngles[2].first) > Vars::CheaterDetection::MaximumNoise.Value * (TICK_INTERVAL / 0.015f))
		return false;

	vAngles.clear();
	return true;
}

bool CCheaterDetection::IsDuckSpeed(CBaseEntity* pEntity)
{
	if (!(Vars::CheaterDetection::Methods.Value & (1 << 3))
		|| !pEntity->IsDucking() || !pEntity->OnSolid() // this may break on movement sim
		|| pEntity->m_vecVelocity().Length2D() < pEntity->m_flMaxspeed() * 0.5f)
	{
		mData[pEntity].iDuckSpeed = 0;
		return false;
	}

	mData[pEntity].iDuckSpeed++;
	if (mData[pEntity].iDuckSpeed > 20)
	{
		mData[pEntity].iDuckSpeed = 0;
		return true;
	}

	return false;
}

void CCheaterDetection::Infract(CBaseEntity* pEntity, std::string sReason)
{
	mData[pEntity].iDetections++;
	const bool bMark = mData[pEntity].iDetections >= Vars::CheaterDetection::DetectionsRequired.Value;

	F::Logs.CheatDetection(mData[pEntity].sName, bMark ? "marked" : "infracted", sReason);
	if (bMark)
	{
		mData[pEntity].iDetections = 0;
		F::PlayerUtils.AddTag(mData[pEntity].friendsID, "Cheater", true, mData[pEntity].sName);
	}
}

void CCheaterDetection::Run()
{
	if (!ShouldScan() || !I::EngineClient->IsConnected())
		return;

	for (auto& pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
	{
		if (!mData[pEntity].bShouldScan)
			continue;

		if (InvalidPitch(pEntity))
			Infract(pEntity, "invalid pitch");
		if (IsChoking(pEntity))
			Infract(pEntity, "choking packets");
		if (IsFlicking(pEntity))
			Infract(pEntity, "flicking");
		if (IsDuckSpeed(pEntity))
			Infract(pEntity, "duck speed");
	}
}

void CCheaterDetection::Fill()
{
	if (!Vars::CheaterDetection::Methods.Value || I::EngineClient->IsPlayingTimeDemo())
		return;

	for (auto& pEntity : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
	{
		mData[pEntity].bShouldScan = false;

		PlayerInfo_t pi{};
		if (!pEntity->IsAlive() || pEntity->IsAGhost() || pEntity->GetDormant()
			|| !I::EngineClient->GetPlayerInfo(pEntity->GetIndex(), &pi) || pi.fakeplayer || F::PlayerUtils.HasTag(pi.friendsID, "Cheater"))
		{
			mData[pEntity].vChokes.clear();
			mData[pEntity].bChoke = false;
			mData[pEntity].vAngles.clear();
			mData[pEntity].iDuckSpeed = 0;
			continue;
		}

		if (pEntity->m_flSimulationTime() == pEntity->m_flOldSimulationTime())
			continue;

		mData[pEntity].bShouldScan = true;
		mData[pEntity].friendsID = pi.friendsID;
		mData[pEntity].sName = pi.name;
		mData[pEntity].vAngles.push_back({ pEntity->GetEyeAngles(), mData[pEntity].bDamage });
		mData[pEntity].bDamage = false;
		if (mData[pEntity].vAngles.size() > 3)
			mData[pEntity].vAngles.pop_front();
	}
}

void CCheaterDetection::Reset()
{
	mData.clear();
}

void CCheaterDetection::ReportChoke(CBaseEntity* pEntity, int iChoke)
{
	if (Vars::CheaterDetection::Methods.Value & (1 << 1))
	{
		mData[pEntity].vChokes.push_back(iChoke);
		if (mData[pEntity].vChokes.size() == 3)
		{
			mData[pEntity].bChoke = true; // check for last 3 choke amounts
			for (auto& iChoke : mData[pEntity].vChokes)
			{
				if (iChoke < Vars::CheaterDetection::MinimumChoking.Value)
					mData[pEntity].bChoke = false;
			}
			mData[pEntity].vChokes.clear();
		}
	}
	else
		mData[pEntity].vChokes.clear();
}

void CCheaterDetection::ReportDamage(CGameEvent* pEvent)
{
	if (!(Vars::CheaterDetection::Methods.Value & (1 << 2)))
		return;

	const int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
	if (iIndex == I::EngineClient->GetLocalPlayer())
		return;

	CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(iIndex);
	if (!pEntity || pEntity->GetDormant())
		return;

	switch (Utils::GetWeaponType(pEntity->GetActiveWeapon()))
	{
	case EWeaponType::UNKNOWN:
	case EWeaponType::PROJECTILE:
		return;
	}

	mData[pEntity].bDamage = true;
}