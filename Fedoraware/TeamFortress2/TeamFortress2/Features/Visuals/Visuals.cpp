#include "Visuals.h"
#include "../Vars.h"
#include "../ESP/LocalConditions/LocalConditions.h"
#include "../Backtrack/Backtrack.h"
#include "../Simulation/ProjectileSimulation/ProjectileSimulation.h"

#include "../Menu/ImGui/imgui_impl_win32.h"
#include "../Menu/ImGui/imgui_stdlib.h"

void CVisuals::DrawHitbox(matrix3x4 bones[128], CBaseEntity* pEntity)
{
	const model_t* pModel = pEntity->GetModel();
	if (!pModel) return;
	const studiohdr_t* pHDR = I::ModelInfoClient->GetStudioModel(pModel);
	if (!pHDR) return;
	const mstudiohitboxset_t* pSet = pHDR->GetHitboxSet(pEntity->GetHitboxSet());
	if (!pSet) return;

	G::BoxesStorage.clear();

	for (int i = 0; i < pSet->numhitboxes; ++i)
	{
		const mstudiobbox_t* bbox = pSet->hitbox(i);
		if (!bbox) { continue; }

		/*if (bbox->m_radius <= 0.f) {*/
		matrix3x4 rotMatrix;
		Math::AngleMatrix(bbox->angle, rotMatrix);

		matrix3x4 matrix;
		Math::ConcatTransforms(bones[bbox->bone], rotMatrix, matrix);

		Vec3 bboxAngle;
		Math::MatrixAngles(matrix, bboxAngle);

		Vec3 matrixOrigin;
		Math::GetMatrixOrigin(matrix, matrixOrigin);

		G::BoxesStorage.push_back({ matrixOrigin, bbox->bbmin, bbox->bbmax, bboxAngle, I::GlobalVars->curtime + 5.f, Colors::HitboxEdge, Colors::HitboxFace });
	}
}

void CVisuals::DrawHitbox(CBaseEntity* pTarget, Vec3 vOrigin, float flTime)
{
	G::BoxesStorage.clear();

	G::BoxesStorage.push_back({ vOrigin, pTarget->GetCollideableMins(), pTarget->GetCollideableMaxs(), Vec3(), flTime, Colors::HitboxEdge, Colors::HitboxFace});
}

void CVisuals::DrawOnScreenConditions(CBaseEntity* pLocal)
{
	if (!Vars::Visuals::DrawOnScreenConditions.Value) { return; }
	if (!pLocal->IsAlive() || pLocal->IsAGhost()) { return; }

	int x = Vars::Visuals::OnScreenConditions.x;
	int y = Vars::Visuals::OnScreenConditions.y + 8;

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

	std::vector<std::wstring> conditionsVec = F::LocalConditions.GetPlayerConditions(pLocal);

	int offset = 0;
	int width, height;
	for (const std::wstring& cond : conditionsVec)
	{
		g_Draw.String(FONT_MENU, x, y + offset, { 255, 255, 255, 255 }, align, cond.data());
		I::VGuiSurface->GetTextSize(g_Draw.m_vecFonts[FONT_MENU].dwFont, cond.data(), width, height);
		offset += height;
	}
}

void CVisuals::DrawOnScreenPing(CBaseEntity* pLocal) {
	if (!Vars::Visuals::DrawOnScreenPing.Value) { return; }

	if (!pLocal || !pLocal->IsAlive()) { return; }

	CTFPlayerResource* cResource = g_EntityCache.GetPR();
	if (!cResource) { return; }

	INetChannel* iNetChan = I::EngineClient->GetNetChannelInfo();
	if (!iNetChan) { return; }

	const float flLatencyIn = iNetChan->GetLatency(FLOW_INCOMING) * 1000.f;
	const float flLatencyOut = iNetChan->GetLatency(FLOW_OUTGOING) * 1000.f;
	const float flFake = std::min((F::Backtrack.GetFake() + (F::Backtrack.flFakeInterp > G::LerpTime ? F::Backtrack.flFakeInterp/* - G::LerpTime*/ : 0.f)) * 1000.f, 800.f);
	const float flLatency = flLatencyIn + flLatencyOut - F::Backtrack.GetFake() * 1000.f;
	const int iLatencyScoreBoard = cResource->GetPing(pLocal->GetIndex());

	int x = Vars::Visuals::OnScreenPing.x;
	int y = Vars::Visuals::OnScreenPing.y + 8;

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

	if (flFake || Vars::Backtrack::Interp.Value && Vars::Backtrack::Enabled.Value)
	{
		if (flLatency > 0.f)
		{
			if (!Vars::Debug::DebugInfo.Value)
				g_Draw.String(FONT_INDICATORS, x, y, { 255, 255, 255, 255 }, align, "Real %.0f (+ %.0f) ms", flLatency, flFake);
			else
				g_Draw.String(FONT_INDICATORS, x, y, { 255, 255, 255, 255 }, align, "In %.0f, Out %.0f (+ %.0f) ms", flLatencyIn, flLatencyOut, flFake);
		}
		else
			g_Draw.String(FONT_INDICATORS, x, y, { 255, 255, 255, 255 }, align, "Syncing");
	}
	else
	{
		if (!Vars::Debug::DebugInfo.Value)
			g_Draw.String(FONT_INDICATORS, x, y, { 255, 255, 255, 255 }, align, "Real %.0f ms", flLatency);
		else
			g_Draw.String(FONT_INDICATORS, x, y, { 255, 255, 255, 255 }, align, "In %.0f, Out %.0f ms", flLatencyIn, flLatencyOut);
	}
	g_Draw.String(FONT_INDICATORS, x, y + Vars::Fonts::FONT_INDICATORS::nTall.Value + 2, { 255, 255, 255, 255 }, align, "Scoreboard %d ms", iLatencyScoreBoard);
	if (Vars::Debug::DebugInfo.Value)
		g_Draw.String(FONT_INDICATORS, x, y + (Vars::Fonts::FONT_INDICATORS::nTall.Value + 2) * 2, { 255, 255, 255, 255 }, align, "GetFake %.0f, flFakeInterp %.0f", F::Backtrack.GetFake() * 1000.f, F::Backtrack.flFakeInterp * 1000.f);
}

void CVisuals::SkyboxChanger()
{
	using LoadNamedSkysFn = bool(_cdecl*)(const char*);
	static auto LoadSkys = (LoadNamedSkysFn)g_Pattern.Find(
		L"engine.dll", L"55 8B EC 81 EC ? ? ? ? 8B 0D ? ? ? ? 53 56 57 8B 01 C7 45");

	const char* skybNames[] = {
		"Custom",
		"sky_tf2_04",
		"sky_upward",
		"sky_dustbowl_01",
		"sky_goldrush_01",
		"sky_granary_01",
		"sky_well_01",
		"sky_gravel_01",
		"sky_badlands_01",
		"sky_hydro_01",
		"sky_night_01",
		"sky_nightfall_01",
		"sky_trainyard_01",
		"sky_stormfront_01",
		"sky_morningsnow_01",
		"sky_alpinestorm_01",
		"sky_harvest_01",
		"sky_harvest_night_01",
		"sky_halloween",
		"sky_halloween_night_01",
		"sky_halloween_night2014_01",
		"sky_island_01",
		"sky_rainbow_01"
	};
	if (Vars::Visuals::SkyboxChanger.Value)
	{
		if (Vars::Skybox::SkyboxNum == 0)
		{
			if (Vars::Misc::BypassPure.Value)
			{
				LoadSkys(Vars::Skybox::SkyboxName.c_str());
			}
			else
			{
				LoadSkys(I::Cvar->FindVar("sv_skyname")->GetString());
			}
		}
		else
		{
			LoadSkys(skybNames[Vars::Skybox::SkyboxNum]);
		}
	}
	else
	{
		LoadSkys(I::Cvar->FindVar("sv_skyname")->GetString());
	}
}

bool CVisuals::RemoveScope(int nPanel)
{
	if (!Vars::Visuals::RemoveScope.Value) { return false; }

	if (!m_nHudZoom && Hash::IsHudScope(I::VGuiPanel->GetName(nPanel)))
	{
		m_nHudZoom = nPanel;
	}

	return (nPanel == m_nHudZoom);
}

void CVisuals::FOV(CViewSetup* pView)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();

	if (pLocal && pView)
	{
		/*
		if (pLocal->IsScoped() && !Vars::Visuals::RemoveZoom.Value)
		{
			return;
		}
		*/

		const int fov = pLocal->IsScoped() ? Vars::Visuals::ZoomFieldOfView.Value : Vars::Visuals::FieldOfView.Value;

		pView->fov = static_cast<float>(fov);

		if (pLocal->IsAlive())
		{
			pLocal->SetFov(fov);
			//pLocal->m_iDefaultFOV() = fov;
		}
	}
}

void CVisuals::ThirdPerson(CViewSetup* pView)
{
	if (const auto& pLocal = g_EntityCache.GetLocal())
	{
		// Toggle key //[but not this obv]
		if (Vars::Visuals::ThirdPersonKey.Value)
		{
			if (!I::EngineVGui->IsGameUIVisible() && !I::VGuiSurface->IsCursorVisible())
			{
				static KeyHelper tpKey{ &Vars::Visuals::ThirdPersonKey.Value };
				if (tpKey.Pressed())
				{
					Vars::Visuals::ThirdPerson.Value = !Vars::Visuals::ThirdPerson.Value;
				}
			}
		}

		if (!Vars::Visuals::ThirdPerson.Value
			|| ((!Vars::Visuals::RemoveScope.Value || Vars::Visuals::ZoomFieldOfView.Value < 70/*!Vars::Visuals::RemoveZoom.Value*/) && pLocal->IsScoped())
			|| !pLocal || pLocal->IsTaunting() || !pLocal->IsAlive() || pLocal->IsAGhost())
		{
			if (I::Input->CAM_IsThirdPerson())
			{
				I::EngineClient->ClientCmd_Unrestricted("firstperson");
			}
		}
		else
		{
			if (!I::Input->CAM_IsThirdPerson())
			{
				{ //lazy
					ConVar* sv_cheats = g_ConVars.FindVar("sv_cheats");
					sv_cheats->m_Value.m_nValue = 1;
					ConVar* cam_ideallag = g_ConVars.FindVar("cam_ideallag");
					cam_ideallag->SetValue(0.f);
				}
				I::EngineClient->ClientCmd_Unrestricted("thirdperson");
			}

			// Thirdperson angles
			I::Prediction->SetLocalViewAngles(G::RealViewAngles);

			// Thirdperson offset
			I::ThirdPersonManager->SetDesiredCameraOffset(Vec3{}); //would've used this but right & up offsets get reversed on trace

			const Vec3 viewangles = I::EngineClient->GetViewAngles();
			Vec3 vForward, vRight, vUp;
			Math::AngleVectors(viewangles, &vForward, &vRight, &vUp);

			const Vec3 viewDiff = pView->origin - pLocal->GetEyePosition();

			Vec3 offset;
			offset += vRight * Vars::Visuals::ThirdpersonRight.Value;
			offset += vUp * Vars::Visuals::ThirdpersonUp.Value;
			offset -= vForward * Vars::Visuals::ThirdpersonDist.Value;

			CGameTrace Trace = {}; CTraceFilterWorldAndPropsOnly Filter = {};
			Utils::TraceHull(pView->origin - viewDiff, pView->origin + offset - viewDiff, {-16.0f, -16.0f, -16.0f}, {16.0f, 16.0f, 16.0f}, MASK_SOLID, &Filter, &Trace);

			pView->origin += offset * Trace.flFraction - viewDiff;
		}
	}
}

void CVisuals::BulletTrace(CBaseEntity* pEntity, Color_t color)
{
	Vector forward, src, dst;
	CGameTrace tr;
	Ray_t ray;
	CTraceFilterHitscan filter;

	Math::AngleVectors(pEntity->GetEyeAngles(), &forward);
	filter.pSkip = pEntity;
	const Vector src3D = pEntity->GetBonePos(6) - Vector(0, 0, 0);
	const Vector dst3D = src3D + (forward * 1000);

	ray.Init(src3D, dst3D);

	I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (!Utils::W2S(src3D, src) || !Utils::W2S(tr.vEndPos, dst))
	{
		return;
	}

	//I::Surface->DrawLine(src.x, src.y, dst.x, dst.y);
	g_Draw.Line(src.x, src.y, dst.x, dst.y, color);
}

void CVisuals::ProjectileTrace() // make it ignore other projectiles
{
	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();
	if (!pWeapon || !pLocal)
		return;

	ProjectileInfo projInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, I::EngineClient->GetViewAngles(), projInfo, true))
		return;

	if (!F::ProjSim.Initialize(projInfo))
		return;

	for (int n = 0; n < TIME_TO_TICKS(projInfo.m_lifetime); n++)
	{
		Vec3 Old = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(projInfo);
		Vec3 New = F::ProjSim.GetOrigin();

		CGameTrace trace = {};
		CTraceFilterHitscan filter = {};
		filter.pSkip = pLocal;
		Utils::TraceHull(Old, New, projInfo.m_hull * -1.f, projInfo.m_hull, MASK_SOLID, &filter, &trace);
		if (trace.DidHit())
		{
			Vec3 angles;
			Math::VectorAngles(trace.Plane.normal, angles);

			Color_t face = Vars::Aimbot::Projectile::ProjectileColor; face.a = 0;
			Color_t edge = Vars::Aimbot::Projectile::ProjectileColor;

			const Vec3 size = { projInfo.m_hull.x * 2.f, 16.f, 16.f };
			RenderBox(trace.vEndPos, size / -2, size / 2, angles, edge, face);

			projInfo.PredictionLines.push_back({ trace.vEndPos, Math::GetRotatedPosition(trace.vEndPos, Math::VelocityToAngles(F::ProjSim.GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value) });

			break;
		}
	}

	DrawSimLine(projInfo.PredictionLines, Vars::Aimbot::Projectile::ProjectileColor);
}

void DebugLine(const char* title, const char* value, std::pair<int, int> offsets, Color_t clr = { 255, 255, 255, 255 })
{
	g_Draw.String(FONT_MENU, offsets.first, offsets.second += 15, clr, ALIGN_DEFAULT, title);
	g_Draw.String(FONT_MENU, offsets.first + 125, offsets.second, clr, ALIGN_DEFAULT, value);
}

void CVisuals::DrawDebugInfo(CBaseEntity* pLocal)
{
	// Debug info
	if (Vars::Debug::DebugInfo.Value)
	{
		int yoffset = 10, xoffset = 10;

		{
			g_Draw.String(FONT_INDICATORS, xoffset, yoffset += 15, Utils::Rainbow(), ALIGN_DEFAULT, "Fedoraware");
		}
		{
			g_Draw.String(FONT_MENU, xoffset, yoffset += 15, { 119, 255, 225, 255 }, ALIGN_DEFAULT, "Local Player"); // header
		}
		// alive
		{
			const bool alive = pLocal->IsAlive();
			Color_t clr = alive ? Color_t{ 153, 232, 0, 255 } : Color_t{ 167, 0, 0, 255 };
			g_Draw.String(FONT_MENU, xoffset, yoffset += 15, clr, ALIGN_DEFAULT, "%s", alive ? "ALIVE" : "DEAD");
		}

		if (!G::LastUserCmd){ return; }
		const float flLastFwd = G::LastUserCmd->forwardmove;
		const float flLastSde = G::LastUserCmd->sidemove;
		{
			g_Draw.String(FONT_MENU, xoffset, yoffset += 15, {255, 255, 255, 255}, ALIGN_DEFAULT, "%.0f", flLastFwd);
			g_Draw.String(FONT_MENU, xoffset, yoffset += 15, {255, 255, 255, 255}, ALIGN_DEFAULT, "%.0f", flLastSde);
		}
	}
}

void CVisuals::DrawAntiAim(CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive() || !I::Input->CAM_IsThirdPerson())
	{
		return;
	}

	if (Vars::AntiHack::AntiAim::Active.Value)
	{
		static constexpr Color_t realColour = { 0, 255,0, 255 };
		static constexpr Color_t fakeColour = { 255, 0, 0, 255 };

		const auto& vOrigin = pLocal->GetAbsOrigin();

		Vec3 vScreen1, vScreen2;
		if (Utils::W2S(vOrigin, vScreen1))
		{
			constexpr auto distance = 50.f;
			if (Utils::W2S(Utils::GetRotatedPosition(vOrigin, G::RealViewAngles.y, distance), vScreen2))
			{
				g_Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, realColour);
			}

			if (Utils::W2S(Utils::GetRotatedPosition(vOrigin, G::FakeViewAngles.y, distance), vScreen2))
			{
				g_Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, fakeColour);
			}
		}
	}
}

#include "../../Resources/DVD-Icon.h"

void CVisuals::DrawTickbaseText()
{
	if (!Vars::CL_Move::DoubleTap::Indicator.Value)
		return;

	const auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal || !pLocal->IsAlive())
		return;

	//const int ticks = std::clamp(G::ShiftedTicks + G::ChokedTicks/*I::ClientState->chokedcommands*/, 0, Vars::CL_Move::DoubleTap::TickLimit.Value);
	const int ticks = G::ShiftedTicks + G::ChokedTicks/*I::ClientState->chokedcommands*/;
	const DragBox_t dtPos = Vars::CL_Move::DoubleTap::Position;

	const auto fontHeight = Vars::Fonts::FONT_INDICATORS::nTall.Value;
	g_Draw.String(FONT_INDICATORS, dtPos.c, dtPos.y + 15 - fontHeight, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"Ticks %d / %d", ticks, Vars::CL_Move::DoubleTap::TickLimit.Value);
	if (G::WaitForShift || !G::ShiftedTicks || G::Recharging)
	{
		g_Draw.String(FONT_INDICATORS, dtPos.c, dtPos.y + fontHeight + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"Not Ready");
	}

	if (Vars::Debug::DebugInfo.Value)
	{
		g_Draw.String(FONT_INDICATORS, dtPos.c, dtPos.y + fontHeight * 2 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::Recharge: %d", G::Recharge);
		g_Draw.String(FONT_INDICATORS, dtPos.c, dtPos.y + fontHeight * 3 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::Recharging: %d", G::Recharging);
		g_Draw.String(FONT_INDICATORS, dtPos.c, dtPos.y + fontHeight * 4 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::ShouldShift: %d", G::ShouldShift);
		g_Draw.String(FONT_INDICATORS, dtPos.c, dtPos.y + fontHeight * 5 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::Teleporting: %d", G::Teleporting);
	}
}
void CVisuals::DrawTickbaseBars()
{
	if (!Vars::CL_Move::DoubleTap::Indicator.Value)
		return;

	const auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal || !pLocal->IsAlive())
		return;

	//const int ticks = std::clamp(G::ShiftedTicks + G::ChokedTicks/*I::ClientState->chokedcommands*/, 0, Vars::CL_Move::DoubleTap::TickLimit.Value);
	const int ticks = G::ShiftedTicks + G::ChokedTicks/*I::ClientState->chokedcommands*/;
	const DragBox_t dtPos = Vars::CL_Move::DoubleTap::Position;
	const float ratioCurrent = (float)ticks / (float)Vars::CL_Move::DoubleTap::TickLimit.Value;

	ImGui::GetBackgroundDrawList()->AddRectFilled(
		ImVec2(dtPos.x, dtPos.y + 18), ImVec2(dtPos.x + 100, dtPos.y + 30),
		ImColor(Colors::DtOutline.r, Colors::DtOutline.g, Colors::DtOutline.b, Colors::DtOutline.a), 10
	);
	if (ticks && ratioCurrent)
	{
		ImGui::GetBackgroundDrawList()->PushClipRect(ImVec2(dtPos.x + 2, dtPos.y + 20), ImVec2(dtPos.x + 2 + 96 * ratioCurrent, dtPos.y + 28), true);
		ImGui::GetBackgroundDrawList()->AddRectFilled(
			ImVec2(dtPos.x + 2, dtPos.y + 20), ImVec2(dtPos.x + 98, dtPos.y + 28),
			ImColor(Vars::Menu::MenuAccent.r, Vars::Menu::MenuAccent.g, Vars::Menu::MenuAccent.b, Vars::Menu::MenuAccent.a), 10
		);
		ImGui::GetBackgroundDrawList()->PopClipRect();
	}
}

void CVisuals::DrawServerHitboxes()
{
	static int iOldTick = I::GlobalVars->tickcount;
	if (iOldTick == I::GlobalVars->tickcount) { return; } iOldTick = I::GlobalVars->tickcount;
	// draw our serverside hitbox on local servers, used to test fakelag & antiaim
	if (I::Input->CAM_IsThirdPerson() && Vars::Visuals::ThirdPersonServerHitbox.Value)
	{
		//	i have no idea what this is
		using GetServerAnimating_t = void* (*)(int);
		static auto GetServerAnimating = reinterpret_cast<GetServerAnimating_t>(g_Pattern.Find(L"server.dll", L"55 8B EC 8B 55 ? 85 D2 7E ? A1"));

		using DrawServerHitboxes_t = void(__thiscall*)(void*, float, bool);	// C_BaseAnimating, Duration, MonoColour
		static auto DrawServerHitboxes = reinterpret_cast<DrawServerHitboxes_t>(g_Pattern.Find(L"server.dll", L"55 8B EC 83 EC ? 57 8B F9 80 BF ? ? ? ? ? 0F 85 ? ? ? ? 83 BF ? ? ? ? ? 75 ? E8 ? ? ? ? 85 C0 74 ? 8B CF E8 ? ? ? ? 8B 97"));

		const auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		if (pLocal && pLocal->IsAlive())
		{
			void* server_animating = GetServerAnimating(pLocal->GetIndex());
			if (server_animating)
			{
				DrawServerHitboxes(server_animating, I::GlobalVars->interval_per_tick, true);
			}
		}
	}
}

void CVisuals::DrawBulletLines()
{
	/*
	const auto& pWeapon = g_EntityCache.GetWeapon();

	if (pWeapon)
	{
		for (auto& Line : G::BulletsStorage)
		{
			if (Line.m_flTime < I::GlobalVars->curtime) continue;

			RenderLine(Line.m_line.first, Line.m_line.second, Line.m_color, false);
		}
	}
	else
	{
		for (auto& Line : G::BulletsStorage)
		{
			Line.m_flTime = -10.f;
		}
	}
	*/
	for (auto& Line : G::BulletsStorage)
	{
		if (Line.m_flTime < I::GlobalVars->curtime) continue;

		RenderLine(Line.m_line.first, Line.m_line.second, Line.m_color, false);
	}
}

void CVisuals::ClearBulletLines()
{
	// in the case of drawing multiple bullets
	//for (auto& Line : G::BulletsStorage)
	//{
	//	if (Line.m_flTime + 0.01f < I::GlobalVars->curtime)
	//	{
			G::BulletsStorage.clear();
	//		break;
	//	}
	//}
}

void CVisuals::RevealBulletLines()
{
	for (auto& Line : G::BulletsStorage)
	{
		Line.m_flTime = I::GlobalVars->curtime + 60.f;
	}
}

void CVisuals::DrawSimLine(std::deque<std::pair<Vec3, Vec3>>& Line, Color_t Color, bool bSeparators)
{
	if (!bSeparators)
	{
		for (size_t i = 1; i < Line.size(); i++)
		{
			RenderLine(Line.at(i - 1).first, Line.at(i).first, Color, false);
		}
	}
	else
	{
		for (size_t i = 1; i < Line.size(); i++)
		{
			const auto& vStart = Line[i - 1].first;
			const auto& vRotate = Line[i - 1].second;	//	splirp vec
			const auto& vEnd = Line[i].first;
			if ((i % Vars::Visuals::SeperatorSpacing.Value) == 0) { RenderLine(vStart, vRotate, Color, false); }
			RenderLine(vStart, vEnd, Color, false);
		}
	}
}

void CVisuals::DrawSimLines()
{
	/*
	const auto& pWeapon = g_EntityCache.GetWeapon();

	if (pWeapon)
	{
		for (auto& Line : G::LinesStorage)
		{
			if (Line.m_flTime < I::GlobalVars->curtime) continue;

			DrawSimLine(Line.m_line, Line.m_color, Vars::Visuals::SimSeperators.Value);
		}
	}
	else
	{
		for (auto& Line : G::LinesStorage)
		{
			Line.m_flTime = -10.f;
		}
	}
	*/
	for (auto& Line : G::LinesStorage)
	{
		if (Line.m_flTime > 0.f && Line.m_flTime < I::GlobalVars->curtime)
			continue;

		DrawSimLine(Line.m_line, Line.m_color, Vars::Visuals::SimSeperators.Value);
	}
}

void CVisuals::RevealSimLines()
{
	for (auto& PredictionLine : G::LinesStorage)
	{
		PredictionLine.m_flTime = I::GlobalVars->curtime + 60.f;
	}
}

void CVisuals::DrawBoxes()
{
	/*
	const auto& pWeapon = g_EntityCache.GetWeapon();

	if (pWeapon)
	{
		for (auto& Box : G::BoxesStorage)
		{
			if (Box.m_flTime < I::GlobalVars->curtime) continue;

			RenderBox(Box.m_vecPos, Box.m_vecMins, Box.m_vecMaxs, Box.m_vecOrientation, Box.m_colorEdge, Box.m_colorFace);
		}
	}
	else
	{
		for (auto& Box : G::BoxesStorage)
		{
			Box.m_flTime = -10.f;
		}
	}
	*/
	for (auto& Box : G::BoxesStorage)
	{
		if (Box.m_flTime < I::GlobalVars->curtime) continue;

		RenderBox(Box.m_vecPos, Box.m_vecMins, Box.m_vecMaxs, Box.m_vecOrientation, Box.m_colorEdge, Box.m_colorFace);
	}
}

void CVisuals::RevealBoxes()
{
	for (auto& Box : G::BoxesStorage)
	{
		Box.m_flTime = I::GlobalVars->curtime + 60.f;
	}
}

// Credits go to reestart entirely
void CVisuals::ManualNetwork(const StartSoundParams_t& params)
{
	if (!Vars::ESP::Main::DormantSoundESP.Value) { return; }
	if (params.soundsource <= 0) { return; }

	Vector vOrigin;
	VectorCopy(params.origin, vOrigin);
	const int iEntIndex = params.soundsource;
	CBaseEntity* pEntity = I::ClientEntityList->GetClientEntity(iEntIndex);

	if (pEntity
		&& iEntIndex != I::EngineClient->GetLocalPlayer()
		&& pEntity->GetDormant()
		&& pEntity->GetClassID() == ETFClassID::CTFPlayer)
	{
		G::DormantPlayerESP[iEntIndex] = { vOrigin, I::EngineClient->Time() };
	}
}

void CVisuals::RenderLine(const Vec3& v1, const Vec3& v2, Color_t c, bool bZBuffer)
{
	static auto RenderLineFn = reinterpret_cast<void(__cdecl*)(const Vector&, const Vector&, Color_t, bool)>(g_Pattern.Find(L"engine.dll", L"55 8B EC 81 EC ? ? ? ? 56 E8 ? ? ? ? 8B 0D ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B F0 85 F6"));
	RenderLineFn(v1, v2, c, bZBuffer);
}

void CVisuals::RenderBox(const Vec3& vPos, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vOrientation, Color_t cEdge, Color_t cFace)
{
	static auto RenderBoxFn = reinterpret_cast<void(__cdecl*)(const Vec3&, const Vec3&, const Vec3&, const Vec3&, Color_t, bool, bool)>(g_Pattern.Find(L"engine.dll", L"55 8B EC 51 8B 45 ? 8B C8 FF 75"));
	RenderBoxFn(vPos, vOrientation, vMins, vMaxs, cFace, false, false);

	static auto RenderWireframeBoxFn = reinterpret_cast<void(__cdecl*)(const Vec3&, const Vec3&, const Vec3&, const Vec3&, Color_t, bool)>(g_Pattern.Find(L"engine.dll", L"55 8B EC 81 EC ? ? ? ? 56 E8 ? ? ? ? 8B 0D ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B F0 89 75 ? 85 F6 74 ? 8B 06 8B CE FF 50 ? A1"));
	RenderWireframeBoxFn(vPos, vOrientation, vMins, vMaxs, cEdge, false);
}

void CVisuals::DrawSightlines()
{
	if (Vars::ESP::Players::SniperSightlines.Value)
	{
		if (!m_SightLines.empty())
		{
			for (const auto& sightline : m_SightLines)
			{
				if (sightline.m_bDraw)
				{
					RenderLine(sightline.m_vStart, sightline.m_vEnd, sightline.m_Color, false);
				}
			}
		}
	}
}

void CVisuals::FillSightlines()
{
	if (Vars::ESP::Players::SniperSightlines.Value)
	{
		Vec3 vShootPos, vForward, vShootEnd;
		CTraceFilterHitscan filter{};
		CGameTrace trace{};
		m_SightLines = {}; // should get rid of residual lines
		for (const auto& pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			const int iEntityIndex = pEnemy->GetIndex();
			if (!(pEnemy->IsAlive()) ||
				!(pEnemy->GetClassNum() == CLASS_SNIPER) ||
				!(pEnemy->GetCond() & TFCond_Zoomed) ||
				(pEnemy->GetDormant()))
			{
				m_SightLines[iEntityIndex] = { Vec3(), Vec3(), Color_t(), false };
				continue;
			}
			vShootPos = pEnemy->GetAbsOrigin() + (pEnemy->IsDucking() ? Vec3(0, 0, 45) : Vec3(0, 0, 75));
			Math::AngleVectors(pEnemy->GetEyeAngles(), &vForward);
			vShootEnd = vShootPos + (vForward * 8192.f);

			filter.pSkip = pEnemy;
			Utils::Trace(vShootPos, vShootEnd, MASK_SHOT, &filter, &trace);

			m_SightLines[pEnemy->GetIndex()] = { vShootPos, trace.vEndPos, Utils::GetEntityDrawColor(pEnemy, Vars::ESP::Main::EnableTeamEnemyColors.Value), true};
		}
	}
}

void CVisuals::AddBulletTracer(const Vec3& vFrom, const Vec3& vTo, const Color_t& clr)
{
/*
	m_vecBulletTracers.push_back({ vFrom, vTo, clr, I::GlobalVars->curtime });
*/
}

void CVisuals::PruneBulletTracers()
{
/*
	const float curtime = I::GlobalVars->curtime;

	if (m_vecBulletTracers.size())
	{
		for (size_t i = 0; i < m_vecBulletTracers.size(); i++)
		{
			const auto& bulletTracer = m_vecBulletTracers.at(i);
			if (curtime > bulletTracer.m_flTimeCreated + 5)
			{
				m_vecBulletTracers.erase(m_vecBulletTracers.begin(), m_vecBulletTracers.begin() + 1);
			}
		}
	}
*/
}

void CVisuals::DrawBulletTracers()
{
/*
	const float curTime = I::GlobalVars->curtime;
	if (m_vecBulletTracers.size())
	{
		for (const auto& bulletTracer : m_vecBulletTracers)
		{
			Color_t tracerColor = bulletTracer.m_Color;
			const float flDistance = curTime - bulletTracer.m_flTimeCreated;
			if (flDistance < 0)
			{
				tracerColor.a = 255;
			}
			else
			{
				//I::Cvar->ConsolePrintf("%f\n", flDistance);
				tracerColor.a = Math::RemapValClamped(flDistance, 0, 1, 255, 0);
			}
			//I::Cvar->ConsolePrintf("a: %d\n", tracerColor.a);
			RenderLine(bulletTracer.m_vStartPos, bulletTracer.m_vEndPos, tracerColor, false);
		}
	}
*/
}

void CVisuals::DrawProjectileTracer(CBaseEntity* pLocal, const Vec3& position)
{
/*
	if (pLocal->IsTaunting())
	{
		return;
	}

	if (!I::Prediction->m_bFirstTimePredicted)
	{
		return;
	}

	static int lastTickcount = 0;
	if (lastTickcount == I::GlobalVars->tickcount)
	{
		return;
	}

	lastTickcount = I::GlobalVars->tickcount;
	const Vec3 vecPos = G::CurWeaponType == EWeaponType::PROJECTILE ? G::PredictedPos : position;
	const Color_t tracerColor = Colors::BulletTracer;
	Vec3 shootPos;
	const int iAttachment = pLocal->GetActiveWeapon()->LookupAttachment("muzzle");
	pLocal->GetActiveWeapon()->GetAttachment(iAttachment, shootPos);
	AddBulletTracer(shootPos, vecPos, tracerColor);
*/
}

void CVisuals::DrawAimbotFOV(CBaseEntity* pLocal)
{
	float curFOV = 0.f;
	if (Vars::Aimbot::Global::Active.Value)
	{
		switch (G::CurWeaponType)
		{
		case EWeaponType::HITSCAN:
			if (Vars::Aimbot::Hitscan::Active.Value)
				curFOV = Vars::Aimbot::Hitscan::AimFOV.Value;
			break;
		case EWeaponType::PROJECTILE:
			if(Vars::Aimbot::Projectile::Active.Value)
				curFOV = Vars::Aimbot::Projectile::AimFOV.Value;
			break;
		case EWeaponType::MELEE:
			if (Vars::Aimbot::Melee::Active.Value)
				curFOV = Vars::Aimbot::Melee::AimFOV.Value;
			break;
		default: curFOV = 0.f; break;
		}
	}

	//Current Active Aimbot FOV
	if (curFOV)
	{
		const float flFOV = static_cast<float>(pLocal->IsScoped() ? Vars::Visuals::ZoomFieldOfView.Value : Vars::Visuals::FieldOfView.Value);
		const float flR = tanf(DEG2RAD(curFOV) / 2.0f) / tanf(
			//DEG2RAD((pLocal->IsScoped() && !Vars::Visuals::RemoveZoom.Value) ? 30.0f : flFOV) /
			DEG2RAD(flFOV) / 2.0f) * g_ScreenSize.w;
		const Color_t clr = Colors::FOVCircle;
		g_Draw.OutlinedCircle(g_ScreenSize.w / 2, g_ScreenSize.h / 2, flR, 68, clr);
	}
}


void CVisuals::StoreMaterialHandles()
{
	for (MaterialHandle_t h = I::MaterialSystem->First(); h != I::MaterialSystem->Invalid(); h = I::MaterialSystem->Next(h))
	{
		if (const auto& pMaterial = I::MaterialSystem->Get(h))
		{
			if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
			{
				continue;
			}

			MaterialHandleData data;

			data.Handle = h;

			data.Material = pMaterial;

			auto sGroup = std::string_view(pMaterial->GetTextureGroupName());

			data.Group = sGroup;

			auto sName = std::string_view(pMaterial->GetName());

			data.Name = sName;

			if (sGroup._Starts_with("SkyBox"))
			{
				data.GroupType = MaterialHandleData::EMatGroupType::GROUP_SKY;
			}
			else if (sGroup._Starts_with("World"))
			{
				data.GroupType = MaterialHandleData::EMatGroupType::GROUP_WORLD;
			}
			else
			{
				data.GroupType = MaterialHandleData::EMatGroupType::GROUP_OTHER;
			}

			if (sName.find("water") != std::string_view::npos || sName.find("glass") != std::string_view::npos
				|| sName.find("door") != std::string_view::npos || sName.find("tools") != std::string_view::npos
				|| sName.find("player") != std::string_view::npos || sName.find("chicken") != std::string_view::npos
				|| sName.find("wall28") != std::string_view::npos || sName.find("wall26") != std::string_view::npos
				|| sName.find("decal") != std::string_view::npos || sName.find("overlay") != std::string_view::npos
				|| sName.find("hay") != std::string_view::npos)
			{
				data.ShouldOverrideTextures = false;
			}
			else
			{
				data.ShouldOverrideTextures = true;
			}

			MaterialHandleDatas.push_back(data);
		}
	}
}

void CVisuals::ClearMaterialHandles()
{
	MaterialHandleDatas.clear();
}

// this whole section below is for world modulation
bool ModColChanged() // check if colours have been changed
{
	static auto oldW = Colors::WorldModulation;
	static auto oldS = Colors::SkyModulation;
	const auto curW = Colors::WorldModulation;
	const auto curS = Colors::SkyModulation;

	if (curW.r != oldW.r || curW.g != oldW.g || curW.b != oldW.b || curS.r != oldS.r || curS.g != oldS.g || curS.b != oldS.b)
	{
		oldW = curW;
		oldS = curS;
		return true;
	}
	return false;
}

bool ModSetChanged() // check if modulation has been switched
{
	static auto oldS = Vars::Visuals::SkyModulation.Value;
	static auto oldW = Vars::Visuals::WorldModulation.Value;
	const auto curS = Vars::Visuals::SkyModulation.Value;
	const auto curW = Vars::Visuals::WorldModulation.Value;

	if (curS != oldS || curW != oldW)
	{
		oldW = curW;
		oldS = curS;
		return true;
	}
	return false;
}

void ApplyModulation(const Color_t& clr)
{
	//for (MaterialHandle_t h = I::MatSystem->First(); h != I::MatSystem->Invalid(); h = I::
	//	MatSystem->Next(h))
	//{
	//	if (const auto& pMaterial = I::MatSystem->Get(h))
	//	{
	//		if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
	//			continue;

	//		std::string_view group(pMaterial->GetTextureGroupName());

	//		if (group.find(_(TEXTURE_GROUP_WORLD)) != group.npos)
	//		{
	//			pMaterial->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
	//		}
	//	}
	//}
	if (F::Visuals.MaterialHandleDatas.empty())
	{
		return;
	}

	for (const auto& material : F::Visuals.MaterialHandleDatas)
	{
		if (material.Material)
		{
			if (material.GroupType != CVisuals::MaterialHandleData::EMatGroupType::GROUP_WORLD)
			{
				continue;
			}
			if (material.Material->IsErrorMaterial() || !material.Material->IsPrecached())
			{
				continue;
			}
			material.Material->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
		}
	}
}

void ApplySkyboxModulation(const Color_t& clr)
{
	//for (MaterialHandle_t h = I::MatSystem->First(); h != I::MatSystem->Invalid(); h = I::
	//	MatSystem->Next(h))
	//{
	//	const auto& pMaterial = I::MatSystem->Get(h);

	//	if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
	//		continue;

	//	std::string_view group(pMaterial->GetTextureGroupName());

	//	if (group._Starts_with("SkyBox"))
	//	{
	//		pMaterial->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
	//	}
	//}
	if (F::Visuals.MaterialHandleDatas.empty())
	{
		return;
	}

	for (const auto& material : F::Visuals.MaterialHandleDatas)
	{
		if (material.Material)
		{
			if (material.GroupType != CVisuals::MaterialHandleData::EMatGroupType::GROUP_SKY)
			{
				continue;
			}
			if (material.Material->IsErrorMaterial() || !material.Material->IsPrecached())
			{
				continue;
			}
			material.Material->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
		}
	}
}

void CVisuals::ModulateWorld()
{
	static bool oConnectionState = (I::EngineClient->IsConnected() && I::EngineClient->IsInGame());
	const bool connectionState = (I::EngineClient->IsConnected() && I::EngineClient->IsInGame());
	const bool isUnchanged = connectionState == oConnectionState;
	static bool shouldModulate = false;

	if (ModColChanged() || ModSetChanged() || !isUnchanged)
	{
		Vars::Visuals::WorldModulation.Value ? ApplyModulation(Colors::WorldModulation) : ApplyModulation({ 255, 255, 255, 255 });
		Vars::Visuals::SkyModulation.Value ? ApplySkyboxModulation(Colors::SkyModulation) : ApplySkyboxModulation({ 255, 255, 255, 255 });
		oConnectionState = connectionState;
		shouldModulate = false;
	}
	else if (!Vars::Visuals::WorldModulation.Value)
	{
		if (!shouldModulate)
		{
			ApplyModulation({ 255, 255, 255, 255 });
			shouldModulate = true;
		}
	} // i don't know why i need to do this
}

void CVisuals::RestoreWorldModulation() // keep this because its mentioned in @DLLMain.cpp if you find a better way to do this, remove it ig.
{
	ApplyModulation({ 255, 255, 255, 255 });
	ApplySkyboxModulation({ 255, 255, 255, 255 });
}

// all world mod stuff above

void CVisuals::OverrideWorldTextures() //This is 100% pasted from spook953
{
	static KeyValues *kv = nullptr;

	if (!kv) {
		kv = new KeyValues("LightmappedGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetString("$color2", "[0.12 0.12 0.15]");
	}

	if (Vars::Visuals::OverrideWorldTextures.Value)
	{
		for (auto h = I::MaterialSystem->First(); h != I::MaterialSystem->Invalid(); h = I::MaterialSystem->Next(h))
		{
			IMaterial *pMaterial = I::MaterialSystem->Get(h);

			if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached()
				|| pMaterial->IsTranslucent() || pMaterial->IsSpriteCard()
				|| std::string_view(pMaterial->GetTextureGroupName()).find("World") == std::string_view::npos)
				continue;

			std::string_view sName = std::string_view(pMaterial->GetName());

			if (sName.find("water") != std::string_view::npos || sName.find("glass") != std::string_view::npos
				|| sName.find("door") != std::string_view::npos || sName.find("tools") != std::string_view::npos
				|| sName.find("player") != std::string_view::npos || sName.find("chicken") != std::string_view::npos
				|| sName.find("wall28") != std::string_view::npos || sName.find("wall26") != std::string_view::npos
				|| sName.find("decal") != std::string_view::npos || sName.find("overlay") != std::string_view::npos
				|| sName.find("hay") != std::string_view::npos)
				continue;

			pMaterial->SetShaderAndParams(kv);
		}
	}
}

void CVisuals::PickupTimers()
{
	if (!Vars::Visuals::PickupTimers.Value) { return; }

	for (auto pickupData = PickupDatas.begin(); pickupData != PickupDatas.end();)
	{
		const float timeDiff = I::EngineClient->Time() - pickupData->Time;
		if (timeDiff > 10.f)
		{
			pickupData = PickupDatas.erase(pickupData);
			continue;
		}

		auto timerText = tfm::format("%.1f", 10.f - timeDiff);
		auto color = pickupData->Type ? Colors::Health : Colors::Ammo;

		Vec3 vScreen;
		if (Utils::W2S(pickupData->Location, vScreen))
		{
			g_Draw.String(FONT_ESP_PICKUPS, vScreen.x, vScreen.y, color, ALIGN_CENTER, timerText.c_str());
		}

		++pickupData;
	}
}
