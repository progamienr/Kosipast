#include "../Hooks.h"
#include "../../Features/Backtrack/Backtrack.h"

MAKE_HOOK(NetChannel_SendDatagram, S::NetChannel_SendDatagram(), int, __fastcall,
	INetChannel* netChannel, void* edx, bf_write* datagram)
{
	if (!netChannel || datagram)
		return Hook.Original<FN>()(netChannel, edx, datagram);

	auto pLocal = g_EntityCache.GetLocal();
	if (!pLocal ||/* !pLocal->IsAlive() || pLocal->IsAGhost() ||*/ !Vars::Backtrack::Enabled.Value || !Vars::Backtrack::Latency.Value)
	{
		F::Backtrack.bFakeLatency = false;
		return Hook.Original<FN>()(netChannel, edx, datagram);
	}
	F::Backtrack.bFakeLatency = true;

	const int nInSequenceNr = netChannel->m_nInSequenceNr;
	const int nInReliableState = netChannel->m_nInReliableState;
	F::Backtrack.AdjustPing(netChannel);
	const int original = Hook.Original<FN>()(netChannel, edx, datagram);
	netChannel->m_nInSequenceNr = nInSequenceNr;
	netChannel->m_nInReliableState = nInReliableState;

	return original;
}