#include "FakeLag.h"
#include "../../Visuals/FakeAngleManager/FakeAng.h"

bool CFakeLag::IsVisible(CBaseEntity* pLocal)
{
	const Vec3 vVisCheckPoint = pLocal->GetEyePosition();
	const Vec3 vPredictedCheckPoint = pLocal->GetEyePosition() + (pLocal->m_vecVelocity() * (I::GlobalVars->interval_per_tick * 6));	//	6 ticks in da future
	for (const auto& pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
	{
		if (!pEnemy || !pEnemy->IsAlive() || pEnemy->IsCloaked() || pEnemy->IsAGhost() || pEnemy->GetFeignDeathReady() || pEnemy->IsBonked())
			continue;

		PlayerInfo_t pInfo{};	//	ignored players shouldn't trigger this
		if (!I::EngineClient->GetPlayerInfo(pEnemy->GetIndex(), &pInfo))
		{
			if (G::IsIgnored(pInfo.friendsID))
				continue;
		}

		const Vec3 vEnemyPos = pEnemy->GetEyePosition();
		if (!Utils::VisPos(pLocal, pEnemy, vVisCheckPoint, vEnemyPos) && (Vars::CL_Move::FakeLag::PredictVisibility.Value ? !Utils::VisPos(pLocal, pEnemy, vPredictedCheckPoint, vEnemyPos) : true))
			continue;

		return true;
	}
	return false;
}

bool CFakeLag::IsAllowed(CBaseEntity* pLocal)
{
	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	const int doubleTapAllowed = 22 - G::ShiftedTicks;
	//const bool retainFakelagTest = Vars::CL_Move::RetainFakelag.Value ? G::ShiftedTicks != 1 : !G::ShiftedTicks;
	const bool retainFakelagTest = G::ShiftedTicks != 1;
	if (!iNetChan) 
		return false; // no netchannel no fakelag

	// Failsafe, in case we're trying to choke too many ticks
	if (std::max(ChokeCounter, iNetChan->m_nChokedPackets) >= 22)
		return false;

	// Should fix an issue with getting teleported back to the ground for now, pretty ghetto imo
	if (!pLocal->OnSolid() && pInAirTicks.first && pInAirTicks.second > 13)
		return false;

	// Are we attacking? TODO: Add more logic here
	if (G::IsAttacking && Vars::CL_Move::FakeLag::UnchokeOnAttack.Value)
		return false;

	// Special Cases
	if (bPreservingBlast || bUnducking)
		return true;

	// Are we recharging
	if ((ChokeCounter >= doubleTapAllowed || G::Recharging || G::Recharge || G::Teleporting || !retainFakelagTest) && Vars::CL_Move::DoubleTap::Enabled.Value)
		return false;

	if (Vars::CL_Move::FakeLag::WhileInAir.Value && !pLocal->OnSolid())
		return false;
	//	no other checks, we want this

	// Is a fakelag key set and pressed?
	static KeyHelper fakelagKey{ &Vars::CL_Move::FakeLag::Key.Value };
	if (!fakelagKey.Down() && Vars::CL_Move::FakeLag::Mode.Value == 1)
		return false;

	// Do we have enough velocity for velocity mode?
	if (Vars::CL_Move::FakeLag::WhileMoving.Value && pLocal->GetVecVelocity().Length2D() < 10.f)
		return false;

	// Are we visible to any valid enemies?
	if (Vars::CL_Move::FakeLag::WhileVisible.Value && !IsVisible(pLocal))
		return false;

	switch (Vars::CL_Move::FakeLag::Type.Value)
	{
		case FL_Plain:
		case FL_Random: return ChokeCounter < ChosenAmount;
		case FL_Adaptive:
		{
			const Vec3 vDelta = vLastPosition - pLocal->m_vecOrigin();
			return vDelta.Length2DSqr() < 4096.f;
		}
		default: return false;
	}
}

void CFakeLag::PreserveBlastJump(const int nOldGround, const int nOldFlags, CUserCmd* pCmd) {
	//if (G::IsAttacking) { return; }
	bPreservingBlast = false;
	if (!Vars::CL_Move::FakeLag::RetainBlastJump.Value)
		return;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || !pLocal->IsPlayer())
		return;

	int key = VK_SPACE; static KeyHelper spaceKey{ &key };
	if (!spaceKey.Down() || pLocal->IsDucking())
		return;

	if (!pLocal->OnSolid() && nOldGround < 0 && !(nOldFlags & FL_ONGROUND))
		return;
	if (pLocal->GetClassNum() != ETFClass::CLASS_SOLDIER)
		return;
	if (pLocal->GetCondEx2() & TFCondEx2_BlastJumping)
		bPreservingBlast = true;
}

void CFakeLag::Unduck(const int nOldFlags){
	if (!Vars::CL_Move::FakeLag::WhileUnducking.Value)
		return;

	CBaseEntity* pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsDucking() || !pLocal->IsPlayer())
		return;
	if (!(nOldFlags & FL_DUCKING))
		return;
	// we were ducking before & are not ducking next
	bUnducking = true; 
}

void CFakeLag::Prediction(const int nOldGroundInt, const int nOldFlags, CUserCmd* pCmd){
	//	do blast jump preservation here.
	PreserveBlastJump(nOldGroundInt, nOldFlags, pCmd);
	Unduck(nOldFlags);
}

void CFakeLag::OnTick(CUserCmd* pCmd, bool* pSendPacket, const int nOldGroundInt, const int nOldFlags)
{
	static KeyHelper fakelagKey{ &Vars::CL_Move::FakeLag::Key.Value };
	if (fakelagKey.Pressed() && Vars::CL_Move::FakeLag::Mode.Value == 2)
		Vars::CL_Move::FakeLag::Enabled.Value = !Vars::CL_Move::FakeLag::Enabled.Value;

	Prediction(nOldGroundInt, nOldFlags, pCmd);
	G::IsChoking = false;	//	do this first
	if (G::ShouldShift)
		return;
	if (!Vars::CL_Move::FakeLag::Enabled.Value && !bPreservingBlast)
	{
		ChokeCounter = 0; G::ChokedTicks = ChokeCounter;
		return;
	}

	// Set the selected choke amount (if not random)
	if (Vars::CL_Move::FakeLag::Type.Value != FL_Random)
		ChosenAmount = Vars::CL_Move::FakeLag::Value.Value;

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive())
	{
		*pSendPacket = true;
		ChokeCounter = 0; G::ChokedTicks = 0;

		return;
	}

	// Are we even allowed to choke?
	if (!IsAllowed(pLocal))
	{
		vLastPosition = pLocal->m_vecOrigin();
		*pSendPacket = true;
		// Set a new random amount (if desired)
		if (Vars::CL_Move::FakeLag::Type.Value == FL_Random)
			ChosenAmount = Utils::RandIntSimple(Vars::CL_Move::FakeLag::Min.Value, Vars::CL_Move::FakeLag::Max.Value);
		ChokeCounter = 0; G::ChokedTicks = 0;
		pInAirTicks = {pLocal->OnSolid(), 0};
		bUnducking = false;
		return;
	}

	G::IsChoking = true;
	*pSendPacket = false;
	ChokeCounter++; G::ChokedTicks = ChokeCounter;

	if (!pLocal->OnSolid())
		pInAirTicks.second++;
}