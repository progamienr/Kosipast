#include "Backtrack.h"
#include "../Simulation/MovementSimulation/MovementSimulation.h"
#include "../Visuals/Visuals.h"
#include "../Menu/Playerlist/PlayerUtils.h"

#define ROUND_TO_TICKS(t) (TICKS_TO_TIME(TIME_TO_TICKS(t)))

void CBacktrack::Restart()
{
	mRecords.clear();
	dSequences.clear();
	flLatencyRampup = 0.f;
	iLastInSequence = 0;
	iMaxUnlag = TIME_TO_TICKS(g_ConVars.sv_maxunlag->GetFloat());
}



bool CBacktrack::IsTracked(const TickRecord& record)
{
	return record.flSimTime >= I::GlobalVars->curtime - 1.f;
}

bool CBacktrack::IsEarly(CBaseEntity* pEntity, const TickRecord& record)
{
	return !Vars::Backtrack::AllowForward.Value && record.flSimTime > pEntity->m_flSimulationTime();
}



// Returns the wish cl_interp
float CBacktrack::GetLerp()
{
	return Vars::Backtrack::Enabled.Value ? std::clamp(static_cast<float>(Vars::Backtrack::Interp.Value)/* + G::LerpTime * 1000.f*/, G::LerpTime * 1000.f, 800.f) / 1000.f : G::LerpTime;
}

// Returns the current (custom) backtrack latency
float CBacktrack::GetFake()
{
	return bFakeLatency ? flLatencyRampup * std::clamp(static_cast<float>(Vars::Backtrack::Latency.Value), 0.f, 800.f) / 1000.f : 0.f;
}

// Returns the current real latency
float CBacktrack::GetReal()
{
	const auto iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan)
		return -1.f;

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

		{
			NET_SetConVar cmd("cl_interp", std::to_string(flTarget).c_str());
			netChannel->SendNetMsg(cmd);
		}
		{
			NET_SetConVar cmd("cl_interp_ratio", "1.0");
			netChannel->SendNetMsg(cmd);
		}
		{
			NET_SetConVar cmd("cl_interpolate", "1");
			netChannel->SendNetMsg(cmd);
		}
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

	const float flCorrect = std::clamp(iNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(flFakeInterp) + GetFake(), 0.0f, g_ConVars.sv_maxunlag->GetFloat());
	const int iServerTick = iTickCount + TIME_TO_TICKS(GetReal()) + (Vars::Misc::NetworkFix.Value ? 1 : -1) + Vars::Backtrack::PassthroughOffset.Value + G::AnticipatedChoke * Vars::Backtrack::ChokePassMod.Value;

	const float flDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(record.flSimTime));

	return fabsf(flDelta) < (Vars::Backtrack::Window.Value - (flDelta > 0.f ? Vars::Backtrack::NWindowSub.Value : Vars::Backtrack::OWindowSub.Value)) / 1000.f;
	// in short, check if the record is +- 200ms from us
}

std::deque<TickRecord>* CBacktrack::GetRecords(CBaseEntity* pEntity)
{
	if (mRecords[pEntity].empty())
		return nullptr;

	return &mRecords[pEntity];
}

std::optional<TickRecord> CBacktrack::GetFirstRecord(CBaseEntity* pEntity)
{
	if (mRecords[pEntity].empty())
		return std::nullopt;
	bool bFirst = true;
	for (const auto& rCurQuery : mRecords[pEntity])
	{
		if (!IsTracked(rCurQuery) || !WithinRewind(rCurQuery) || IsEarly(pEntity, rCurQuery))
		{
			bFirst = false;
			continue;
		}
		if (bFirst)
			return std::nullopt;
		return rCurQuery;
	}
	return std::nullopt;
}
std::optional<TickRecord> CBacktrack::GetLastRecord(CBaseEntity* pEntity)
{
	if (mRecords[pEntity].empty())
		return std::nullopt;
	std::optional<TickRecord> rReturnRecord = std::nullopt;
	for (const auto& rCurQuery : mRecords[pEntity])
	{
		if (!IsTracked(rCurQuery) || !WithinRewind(rCurQuery) || IsEarly(pEntity, rCurQuery))
			continue;
		rReturnRecord = rCurQuery;
	}
	return rReturnRecord;
}

std::deque<TickRecord> CBacktrack::GetValidRecords(CBaseEntity* pEntity, std::deque<TickRecord> pRecords, BacktrackMode iMode, CBaseEntity* pLocal)
{
	std::deque<TickRecord> validRecords;

	switch (iMode)
	{
	case BacktrackMode::ALL:
		for (auto& pTick : pRecords)
		{
			if (!WithinRewind(pTick) || !IsTracked(pTick) || IsEarly(pEntity, pTick))
				continue;

			validRecords.push_back(pTick);
		}

		break;

	case BacktrackMode::LAST:
		for (auto pTick = pRecords.rbegin(); pTick != pRecords.rend(); ++pTick)
		{
			if (!WithinRewind(*pTick) || !IsTracked(*pTick) || IsEarly(pEntity, *pTick))
				continue;
			
			validRecords.push_back(*pTick);
			break;
		}

		break;

	case BacktrackMode::PREFERONSHOT:
		std::deque<TickRecord> frontRecords, backRecords;

		for (auto& pTick : pRecords)
		{
			if (!WithinRewind(pTick) || !IsTracked(pTick) || IsEarly(pEntity, pTick))
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

	// if we have plocal, sort by distance from player,
	if (pLocal)
		std::sort(validRecords.begin(), validRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
			{
				return pLocal->m_vecOrigin().DistTo(a.vOrigin) < pLocal->m_vecOrigin().DistTo(b.vOrigin);
			});
	// otherwise sort by distance from fake latency value
	else
	{
		const auto iNetChan = I::EngineClient->GetNetChannelInfo();
		if (!iNetChan)
			return validRecords;

		const float flCorrect = std::clamp(iNetChan->GetLatency(FLOW_OUTGOING) + ROUND_TO_TICKS(flFakeInterp) + GetFake(), 0.0f, g_ConVars.sv_maxunlag->GetFloat());
		const int iServerTick = iTickCount + TIME_TO_TICKS(GetReal()) + Vars::Backtrack::PassthroughOffset.Value + G::AnticipatedChoke * Vars::Backtrack::ChokePassMod.Value;

		std::sort(validRecords.begin(), validRecords.end(), [&](const TickRecord& a, const TickRecord& b) -> bool
			{
				const float flADelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(a.flSimTime));
				const float flBDelta = flCorrect - TICKS_TO_TIME(iServerTick - TIME_TO_TICKS(b.flSimTime));
				return fabsf(flADelta) < fabsf(flBDelta);
			});
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

		// bones are more of a placeholder as i haven't gotten them to work correctly yet
		bSettingUpBones = true;
		noInterpBones[pEntity->GetIndex()].first = pEntity->SetupBones(noInterpBones[pEntity->GetIndex()].second, 128, BONE_USED_BY_ANYTHING, pEntity->m_flSimulationTime());
		bSettingUpBones = false;

		noInterpEyeAngles[pEntity->GetIndex()] = pEntity->GetEyeAngles();
	}
}

void CBacktrack::MakeRecords()
{
	if (!iMaxUnlag)
		iMaxUnlag = 67;

	if (iLastCreationTick == I::GlobalVars->tickcount)
		return;
	iLastCreationTick = I::GlobalVars->tickcount;

	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pEntity || !pEntity->IsPlayer() || n == I::EngineClient->GetLocalPlayer())
			continue;

		const float flSimTime = pEntity->m_flSimulationTime(), flOldSimTime = pEntity->m_flOldSimulationTime();
		const float flDelta = flSimTime - flOldSimTime;

		const Vec3 vOrigin = pEntity->m_vecOrigin();
		if (!mRecords[pEntity].empty())
		{	// as long as we have 1 record we can check for lagcomp breaking here
			const Vec3 vPrevOrigin = mRecords[pEntity].front().vOrigin;
			const Vec3 vDelta = vOrigin - vPrevOrigin;
			if (vDelta.Length2DSqr() > 4096.f)
				mRecords[pEntity].clear();
		}

		if (TIME_TO_TICKS(flDelta) > 0)
		{	// create record on simulated players
			if (!noInterpBones[pEntity->GetIndex()].first)
				continue;

			mRecords[pEntity].push_front({
				flSimTime,
				I::GlobalVars->curtime,
				I::GlobalVars->tickcount,
				mDidShoot[pEntity->GetIndex()],
				*reinterpret_cast<BoneMatrixes*>(&noInterpBones[pEntity->GetIndex()].second),
				vOrigin
			});
		}
		else if (Vars::Backtrack::UnchokePrediction.Value && mRecords[pEntity].size() < 3)
		{	// user is choking, predict location of next record
			const Vec3 vOriginalPos = pEntity->GetAbsOrigin();
			const Vec3 vOriginalEyeAngles = pEntity->GetEyeAngles();
			const float flNextSimTime = flSimTime + I::GlobalVars->interval_per_tick;
			const float flDeltaRecorded = flNextSimTime - (mRecords[pEntity].empty() ? flSimTime : mRecords[pEntity].front().flSimTime);
			if (TICKS_TO_TIME(flDeltaRecorded) < 1)
				continue;
			//if (pEntity->GetVelocity().Length2D() > 4096.f)
			//	continue; // this will only happen on people that are stuck or it will be caught elsewhere, dont use

			PlayerStorage storage;
			if (F::MoveSim.Initialize(pEntity, storage))
			{
				F::MoveSim.RunTick(storage);

				// we've predicted their next record, and we can probably predict their next lag record but if they are fakelagging it's pointless n shieet
				// i was going to check if this lag comp would be valid here but it seems almost pointless now, dont do it.
				// might need to do this for setupbones
				pEntity->SetAbsOrigin(storage.m_MoveData.m_vecOrigin);
				pEntity->SetEyeAngles(storage.m_MoveData.m_vecViewAngles);

				if (noInterpBones[pEntity->GetIndex()].first)
				{
					mRecords[pEntity].push_front({
						flNextSimTime,
						I::GlobalVars->curtime + I::GlobalVars->interval_per_tick,
						I::GlobalVars->tickcount + 1,
						false,
						*reinterpret_cast<BoneMatrixes*>(&noInterpBones[pEntity->GetIndex()].second),
						storage.m_MoveData.m_vecOrigin
					});
				}

				pEntity->SetAbsOrigin(vOriginalPos);
				pEntity->SetEyeAngles(vOriginalEyeAngles);
				F::MoveSim.Restore(storage);
			}
		}
		// cleanup
		mDidShoot[pEntity->GetIndex()] = false;
		if (mRecords[pEntity].size() > iMaxUnlag)
			mRecords[pEntity].pop_back();
	}
}

void CBacktrack::CleanRecords()
{
	for (int n = 1; n <= I::EngineClient->GetMaxClients(); n++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pEntity)
			continue;

		if (!pEntity->IsPlayer() || pEntity->GetDormant() || !pEntity->IsAlive() || pEntity->IsAGhost())
		{
			mRecords[pEntity].clear();
			continue;
		}

		if (mRecords[pEntity].empty())
			continue;
		if (!IsTracked(mRecords[pEntity].back()))
			mRecords[pEntity].pop_back();
		if (mRecords[pEntity].size() > 67)
			mRecords[pEntity].pop_back();
	}
}



void CBacktrack::FrameStageNotify()
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pLocal || !iNetChan)
		return Restart();

	flLatencyRampup = std::min(1.f, flLatencyRampup += I::GlobalVars->interval_per_tick);
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
			pCmd->tick_count = TIME_TO_TICKS(cReturnTick->flSimTime) + TIME_TO_TICKS(flFakeInterp) + Vars::Backtrack::TicksetOffset.Value + G::AnticipatedChoke * Vars::Backtrack::ChokeSetMod.Value;
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