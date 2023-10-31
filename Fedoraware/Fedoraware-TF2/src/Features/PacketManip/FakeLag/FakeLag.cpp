#include "FakeLag.h"
#include "../../Visuals/FakeAngleManager/FakeAng.h"
#include "../../Simulation/MovementSimulation/MovementSimulation.h"

bool CFakeLag::IsAllowed(CBaseEntity* pLocal)
{
	const int iMaxSend = 22 - G::ShiftedTicks;
	const bool bVar = Vars::CL_Move::FakeLag::Enabled.Value || bPreservingBlast || bUnducking;
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

	switch (Vars::CL_Move::FakeLag::Type.Value) 
	{
	case FL_Plain:
	case FL_Random:
		return G::ChokeAmount < G::ChokeGoal;
	case FL_Adaptive:
	{
		const Vec3 vDelta = vLastPosition - pLocal->m_vecOrigin();
		return vDelta.Length2DSqr() < 4096.f;
	}
	}

	return false;
}

void CFakeLag::PreserveBlastJump()
{
	bPreservingBlast = false;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost() || !pLocal->IsPlayer())
		return;

	const bool bVar = Vars::CL_Move::FakeLag::RetainBlastJump.Value;
	//static bool bOldSolid = false;
	//const bool bPlayerReady = pLocal->OnSolid() || bOldSolid;
	//bOldSolid = pLocal->OnSolid();
	bool bPlayerReady = pLocal->OnSolid();
	if (!bPlayerReady)
	{
		PlayerStorage localStorage;
		F::MoveSim.Initialize(pLocal, localStorage, false, true);
		F::MoveSim.RunTick(localStorage);
		bPlayerReady = pLocal->OnSolid();
		F::MoveSim.Restore(localStorage);
	}
	const bool bCanPreserve = pLocal->GetClassNum() == ETFClass::CLASS_SOLDIER && pLocal->GetCondEx2() & TFCondEx2_BlastJumping;
	const bool bValid = GetAsyncKeyState(VK_SPACE) & 0x8000 && !pLocal->IsDucking();

	bPreservingBlast = bVar && bPlayerReady && bCanPreserve && bValid;
}

void CFakeLag::Unduck(CUserCmd* pCmd)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost())
		return;

	const bool bVar = Vars::CL_Move::FakeLag::WhileUnducking.Value;
	const bool bPlayerReady = pLocal->IsPlayer() && pLocal->OnSolid() && pLocal->IsDucking() && !(pCmd->buttons & IN_DUCK);

	bUnducking = bVar && bPlayerReady;
}

void CFakeLag::Prediction(CUserCmd* pCmd)
{
	PreserveBlastJump();
	Unduck(pCmd);
}

void CFakeLag::Run(CUserCmd* pCmd, bool* pSendPacket)
{
	if (Vars::CL_Move::FakeLag::Mode.Value == 2 && F::KeyHandler.Pressed(Vars::CL_Move::FakeLag::Key.Value))
		Vars::CL_Move::FakeLag::Enabled.Value = !Vars::CL_Move::FakeLag::Enabled.Value;

	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!iNetChan || !pLocal)
		return;

	G::ChokeAmount = iNetChan->m_nChokedPackets;
	Prediction(pCmd);

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
		G::ChokeGoal = 0;
		iAirTicks = 0;
		bUnducking = false;
		if (Vars::CL_Move::FakeLag::Type.Value == FL_Random)
			G::ChokeGoal = Utils::RandIntSimple(Vars::CL_Move::FakeLag::Min.Value, Vars::CL_Move::FakeLag::Max.Value);
		return;
	}

	*pSendPacket = false;

	if (!pLocal->OnSolid())
		iAirTicks++;
}