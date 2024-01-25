#include "Radar.h"

#include "../../Vars.h"
#include "../../Color.h"

#include "../../Menu/Menu.h"

constexpr Color_t clrBlack = { 0, 0, 0, 255 };
constexpr Color_t clrWhite = { 255, 255, 255, 255 };

void CRadar::Run()
{
	if (!ShouldRun())
		return;

	//Draw background, handle input.
	DrawRadar();

	if (const auto& pLocal = g_EntityCache.GetLocal())
		DrawPoints(pLocal);
}

bool CRadar::ShouldRun()
{
	if (!Vars::Radar::Main::Active.Value)
		return false;

	return true;
}

void CRadar::DrawRadar()
{
	//Build the bg color with the wanted alpha.
	const Color_t clrBack = { 36, 36, 36, static_cast<byte>(Vars::Radar::Main::BackAlpha.Value) };

	const WindowBox_t& info = Vars::Radar::Main::Window.Value;

	//Background
	g_Draw.FillRect(info.x, info.y, info.w, info.w, clrBack);

	//Outline
	g_Draw.LineRect(info.x, info.y, info.w, info.w, { 43, 43, 45, static_cast<byte>(Vars::Radar::Main::LineAlpha.Value) });

	//Center lines
	g_Draw.Line(info.x + info.w / 2, info.y, info.x + info.w / 2, info.y + info.w, { 43, 43, 45, static_cast<byte>(Vars::Radar::Main::LineAlpha.Value) });
	g_Draw.Line(info.x, info.y + info.w / 2, info.x + info.w, info.y + info.w / 2, { 43, 43, 45, static_cast<byte>(Vars::Radar::Main::LineAlpha.Value) });
}

bool CRadar::GetDrawPosition(int& x, int& y, int& z, CBaseEntity* pEntity)
{
	//Calculate the delta and initial position of the entity
	const Vec3 vDelta = pEntity->GetAbsOrigin() - LocalOrigin;
	auto vPos = Vec2((vDelta.y * (-LocalCos) + vDelta.x * LocalSin), (vDelta.x * (-LocalCos) - vDelta.y * LocalSin));

	//Range, keep in bounds
	//Credits are due to whoever wrote this, does what I want and is fast so idc. Code probably older than Jesus.
	//Found it in my 2015 CSGO pasta so I am sure it is made by someone else than me.
	if (Vars::Radar::Main::AlwaysDraw.Value && (fabs(vPos.x) > Range || fabs(vPos.y) > Range))
	{
		if (vPos.y > vPos.x)
		{
			if (vPos.y > -vPos.x)
				vPos.x = Range * vPos.x / vPos.y, vPos.y = Range;
			else
				vPos.y = -Range * vPos.y / vPos.x, vPos.x = -Range;
		}
		else
		{
			if (vPos.y > -vPos.x)
				vPos.y = Range * vPos.y / vPos.x, vPos.x = Range;
			else
				vPos.x = -Range * vPos.x / vPos.y, vPos.y = -Range;
		}
	}

	const WindowBox_t& info = Vars::Radar::Main::Window.Value;

	x = info.x + vPos.x / Range * info.w / 2 + float(info.w) / 2;
	y = info.y + vPos.y / Range * info.w / 2 + float(info.w) / 2;
	z = vDelta.z;

	if (!Vars::Radar::Main::AlwaysDraw.Value)
	{
		if (x < info.x || info.x + info.w < x)
			x = -1;
		if (y < info.y || info.y + info.w < y)
			y = -1;
	}

	//Just confirm that they were both set.
	return (x != -1 && y != -1);
}

void CRadar::DrawPoints(CBaseEntity* pLocal)
{
	//Update members that we use calculating the draw position in "GetDrawPosition()"
	LocalOrigin = pLocal->GetAbsOrigin();
	LocalYaw = I::EngineClient->GetViewAngles().y * (PI / 180.f);
	Range = static_cast<float>(Vars::Radar::Main::Range.Value);
	LocalCos = cos(LocalYaw), LocalSin = sin(LocalYaw);

	// Draw Ammo & Health
	if (Vars::Radar::World::Active.Value)
	{
		const int nSize = Vars::Radar::World::IconSize.Value;

		if (Vars::Radar::World::Ammo.Value)
		{
			for (const auto& ammo : g_EntityCache.GetGroup(EGroupType::WORLD_AMMO))
			{
				int nX = -1, nY = -1, nZ = 0;
				if (GetDrawPosition(nX, nY, nZ, ammo))
				{
					nX -= (nSize / 2), nY -= (nSize / 2);
					g_Draw.Texture(nX, nY, nSize, nSize, clrWhite, 55);
				}
			}
		}

		if (Vars::Radar::World::Health.Value)
		{
			for (const auto& health : g_EntityCache.GetGroup(EGroupType::WORLD_HEALTH))
			{
				int nX = -1, nY = -1, nZ = 0;
				if (GetDrawPosition(nX, nY, nZ, health))
				{
					nX -= (nSize / 2), nY -= (nSize / 2);
					g_Draw.Texture(nX, nY, nSize, nSize, clrWhite, 50);
				}
			}
		}
	}

	// Draw buildings
	if (Vars::Radar::Buildings::Active.Value)
	{

		for (const auto& pBuilding : g_EntityCache.GetGroup(Vars::Radar::Buildings::IgnoreTeam.Value ? EGroupType::BUILDINGS_ENEMIES : EGroupType::BUILDINGS_ALL))
		{
			if (!pBuilding->IsAlive())
				continue;

			int nX = -1, nY = -1, nZ = 0;
			if (GetDrawPosition(nX, nY, nZ, pBuilding))
			{
				Color_t clrDraw = GetEntityDrawColor(pBuilding, Vars::Colors::Relative.Value);

				const int nSize = Vars::Radar::Buildings::IconSize.Value;
				nX -= (nSize / 2), nY -= (nSize / 2);

				switch (pBuilding->GetClassID())
				{
					case ETFClassID::CObjectSentrygun:
					{
						int nTexture = (pBuilding->m_iUpgradeLevel() + 40);

						if (Vars::Radar::Buildings::Outline.Value)
							g_Draw.Texture(nX - 2, nY - 2, nSize + 4, nSize + 4, clrBlack, nTexture);

						g_Draw.Texture(nX, nY, nSize, nSize, clrDraw, nTexture);
						break;
					}
					case ETFClassID::CObjectDispenser:
					{
						if (Vars::Radar::Buildings::Outline.Value)
							g_Draw.Texture(nX - 2, nY - 2, nSize + 4, nSize + 4, clrBlack, 44);

						g_Draw.Texture(nX, nY, nSize, nSize, clrDraw, 44);
						break;
					}
					case ETFClassID::CObjectTeleporter:
					{
						int nTexture = 46; //Exit texture ID

						//If "YawToExit" is not zero, it most like is an entrance
						if (pBuilding->GetYawToExit())
							nTexture -= 1; //In that case, -1 from "nTexture" so we get entrace texture ID

						if (Vars::Radar::Buildings::Outline.Value)
							g_Draw.Texture(nX - 2, nY - 2, nSize + 4, nSize + 4, clrBlack, nTexture);

						g_Draw.Texture(nX, nY, nSize, nSize, clrDraw, nTexture);
						break;
					}
					default: break;
				}

				if (Vars::Radar::Buildings::Health.Value)
				{
					const int nHealth = pBuilding->m_iBOHealth();
					const int nMaxHealth = pBuilding->m_iMaxHealth();
					Color_t clrHealth = GetHealthColor(nHealth, nMaxHealth);

					const auto flHealth = static_cast<float>(nHealth);
					const auto flMaxHealth = static_cast<float>(nMaxHealth);

					static constexpr int nW = 2;

					const float flRatio = (flHealth / flMaxHealth);

					g_Draw.FillRect(((nX - nW) - 1), nY, nW, nSize, { 0, 0, 0, 255 });
					g_Draw.FillRect(((nX - nW) - 1), (nY + nSize - (nSize * flRatio)), nW, (nSize * flRatio), clrHealth);
				}
			}
		}
	}

	// Draw Players
	if (Vars::Radar::Players::Active.Value)
	{
		for (const auto& player : g_EntityCache.GetGroup(EGroupType::PLAYERS_ALL))
		{
			if (!player->IsAlive() || player == g_EntityCache.GetObservedTarget() || player->IsAGhost() || player == pLocal)
				continue;

			const int nEntTeam = player->m_iTeamNum();
			const int nLocalTeam = pLocal->m_iTeamNum();

			switch (Vars::Radar::Players::IgnoreCloaked.Value)
			{
				case 0: break;
				case 1:
				{
					if (player->IsCloaked())
						continue;
					break;
				}
				case 2:
				{
					if (player->IsCloaked() && nEntTeam != nLocalTeam)
						continue;
					break;
				}
			}

			const bool bIsFriend = g_EntityCache.IsFriend(player->GetIndex());

			switch (Vars::Radar::Players::IgnoreTeam.Value)
			{
				case 0: break;
				case 1:
				{
					if (nEntTeam == nLocalTeam)
						continue;
					break;
				}
				case 2:
				{
					if (nEntTeam == nLocalTeam && !bIsFriend)
						continue;
					break;
				}
			}

			int nX = -1, nY = -1, nZ = 0;
			if (GetDrawPosition(nX, nY, nZ, player))
			{
				const int nSize = Vars::Radar::Players::IconSize.Value;
				nX -= (nSize / 2), nY -= (nSize / 2);

				Color_t clrDraw = GetEntityDrawColor(player, Vars::Colors::Relative.Value);

				//Background
				//Just a filled rect or a bit better looking texture RN
				//TODO:
				//Add circle background, and add outline for that
				//I think I saw a nice circle texture actually, gonna try to find that again later.
				if (Vars::Radar::Players::BackGroundType.Value)
				{
					int nTexture = 0;

					if (Vars::Radar::Players::BackGroundType.Value == 2)
						nTexture += (nEntTeam + 50);

					nTexture
						? g_Draw.Texture(nX, nY, nSize, nSize, clrDraw, nTexture)
						: g_Draw.FillRect(nX, nY, nSize, nSize, clrDraw);
				}

				//Prepare the correct texture index for player icon, and draw it
				{
					PlayerInfo_t pi{};
					if (Vars::Radar::Players::IconType.Value == 2 && I::EngineClient->GetPlayerInfo(player->GetIndex(), &pi) && !pi.fakeplayer) // Avatar
						g_Draw.Avatar(nX, nY, nSize, nSize, pi.friendsID);
					else
					{
						int nTexture = player->m_iClass();

						// Portrait
						if (Vars::Radar::Players::IconType.Value == 1)
						{
							nTexture += 10;
							if (nEntTeam == 3)
								nTexture += 10;
						}

						g_Draw.Texture(nX, nY, nSize, nSize, { 255, 255, 255, 255 }, nTexture);
					}
				}

				//TODO:
				//Correct this for the circle once it's added.
				if (Vars::Radar::Players::Outline.Value)
				{
					//idk if this is kinda slow
					Color_t clrOutLine = Vars::Radar::Players::BackGroundType.Value == 1 ? clrBlack : clrDraw;
					g_Draw.LineRect(nX, nY, nSize, nSize, clrOutLine);
				}

				//TODO:
				//Make the healthbar toggleable from left side to bottom.
				if (Vars::Radar::Players::Health.Value)
				{
					const int nHealth = player->m_iHealth();
					const int nMaxHealth = player->GetMaxHealth();
					Color_t clrHealth = GetHealthColor(nHealth, nMaxHealth);

					auto flHealth = static_cast<float>(nHealth);
					const auto flMaxHealth = static_cast<float>(nMaxHealth);
					float flOverHeal = 0.0f;

					if (flHealth > flMaxHealth)
					{
						flOverHeal = fmod(flHealth, flMaxHealth);
						flHealth = flMaxHealth;
					}

					static constexpr int nWidth = 2;

					float flRatio = (flHealth / flMaxHealth);

					g_Draw.FillRect(((nX - nWidth) - 1), nY, nWidth, nSize, { 0, 0, 0, 255 });
					g_Draw.FillRect(((nX - nWidth) - 1), (nY + nSize - (nSize * flRatio)), nWidth, (nSize * flRatio), clrHealth);

					if (flOverHeal > 0.0f)
					{
						flRatio = (flOverHeal / flMaxHealth);
						g_Draw.FillRect(((nX - nWidth) - 1), (nY + (nSize + 1) - (nSize * flRatio)), nWidth, (nSize * flRatio), Vars::Colors::Overheal.Value);
					}
				}

				// Draw height indicator (higher / lower)
				if (Vars::Radar::Players::Height.Value && std::abs(nZ) > 80.f)
				{
					const int triOffset = nZ > 0 ? -5 : 5;
					const int yPos = nZ > 0 ? nY : nY + nSize;

					g_Draw.DrawFillTriangle({ Vec2(nX, yPos), Vec2(nX + nSize * 0.5f, yPos + triOffset), Vec2(nX + nSize, yPos) }, clrDraw);
				}
			}
		}
	}
}
