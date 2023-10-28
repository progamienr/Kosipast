#include "FakeLag.h"
#include "../../Visuals/FakeAngleManager/FakeAng.h"

bool CFakeLag::IsAllowed(CBaseEntity* pLocal)
{
	const int iMaxSend = 22 - G::ShiftedTicks;
	const bool bVar = Vars::CL_Move::FakeLag::Enabled.Value || bPreservingBlast;
	const bool bChargePrio = (iMaxSend > 0 && G::ChokeAmount < iMaxSend) || !G::ShiftedTicks;
	const bool bNeedAirUpdate = iAirTicks >= 14 && !pLocal->OnSolid();
	const bool bAttacking = G::IsAttacking && Vars::CL_Move::FakeLag::UnchokeOnAttack.Value;
	const bool bNotAir = Vars::CL_Move::FakeLag::WhileGrounded.Value && !pLocal->OnSolid();

	if (!bVar || bNeedAirUpdate || !bChargePrio || bAttacking || bNotAir)
		return false;

	if (bPreservingBlast || bUnducking)
		return true;

	if (G::ShiftedGoal != G::ShiftedTicks)
		return false;
	
	const bool bMoving = !Vars::CL_Move::FakeLag::WhileMoving.Value || pLocal->GetVecVelocity().Length2D() > 10.f;
	if (!bMoving)
		return false;

	if (Vars::CL_Move::FakeLag::Mode.Value == 1 && !F::KeyHandler.Down(Vars::CL_Move::FakeLag::Key.Value))
		return false;

	switch (Vars::CL_Move::FakeLag::Mode.Value) 
	{
	case FL_Plain:
	case FL_Random: {
		return G::ChokeAmount < G::ChokeGoal;
	}
	case FL_Adaptive: {
		const Vec3 vDelta = vLastPosition - pLocal->m_vecOrigin();
		return vDelta.Length2DSqr() < 4096.f;
	}
	default: { return false; }
	}
}

void CFakeLag::PreserveBlastJump(const int nOldGround, const int nOldFlags)
{
	bPreservingBlast = false;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost())
		return;

	const bool bVar = Vars::CL_Move::FakeLag::RetainBlastJump.Value;
	const bool bPlayerReady = pLocal->IsPlayer() && (pLocal->OnSolid() || nOldGround >= 0 || nOldFlags & FL_ONGROUND);
	const bool bCanPreserve = pLocal->GetClassNum() == ETFClass::CLASS_SOLDIER && pLocal->GetCondEx2() & TFCondEx2_BlastJumping;
	const bool bValid = GetAsyncKeyState(VK_SPACE) & 0x8000 && !pLocal->IsDucking();

	bPreservingBlast = bVar && bPlayerReady && bCanPreserve && bValid;
}

void CFakeLag::Unduck(const int nOldFlags)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost())
		return;

	const bool bVar = Vars::CL_Move::FakeLag::WhileUnducking.Value;
	const bool bPlayerReady = pLocal->IsPlayer() && pLocal->OnSolid() && pLocal->IsDucking() && nOldFlags & FL_ONGROUND;

	bUnducking = bVar && bPlayerReady;
}

void CFakeLag::Prediction(const int nOldGroundInt, const int nOldFlags)
{
	PreserveBlastJump(nOldGroundInt, nOldFlags);
	Unduck(nOldFlags);
}

void CFakeLag::Run(CUserCmd* pCmd, bool* pSendPacket, const int nOldGroundInt, const int nOldFlags)
{
	if (Vars::CL_Move::FakeLag::Mode.Value == 2 && F::KeyHandler.Pressed(Vars::CL_Move::FakeLag::Key.Value))
		Vars::CL_Move::FakeLag::Enabled.Value = !Vars::CL_Move::FakeLag::Enabled.Value;

	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!iNetChan || !pLocal)
		return;

	G::ChokeAmount = iNetChan->m_nChokedPackets;
	G::ChokeGoal = Vars::CL_Move::FakeLag::Value.Value;
	Prediction(nOldGroundInt, nOldFlags);

	// Set the selected choke amount (if not random)
	switch (Vars::CL_Move::FakeLag::Type.Value)
	{
	case FL_Plain: G::ChokeGoal = Vars::CL_Move::FakeLag::Value.Value; break;
	case FL_Adaptive: G::ChokeGoal = 22; break;
	}

	// Are we even allowed to choke?
	if (!IsAllowed(pLocal))
	{
		vLastPosition = pLocal->m_vecOrigin();
		if (Vars::CL_Move::FakeLag::Type.Value == FL_Random)
			G::ChokeGoal = Utils::RandIntSimple(Vars::CL_Move::FakeLag::Min.Value, Vars::CL_Move::FakeLag::Max.Value);
		iAirTicks = 0;
		bUnducking = false;
		return;
	}

	*pSendPacket = false;

	if (!pLocal->OnSolid())
		iAirTicks++;
}