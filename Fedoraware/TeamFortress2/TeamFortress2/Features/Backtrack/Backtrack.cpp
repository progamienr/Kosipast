#include "Backtrack.h"
#include "../Simulation/MovementSimulation/MovementSimulation.h"

#define ROUND_TO_TICKS(t) (TICKS_TO_TIME(TIME_TO_TICKS(t)))

// Manages cl_interp client value
void CBacktrack::SetLerp(CGameEvent* pEvent)
{
	const bool bLocal = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")) == I::EngineClient->GetLocalPlayer();
	if (bLocal)
		flFakeInterp = flWishInterp;
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
	if (!iNetChan) { return -1.f; }

	return iNetChan->GetLatency(FLOW_INCOMING) + iNetChan->GetLatency(FLOW_OUTGOING) - GetFake();
}



bool CBacktrack::IsTracked(const TickRecord& record)
{
	return record.flSimTime >= I::GlobalVars->curtime - 1.f;
}

//	i know this seems stupid, but i think it's a good idea to wait until the record is created rather than try and forward track.
bool CBacktrack::IsEarly(CBaseEntity* pEntity, const TickRecord& record) {
	return record.flSimTime > pEntity->GetSimulationTime();
}

//	should return true if the current position on the client has a lag comp record created for it by the server (SHOULD)
//	if the player has updated more than once, only the first update will have a backtrack record (i think)
//	dont use this yet
bool CBacktrack::IsSimulationReliable(CBaseEntity* pEntity)
{
	const float flSimTimeDelta = pEntity->GetSimulationTime() - pEntity->GetOldSimulationTime();
	return flSimTimeDelta > 0 && flSimTimeDelta <= I::GlobalVars->interval_per_tick;
}



bool CBacktrack::WithinRewind(const TickRecord& record)
{
	const auto iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan) { return false; }

	int iTarget = TIME_TO_TICKS(record.flSimTime);
	int iLerpTicks = TIME_TO_TICKS(flFakeInterp);

	float flCorrect = std::clamp(iNetChan->GetLatency(FLOW_OUTGOING) + TICKS_TO_TIME(iLerpTicks) + GetFake(), 0.0f, g_ConVars.sv_maxunlag->GetFloat());

	int iServerTick = iTickCount + TIME_TO_TICKS(GetReal()) - 1 + Vars::Backtrack::PassthroughOffset.Value;
	float flDelta = flCorrect - TICKS_TO_TIME(iServerTick - iTarget);

	return fabsf(flDelta) < 0.2f - TICKS_TO_TIME(Vars::Backtrack::Protect.Value + (flDelta < 0.f ? 1 : 0)); // older end seems more unreliable, possibly due to 1 tick choke ?
	// in short, check if the record is +- 200ms from us
}

//	hypothetically if their simtime is within 200ms of us we can hit their original, but idc
/* unused
bool CBacktrack::CanHitOriginal(CBaseEntity* pEntity) {
	if (GetFake() > 0.2f) { return false; }

	const auto iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan) { return false; }

	int iTarget = TIME_TO_TICKS(pEntity->GetSimulationTime());
	int iLerpTicks = TIME_TO_TICKS(flFakeInterp);

	float flCorrect = std::clamp(iNetChan->GetLatency(FLOW_OUTGOING) + TICKS_TO_TIME(iLerpTicks) + GetFake(), 0.0f, g_ConVars.sv_maxunlag->GetFloat());

	int iServerTick = iTickCount + TIME_TO_TICKS(GetReal());
	float flDelta = flCorrect - TICKS_TO_TIME(iServerTick - iTarget + Vars::Backtrack::PassthroughOffset.Value);

	return fabsf(flDelta) < 0.2f - TICKS_TO_TIME(Vars::Backtrack::Protect.Value);
}
*/

std::deque<TickRecord>* CBacktrack::GetRecords(CBaseEntity* pEntity)
{
	if (mRecords[pEntity].empty())
	{
		return nullptr;
	}

	return &mRecords[pEntity];
}

std::optional<TickRecord> CBacktrack::GetLastRecord(CBaseEntity* pEntity)
{
	if (mRecords[pEntity].empty()) { return std::nullopt; }
	std::optional<TickRecord> rReturnRecord = std::nullopt;
	for (const auto& rCurQuery : mRecords[pEntity])
	{
		if (!IsTracked(rCurQuery) || !WithinRewind(rCurQuery) || IsEarly(pEntity, rCurQuery)) { continue; }
		rReturnRecord = rCurQuery;
	}
	return rReturnRecord;
}

/* unused
std::optional<TickRecord> CBacktrack::GetFirstRecord(CBaseEntity* pEntity)
{
	//	UNUSED AND TRASH! UPDATE B4 USE
	//if (mRecords[pEntity].empty()) { return std::nullopt; }
	//std::optional<TickRecord> rReturnRecord = std::nullopt;
	//for (int nIndex = 2; nIndex < mRecords[pEntity].size(); nIndex++)
	//{
	//	if (!IsTracked(mRecords[pEntity][nIndex]) || !WithinRewind(mRecords[pEntity][nIndex])) { continue; }
	//	return mRecords[pEntity][nIndex];
	//}
	return std::nullopt;
}
*/

std::deque<TickRecord> CBacktrack::GetValidRecords(CBaseEntity* pEntity, std::deque<TickRecord> pRecords, BacktrackMode iMode)
{
	std::deque<TickRecord> validRecords;

	switch (iMode)
	{
	case BacktrackMode::ALL:
		for (auto& pTick : pRecords)
		{
			if (!WithinRewind(pTick) || !IsTracked(pTick) || IsEarly(pEntity, pTick)) { continue; }

			validRecords.push_back(pTick);
		}

		break;

	case BacktrackMode::LAST:
		for (auto pTick = pRecords.rbegin(); pTick != pRecords.rend(); ++pTick)
		{
			if (!WithinRewind(*pTick) || !IsTracked(*pTick) || IsEarly(pEntity, *pTick)) { continue; }
			
			validRecords.push_back(*pTick);
			break;
		}

		break;

	case BacktrackMode::PREFERONSHOT:
		std::deque<TickRecord> frontRecords, backRecords;

		for (auto& pTick : pRecords)
		{
			if (!WithinRewind(pTick) || !IsTracked(pTick) || IsEarly(pEntity, pTick)) { continue; }

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

	return validRecords;
}



void CBacktrack::CleanRecords()
{
	for (int n = 1; n < I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pEntity) { continue; }

		if (pEntity->GetDormant() || !pEntity->IsAlive() || !pEntity->IsPlayer())
		{
			mRecords[pEntity].clear();
			continue;
		}

		if (mRecords[pEntity].empty()) { continue; }
		if (!IsTracked(mRecords[pEntity].back())) { mRecords[pEntity].pop_back(); }
		if (mRecords[pEntity].size() > 67) { mRecords[pEntity].pop_back(); }
	}
}

void CBacktrack::MakeRecords()
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal) { return; }

	const float flCurTime = I::GlobalVars->curtime;
	const int iTickcount = I::GlobalVars->tickcount;
	if (iLastCreationTick == iTickcount) { return; }
	iLastCreationTick = iTickcount;

	for (int n = 1; n < I::ClientEntityList->GetHighestEntityIndex(); n++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(n);
		if (!pEntity || n == pLocal->GetIndex()) { continue; }
		if (!pEntity->IsPlayer()) { return; }

		noInterpEyeAngles[pEntity->GetIndex()] = pEntity->GetEyeAngles();

		const float flSimTime = pEntity->GetSimulationTime(), flOldSimTime = pEntity->GetOldSimulationTime();
		const float flDelta = flSimTime - flOldSimTime;

		const Vec3 vOrigin = pEntity->GetAbsOrigin();//m_vecOrigin();
		if (!mRecords[pEntity].empty())
		{
			// as long as we have 1 record we can check for lagcomp breaking here
			const Vec3 vPrevOrigin = mRecords[pEntity].front().vOrigin;
			const Vec3 vDelta = vOrigin - vPrevOrigin;
			if (vDelta.Length2DSqr() > 4096.f)
			{
				mRecords[pEntity].clear();
			}
		}

		//fix unchokepred
		if (Vars::Backtrack::UnchokePrediction.Value ? IsSimulationReliable(pEntity) : flDelta > 0)	//	this is silly
		{
			//	create record on simulated players
			matrix3x4 bones[128];
			if (!pEntity->SetupBones(bones, 128, BONE_USED_BY_ANYTHING, flSimTime)) { continue; }

			mRecords[pEntity].push_front({
				flSimTime,
				flCurTime,
				iTickcount,
				mDidShoot[pEntity->GetIndex()],
				*reinterpret_cast<BoneMatrixes*>(&bones),
				vOrigin
			});
		}
		else if (Vars::Backtrack::UnchokePrediction.Value) {	//	user is choking, predict location of next record.
			//	IF THE CHEAT LAGS HERE, IT CAN CREATE A PREDICTED RECORD FOR AFTER A PLAYER HAS EXITED A CHOKE, WHICH IS BAD (probably)!!!!
			const Vec3 vOriginalPos = pEntity->GetAbsOrigin();
			const Vec3 vOriginalEyeAngles = pEntity->GetEyeAngles();
			const float flNextSimTime = flSimTime + I::GlobalVars->interval_per_tick;
			const float flDeltaRecorded = flNextSimTime - mRecords[pEntity].empty() ? flSimTime : mRecords[pEntity].front().flSimTime;
			if (flDeltaRecorded < I::GlobalVars->interval_per_tick) { continue; }	//	maybe they are smooth warping???.
			//if (pEntity->GetVelocity().Length2D() > 4096.f) { continue; }	//	this will only happen on people that are stuck or it will be caught elsewhere, dont use
			
			PlayerStorage storage;
			if (F::MoveSim.Initialize(pEntity, storage))
			{
				F::MoveSim.RunTick(storage);

				//	we've predicted their next record, and we can probably predict their next lag record but if they are fakelagging it's pointless n shieet
				//	i was going to check if this lag comp would be valid here but it seems almost pointless now, dont do it.
				//	might need to do this for setupbones
				pEntity->SetAbsOrigin(storage.m_MoveData.m_vecAbsOrigin);
				pEntity->SetEyeAngles(storage.m_MoveData.m_vecViewAngles);

				matrix3x4 bones[128];
				if (!pEntity->SetupBones(bones, 128, BONE_USED_BY_ANYTHING, flNextSimTime)) { //	if we fail bones we fail lyfe
					pEntity->SetAbsOrigin(vOriginalPos);
					pEntity->SetEyeAngles(vOriginalEyeAngles);
					continue;
				}

				//create lag record
				mRecords[pEntity].push_front({
					flNextSimTime,
					flCurTime + I::GlobalVars->interval_per_tick,
					iTickcount + 1,
					false,
					*reinterpret_cast<BoneMatrixes*>(&bones),
					storage.m_MoveData.m_vecAbsOrigin
				});

				//restore
				pEntity->SetAbsOrigin(vOriginalPos);
				pEntity->SetEyeAngles(vOriginalEyeAngles);

				F::MoveSim.Restore(storage);
			}
		}
		//cleanup
		mDidShoot[pEntity->GetIndex()] = false;
		if (mRecords[pEntity].size() > 67)
		{
			/*Utils::ConLog("LagCompensation", "Manually removed tick record", {255, 0, 0, 255});*/
			mRecords[pEntity].pop_back();
		} //	schizoid check
	}
}

void CBacktrack::PlayerHurt(CGameEvent* pEvent)
{
	//const int iIndex = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
	//if (CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(iIndex)){
	//	mRecords[pEntity].clear();	//	bone cache has gone to poop for this entity, they must be cleansed in holy fire :smiling_imp:
	//}
}

void CBacktrack::ResolverUpdate(CBaseEntity* pEntity) {
	mRecords[pEntity].clear();	//	TODO: eventually remake records and rotate them or smthn idk, maybe just rotate them
}

void CBacktrack::FrameStageNotify()
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!pLocal || !iNetChan) { return Restart(); }

	flLatencyRampup = std::min(1.f, flLatencyRampup += I::GlobalVars->interval_per_tick);
	MakeRecords();
	CleanRecords();
}

void CBacktrack::ReportShot(int iIndex)
{
	CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(iIndex);
	if (!pEntity) { return; }
	mDidShoot[pEntity->GetIndex()] = true;
}

std::optional<TickRecord> CBacktrack::GetHitRecord(CUserCmd* pCmd, CBaseEntity* pEntity, const Vec3 vAngles, const Vec3 vPos)
{
	std::optional<TickRecord> cReturnRecord{};
	float flLastAngle = 45.f;

	for (const auto& rCurQuery : mRecords[pEntity])
	{
		if (!WithinRewind(rCurQuery)) { continue; }
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

std::optional<TickRecord> CBacktrack::Run(CUserCmd* pCmd) // backtrack to crosshair
{
	if (!Vars::Backtrack::Enabled.Value)
		return std::nullopt;
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
			if (!pEnemy || !pEnemy->IsAlive())
				continue; //	dont scan

			PlayerInfo_t pInfo{}; //	dont care about ignored players
			if (!I::EngineClient->GetPlayerInfo(pEnemy->GetIndex(), &pInfo))
			{
				if (G::IsIgnored(pInfo.friendsID))
					continue;
			}

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

/* unused
std::optional<TickRecord> CBacktrack::Aimbot(CBaseEntity* pEntity, BacktrackMode iMode, int nHitbox)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal) { return std::nullopt; }
	if (mRecords[pEntity].empty()) { return std::nullopt; }
	switch (iMode)
	{
	case BacktrackMode::ALL:
	{
		for (const auto& rCurQuery : mRecords[pEntity])
		{
			if (!WithinRewind(rCurQuery) || !IsTracked(rCurQuery) || IsEarly(pEntity, rCurQuery)) { continue; }
			const Vec3 vHitboxPos = pEntity->GetHitboxPosMatrix(nHitbox, (matrix3x4*)(&rCurQuery.BoneMatrix.BoneMatrix));
			if (Utils::VisPos(pLocal, pEntity, pLocal->GetShootPos(), vHitboxPos)) { return rCurQuery; }
		}
		return std::nullopt;
	}

	case BacktrackMode::LAST:
	{
		if (std::optional<TickRecord> LastRecord = GetLastRecord(pEntity))
		{
			const Vec3 vHitboxPos = pEntity->GetHitboxPosMatrix(nHitbox, (matrix3x4*)(&LastRecord->BoneMatrix.BoneMatrix));
			if (Utils::VisPos(pLocal, pEntity, pLocal->GetShootPos(), vHitboxPos)) { return LastRecord; }
		}
		return std::nullopt;
	}

	case BacktrackMode::PREFERONSHOT:
	{
		std::optional<TickRecord> ReturnTick{};
		for (const auto& rCurQuery : mRecords[pEntity])
		{
			if (!WithinRewind(rCurQuery) || !IsTracked(rCurQuery) || IsEarly(pEntity, rCurQuery)) { continue; }
			const Vec3 vHitboxPos = pEntity->GetHitboxPosMatrix(nHitbox, (matrix3x4*)(&rCurQuery.BoneMatrix.BoneMatrix));
			if (Utils::VisPos(pLocal, pEntity, pLocal->GetShootPos(), vHitboxPos)) { ReturnTick = rCurQuery; }
			if (ReturnTick.has_value())
			{
				if (ReturnTick->bOnShot) { break; }
			}
		}
		return ReturnTick;
	}
	}
	return std::nullopt;
}
*/

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

void CBacktrack::Restart()
{
	mRecords.clear();
	dSequences.clear();
	flLatencyRampup = 0.f;
	iLastInSequence = 0;
}