#include "CritHack.h"

#define TF_DAMAGE_CRIT_MULTIPLIER		3.0f
#define TF_DAMAGE_CRIT_CHANCE_RAPID		0.02f
#define TF_DAMAGE_CRIT_DURATION_RAPID	2.0f

bool CCritHack::IsEnabled()
{
	if (!Vars::CritHack::Active.Value)
		return false;
	if (!AreRandomCritsEnabled())
		return false;
	if (!I::EngineClient->IsInGame())
		return false;

	return true;
}

bool CCritHack::AreRandomCritsEnabled()
{
	if (static auto tf_weapon_criticals = g_ConVars.FindVar("tf_weapon_criticals"); tf_weapon_criticals)
		return tf_weapon_criticals->GetBool();
	return true;
}

bool CCritHack::IsAttacking(const CUserCmd* pCmd, CBaseCombatWeapon* pWeapon)
{
	if (pWeapon->GetItemDefIndex() == Soldier_m_TheBeggarsBazooka)
	{
		static bool bLoading = false, bFiring = false;

		if (pWeapon->GetClip1() == 0)
			bLoading = false,
			bFiring = false;
		else if (!bFiring)
			bLoading = true;

		if ((bFiring || bLoading && !(pCmd->buttons & IN_ATTACK)) && G::WeaponCanAttack)
		{
			bFiring = true;
			bLoading = false;
			return true;
		}
	}
	else
	{
		const int id = pWeapon->GetWeaponID();
		switch (id) {
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_STICKY_BALL_LAUNCHER:
		case TF_WEAPON_GRENADE_STICKY_BALL:
		case TF_WEAPON_CANNON:
		{
			static bool bCharging = false;

			if (pWeapon->GetChargeBeginTime() > 0.0f)
				bCharging = true;

			if (!(pCmd->buttons & IN_ATTACK) && bCharging)
			{
				bCharging = false;
				return true;
			}
			break;
		}
		case TF_WEAPON_JAR:
		case TF_WEAPON_JAR_MILK:
		case TF_WEAPON_JAR_GAS:
		case TF_WEAPON_GRENADE_JAR_GAS:
		case TF_WEAPON_CLEAVER:
		{
			static float flThrowTime = 0.0f;

			if ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack && !flThrowTime)
				flThrowTime = I::GlobalVars->curtime + I::GlobalVars->interval_per_tick;

			if (flThrowTime && I::GlobalVars->curtime >= flThrowTime)
			{
				flThrowTime = 0.0f;
				return true;
			}
			break;
		}
		case TF_WEAPON_MINIGUN:
		{
			if (pWeapon->GetMinigunState() == AC_STATE_FIRING && (pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack)
				return true;
			break;
		}
		default:
		{
			if ((pCmd->buttons & IN_ATTACK) && G::WeaponCanAttack)
				return true;
			break;
		}
		}
	}

	return false;
}



void CCritHack::Fill(CBaseCombatWeapon* pWeapon, const CUserCmd* pCmd, int loops)
{
	static int prev_weapon = 0;
	static int previousCrit = 0;
	static int starting_num = pCmd->command_number;

	if (/*Utils::*/IsAttacking(pCmd, pWeapon) && G::WeaponCanAttack/* || pCmd->buttons & IN_ATTACK*/)
		return;

	if (prev_weapon != pWeapon->GetIndex())
	{
		starting_num = pCmd->command_number;
		prev_weapon = pWeapon->GetIndex();
		ForceCmds.clear();
		SkipCmds.clear();
	}

	//ProtectData = true;
	//const int seed_backup = MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF;
	for (int i = 0; i < loops; i++)
	{
		if (ForceCmds.size() >= 15)
			break;

		const int cmd_num = starting_num + i;

		if (IsCritCommand(cmd_num))
			ForceCmds.push_back(cmd_num);
	}
	for (int i = 0; i < loops; i++)
	{
		if (SkipCmds.size() >= 15)
			break;

		const int cmd_num = starting_num + i;

		if (IsCritCommand(cmd_num, false))
			SkipCmds.push_back(cmd_num);
	}
	starting_num += loops;
	//ProtectData = false;
	*reinterpret_cast<int*>(reinterpret_cast<DWORD>(pWeapon) + 0xA5C) = 0;
	//*I::RandomSeed = seed_backup;
}

int CCritHack::LastGoodCritTick(const CUserCmd* pCmd)
{
	int retVal = -1;
	bool popBack = false;

	for (const auto& cmd : ForceCmds)
	{
		if (cmd >= pCmd->command_number)
			retVal = cmd;
		else
			popBack = true;
	}

	if (popBack)
		ForceCmds.pop_front();

	return retVal;
}

int CCritHack::LastGoodSkipTick(const CUserCmd* pCmd)
{
	int retVal = -1;
	bool popBack = false;

	for (const auto& cmd : SkipCmds)
	{
		if (cmd >= pCmd->command_number)
			retVal = cmd;
		else
			popBack = true;
	}

	if (popBack)
		SkipCmds.pop_front();

	return retVal;
}



bool CCritHack::IsCritCommand(const i32 command_number, const bool crit)
{
	const auto& pWeapon = g_EntityCache.GetWeapon();
	if (!pWeapon)
		return false;

	const auto random_seed = MD5_PseudoRandom(command_number) & 0x7FFFFFFF;
	Utils::RandomSeed(DecryptOrEncryptSeed(pWeapon, random_seed));
	const auto random_int = Utils::RandomInt(0, 9999);

	return crit ? random_int < 100 : random_int > 6000;
}

u32 CCritHack::DecryptOrEncryptSeed(CBaseCombatWeapon* pWeapon, u32 seed)
{
	if (!pWeapon)
		return 0;

	unsigned int iMask = pWeapon->GetIndex() << 8 | I::EngineClient->GetLocalPlayer();

	if (pWeapon->GetSlot() == SLOT_MELEE)
		iMask <<= 8;

	return iMask ^ seed;
}



void CCritHack::GetTotalCrits(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon) // fix rapid cost
{
	if (!pLocal || !pWeapon || pLocal->deadflag())
		return;

	if (!Vars::CritHack::Active.Value || Storage[pWeapon->GetSlot()].BaseDamage == 0)
		return;

	Storage[pWeapon->GetSlot()].Damage = Storage[pWeapon->GetSlot()].BaseDamage;
	if (pWeapon->IsStreamingWeapon())
	{
		Storage[pWeapon->GetSlot()].Damage = Storage[pWeapon->GetSlot()].BaseDamage * TF_DAMAGE_CRIT_DURATION_RAPID / pWeapon->GetWeaponData().m_flTimeFireDelay;

		if (Storage[pWeapon->GetSlot()].Damage * TF_DAMAGE_CRIT_MULTIPLIER > BucketCap)
			Storage[pWeapon->GetSlot()].Damage = BucketCap / TF_DAMAGE_CRIT_MULTIPLIER;
	}
	if (pWeapon->GetSlot() == SLOT_MELEE)
		Storage[pWeapon->GetSlot()].Damage *= 4.f / 3.f;

	float flMult = 1.f;
	if (pWeapon->GetSlot() == SLOT_MELEE)
		flMult = 0.5f;
	else if (Storage[pWeapon->GetSlot()].ShotsCrits.second > 0 && Storage[pWeapon->GetSlot()].ShotsCrits.first > 0)
		flMult = Math::RemapValClamped((float)Storage[pWeapon->GetSlot()].ShotsCrits.second / (float)Storage[pWeapon->GetSlot()].ShotsCrits.first, 0.1f, 1.f, 1.f, 3.f);

	if (pWeapon->GetSlot() == SLOT_MELEE)
	{
		const float cost = Storage[pWeapon->GetSlot()].Damage * flMult;
		Storage[pWeapon->GetSlot()].BaseCost = cost;
		Storage[pWeapon->GetSlot()].Cost = cost;
	}
	else
	{
		Storage[pWeapon->GetSlot()].BaseCost = Storage[pWeapon->GetSlot()].Damage * 3;
		Storage[pWeapon->GetSlot()].Cost = Storage[pWeapon->GetSlot()].BaseCost * flMult;
	}

	auto Bucket = Storage[pWeapon->GetSlot()].Bucket;

	if (BucketCap)
		Storage[pWeapon->GetSlot()].PotentialCrits = static_cast<unsigned int>((BucketCap - BucketBottom) / Storage[pWeapon->GetSlot()].BaseCost);

	if (pWeapon->GetSlot() == SLOT_MELEE)
		Storage[pWeapon->GetSlot()].AvailableCrits = std::floor((Bucket - BucketBottom) / Storage[pWeapon->GetSlot()].Cost);
	else
	{
		int shots = Storage[pWeapon->GetSlot()].ShotsCrits.first, crits = Storage[pWeapon->GetSlot()].ShotsCrits.second;
		if (shots > 0 && crits > 0)
		{
			int iCrits = 0;

			float bucket = Bucket - BucketBottom, flCost = Storage[pWeapon->GetSlot()].BaseCost;
			const int iAttempts = std::min(Storage[pWeapon->GetSlot()].PotentialCrits + 1, 100); // just in case
			for (int i = 0; i < iAttempts; i++)
			{
				flMult = Math::RemapValClamped((float)crits / (float)shots, 0.1f, 1.f, 1.f, 3.f);

				const float cost = flCost * flMult;
				bucket -= cost;

				if (bucket < 0.f)
					break;

				shots++; crits++;

				iCrits++;
			}

			Storage[pWeapon->GetSlot()].AvailableCrits = iCrits;
		}
		else
			Storage[pWeapon->GetSlot()].AvailableCrits = std::floor((Bucket - BucketBottom) / Storage[pWeapon->GetSlot()].Cost);
	}
}

void CCritHack::CanFireCriticalShotHandler(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	//pWeapon->SetObservedCritChance(0.f);
	CritBanned = false;
	CritChance = 0.02f * pLocal->GetCritMult() + 0.1f;
	if (pWeapon->IsStreamingWeapon())
	{
		float flNonCritDuration = (TF_DAMAGE_CRIT_DURATION_RAPID / CritChance) - TF_DAMAGE_CRIT_DURATION_RAPID;
		CritChance = 1.f / flNonCritDuration;
	}

	if (CritDamage == 0.f || pWeapon->GetSlot() == SLOT_MELEE)
		return;

	const auto divCritDamage = CritDamage / TF_DAMAGE_CRIT_MULTIPLIER;
	const auto ratio = divCritDamage / (AllDamage - 2 * divCritDamage);
	//pWeapon->SetObservedCritChance(ratio);

	CritBanned = ratio >= CritChance;
}

void CCritHack::GetDamageTilUnban(CBaseEntity* pLocal)
{
	DamageTilUnban = 0;

	if (!CritBanned)
		return;

	const auto divCritDamage = CritDamage / TF_DAMAGE_CRIT_MULTIPLIER;

	DamageTilUnban = divCritDamage / CritChance + 2 * divCritDamage - AllDamage; // might be 10x or 100x more than necessary ?
}

void CCritHack::FixHeavyRevBug(CUserCmd* pCmd)
{
	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();
	if (!pLocal || !pWeapon || pLocal->deadflag())
		return;

	if (!pLocal->IsClass(CLASS_HEAVY) || pWeapon->GetWeaponID() != TF_WEAPON_MINIGUN)
		return;

	if (pCmd->buttons & IN_ATTACK)
		pCmd->buttons &= ~IN_ATTACK2;
}

bool CCritHack::WeaponCanCrit(CBaseCombatWeapon* pWeapon)
{
	bool result = true;
	switch (pWeapon->GetItemDefIndex())
	{
	case Scout_s_MadMilk:
	case Scout_s_MutatedMilk:
	case Scout_s_BonkAtomicPunch:
	case Scout_s_CritaCola:
	case Soldier_m_RocketJumper:
	case Soldier_t_TheMarketGardener:
	case Soldier_s_TheBuffBanner:
	case Soldier_s_FestiveBuffBanner:
	case Soldier_s_TheConcheror:
	case Soldier_s_TheBattalionsBackup:
	case Pyro_m_DragonsFury:
	case Pyro_m_ThePhlogistinator:
	case Pyro_s_TheManmelter:
	case Pyro_s_GasPasser:
	case Pyro_t_NeonAnnihilator:
	case Pyro_t_NeonAnnihilatorG:
	case Pyro_t_TheAxtinguisher:
	case Pyro_t_TheFestiveAxtinguisher:
	case Pyro_t_ThePostalPummeler:
	case Demoman_s_StickyJumper:
	case Demoman_t_UllapoolCaber:
	case Engi_m_ThePomson6000:
	case Engi_m_TheFrontierJustice:
	case Engi_m_FestiveFrontierJustice:
	case Engi_s_TheShortCircuit:
	case Engi_s_TheWrangler:
	case Engi_s_FestiveWrangler:
	case Engi_s_TheGigarCounter:
	case Engi_t_TheGunslinger:
	case Engi_t_TheSouthernHospitality:
	case Sniper_m_TheHuntsman:
	case Sniper_m_TheFortifiedCompound:
	case Sniper_s_Jarate:
	case Sniper_s_TheSelfAwareBeautyMark:
	case Sniper_t_TheBushwacka:
	case Spy_m_TheAmbassador:
	case Spy_m_FestiveAmbassador:
	case Spy_m_TheDiamondback:
	case Spy_m_TheEnforcer:
	{
		result = false;
		break;
	}
	default:
	{
		switch (pWeapon->GetWeaponID())
		{
		case TF_WEAPON_SNIPERRIFLE:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		case TF_WEAPON_SNIPERRIFLE_DECAP:
		case TF_WEAPON_CLEAVER:
		case TF_WEAPON_FLAMETHROWER_ROCKET:
		case TF_WEAPON_MEDIGUN:
		case TF_WEAPON_SWORD:
		case TF_WEAPON_KNIFE:
		case TF_WEAPON_PDA_SPY:
		case TF_WEAPON_BUILDER:
		case TF_WEAPON_PDA_SPY_BUILD:
		case TF_WEAPON_PDA:
		case TF_WEAPON_PDA_ENGINEER_BUILD:
		case TF_WEAPON_PDA_ENGINEER_DESTROY:
		case TF_WEAPON_PARTICLE_CANNON:
		case TF_WEAPON_LUNCHBOX:
		{
			result = false;
			break;
		}
		}
	}
	}

	return result;
}



void CCritHack::ResetWeapon(CBaseCombatWeapon* pWeapon)
{
	const auto slot = pWeapon->GetSlot();
	const auto index = pWeapon->GetItemDefIndex();
	if (Storage[slot].DefIndex == index)
		return;

	if (Vars::Debug::DebugInfo.Value)
		I::Cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "Resetting weapon.\n");

	Storage[slot] = {};
	Storage[slot].DefIndex = index;
}

void CCritHack::Reset()
{
	if (Vars::Debug::DebugInfo.Value)
		I::Cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "Resetting.\n");

	ForceCmds = {};
	SkipCmds = {};

	Storage = {};

	CritDamage = 0;
	AllDamage = 0;

	CritBanned = false;
	DamageTilUnban = 0;
	CritChance = 0.f;
	ProtectData = false;
}



void CCritHack::Run(CUserCmd* pCmd)
{
	FixHeavyRevBug(pCmd);

	auto bucketDefault = g_ConVars.FindVar("tf_weapon_criticals_bucket_default");
	auto bucketBottom = g_ConVars.FindVar("tf_weapon_criticals_bucket_bottom");
	auto bucketCap = g_ConVars.FindVar("tf_weapon_criticals_bucket_cap");

	if (!bucketDefault || !bucketBottom || !bucketCap)
		return;

	BucketDefault = bucketDefault->GetFloat();
	BucketBottom = bucketBottom->GetFloat();
	BucketCap = bucketCap->GetFloat();

	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();
	const auto& resource = g_EntityCache.GetPR();

	if (!pLocal || !pWeapon || !resource)
		return;

	if (Storage[pWeapon->GetSlot()].StreamWait <= I::GlobalVars->tickcount - 1)
		Storage[pWeapon->GetSlot()].StreamWait = -1;
	if (Storage[pWeapon->GetSlot()].StreamEnd <= I::GlobalVars->tickcount - 1)
		Storage[pWeapon->GetSlot()].StreamEnd = -1;

	const bool bRapidFire = pWeapon->IsStreamingWeapon();
	const bool bStreamWait = Storage[pWeapon->GetSlot()].StreamWait > 0;
	const bool bStreamEnd = Storage[pWeapon->GetSlot()].StreamEnd > 0;

	ResetWeapon(pWeapon);
	if (pLocal->deadflag() || !WeaponCanCrit(pWeapon) || pLocal->InCond(TF_COND_CRITBOOSTED)/* || !pWeapon->CanFireCriticalShot(false)*/)
		return;

	if (!G::ShouldShift)
	{
		CanFireCriticalShotHandler(pLocal, pWeapon);
		GetDamageTilUnban(pLocal);
		Fill(pWeapon, pCmd, 15);
	}

	if (Storage[pWeapon->GetSlot()].BaseDamage == 0)
	{
		pWeapon->WillCrit();
		GetTotalCrits(pLocal, pWeapon);
	}

	const int closestCrit = LastGoodCritTick(pCmd);
	const int closestSkip = LastGoodSkipTick(pCmd);

	static KeyHelper critKey{ &Vars::CritHack::CritKey.Value };
	bool pressed = critKey.Down();
	if (!pressed && Vars::CritHack::AlwaysMelee.Value && pWeapon->GetSlot() == SLOT_MELEE)
		pressed = true;
	if (IsAttacking(pCmd, pWeapon) && !pWeapon->IsInReload()/*&& !CritBanned && !G::ShouldShift*/) //	is it valid & should we even use it
	{
		//ProtectData = true;
		Storage[pWeapon->GetSlot()].ShotsCrits.first += 1;

		if (IsEnabled())
		{
			if (pressed && Storage[pWeapon->GetSlot()].AvailableCrits > 0 && (!CritBanned || pWeapon->GetSlot() == SLOT_MELEE) && closestCrit >= 0 && !bStreamWait && !bStreamEnd)
				pCmd->command_number = closestCrit;
			else if (Vars::CritHack::AvoidRandom.Value && closestSkip >= 0)
				pCmd->command_number = closestSkip;
		}

		if (bRapidFire)
		{
			if (bStreamEnd)
			{
				Storage[pWeapon->GetSlot()].ShotsCrits.second += 1;
				goto out;
			}
			if (bStreamWait)
				goto out;
		}

		if (bRapidFire && !bStreamWait)
			Storage[pWeapon->GetSlot()].StreamWait = I::GlobalVars->tickcount + 1 / I::GlobalVars->interval_per_tick;

		if (IsCritCommand(pCmd->command_number))
		{
			if (bRapidFire)
				Storage[pWeapon->GetSlot()].StreamEnd = I::GlobalVars->tickcount + 2 / I::GlobalVars->interval_per_tick;
			Storage[pWeapon->GetSlot()].ShotsCrits.second += 1;
			Storage[pWeapon->GetSlot()].Bucket = std::max(Storage[pWeapon->GetSlot()].Bucket - Storage[pWeapon->GetSlot()].Cost, BucketBottom);
		}
		else
			Storage[pWeapon->GetSlot()].Bucket = std::min(Storage[pWeapon->GetSlot()].Bucket + Storage[pWeapon->GetSlot()].Damage, BucketCap);
	}
	/*
	else if (IsEnabled() && closest_skip >= 0)
	{
		pCmd->command_number = closest_skip;
	}
	*/

out:
	//ProtectData = false;
	*I::RandomSeed = MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF;

	if (!G::ShouldShift)
		GetTotalCrits(pLocal, pWeapon);

	if (pCmd->command_number == closestCrit)
		ForceCmds.pop_front();
	else if (pCmd->command_number == closestSkip)
		SkipCmds.pop_front();
}

bool CCritHack::CalcIsAttackCriticalHandler(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	if (I::Prediction->m_bFirstTimePredicted)
		return false;

	if (!pWeapon || pLocal)
		return false;

	{
		static int s_nPreviousTickcount = 0;

		if (s_nPreviousTickcount == I::GlobalVars->tickcount)
			return false;

		s_nPreviousTickcount = I::GlobalVars->tickcount;
	}

	{
		if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN ||
			pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER)
		{
			auto nPreviousAmmoCount = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());
			static auto nNewAmmoCount = nPreviousAmmoCount;

			const auto bHasFiredBullet = nNewAmmoCount != nPreviousAmmoCount;

			if (!bHasFiredBullet)
				return false;
		}
	}

	return true;
}

void CCritHack::Event(CGameEvent* pEvent, FNV1A_t uNameHash)
{
	if (uNameHash == FNV1A::HashConst("player_hurt"))
	{
		const auto& pLocal = g_EntityCache.GetLocal();
		if (!pLocal)
			return;

		const auto attacked = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"));
		const auto attacker = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker"));
		auto	   crit = pEvent->GetInt("crit");
		auto       damage = pEvent->GetInt("damageamount");
		const auto health = pEvent->GetInt("health");
		const auto weapon_id = pEvent->GetInt("weaponid");

		if (attacker == pLocal->GetIndex() && attacked != attacker)
		{
			CBaseCombatWeapon* pWeapon = reinterpret_cast<CBaseCombatWeapon*>(I::ClientEntityList->GetClientEntityFromHandle(pEvent->GetInt("weaponid")));
			if (!pWeapon)
				pWeapon = g_EntityCache.GetWeapon();
			if (!pWeapon)
				return;

			if (pWeapon->GetSlot() == SLOT_MELEE || pLocal->InCond(TF_COND_CRITBOOSTED))
				return;

			AllDamage += damage;
			if (crit)
			{
				CritDamage += damage;

				if (Vars::Debug::DebugInfo.Value)
					I::Cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "Adding crit.\n");
			}
		}
	}
	else if (uNameHash == FNV1A::HashConst("teamplay_round_start"))
	{
		CritDamage = 0.f;
		AllDamage = 0.f;
	}
	else if (uNameHash == FNV1A::HashConst("client_beginconnect") || uNameHash == FNV1A::HashConst("client_disconnect") ||
		uNameHash == FNV1A::HashConst("game_newmap")) // i believe newmap is practically rejoining ?
	{
		if (Vars::Debug::DebugInfo.Value)
			I::Cvar->ConsoleColorPrintf({ 0, 255, 255, 255 }, "Resetting.\n");
		Reset();
	}
	else if (uNameHash == FNV1A::HashConst("player_changeclass"))
	{
		const bool bLocal = I::EngineClient->GetPlayerForUserID(pEvent->GetInt("userid")) == I::EngineClient->GetLocalPlayer();
		if (bLocal)
			Storage = {};
	}
}

void CCritHack::Draw()
{
	if (!Vars::CritHack::Active.Value || !Vars::CritHack::Indicators.Value) { return; }
	if (!IsEnabled() || !G::CurrentUserCmd)
		return;

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive())
		return;

	const auto& pWeapon = pLocal->GetActiveWeapon();
	if (!pWeapon)
		return;

	int x = Vars::CritHack::IndicatorPos.x;
	int y = Vars::CritHack::IndicatorPos.y + 8; // + Vars::Fonts::FONT_INDICATORS::nTall.Value

	EStringAlign align = ALIGN_CENTERHORIZONTAL;
	if (x <= 100)
	{
		x += 8;
		align = ALIGN_DEFAULT;
	}
	else if (x >= g_ScreenSize.w - 200)
	{
		x += 92;
		align = ALIGN_REVERSE;
	}
	else
		x += 50;

	if (WeaponCanCrit(pWeapon))
	{
		const auto slot = pWeapon->GetSlot();
		const auto bRapidFire = pWeapon->IsStreamingWeapon();

		if (Storage[slot].BaseDamage > 0)
		{
			if (pLocal->IsCritBoosted())
				g_Draw.String(FONT_INDICATORS, x, y, { 100, 255, 255, 255 }, align, "Crit Boosted");
			else if (bRapidFire && Storage[slot].StreamEnd > 0)
			{
				const float time = TICKS_TO_TIME(Storage[slot].StreamEnd - I::GlobalVars->tickcount);
				g_Draw.String(FONT_INDICATORS, x, y, { 100, 255, 255, 255 }, align, tfm::format("Streaming crits %.1fs", time).c_str());
			}
			else if (!CritBanned)
			{
				if (Storage[slot].AvailableCrits > 0)
				{
					if (bRapidFire && Storage[slot].StreamWait > 0)
					{
						const float time = TICKS_TO_TIME(Storage[slot].StreamWait - I::GlobalVars->tickcount);
						g_Draw.String(FONT_INDICATORS, x, y, { 255, 255, 255, 255 }, align, tfm::format("Wait %.1fs", time).c_str());
					}
					else
						g_Draw.String(FONT_INDICATORS, x, y, { 150, 255, 150, 255 }, align, "Crit Ready");
				}
				else
				{
					const float damage = Storage[slot].Damage;
					const int shots = Storage[slot].Cost / damage - (Storage[slot].Bucket - BucketBottom) / damage + 1;
					g_Draw.String(FONT_INDICATORS, x, y, { 255, 150, 150, 255 }, align, tfm::format(shots == 1 ? "Crit in %i shot" : "Crit in %i shots", shots).c_str());
				}
			}
			else
				g_Draw.String(FONT_INDICATORS, x, y, { 255, 150, 150, 255 }, align, tfm::format("Deal %i damage", DamageTilUnban).c_str());

			g_Draw.String(FONT_INDICATORS, x, y + Vars::Fonts::FONT_INDICATORS::nTall.Value + 2, { 255, 255, 255, 255 }, align, tfm::format("%i / %i potential Crits", Storage[slot].AvailableCrits, Storage[slot].PotentialCrits).c_str());
		}
		else
			g_Draw.String(FONT_INDICATORS, x, y, { 255, 255, 255, 255 }, align, "Calculating");

		if (Vars::Debug::DebugInfo.Value)
		{
			const int height = Vars::Fonts::FONT_INDICATORS::nTall.Value;
			g_Draw.String(FONT_INDICATORS, x, y + height * 3, { 255, 255, 255, 255 }, align, tfm::format("AllDamage: %i, CritDamage: %i", AllDamage, CritDamage).c_str());
			g_Draw.String(FONT_INDICATORS, x, y + height * 4, { 255, 255, 255, 255 }, align, tfm::format("Bucket: %i", Storage[slot].Bucket).c_str());
			g_Draw.String(FONT_INDICATORS, x, y + height * 5, { 255, 255, 255, 255 }, align, tfm::format("Base: %.2f, Damage: %.2f, Cost: %.2f", Storage[slot].BaseDamage, Storage[slot].Damage, Storage[slot].Cost).c_str());
			g_Draw.String(FONT_INDICATORS, x, y + height * 6, { 255, 255, 255, 255 }, align, tfm::format("Shots: %i, Crits: %i", Storage[slot].ShotsCrits.first, Storage[slot].ShotsCrits.second).c_str());
			g_Draw.String(FONT_INDICATORS, x, y + height * 7, { 255, 255, 255, 255 }, align, tfm::format("CritBanned: %i, DamageTilUnban: %i", CritBanned, DamageTilUnban).c_str());
			g_Draw.String(FONT_INDICATORS, x, y + height * 8, { 255, 255, 255, 255 }, align, tfm::format("CritChance: %.2f", CritChance).c_str());
			g_Draw.String(FONT_INDICATORS, x, y + height * 9, { 255, 255, 255, 255 }, align, tfm::format("Force: %d, Skip: %d", ForceCmds.size(), SkipCmds.size()).c_str());

			g_Draw.String(FONT_INDICATORS, x, y + height * 11, { 255, 255, 255, 255 }, align, tfm::format("G::WeaponCanAttack %d", G::WeaponCanAttack).c_str());
		}
	}
}