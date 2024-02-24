#include "Backtrack.h"
#include "../Simulation/MovementSimulation/MovementSimulation.h"
#include "../Visuals/Visuals.h"
#include "../Menu/Playerlist/PlayerUtils.h"

#define ROUND_TO_TICKS(t) (TICKS_TO_TIME(TIME_TO_TICKS(t)))

void CBacktrack::Restart()
{
	mRecords.clear();
	dSequences.clear();
	iLastInSequence = 0;
}



// Returns the wish cl_interp
float CBacktrack::GetLerp()
{
	return Vars::Backtrack::Enabled.Value ? std::clamp(static_cast<float>(Vars::Backtrack::Interp.Value), G::LerpTime * 1000.f, flMaxUnlag * 1000.f) / 1000.f : G::LerpTime;
}

// Returns the current (custom) backtrack latency
float CBacktrack::GetFake()
{
	return bFakeLatency ? std::clamp(static_cast<float>(Vars::Backtrack::Latency.Value), 0.f, flMaxUnlag * 1000.f) / 1000.f : 0.f;
}

// Returns the current real latency
float CBacktrack::GetReal()
{
	const auto iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan)
		return 0.f;

	return iNetChan->GetLatency(FLOW_INCOMING) + iNetChan->GetLatency(FLOW_OUTGOING) - GetFake();
}

void CBacktrack::SendLerp()
{
	CNetChannel* netChannel = I::EngineClient->GetNetChannelInfo();
	if (!netChannel) return;

	static Timer interpTimer{};
	if (interpTimer.Run(100))
	{
		float flTarget = GetLerp();
		if (flTarget == flWishInterp) return;
		flWishInterp = flTarget;

		Utils::ConLog("SendNetMsg", std::format("cl_interp: {}", flTarget).c_str(), { 224, 255, 131, 255 }, Vars::Debug::Logging.Value);

		NET_SetConVar cl_interp("cl_interp", std::to_string(flTarget).c_str());
		netChannel->SendNetMsg(cl_interp);

		NET_SetConVar cl_interp_ratio("cl_interp_ratio", "1.0");
		netChannel->SendNetMsg(cl_interp_ratio);

		NET_SetConVar cl_interpolate("cl_interpolate", "1");
		netChannel->SendNetMsg(cl_interpolate);
	}
}

// Manages cl_interp client value
void CBacktrack::SetLerp(CGameEvent* pEvent)
{
	const bool bLocal = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")) == I::EngineClient->GetLocalPlayer();
	if (bLocal)
		flFakeInterp = flWishInterp;
}

// Store the last 2048 sequences
void CBacktrack::UpdateDatagram()
{
	if (INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo())
	{
		if (iNetChan->m_nInSequenceNr > iLastInSequence)
		{
			iLastInSequence = iNetChan->m_nInSequenceNr;
			dSequences.push_front(CIncomingSequence(iNetChan->m_nInReliableState, iNetChan->m_nInSequenceNr, I::GlobalVars->realtime));
		}

		if (dSequences.size() > 2048)
		{
			dSequences.pop_back();
		}
	}
}



bool CBacktrack::WithinRewind(const TickRecord& record)
{
	const auto iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan)
		return false;

	const float flCorrect = std::clamp(iNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(flFakeInterp) + GetFake(), 0.f, flMaxUnlag) - iNetChan->GetLatency(FLOW_OUTGOING);
	const int iServerTick = iTickCount + (Vars::Misc::NetworkFix.Value ? 1 : 0) + G::AnticipatedChoke + Vars::Backtrack::Offset.Value;

	const float flDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(record.flSimTime));

	return fabsf(flDelta) < float(Vars::Backtrack::Window.Value) / 1000;
}

std::deque<TickRecord>* CBacktrack::GetRecords(CBaseEntity* pEntity)
{
	if (mRecords[pEntity].empty())
		return nullptr;

	return &mRecords[pEntity];
}

std::deque<TickRecord> CBacktrack::GetValidRecords(std::deque<TickRecord>* pRecords, BacktrackMode iMode, CBaseEntity* pLocal, bool bDistance)
{
	std::deque<TickRecord> validRecords = {};
	if (!pRecords)
		return validRecords;

	switch (iMode)
	{
	case BacktrackMode::ALL:
		for (auto& pTick : *pRecords)
		{
			if (!WithinRewind(pTick))
				continue;

			validRecords.push_back(pTick);
		}

		break;

	case BacktrackMode::LAST:
		for (auto pTick = pRecords->rbegin(); pTick != pRecords->rend(); ++pTick)
		{
			if (!WithinRewind(*pTick))
				continue;
			
			validRecords.push_back(*pTick);
			break;
		}

		break;

	case BacktrackMode::PREFERONSHOT:
		std::deque<TickRecord> frontRecords, backRecords;

		for (auto& pTick : *pRecords)
		{
			if (!WithinRewind(pTick))
				continue;

			if (pTick.bOnShot)
				frontRecords.push_back(pTick);
			else
				backRecords.push_back(pTick);
		}

		for (auto& pTick : frontRecords)
			validRecords.push_back(pTick);
		for (auto& pTick : backRecords)
			validRecords.push_back(pTick);

		break;
	}

	if (pLocal)
	{
		if (bDistance)
			std::sort(validRecords.begin(), validRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
				{
					return pLocal->m_vecOrigin().DistTo(a.vOrigin) < pLocal->m_vecOrigin().DistTo(b.vOrigin);
				});
		else
		{
			const auto iNetChan = I::EngineClient->GetNetChannelInfo();
			if (!iNetChan)
				return validRecords;

			const float flCorrect = std::clamp(iNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(flFakeInterp) + GetFake(), 0.f, flMaxUnlag) - iNetChan->GetLatency(FLOW_OUTGOING);
			const int iServerTick = iTickCount + (Vars::Misc::NetworkFix.Value ? 1 : 0) + G::AnticipatedChoke + Vars::Backtrack::Offset.Value;

			std::sort(validRecords.begin(), validRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
				{
					const float flADelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(a.flSimTime));
					const float flBDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(b.flSimTime));
					return fabsf(flADelta) < fabsf(flBDelta);
				});
		}
	}

	return validRecords;
}

std::optional<TickRecord> CBacktrack::GetHitRecord(CUserCmd* pCmd, CBaseEntity* pEntity, const Vec3 vAngles, const Vec3 vPos)
{
	std::optional<TickRecord> cReturnRecord{};
	float flLastAngle = 45.f;

	for (const auto& rCurQuery : mRecords[pEntity])
	{
		if (!WithinRewind(rCurQuery))
			continue;

		for (int iCurHitbox = 0; iCurHitbox < 18; iCurHitbox++)
		{
			//	it's possible to set entity positions and bones back to this record and then see what hitbox we will hit and rewind to that record, bt i dont wanna
			const Vec3 vHitboxPos = pEntity->GetHitboxPosMatrix(iCurHitbox, (matrix3x4*)(&rCurQuery.BoneMatrix.BoneMatrix));
			const Vec3 vAngleTo = Math::CalcAngle(vPos, vHitboxPos);
			const float flFOVTo = Math::CalcFov(vAngles, vAngleTo);
			if (flFOVTo < flLastAngle)
			{
				cReturnRecord = rCurQuery;
				flLastAngle = flFOVTo;
			}
		}
	}
	return cReturnRecord;
}



void CBacktrack::StoreNolerp()
{
	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pEntity || !pEntity->IsPlayer() || n == I::EngineClient->GetLocalPlayer())
			continue;

		// more of a placeholder, still interpolated iirc
		bSettingUpBones = true;
		mBones[pEntity].first = pEntity->SetupBones(mBones[pEntity].second, 128, BONE_USED_BY_ANYTHING, pEntity->m_flSimulationTime());
		bSettingUpBones = false;

		mEyeAngles[pEntity] = pEntity->GetEyeAngles();
	}
}

void CBacktrack::MakeRecords()
{
	if (iLastCreationTick == I::GlobalVars->tickcount)
		return;
	iLastCreationTick = I::GlobalVars->tickcount;

	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pEntity || !pEntity->IsPlayer() || n == I::EngineClient->GetLocalPlayer() || !mBones[pEntity].first)
			continue;

		const float flSimTime = pEntity->m_flSimulationTime(), flOldSimTime = pEntity->m_flOldSimulationTime();
		if (TIME_TO_TICKS(flSimTime - flOldSimTime) <= 0)
			continue;

		const TickRecord curRecord = {
			flSimTime,
			I::GlobalVars->curtime,
			I::GlobalVars->tickcount,
			mDidShoot[pEntity->GetIndex()],
			*reinterpret_cast<BoneMatrixes*>(&mBones[pEntity].second),
			pEntity->m_vecOrigin(),
			pEntity->GetWorldSpaceCenter()
		};

		bool bLagComp = false;
		if (!mRecords[pEntity].empty()) // check for lagcomp breaking here
		{
			const Vec3 vDelta = curRecord.vOrigin - mRecords[pEntity].front().vOrigin;

			static auto sv_lagcompensation_teleport_dist = I::Cvar->FindVar("sv_lagcompensation_teleport_dist");
			const float flDist = sv_lagcompensation_teleport_dist ? powf(sv_lagcompensation_teleport_dist->GetFloat(), 2.f) : 4096.f;
			if (vDelta.Length2DSqr() > flDist)
			{
				bLagComp = true;
				for (auto& pRecord : mRecords[pEntity])
					pRecord.bInvalid = true;
			}

			for (auto& pRecord : mRecords[pEntity]) // worse results with higher ping, possibly implement prediction based on ping?
			{
				if (!pRecord.bInvalid)
					continue;

				pRecord.bOnShot = curRecord.bOnShot;
				pRecord.BoneMatrix = curRecord.BoneMatrix;
				pRecord.vOrigin = curRecord.vOrigin;
				pRecord.vCenter = curRecord.vCenter;
			}
		}

		mRecords[pEntity].push_front(curRecord);
		mLagCompensation[pEntity] = bLagComp;

		mDidShoot[pEntity->GetIndex()] = false;
	}
}

void CBacktrack::CleanRecords()
{
	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pEntity || n == I::EngineClient->GetLocalPlayer())
			continue;

		if (!pEntity->IsPlayer() || pEntity->GetDormant() || !pEntity->IsAlive() || pEntity->IsAGhost())
		{
			mRecords[pEntity].clear();
			continue;
		}

		//const int iOldSize = pRecords.size();

		const int flDeadtime = I::GlobalVars->curtime + GetReal() - flMaxUnlag; // int ???
		while (!mRecords[pEntity].empty())
		{
			if (mRecords[pEntity].back().flSimTime >= flDeadtime)
				break;

			mRecords[pEntity].pop_back();
		}

		//const int iNewSize = pRecords.size();
		//if (iOldSize != iNewSize)
		//	Utils::ConLog("Clear", std::format("{} -> {}", iOldSize, iNewSize).c_str(), { 255, 0, 200, 255 }, Vars::Debug::Logging.Value);
	}
}



void CBacktrack::FrameStageNotify()
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pLocal || !iNetChan)
		return Restart();

	flMaxUnlag = g_ConVars.sv_maxunlag->GetFloat();

	StoreNolerp();
	MakeRecords();
	CleanRecords();
}

std::optional<TickRecord> CBacktrack::Run(CUserCmd* pCmd) // backtrack to crosshair
{
	SendLerp();

	if (!Vars::Backtrack::Enabled.Value)
		return std::nullopt;

	// might not even be necessary
	G::AnticipatedChoke = 0;
	if (G::ShiftedTicks != G::MaxShift)
	{
		switch (G::CurWeaponType)
		{
		case EWeaponType::PROJECTILE:
			if (Vars::Aimbot::Projectile::AimMethod.Value == 2)
				G::AnticipatedChoke = 1;
			break;
		case EWeaponType::MELEE:
			if (Vars::Aimbot::Melee::AimMethod.Value == 2)
				G::AnticipatedChoke = 1;
			break;
		}
	}
	const int iChoke = G::ChokeAmount;
	if (iChoke && !Vars::CL_Move::FakeLag::UnchokeOnAttack.Value &&
		G::ShiftedTicks == G::ShiftedGoal && !G::DoubleTap)
	{
		G::AnticipatedChoke = G::ChokeGoal - iChoke;
	}
	// iffy, unsure if there is a good way to get it to work well without unchoking

	UpdateDatagram();

	//if (G::IsAttacking)
	const bool bShouldRun = G::CurItemDefIndex != Sniper_m_TheClassic && pCmd->buttons & IN_ATTACK ||
		G::CurItemDefIndex == Sniper_m_TheClassic && pCmd->buttons & ~IN_ATTACK && bLastTickHeld;
	bLastTickHeld = pCmd->buttons & IN_ATTACK;
	if (bShouldRun)
	{
		CBaseEntity* pLocal = g_EntityCache.GetLocal();
		if (!pLocal)
			return std::nullopt;

		const Vec3 vShootPos = pLocal->GetShootPos();
		const Vec3 vAngles = pCmd->viewangles;

		std::optional<TickRecord> cReturnTick;
		for (const auto& pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			if (!pEnemy || !pEnemy->IsAlive() || pEnemy->IsAGhost())
				continue;

			PlayerInfo_t pi{}; // dont care about ignored players
			if (I::EngineClient->GetPlayerInfo(pEnemy->GetIndex(), &pi) && F::PlayerUtils.IsIgnored(pi.friendsID))
				continue;

			if (const std::optional<TickRecord> checkRec = GetHitRecord(pCmd, pEnemy, vAngles, vShootPos))
			{
				cReturnTick = checkRec;
				break;
			}
		}
		if (cReturnTick)
		{
			pCmd->tick_count = TIME_TO_TICKS(cReturnTick->flSimTime) + TIME_TO_TICKS(flFakeInterp);
			return std::nullopt;
		}
	}
	return std::nullopt;
}

void CBacktrack::PlayerHurt(CGameEvent* pEvent)
{
	//const int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
	//if (CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(iIndex)){
	//	mRecords[pEntity].clear();	//	bone cache has gone to poop for this entity, they must be cleansed in holy fire :smiling_imp:
	//}
}

void CBacktrack::ResolverUpdate(CBaseEntity* pEntity)
{
	mRecords[pEntity].clear();	//	TODO: eventually remake records and rotate them or smthn idk, maybe just rotate them
}

void CBacktrack::ReportShot(int iIndex)
{
	CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(iIndex);
	if (!pEntity)
		return;
	mDidShoot[pEntity->GetIndex()] = true;
}

// Adjusts the fake latency ping
void CBacktrack::AdjustPing(INetChannel* netChannel)
{
	for (const auto& cSequence : dSequences)
	{
		if (I::GlobalVars->realtime - cSequence.CurTime >= GetFake())
		{
			netChannel->m_nInReliableState = cSequence.InReliableState;
			netChannel->m_nInSequenceNr = cSequence.SequenceNr;
			break;
		}
	}
}