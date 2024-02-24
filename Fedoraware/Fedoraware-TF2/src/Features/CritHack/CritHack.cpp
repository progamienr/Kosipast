#include "CritHack.h"
#include "../TickHandler/TickHandler.h"
#include "../Auto/AutoJump/AutoJump.h"

#define TF_DAMAGE_CRIT_MULTIPLIER		3.0f
#define TF_DAMAGE_CRIT_CHANCE			0.02f
#define TF_DAMAGE_CRIT_CHANCE_RAPID		0.02f
#define TF_DAMAGE_CRIT_DURATION_RAPID	2.0f
#define TF_DAMAGE_CRIT_CHANCE_MELEE		0.15f

bool CCritHack::IsEnabled()
{
	if (!Vars::CritHack::Active.Value || !I::EngineClient->IsInGame())
		return false;
	static auto tf_weapon_criticals = g_ConVars.FindVar("tf_weapon_criticals");
	if (tf_weapon_criticals && !tf_weapon_criticals->GetBool())
		return false;

	return true;
}



void CCritHack::Fill(CBaseCombatWeapon* pWeapon, const CUserCmd* pCmd, const bool bAttacking, int n)
{
	static int prev_weapon = 0;
	static int previousCrit = 0;
	static int starting_num = pCmd->command_number;

	//if (bAttacking && G::CanPrimaryAttack/* || pCmd->buttons & IN_ATTACK*/)
	//	return;

	if (prev_weapon != pWeapon->GetIndex())
	{
		starting_num = pCmd->command_number;
		prev_weapon = pWeapon->GetIndex();
		ForceCmds.clear();
		SkipCmds.clear();
	}

	for (int i = 0; i < n; i++)
	{
		if (ForceCmds.size() >= 15)
			break;

		const int cmd_num = starting_num + i;

		if (IsCritCommand(cmd_num))
			ForceCmds.push_back(cmd_num);
	}
	for (int i = 0; i < n; i++)
	{
		if (SkipCmds.size() >= 15)
			break;

		const int cmd_num = starting_num + i;

		if (IsCritCommand(cmd_num, false))
			SkipCmds.push_back(cmd_num);
	}
	starting_num += n;
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

	//return crit ? random_int < 100 : random_int > 6000;
	return crit ? random_int < 50 : random_int > 8000;
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



void CCritHack::GetTotalCrits(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	if (!Vars::CritHack::Active.Value)
		return;

	const int slot = pWeapon->GetSlot();
	auto tfWeaponInfo = pWeapon->GetTFWeaponInfo();
	if (!tfWeaponInfo)
		return;
	auto& weaponData = tfWeaponInfo->GetWeaponData(0);

	float flDamage = weaponData.m_nDamage;
	flDamage = Utils::ATTRIB_HOOK_FLOAT(flDamage, "mult_dmg", pWeapon);
	int nProjectilesPerShot = weaponData.m_nBulletsPerShot;
	if (nProjectilesPerShot >= 1)
		nProjectilesPerShot = Utils::ATTRIB_HOOK_FLOAT(nProjectilesPerShot, "mult_bullets_per_shot", pWeapon);
	else
		nProjectilesPerShot = 1;
	Storage[slot].Damage = flDamage *= nProjectilesPerShot;

	if (pWeapon->IsRapidFire())
	{
		flDamage *= TF_DAMAGE_CRIT_DURATION_RAPID / weaponData.m_flTimeFireDelay;

		if (flDamage * TF_DAMAGE_CRIT_MULTIPLIER > BucketCap)
			flDamage = BucketCap / TF_DAMAGE_CRIT_MULTIPLIER;
	}

	float flMult = slot == SLOT_MELEE ? 0.5f : Math::RemapValClamped((float)(Storage[slot].ShotsCrits.second + 1) / (float)(Storage[slot].ShotsCrits.first + 1), 0.1f, 1.f, 1.f, 3.f);
	Storage[slot].Cost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER * flMult;

	if (BucketCap)
		Storage[slot].PotentialCrits = static_cast<unsigned int>((BucketCap - Storage[slot].Damage) / (3 * flDamage / (slot == SLOT_MELEE ? 2 : 1) - Storage[slot].Damage));
	
	int iCrits = 0;
	{
		int shots = Storage[slot].ShotsCrits.first, crits = Storage[slot].ShotsCrits.second;
		float bucket = Storage[slot].Bucket, flCost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER;
		const int iAttempts = std::min(Storage[slot].PotentialCrits + 1, 100);
		for (int i = 0; i < iAttempts; i++)
		{
			shots++; crits++;

			flMult = slot == SLOT_MELEE ? 0.5f : Math::RemapValClamped((float)crits / (float)shots, 0.1f, 1.f, 1.f, 3.f);
			bucket = std::min(bucket + Storage[slot].Damage, BucketCap) - flCost * flMult;
			if (bucket < 0.f)
				break;

			iCrits++;
		}
	}

	if (iCrits <= 0)
	{
		iCrits = 0;

		int shots = Storage[slot].ShotsCrits.first + 1, crits = Storage[slot].ShotsCrits.second + 1;
		float bucket = std::min(Storage[slot].Bucket + Storage[slot].Damage, BucketCap), flCost = flDamage * TF_DAMAGE_CRIT_MULTIPLIER;
		for (int i = 0; i < 100; i++)
		{
			iCrits--;
			if (!pWeapon->IsRapidFire() || !(i % int(weaponData.m_flTimeFireDelay / I::GlobalVars->interval_per_tick)))
				shots++;

			flMult = slot == SLOT_MELEE ? 0.5f : Math::RemapValClamped((float)crits / (float)shots, 0.1f, 1.f, 1.f, 3.f);
			bucket = std::min(bucket + Storage[slot].Damage, BucketCap);
			if (bucket >= flCost * flMult)
				break;
		}
	}

	Storage[slot].AvailableCrits = iCrits;
}

void CCritHack::CanFireCriticalShotHandler(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	CritBanned = false;

	CritChance = 0.02f * pLocal->GetCritMult();
	if (pWeapon->IsRapidFire())
	{
		float flNonCritDuration = (TF_DAMAGE_CRIT_DURATION_RAPID / CritChance) - TF_DAMAGE_CRIT_DURATION_RAPID;
		CritChance = 1.f / flNonCritDuration;
	}
	CritChance += 0.1f;

	if (CritDamage == 0.f || pWeapon->GetSlot() == SLOT_MELEE)
		return;

	const auto divCritDamage = CritDamage / TF_DAMAGE_CRIT_MULTIPLIER;
	const auto ratio = divCritDamage / (AllDamage - 2 * divCritDamage);

	CritBanned = ratio >= CritChance;
}

void CCritHack::GetDamageTilUnban(CBaseEntity* pLocal)
{
	DamageTilUnban = 0;

	if (!CritBanned)
		return;

	const auto divCritDamage = CritDamage / TF_DAMAGE_CRIT_MULTIPLIER;

	DamageTilUnban = divCritDamage / CritChance + 2 * divCritDamage - AllDamage;
}

bool CCritHack::WeaponCanCrit(CBaseCombatWeapon* pWeapon)
{
	if (Utils::ATTRIB_HOOK_FLOAT(1.f, "mult_crit_chance", pWeapon) <= 0.f)
		return false;

	switch (pWeapon->GetWeaponID())
	{
	case TF_WEAPON_LUNCHBOX:
	case TF_WEAPON_JAR_MILK:
	case TF_WEAPON_BUFF_ITEM:
	case TF_WEAPON_FLAME_BALL:
	case TF_WEAPON_JAR_GAS:
	case TF_WEAPON_ROCKETPACK:
	case TF_WEAPON_LASER_POINTER:
	case TF_WEAPON_MEDIGUN:
	case TF_WEAPON_SNIPERRIFLE:
	case TF_WEAPON_SNIPERRIFLE_DECAP:
	case TF_WEAPON_SNIPERRIFLE_CLASSIC:
	case TF_WEAPON_COMPOUND_BOW:
	case TF_WEAPON_JAR:
	case TF_WEAPON_KNIFE:
	case TF_WEAPON_PDA_SPY:
	case TF_WEAPON_PDA_SPY_BUILD:
	case TF_WEAPON_PDA:
	case TF_WEAPON_PDA_ENGINEER_BUILD:
	case TF_WEAPON_PDA_ENGINEER_DESTROY:
	case TF_WEAPON_BUILDER:
		return false;
	}

	return true;
}



void CCritHack::ResetWeapons(CBaseEntity* pLocal)
{
	std::unordered_map<int, bool> mWeapons = {};
	const auto hWeapons = pLocal ? pLocal->GetMyWeapons() : nullptr;
	if (!pLocal || !hWeapons)
		return;

	for (int i = 0; hWeapons[i]; i++)
	{
		if (Utils::HandleToIDX(hWeapons[i]) < 0 || Utils::HandleToIDX(hWeapons[i]) >= 2048)
			continue;

		if (const auto pWeapon = reinterpret_cast<CBaseCombatWeapon*>(I::ClientEntityList->GetClientEntityFromHandle(hWeapons[i])))
		{
			const int slot = pWeapon->GetSlot();
			const int index = pWeapon->m_iItemDefinitionIndex();
			mWeapons[slot] = true;

			if (Storage[slot].DefIndex == index)
				continue;

			Storage[slot] = {};
			Storage[slot].Bucket = BucketDefault;
			Storage[slot].DefIndex = index;

			Utils::ConLog("Crithack", std::format("Resetting weapon {}", index).c_str(), { 0, 255, 255, 255 }, Vars::Debug::Logging.Value);
		}
	}

	for (auto& [slot, _] : Storage)
	{
		if (!mWeapons[slot])
			Storage[slot] = {};
	}
}

void CCritHack::Reset()
{
	ForceCmds = {};
	SkipCmds = {};

	Storage = {};

	CritDamage = 0;
	AllDamage = 0;

	CritBanned = false;
	DamageTilUnban = 0;
	CritChance = 0.f;
	ProtectData = false;

	Utils::ConLog("Crithack", "Resetting all", { 0, 255, 255, 255 }, Vars::Debug::Logging.Value);
}



void CCritHack::Run(CUserCmd* pCmd)
{
	auto bucketDefault = g_ConVars.FindVar("tf_weapon_criticals_bucket_default");
	BucketDefault = bucketDefault ? bucketDefault->GetFloat() : 300.f;
	auto bucketBottom = g_ConVars.FindVar("tf_weapon_criticals_bucket_bottom");
	BucketBottom = bucketBottom ? bucketBottom->GetFloat() : -250.f;
	auto bucketCap = g_ConVars.FindVar("tf_weapon_criticals_bucket_cap");
	BucketCap = bucketCap ? bucketCap->GetFloat() : 1000.f;

	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();
	ResetWeapons(pLocal);
	if (!pLocal || !pWeapon || !pLocal->IsAlive() || pLocal->IsCritBoosted() || !WeaponCanCrit(pWeapon))
		return;

	if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN && pCmd->buttons & IN_ATTACK)
		pCmd->buttons &= ~IN_ATTACK2;

	bool bAttacking = G::IsAttacking && F::AutoJump.iFrame < 1;
	if (G::CurWeaponType == EWeaponType::MELEE)
	{
		bAttacking = G::CanPrimaryAttack && pCmd->buttons & IN_ATTACK;
		if (!bAttacking && pWeapon->GetWeaponID() == TF_WEAPON_FISTS)
			bAttacking = G::CanPrimaryAttack && pCmd->buttons & IN_ATTACK2;
	}
	if (pWeapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER || pWeapon->GetWeaponID() == TF_WEAPON_STICKY_BALL_LAUNCHER || pWeapon->GetWeaponID() == TF_WEAPON_GRENADE_STICKY_BALL
		|| pWeapon->GetWeaponID() == TF_WEAPON_CANNON)
	{
		static float flBegin = -1.f;
		if (pCmd->buttons & IN_ATTACK && flBegin < 0.f && G::CanPrimaryAttack)
			flBegin = I::GlobalVars->curtime;

		const float flCharge = flBegin > 0.f ? I::GlobalVars->curtime - flBegin : 0.f;
		const float flAmount = pWeapon->GetWeaponID() != TF_WEAPON_CANNON
			? Math::RemapValClamped(flCharge, 0.f, Utils::ATTRIB_HOOK_FLOAT(4.f, "stickybomb_charge_rate", pWeapon), 0.f, 1.f)
			: Math::RemapValClamped(flCharge, 0.f, Utils::ATTRIB_HOOK_FLOAT(0.f, "grenade_launcher_mortar_mode", pWeapon), 0.f, 1.f);

		const bool bUnheld = !(pCmd->buttons & IN_ATTACK) && flAmount > 0.f;
		const bool bSwapping = pCmd->weaponselect;
		const bool bFull = flAmount == 1.f; // possibly add exception to skip when full with cannon
		
		bAttacking = (bUnheld || bFull) && !bSwapping;
		if (bAttacking || !G::CanPrimaryAttack || bSwapping)
			flBegin = -1.f;
	}
	if ((pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN || pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER) && !(G::LastUserCmd->buttons & IN_ATTACK)) // silly
		bAttacking = false;

	if (Storage[pWeapon->GetSlot()].StreamWait <= I::GlobalVars->tickcount - 1)
		Storage[pWeapon->GetSlot()].StreamWait = -1;
	if (Storage[pWeapon->GetSlot()].StreamEnd <= I::GlobalVars->tickcount - 1)
		Storage[pWeapon->GetSlot()].StreamEnd = -1;

	const bool bRapidFire = pWeapon->IsRapidFire();
	const bool bStreamWait = Storage[pWeapon->GetSlot()].StreamWait > 0;
	const bool bStreamEnd = Storage[pWeapon->GetSlot()].StreamEnd > 0;

	GetTotalCrits(pLocal, pWeapon);
	CanFireCriticalShotHandler(pLocal, pWeapon);
	GetDamageTilUnban(pLocal);
	Fill(pWeapon, pCmd, bAttacking, 15);

	const int closestCrit = LastGoodCritTick(pCmd);
	const int closestSkip = LastGoodSkipTick(pCmd);

	bool pressed = F::KeyHandler.Down(Vars::CritHack::CritKey.Value);
	if (!pressed && Vars::CritHack::AlwaysMelee.Value && pWeapon->GetSlot() == SLOT_MELEE)
		pressed = true;
	static bool bFirstTimePredicted = false;
	if (!I::ClientState->chokedcommands)
		bFirstTimePredicted = false;
	if (bAttacking && !pWeapon->IsInReload() && !bFirstTimePredicted) // is it valid & should we even use it
	{
		bFirstTimePredicted = true;

		if (IsEnabled())
		{
			if (pressed && Storage[pWeapon->GetSlot()].AvailableCrits > 0 && (!CritBanned || pWeapon->GetSlot() == SLOT_MELEE) && closestCrit >= 0 && !bStreamWait && !bStreamEnd)
				pCmd->command_number = closestCrit;
			else if (Vars::CritHack::AvoidRandom.Value && closestSkip >= 0)
				pCmd->command_number = closestSkip;
		}

		if (bRapidFire && !bStreamWait)
			Storage[pWeapon->GetSlot()].StreamWait = I::GlobalVars->tickcount + 1 / I::GlobalVars->interval_per_tick;

		if (!bRapidFire || !bStreamEnd)
		{
			if (!bRapidFire || !bStreamWait)
				Storage[pWeapon->GetSlot()].ShotsCrits.first += 1;

			// damage gets added to bucket whether or not it's a crit
			Storage[pWeapon->GetSlot()].Bucket = std::clamp(Storage[pWeapon->GetSlot()].Bucket + Storage[pWeapon->GetSlot()].Damage, std::max(BucketBottom, 0.f), BucketCap);
			if (IsCritCommand(pCmd->command_number))
			{
				if (bRapidFire)
					Storage[pWeapon->GetSlot()].StreamEnd = I::GlobalVars->tickcount + 2 / I::GlobalVars->interval_per_tick;
				Storage[pWeapon->GetSlot()].ShotsCrits.second += 1;
				Storage[pWeapon->GetSlot()].Bucket = std::clamp(Storage[pWeapon->GetSlot()].Bucket - Storage[pWeapon->GetSlot()].Cost, std::max(BucketBottom, 0.f), BucketCap);
			}
		}
	}
	else if (IsEnabled() && closestSkip >= 0)
		pCmd->command_number = closestSkip;

	//if (pCmd->command_number == closestCrit || pCmd->command_number == closestSkip)
		WishRandomSeed = MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF;

	if (pCmd->command_number == closestCrit)
		ForceCmds.pop_front();
	else if (pCmd->command_number == closestSkip)
		SkipCmds.pop_front();
}

bool CCritHack::CalcIsAttackCriticalHandler(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon)
{
	if (!I::Prediction->m_bFirstTimePredicted || !pLocal || !pWeapon)
		return false;

	if (pWeapon->GetWeaponID() == TF_WEAPON_MINIGUN || pWeapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER)
	{
		static int iOldAmmo = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());
		const int iNewAmmo = pLocal->GetAmmoCount(pWeapon->m_iPrimaryAmmoType());

		const bool bFired = iOldAmmo != iNewAmmo;

		iOldAmmo = iNewAmmo;

		if (!bFired)
			return false;
	}

	if (WishRandomSeed)
	{
		*I::RandomSeed = WishRandomSeed;
		WishRandomSeed = 0;
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
		const auto weaponid = pEvent->GetInt("weaponid");

		if (attacker == pLocal->GetIndex() && attacked != attacker)
		{
			const auto& pWeapon = g_EntityCache.GetWeapon();
			if (!pWeapon || pWeapon->GetWeaponID() == weaponid && pWeapon->GetSlot() == SLOT_MELEE || pLocal->InCond(TF_COND_CRITBOOSTED))
				return;

			AllDamage += damage;
			if (crit)
				CritDamage += damage;
		}
	}
	else if (uNameHash == FNV1A::HashConst("teamplay_round_start"))
	{
		CritDamage = 0.f;
		AllDamage = 0.f;
	}
	else if (uNameHash == FNV1A::HashConst("client_beginconnect") || uNameHash == FNV1A::HashConst("client_disconnect") ||
		uNameHash == FNV1A::HashConst("game_newmap"))
	{
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
	if (!Vars::CritHack::Active.Value || !(Vars::Menu::Indicators.Value & (1 << 1)))
		return;
	if (!IsEnabled() || !G::CurrentUserCmd)
		return;

	const auto& pLocal = g_EntityCache.GetLocal();
	if (!pLocal || !pLocal->IsAlive() || pLocal->IsAGhost())
		return;

	const auto& pWeapon = pLocal->GetActiveWeapon();
	if (!pWeapon)
		return;

	int x = Vars::Menu::CritsDisplay.Value.x;
	int y = Vars::Menu::CritsDisplay.Value.y + 8;

	const auto& fFont = g_Draw.GetFont(FONT_INDICATORS);

	EAlign align = ALIGN_TOP;
	if (x <= (100 + 50 * Vars::Menu::DPI.Value))
	{
		x -= 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPLEFT;
	}
	else if (x >= g_ScreenSize.w - (100 + 50 * Vars::Menu::DPI.Value))
	{
		x += 42 * Vars::Menu::DPI.Value;
		align = ALIGN_TOPRIGHT;
	}

	if (WeaponCanCrit(pWeapon))
	{
		const auto slot = pWeapon->GetSlot();
		const auto bRapidFire = pWeapon->IsRapidFire();

		if (Storage[slot].Damage > 0)
		{
			if (pLocal->IsCritBoosted())
				g_Draw.String(fFont, x, y, { 100, 255, 255, 255 }, align, "Crit Boosted");
			else if (bRapidFire && Storage[slot].StreamEnd > 0)
			{
				const float time = std::max(TICKS_TO_TIME(Storage[slot].StreamEnd - pLocal->m_nTickBase()), 0.f);
				g_Draw.String(fFont, x, y, { 100, 255, 255, 255 }, align, std::format("Streaming crits {:.1f}s", time).c_str());
			}
			else if (!CritBanned)
			{
				if (Storage[slot].AvailableCrits > 0)
				{
					if (bRapidFire && Storage[slot].StreamWait > 0)
					{
						const float time = std::max((TICKS_TO_TIME(Storage[slot].StreamWait - pLocal->m_nTickBase())), 0.f);
						g_Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, std::format("Wait {:.1f}s", time).c_str());
					}
					else
						g_Draw.String(fFont, x, y, { 150, 255, 150, 255 }, align, "Crit Ready");
				}
				else
				{
					const int shots = -Storage[slot].AvailableCrits;
					g_Draw.String(fFont, x, y, { 255, 150, 150, 255 }, align, shots == 1 ? std::format("Crit in {} shot", shots).c_str() : std::format("Crit in {} shots", shots).c_str());
				}
			}
			else
				g_Draw.String(fFont, x, y, { 255, 150, 150, 255 }, align, std::format("Deal {} damage", DamageTilUnban).c_str());

			g_Draw.String(fFont, x, y + fFont.nTall + 1, Vars::Menu::Theme::Active.Value, align, std::format("{} / {} potential crits", std::max(Storage[slot].AvailableCrits, 0), Storage[slot].PotentialCrits).c_str());
		}
		else
			g_Draw.String(fFont, x, y, Vars::Menu::Theme::Active.Value, align, "Calculating");

		if (Vars::Debug::Info.Value)
		{
			g_Draw.String(fFont, x, y + fFont.nTall * 3, { 255, 255, 255, 255 }, align, std::format("AllDamage: {}, CritDamage: {}", AllDamage, CritDamage).c_str());
			g_Draw.String(fFont, x, y + fFont.nTall * 4, { 255, 255, 255, 255 }, align, std::format("Bucket: ({} / {})", Storage[slot].Bucket, pWeapon->CritTokenBucket()).c_str());
			g_Draw.String(fFont, x, y + fFont.nTall * 5, { 255, 255, 255, 255 }, align, std::format("Damage: {}, Cost: {}", Storage[slot].Damage, Storage[slot].Cost).c_str());
			g_Draw.String(fFont, x, y + fFont.nTall * 6, { 255, 255, 255, 255 }, align, std::format("Shots: ({} / {}), Crits: ({} / {})", Storage[slot].ShotsCrits.first, pWeapon->CritChecks(), Storage[slot].ShotsCrits.second, pWeapon->CritSeedRequests()).c_str());
			g_Draw.String(fFont, x, y + fFont.nTall * 7, { 255, 255, 255, 255 }, align, std::format("CritBanned: {}, DamageTilUnban: {}", CritBanned, DamageTilUnban).c_str());
			g_Draw.String(fFont, x, y + fFont.nTall * 8, { 255, 255, 255, 255 }, align, std::format("CritChance: {:.2f}", CritChance).c_str());
			g_Draw.String(fFont, x, y + fFont.nTall * 9, { 255, 255, 255, 255 }, align, std::format("Force: {}, Skip: {}", ForceCmds.size(), SkipCmds.size()).c_str());

			g_Draw.String(fFont, x, y + fFont.nTall * 11, { 255, 255, 255, 255 }, align, L"G::CanPrimaryAttack %d (%d)", G::CanPrimaryAttack, I::GlobalVars->tickcount % 66);
			g_Draw.String(fFont, x, y + fFont.nTall * 12, { 255, 255, 255, 255 }, align, L"G::IsAttacking %d", G::IsAttacking);
		}
	}
}