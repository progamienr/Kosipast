#include "AutoBlast.h"

#include "../../Vars.h"
#include "../AutoGlobal/AutoGlobal.h"

void CAutoAirblast::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Auto::Airblast::Active.Value || !G::WeaponCanSecondaryAttack)
		return;

	id = pWeapon->GetWeaponID();

	if (id != TF_WEAPON_FLAMETHROWER && id != TF_WEAPON_FLAME_BALL)
		return;

	if (G::CurItemDefIndex == Pyro_m_ThePhlogistinator)
		return;

	if (const auto& pNet = I::EngineClient->GetNetChannelInfo())
	{
		const Vec3 vEyePos = pLocal->GetEyePosition();
		const float flLatency = (pNet->GetLatency(FLOW_INCOMING) + pNet->GetLatency(FLOW_OUTGOING));
		// pretty sure the game shows the predicted position of projectiles so accounting for incoming ping seems useless.
		bool bShouldBlast = false;

		for (const auto& pProjectile : g_EntityCache.GetGroup(EGroupType::WORLD_PROJECTILES))
		{
			if (pProjectile->m_iTeamNum() == pLocal->m_iTeamNum())
				continue; //Ignore team's projectiles

			switch (pProjectile->GetClassID())
			{
			case ETFClassID::CTFGrenadePipebombProjectile:
			case ETFClassID::CTFStunBall:
			{
				if (pProjectile->GetTouched())
					continue; //Ignore landed stickies and sandman balls
				break;
			}
			case ETFClassID::CTFProjectile_Arrow:
			{
				if (pProjectile->GetVelocity().IsZero())
					continue; //Ignore arrows with no velocity / not moving
				break;
			}
			}

			Vec3 vPredicted = (pProjectile->GetAbsOrigin() + pProjectile->GetVelocity().Scale(flLatency / 1000.f));
			CGameTrace trace = {};
			static CTraceFilterWorldAndPropsOnly traceFilter = {};
			Utils::TraceHull(pProjectile->GetAbsOrigin(), vPredicted, pProjectile->m_vecMins(), pProjectile->m_vecMaxs() * -1.f, MASK_SHOT_HULL, &traceFilter, &trace);
			if (trace.flFraction < 0.98f && !trace.entity) { continue; }

			if (Vars::Auto::Airblast::Rage.Value) //possibly implement proj aimbot somehow ?
			{
				{ //see if it is possible to reflect with existing viewangles
					Vec3 vForward = {};
					Math::AngleVectors(pCmd->viewangles, &vForward);
					const Vec3 bBoxOrigin = pLocal->GetShootPos() + (vForward * 128.f);

					if (std::abs(bBoxOrigin.x - vPredicted.x) <= 128.0f &&
						std::abs(bBoxOrigin.y - vPredicted.y) <= 128.0f &&
						std::abs(bBoxOrigin.z - vPredicted.z) <= 128.0f &&
						Utils::VisPos(pLocal, pProjectile, vEyePos, vPredicted))
					{
						bShouldBlast = true;
						break;
					}
				}
				{ //if not then manipulate viewangles
					Vec3 pAngle = Math::CalcAngle(vEyePos, vPredicted);
					Vec3 vForward = {};
					Math::AngleVectors(pAngle, &vForward);
					const Vec3 bBoxOrigin = pLocal->GetShootPos() + (vForward * 128.f);

					if (std::abs(bBoxOrigin.x - vPredicted.x) <= 128.0f &&
						std::abs(bBoxOrigin.y - vPredicted.y) <= 128.0f &&
						std::abs(bBoxOrigin.z - vPredicted.z) <= 128.0f &&
						Utils::VisPos(pLocal, pProjectile, vEyePos, vPredicted))
					{
						pCmd->viewangles = pAngle;
						bShouldBlast = true;
						break;
					}
				}
			}
			if (Math::GetFov(I::EngineClient->GetViewAngles(), vEyePos, vPredicted) <= Vars::Auto::Airblast::Fov.Value)
			{
				Vec3 vForward = {};
				Math::AngleVectors(pCmd->viewangles, &vForward);
				const Vec3 bBoxOrigin = pLocal->GetShootPos() + (vForward * 128.f);

				if (std::abs(bBoxOrigin.x - vPredicted.x) <= 128.0f &&
					std::abs(bBoxOrigin.y - vPredicted.y) <= 128.0f &&
					std::abs(bBoxOrigin.z - vPredicted.z) <= 128.0f &&
					Utils::VisPos(pLocal, pProjectile, vEyePos, vPredicted))
				{
					bShouldBlast = true;
					break;
				}
			}
		}

		if (Vars::Auto::Airblast::ExtinguishPlayers.Value)
		{
			for (const auto& pBurningPlayer : g_EntityCache.GetGroup(EGroupType::PLAYERS_TEAMMATES))
			{
				if (!pBurningPlayer->IsOnFire() || !pBurningPlayer->IsAlive() || pBurningPlayer->IsAGhost())
					continue;

				if (Vars::Auto::Airblast::Rage.Value)
				{
					Vec3 pAngle = Math::CalcAngle(vEyePos, pBurningPlayer->m_vecOrigin());
					Vec3 vForward = {};
					Math::AngleVectors(pAngle, &vForward);
					const Vec3 bBoxOrigin = pLocal->GetShootPos() + (vForward * 128.f);

					if (std::abs(bBoxOrigin.x - pBurningPlayer->m_vecOrigin().x) <= 128.0f && // in reality is an intersection check, but should work fine here
						std::abs(bBoxOrigin.y - pBurningPlayer->m_vecOrigin().y) <= 128.0f &&
						std::abs(bBoxOrigin.z - pBurningPlayer->m_vecOrigin().z) <= 128.0f &&
						Utils::VisPos(pLocal, pBurningPlayer, vEyePos, pBurningPlayer->m_vecOrigin()))
					{
						pCmd->viewangles = pAngle;
						bShouldBlast = true;
						break;
					}
				}
				if (Math::GetFov(I::EngineClient->GetViewAngles(), vEyePos, pBurningPlayer->m_vecOrigin()) <= Vars::Auto::Airblast::Fov.Value)
				{
					Vec3 vForward = {};
					Math::AngleVectors(pCmd->viewangles, &vForward);
					const Vec3 bBoxOrigin = pLocal->GetShootPos() + (vForward * 128.f);

					if (std::abs(bBoxOrigin.x - pBurningPlayer->m_vecOrigin().x) <= 128.0f &&
						std::abs(bBoxOrigin.y - pBurningPlayer->m_vecOrigin().y) <= 128.0f &&
						std::abs(bBoxOrigin.z - pBurningPlayer->m_vecOrigin().z) <= 128.0f &&
						Utils::VisPos(pLocal, pBurningPlayer, vEyePos, pBurningPlayer->m_vecOrigin()))
					{
						bShouldBlast = true;
						break;
					}
				}
			}
		}

		if (bShouldBlast)
		{
			if (Vars::Auto::Airblast::Rage.Value || Vars::Auto::Airblast::Fov.Value == 0 && Vars::Auto::Airblast::Silent.Value)
				G::SilentTime = true;

			G::IsAttacking = true;
			pCmd->buttons |= IN_ATTACK2;
		}
	}
}