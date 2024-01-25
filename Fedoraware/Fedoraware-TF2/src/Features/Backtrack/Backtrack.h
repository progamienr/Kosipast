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
	// logic
	bool IsTracked(const TickRecord& record);
	bool IsEarly(CBaseEntity* pEntity, const TickRecord& record);

	// utils
	void SendLerp();
	void UpdateDatagram();
	void StoreNolerp();
	void MakeRecords();
	void CleanRecords();
	std::optional<TickRecord> GetHitRecord(CUserCmd* pCmd, CBaseEntity* pEntity, Vec3 vAngles, Vec3 vPos);

	// data
	std::unordered_map<int, bool> mDidShoot;
	int iLastCreationTick = 0;

	// data - fake latency
	std::deque<CIncomingSequence> dSequences;
	float flLatencyRampup = 0.f;
	int iLastInSequence = 0;

	size_t iMaxUnlag = 0;
	bool bLastTickHeld = false;

public:
	float GetLerp();
	float GetFake();
	float GetReal();
	int iTickCount = 0;

	bool WithinRewind(const TickRecord& record);
	std::deque<TickRecord>* GetRecords(CBaseEntity* pEntity);
	std::optional<TickRecord> GetLastRecord(CBaseEntity* pEntity);
	std::optional<TickRecord> GetFirstRecord(CBaseEntity* pEntity);
	std::deque<TickRecord> GetValidRecords(CBaseEntity* pEntity, std::deque<TickRecord> pRecords, BacktrackMode iMode = BacktrackMode::ALL, CBaseEntity* pLocal = nullptr);

	void Restart();
	void FrameStageNotify();
	std::optional<TickRecord> Run(CUserCmd* pCmd);
	void PlayerHurt(CGameEvent* pEvent);
	void SetLerp(CGameEvent* pEvent);
	void ResolverUpdate(CBaseEntity* pEntity);
	void ReportShot(int iIndex);
	void AdjustPing(INetChannel* netChannel);

	bool bFakeLatency = false;
	float flWishInterp = G::LerpTime;
	float flFakeInterp = G::LerpTime;
	std::unordered_map<CBaseEntity*, std::deque<TickRecord>> mRecords;
	std::unordered_map<int, std::pair<int, matrix3x4[128]>> noInterpBones;
	std::unordered_map<int, Vec3> noInterpEyeAngles;

	bool bSettingUpBones = false;
};

ADD_FEATURE(CBacktrack, Backtrack)