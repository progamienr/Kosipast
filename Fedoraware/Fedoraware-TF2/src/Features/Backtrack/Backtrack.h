#pragma once
#include "../Feature.h"

#pragma warning ( disable : 4091 )

class CIncomingSequence
{
public:
	int InReliableState;
	int SequenceNr;
	float CurTime;

	CIncomingSequence(int inState, int seqNr, float time)
	{
		InReliableState = inState;
		SequenceNr = seqNr;
		CurTime = time;
	}
};

using BoneMatrixes = struct
{
	float BoneMatrix[128][3][4];
};

struct TickRecord
{
	float flSimTime = 0.f;
	float flCreateTime = 0.f;
	int iTickCount = 0;
	bool bOnShot = false;
	BoneMatrixes BoneMatrix{};
	Vec3 vOrigin = {};
};

enum class BacktrackMode
{
	ALL, //	iterates through every tick (slow probably)
	LAST, // last
	PREFERONSHOT, // prefers on shot records, last
};

class CBacktrack
{
//	logic
	bool IsTracked(const TickRecord& record);
	bool IsSimulationReliable(CBaseEntity* pEntity);
	bool IsEarly(CBaseEntity* pEntity, const TickRecord& record);
	//bool IsBackLagComped(CBaseEntity* pEntity);

	//	utils
	void CleanRecords();
	void MakeRecords();
	std::optional<TickRecord> GetHitRecord(CUserCmd* pCmd, CBaseEntity* pEntity, Vec3 vAngles, Vec3 vPos);
	//	utils - fake latency
	void UpdateDatagram();

	//	data
//	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> mRecords;
	std::unordered_map<int, bool> mDidShoot;
	int iLastCreationTick = 0;

	//	data - fake latency
	std::deque<CIncomingSequence> dSequences;
	float flLatencyRampup = 0.f;
	int iLastInSequence = 0;

	bool bLastTickHeld = false;

public:
	void SetLerp(CGameEvent* pEvent);
	float GetLerp();
	float GetFake();
	float GetReal();
	int iTickCount = 0;

	bool WithinRewind(const TickRecord& record);
	//bool CanHitOriginal(CBaseEntity* pEntity); unused
	std::deque<TickRecord>* GetRecords(CBaseEntity* pEntity);
	std::optional<TickRecord> GetLastRecord(CBaseEntity* pEntity);
	//std::optional<TickRecord> GetFirstRecord(CBaseEntity* pEntity); unused
	std::deque<TickRecord> GetValidRecords(CBaseEntity* pEntity, std::deque<TickRecord> pRecords, BacktrackMode iMode = BacktrackMode::ALL);

	void PlayerHurt(CGameEvent* pEvent); //	called on player_hurt event
	void ResolverUpdate(CBaseEntity* pEntity);	//	omfg
	void FrameStageNotify(); //	called in FrameStageNotify
	void ReportShot(int iIndex);
	std::optional<TickRecord> Run(CUserCmd* pCmd); //	returns a valid record
	//std::optional<TickRecord> Aimbot(CBaseEntity* pEntity, BacktrackMode iMode, int nHitbox); unused
	void AdjustPing(INetChannel* netChannel); //	blurgh
	void Restart(); //	called whenever lol

	bool bFakeLatency = false;
	float flWishInterp = G::LerpTime;
	float flFakeInterp = G::LerpTime;
	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> mRecords;
	std::unordered_map<int, Vec3> noInterpEyeAngles; // i think this works
};

ADD_FEATURE(CBacktrack, Backtrack)