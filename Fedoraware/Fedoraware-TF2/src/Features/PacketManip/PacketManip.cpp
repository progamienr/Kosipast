#include "PacketManip.h"
#include "../Visuals/FakeAngle/FakeAngle.h"

inline bool CPacketManip::WillTimeOut()
{
	return I::ClientState->chokedcommands >= 21;
}

inline bool CPacketManip::AACheck(CUserCmd* pCmd)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal)
		return false;

	if ((G::DoubleTap || G::Teleport) && G::ShiftedTicks == G::ShiftedGoal)
		return false;

	return I::ClientState->chokedcommands < 3 && F::AntiAim.ShouldAntiAim(pLocal);
}

void CPacketManip::CreateMove(CUserCmd* pCmd, bool* pSendPacket)
{
	*pSendPacket = true;
	const bool bTimeout = WillTimeOut(); // prevent overchoking by just not running anything below if we believe it will cause us to time out

	if (!bTimeout)
		F::FakeLag.Run(pCmd, pSendPacket);
	else
		G::ChokeAmount = 0;

	if (!bTimeout && AACheck(pCmd) && !G::PSilentAngles)
		*pSendPacket = false;
	F::AntiAim.Run(pCmd, pSendPacket);

	F::FakeAngle.DrawChams = Vars::AntiHack::AntiAim::Active.Value || Vars::CL_Move::FakeLag::Enabled.Value && (Vars::CL_Move::FakeLag::Mode.Value == 1 ? F::KeyHandler.Down(Vars::CL_Move::FakeLag::Key.Value) : true);
}