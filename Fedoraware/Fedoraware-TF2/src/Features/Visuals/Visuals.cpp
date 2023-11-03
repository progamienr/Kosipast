#include "Visuals.h"

#include "../Vars.h"
#include "../Color.h"
#include "../Visuals/LocalConditions/LocalConditions.h"
#include "../Backtrack/Backtrack.h"
#include "../PacketManip/AntiAim/AntiAim.h"
#include "../Simulation/ProjectileSimulation/ProjectileSimulation.h"

#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_stdlib.h>

namespace S
{
	MAKE_SIGNATURE(RenderLine, ENGINE_DLL, "55 8B EC 81 EC ? ? ? ? 56 E8 ? ? ? ? 8B 0D ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B F0 85 F6", 0x0);
	MAKE_SIGNATURE(RenderBox, ENGINE_DLL, "55 8B EC 51 8B 45 ? 8B C8 FF 75", 0x0);
	MAKE_SIGNATURE(RenderWireframeBox, ENGINE_DLL, "55 8B EC 81 EC ? ? ? ? 56 E8 ? ? ? ? 8B 0D ? ? ? ? 8B 01 FF 90 ? ? ? ? 8B F0 89 75 ? 85 F6 74 ? 8B 06 8B CE FF 50 ? A1", 0x0);
	MAKE_SIGNATURE(DrawServerHitboxes, SERVER_DLL, "55 8B EC 83 EC ? 57 8B F9 80 BF ? ? ? ? ? 0F 85 ? ? ? ? 83 BF ? ? ? ? ? 75 ? E8 ? ? ? ? 85 C0 74 ? 8B CF E8 ? ? ? ? 8B 97", 0x0);
	MAKE_SIGNATURE(GetServerAnimating, SERVER_DLL, "55 8B EC 8B 55 ? 85 D2 7E ? A1", 0x0);
	MAKE_SIGNATURE(LoadSkys, ENGINE_DLL, "55 8B EC 81 EC ? ? ? ? 8B 0D ? ? ? ? 53 56 57 8B 01 C7 45", 0x0);
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
			if (Vars::Aimbot::Projectile::Active.Value)
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
		const Color_t clr = Vars::Colors::FOVCircle.Value;
		g_Draw.OutlinedCircle(g_ScreenSize.w / 2, g_ScreenSize.h / 2, flR, 68, clr);
	}
}

void CVisuals::DrawTickbaseText()
{
	if (!Vars::CL_Move::DoubleTap::Indicator.Value)
		return;

	const auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal || !pLocal->IsAlive())
		return;

	const int iTicks = std::clamp(G::ShiftedTicks + G::ChokeAmount - (G::AntiAim ? 1 : 0), 0, G::MaxShift);

	const DragBox_t dtPos = Vars::CL_Move::DoubleTap::Position.Value;
	const auto& fFont = g_Draw.GetFont(FONT_INDICATORS);
	const int iHeight = Vars::Fonts::FONT_INDICATORS::nTall.Value;

	g_Draw.String(fFont, dtPos.c, dtPos.y + 15 - iHeight, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"Ticks %d / %d", iTicks, G::MaxShift);
	if (G::WaitForShift)
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"Not Ready");

	if (Vars::Debug::Info.Value)
	{
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight * 3 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::Recharge: %d", G::Recharge);
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight * 4 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::DoubleTap: %d", G::DoubleTap);
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight * 5 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::Teleport: %d", G::Teleport);
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight * 6 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::ShiftedGoal: %d", G::ShiftedGoal);
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight * 7 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::AntiWarp: %d", G::AntiWarp);
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight * 8 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::MaxShift: %d", G::MaxShift);
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight * 9 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::WeaponCanAttack %d (%d)", G::WeaponCanAttack, I::GlobalVars->tickcount % 66);
		g_Draw.String(fFont, dtPos.c, dtPos.y + iHeight * 10 + 19, { 255,255,255,255 }, ALIGN_CENTERHORIZONTAL, L"G::IsAttacking %d", G::IsAttacking);
	}
}
void CVisuals::DrawTickbaseBars()
{
	if (!Vars::CL_Move::DoubleTap::Indicator.Value)
		return;

	const auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
	if (!pLocal || !pLocal->IsAlive())
		return;

	const int iTicks = std::clamp(G::ShiftedTicks + G::ChokeAmount - (G::AntiAim ? 1 : 0), 0, G::MaxShift);

	const DragBox_t dtPos = Vars::CL_Move::DoubleTap::Position.Value;
	const float ratioCurrent = (float)iTicks / (float)G::MaxShift;

	ImGui::GetBackgroundDrawList()->AddRectFilled(
		ImVec2(dtPos.x, dtPos.y + 18), ImVec2(dtPos.x + 100, dtPos.y + 30),
		ImColor(Vars::Colors::DtOutline.Value.r, Vars::Colors::DtOutline.Value.g, Vars::Colors::DtOutline.Value.b, Vars::Colors::DtOutline.Value.a), 10
	);
	if (iTicks && ratioCurrent)
	{
		ImGui::GetBackgroundDrawList()->PushClipRect(ImVec2(dtPos.x + 2, dtPos.y + 20), ImVec2(dtPos.x + 2 + 96 * ratioCurrent, dtPos.y + 28), true);
		ImGui::GetBackgroundDrawList()->AddRectFilled(
			ImVec2(dtPos.x + 2, dtPos.y + 20), ImVec2(dtPos.x + 98, dtPos.y + 28),
			ImColor(Vars::Menu::Theme::Accent.Value.r, Vars::Menu::Theme::Accent.Value.g, Vars::Menu::Theme::Accent.Value.b, Vars::Menu::Theme::Accent.Value.a), 10
		);
		ImGui::GetBackgroundDrawList()->PopClipRect();
	}
}

void CVisuals::DrawOnScreenConditions(CBaseEntity* pLocal)
{
	if (!Vars::Visuals::DrawOnScreenConditions.Value) { return; }
	if (!pLocal->IsAlive() || pLocal->IsAGhost()) { return; }

	int x = Vars::Visuals::OnScreenConditions.Value.x;
	int y = Vars::Visuals::OnScreenConditions.Value.y + 8;
	const auto& fFont = g_Draw.GetFont(FONT_MENU);

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
		g_Draw.String(fFont, x, y + offset, { 255, 255, 255, 255 }, align, cond.data());
		I::VGuiSurface->GetTextSize(fFont.dwFont, cond.data(), width, height);
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

	int x = Vars::Visuals::OnScreenPing.Value.x;
	int y = Vars::Visuals::OnScreenPing.Value.y + 8;
	const auto& fFont = g_Draw.GetFont(FONT_MENU);

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
			if (!Vars::Debug::Info.Value)
				g_Draw.String(fFont, x, y, { 255, 255, 255, 255 }, align, "Real %.0f (+ %.0f) ms", flLatency, flFake);
			else
				g_Draw.String(fFont, x, y, { 255, 255, 255, 255 }, align, "In %.0f, Out %.0f (+ %.0f) ms", flLatencyIn, flLatencyOut, flFake);
		}
		else
			g_Draw.String(fFont, x, y, { 255, 255, 255, 255 }, align, "Syncing");
	}
	else
	{
		if (!Vars::Debug::Info.Value)
			g_Draw.String(fFont, x, y, { 255, 255, 255, 255 }, align, "Real %.0f ms", flLatency);
		else
			g_Draw.String(fFont, x, y, { 255, 255, 255, 255 }, align, "In %.0f, Out %.0f ms", flLatencyIn, flLatencyOut);
	}
	g_Draw.String(fFont, x, y + Vars::Fonts::FONT_INDICATORS::nTall.Value + 2, { 255, 255, 255, 255 }, align, "Scoreboard %d ms", iLatencyScoreBoard);
	if (Vars::Debug::Info.Value)
	{
		g_Draw.String(fFont, x, y + Vars::Fonts::FONT_INDICATORS::nTall.Value * 2 + 2, { 255, 255, 255, 255 }, align, "GetFake %.0f, flFakeInterp %.0f", F::Backtrack.GetFake() * 1000.f, F::Backtrack.flFakeInterp * 1000.f);
		g_Draw.String(fFont, x, y + Vars::Fonts::FONT_INDICATORS::nTall.Value * 3 + 2, { 255, 255, 255, 255 }, align, "G::AnticipatedChoke %i", G::AnticipatedChoke);
	}
}

void CVisuals::ProjectileTrace()
{
	if (!Vars::Visuals::ProjectileTrajectory.Value)
		return;

	const auto& pLocal = g_EntityCache.GetLocal();
	const auto& pWeapon = g_EntityCache.GetWeapon();
	if (!pLocal || !pWeapon)
		return;

	ProjectileInfo projInfo = {};
	if (!F::ProjSim.GetInfo(pLocal, pWeapon, I::EngineClient->GetViewAngles(), projInfo, true, Vars::Aimbot::Projectile::AutoRelease.Value ? Vars::Aimbot::Projectile::AutoReleaseAt.Value : -1.f))
		return;

	if (!F::ProjSim.Initialize(projInfo))
		return;

	for (int n = 0; n < TIME_TO_TICKS(projInfo.m_lifetime); n++)
	{
		Vec3 Old = F::ProjSim.GetOrigin();
		F::ProjSim.RunTick(projInfo);
		Vec3 New = F::ProjSim.GetOrigin();

		CGameTrace trace = {};
		CTraceFilterProjectile filter = {};
		filter.pSkip = pLocal;
		Utils::TraceHull(Old, New, projInfo.m_hull * -1, projInfo.m_hull, MASK_SOLID, &filter, &trace);
		if (trace.DidHit())
		{
			Vec3 angles;
			Math::VectorAngles(trace.Plane.normal, angles);

			//const Vec3 vOrigin = trace.vEndPos - trace.Plane.normal * (projInfo.m_hull.x - 1.f);
			const float flSize = std::max(projInfo.m_hull.x, 1.f);
			const Vec3 vSize = { 1.f, flSize, flSize };

			RenderBox(trace.vEndPos, vSize * -1, vSize, angles, Vars::Colors::ProjectileColor.Value, { 0, 0, 0, 0 });
			if (Vars::Colors::ClippedColor.Value.a)
				RenderBox(trace.vEndPos, vSize * -1, vSize, angles, Vars::Colors::ClippedColor.Value, { 0, 0, 0, 0 }, true);

			projInfo.PredictionLines.push_back({ trace.vEndPos, Math::GetRotatedPosition(trace.vEndPos, Math::VelocityToAngles(F::ProjSim.GetVelocity() * Vec3(1, 1, 0)).Length2D() + 90, Vars::Visuals::SeperatorLength.Value) });

			break;
		}
	}

	DrawSimLine(projInfo.PredictionLines, Vars::Colors::ProjectileColor.Value);
	if (Vars::Colors::ClippedColor.Value.a)
		DrawSimLine(projInfo.PredictionLines, Vars::Colors::ClippedColor.Value, false, true);
}

void CVisuals::DrawAntiAim(CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive() || pLocal->IsAGhost() || !I::Input->CAM_IsThirdPerson())
		return;

	if (Vars::AntiHack::AntiAim::Active.Value && Vars::Debug::AntiAimLines.Value)
	{
		static constexpr Color_t realColour = { 0, 255, 0, 255 };
		static constexpr Color_t fakeColour = { 255, 0, 0, 255 };

		const auto& vOrigin = pLocal->GetAbsOrigin();

		Vec3 vScreen1, vScreen2;
		if (Utils::W2S(vOrigin, vScreen1))
		{
			constexpr auto distance = 50.f;
			if (Utils::W2S(Utils::GetRotatedPosition(vOrigin, F::AntiAim.vRealAngles.y, distance), vScreen2))
				g_Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, realColour);

			if (Utils::W2S(Utils::GetRotatedPosition(vOrigin, F::AntiAim.vFakeAngles.y, distance), vScreen2))
				g_Draw.Line(vScreen1.x, vScreen1.y, vScreen2.x, vScreen2.y, fakeColour);
		}
	}
}

void CVisuals::DrawDebugInfo(CBaseEntity* pLocal)
{
	// Debug info
	if (Vars::Debug::Info.Value)
	{
		int yoffset = 10, xoffset = 10;
		const auto& fFont = g_Draw.GetFont(FONT_MENU);

		g_Draw.String(fFont, xoffset, yoffset += 15, Utils::Rainbow(), ALIGN_DEFAULT, "Fedoraware");
		g_Draw.String(fFont, xoffset, yoffset += 15, { 119, 255, 225, 255 }, ALIGN_DEFAULT, "Local Player"); // header
		// alive
		{
			const bool alive = pLocal->IsAlive();
			Color_t clr = alive ? Color_t{ 153, 232, 0, 255 } : Color_t{ 167, 0, 0, 255 };
			g_Draw.String(fFont, xoffset, yoffset += 15, clr, ALIGN_DEFAULT, "%s", alive ? "ALIVE" : "DEAD");
		}

		if (!G::LastUserCmd) { return; }
		const float flLastFwd = G::LastUserCmd->forwardmove;
		const float flLastSde = G::LastUserCmd->sidemove;
		{
			g_Draw.String(fFont, xoffset, yoffset += 15, { 255, 255, 255, 255 }, ALIGN_DEFAULT, "%.0f", flLastFwd);
			g_Draw.String(fFont, xoffset, yoffset += 15, { 255, 255, 255, 255 }, ALIGN_DEFAULT, "%.0f", flLastSde);
		}
	}
}



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

		G::BoxesStorage.push_back({ matrixOrigin, bbox->bbmin, bbox->bbmax, bboxAngle, I::GlobalVars->curtime + 5.f, Vars::Colors::HitboxEdge.Value, Vars::Colors::HitboxFace.Value });
	}
}

void CVisuals::DrawHitbox(CBaseEntity* pTarget, Vec3 vOrigin, float flTime)
{
	G::BoxesStorage.clear();

	G::BoxesStorage.push_back({ vOrigin, pTarget->GetCollideableMins(), pTarget->GetCollideableMaxs(), Vec3(), flTime, Vars::Colors::HitboxEdge.Value, Vars::Colors::HitboxFace.Value });
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

			RenderLine(Line.m_line.first, Line.m_line.second, Line.m_color);
		}
	}
	else
	{
		for (auto& Line : G::BulletsStorage)
			Line.m_flTime = -10.f;
	}
	*/
	for (auto& Line : G::BulletsStorage)
	{
		if (Line.m_flTime < I::GlobalVars->curtime) continue;

		RenderLine(Line.m_line.first, Line.m_line.second, Line.m_color);
	}
}

void CVisuals::DrawSimLine(std::deque<std::pair<Vec3, Vec3>>& Line, Color_t Color, bool bSeparators, bool bZBuffer)
{
	if (!bSeparators)
	{
		for (size_t i = 1; i < Line.size(); i++)
			RenderLine(Line.at(i - 1).first, Line.at(i).first, Color, bZBuffer);
	}
	else
	{
		for (size_t i = 1; i < Line.size(); i++)
		{
			const auto& vStart = Line[i - 1].first;
			const auto& vRotate = Line[i - 1].second;	//	splirp vec
			const auto& vEnd = Line[i].first;
			if ((i % Vars::Visuals::SeperatorSpacing.Value) == 0) { RenderLine(vStart, vRotate, Color); }
			RenderLine(vStart, vEnd, Color);
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
			Line.m_flTime = -10.f;
	}
	*/
	for (auto& Line : G::LinesStorage)
	{
		if (Line.m_flTime > 0.f && Line.m_flTime < I::GlobalVars->curtime)
			continue;

		DrawSimLine(Line.m_line, Line.m_color, Vars::Visuals::SimSeperators.Value);
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
			Box.m_flTime = -10.f;
	}
	*/
	for (auto& Box : G::BoxesStorage)
	{
		if (Box.m_flTime < I::GlobalVars->curtime) continue;

		RenderBox(Box.m_vecPos, Box.m_vecMins, Box.m_vecMaxs, Box.m_vecOrientation, Box.m_colorEdge, Box.m_colorFace);
	}
}

void CVisuals::RevealBulletLines()
{
	for (auto& Line : G::BulletsStorage)
		Line.m_flTime = I::GlobalVars->curtime + 60.f;
}

void CVisuals::RevealSimLines()
{
	for (auto& PredictionLine : G::LinesStorage)
		PredictionLine.m_flTime = I::GlobalVars->curtime + 60.f;
}

void CVisuals::RevealBoxes()
{
	for (auto& Box : G::BoxesStorage)
		Box.m_flTime = I::GlobalVars->curtime + 60.f;
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

void CVisuals::DrawServerHitboxes()
{
	static int iOldTick = I::GlobalVars->tickcount;
	if (iOldTick == I::GlobalVars->tickcount) { return; }
	iOldTick = I::GlobalVars->tickcount;
	// draw our serverside hitbox on local servers, used to test fakelag & antiaim
	if (I::Input->CAM_IsThirdPerson() && Vars::Debug::ServerHitbox.Value)
	{
		//	i have no idea what this is
		using GetServerAnimating_t = void* (*)(int);
		static auto GetServerAnimating = S::GetServerAnimating.As<GetServerAnimating_t>();

		using DrawServerHitboxes_t = void(__thiscall*)(void*, float, bool); // C_BaseAnimating, Duration, MonoColour
		static auto DrawServerHitboxes = S::DrawServerHitboxes.As<DrawServerHitboxes_t>();

		const auto pLocal = I::ClientEntityList->GetClientEntity(I::EngineClient->GetLocalPlayer());
		if (pLocal && pLocal->IsAlive())
		{
			void* server_animating = GetServerAnimating(pLocal->GetIndex());
			if (server_animating)
				DrawServerHitboxes(server_animating, I::GlobalVars->interval_per_tick, true);
		}
	}
}

void CVisuals::RenderLine(const Vec3& v1, const Vec3& v2, Color_t c, bool bZBuffer)
{
	using FN = void(__cdecl*)(const Vector&, const Vector&, Color_t, bool);
	static auto fnRenderLine = S::RenderLine.As<FN>();
	fnRenderLine(v1, v2, c, bZBuffer);
}

void CVisuals::RenderBox(const Vec3& vPos, const Vec3& vMins, const Vec3& vMaxs, const Vec3& vOrientation, Color_t cEdge, Color_t cFace, bool bZBuffer)
{
	{
		using FN = void(__cdecl*)(const Vec3&, const Vec3&, const Vec3&, const Vec3&, Color_t, bool, bool);
		static auto fnRenderBox = S::RenderBox.As<FN>();
		fnRenderBox(vPos, vOrientation, vMins, vMaxs, cFace, bZBuffer, false);
	}
	{
		using FN = void(__cdecl*)(const Vec3&, const Vec3&, const Vec3&, const Vec3&, Color_t, bool);
		static auto fnRenderBox = S::RenderWireframeBox.As<FN>();
		fnRenderBox(vPos, vOrientation, vMins, vMaxs, cEdge, bZBuffer);
	}
}



void CVisuals::FOV(CViewSetup* pView)
{
	CBaseEntity* pLocal = g_EntityCache.GetLocal();

	if (pLocal && pView)
	{
		/*
		if (pLocal->IsScoped() && !Vars::Visuals::RemoveZoom.Value)
			return;
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
		// Toggle key
		if (Vars::Visuals::ThirdPerson::Key.Value)
		{
			if (!I::EngineVGui->IsGameUIVisible() && !I::VGuiSurface->IsCursorVisible())
			{
				if (F::KeyHandler.Pressed(Vars::Visuals::ThirdPerson::Key.Value))
					Vars::Visuals::ThirdPerson::Active.Value = !Vars::Visuals::ThirdPerson::Active.Value;
			}
		}

		if (!Vars::Visuals::ThirdPerson::Active.Value
			|| ((!Vars::Visuals::RemoveScope.Value || Vars::Visuals::ZoomFieldOfView.Value < 70/*!Vars::Visuals::RemoveZoom.Value*/) && pLocal->IsScoped())
			|| !pLocal || pLocal->IsTaunting() || !pLocal->IsAlive() || pLocal->IsAGhost())
		{
			if (I::Input->CAM_IsThirdPerson())
				I::EngineClient->ClientCmd_Unrestricted("firstperson");
		}
		else
		{
			if (!I::Input->CAM_IsThirdPerson())
			{
				//lazy
				ConVar* sv_cheats = g_ConVars.FindVar("sv_cheats");
				sv_cheats->m_Value.m_nValue = 1;
				ConVar* cam_ideallag = g_ConVars.FindVar("cam_ideallag");
				cam_ideallag->SetValue(0.f);
				
				I::EngineClient->ClientCmd_Unrestricted("thirdperson");
			}

			// Thirdperson angles
			Vec3 vAngles = { F::AntiAim.vRealAngles.x, F::AntiAim.vRealAngles.y, 0 };
			I::Prediction->SetLocalViewAngles(vAngles);
			//I::Prediction->SetLocalViewAngles(G::ViewAngles); // it's probably better to keep track of the predicted yaw manually

			// Thirdperson offset
			I::ThirdPersonManager->SetDesiredCameraOffset(Vec3{}); //would've used this but right & up offsets get reversed on trace

			const Vec3 viewangles = I::EngineClient->GetViewAngles();
			Vec3 vForward, vRight, vUp;
			Math::AngleVectors(viewangles, &vForward, &vRight, &vUp);

			const Vec3 viewDiff = pView->origin - pLocal->GetEyePosition();

			Vec3 offset;
			offset += vRight * Vars::Visuals::ThirdPerson::Right.Value;
			offset += vUp * Vars::Visuals::ThirdPerson::Up.Value;
			offset -= vForward * Vars::Visuals::ThirdPerson::Distance.Value;

			CGameTrace Trace = {}; CTraceFilterWorldAndPropsOnly Filter = {};
			Utils::TraceHull(pView->origin - viewDiff, pView->origin + offset - viewDiff, { -16.0f, -16.0f, -16.0f }, { 16.0f, 16.0f, 16.0f }, MASK_SOLID, &Filter, &Trace);

			pView->origin += offset * Trace.flFraction - viewDiff;
		}
	}
}

bool CVisuals::RemoveScope(int nPanel)
{
	if (!Vars::Visuals::RemoveScope.Value) { return false; }

	if (!m_nHudZoom && Hash::IsHudScope(I::VGuiPanel->GetName(nPanel)))
		m_nHudZoom = nPanel;

	return (nPanel == m_nHudZoom);
}

void CVisuals::DrawSightlines()
{
	if (Vars::Visuals::SniperSightlines.Value)
	{
		if (!m_SightLines.empty())
		{
			for (const auto& sightline : m_SightLines)
			{
				if (sightline.m_bDraw)
					RenderLine(sightline.m_vStart, sightline.m_vEnd, sightline.m_Color);
			}
		}
	}
}

void CVisuals::FillSightlines()
{
	if (Vars::Visuals::SniperSightlines.Value)
	{
		Vec3 vShootPos, vForward, vShootEnd;
		CTraceFilterHitscan filter{};
		CGameTrace trace{};
		m_SightLines = {}; // should get rid of residual lines

		for (const auto& pEnemy : g_EntityCache.GetGroup(EGroupType::PLAYERS_ENEMIES))
		{
			const int iEntityIndex = pEnemy->GetIndex();
			if (!pEnemy->IsAlive() ||
				pEnemy->IsAGhost() ||
				pEnemy->GetClassNum() != CLASS_SNIPER ||
				!(pEnemy->GetCond() & TFCond_Zoomed) ||
				pEnemy->GetDormant())
			{
				m_SightLines[iEntityIndex] = { Vec3(), Vec3(), Color_t(), false };
				continue;
			}
			vShootPos = pEnemy->GetAbsOrigin() + (pEnemy->IsDucking() ? Vec3(0, 0, 45) : Vec3(0, 0, 75));
			Math::AngleVectors(pEnemy->GetEyeAngles(), &vForward);
			vShootEnd = vShootPos + (vForward * 8192.f);

			filter.pSkip = pEnemy;
			Utils::Trace(vShootPos, vShootEnd, MASK_SHOT, &filter, &trace);

			m_SightLines[pEnemy->GetIndex()] = { vShootPos, trace.vEndPos, GetEntityDrawColor(pEnemy, Vars::ESP::Main::EnableTeamEnemyColors.Value), true };
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

		auto timerText = std::format("{:.1f}", 10.f - timeDiff);
		auto color = pickupData->Type ? Vars::Colors::Health.Value : Vars::Colors::Ammo.Value;

		Vec3 vScreen;
		if (Utils::W2S(pickupData->Location, vScreen))
			g_Draw.String(g_Draw.GetFont(FONT_ESP_PICKUPS), vScreen.x, vScreen.y, color, ALIGN_CENTER, timerText.c_str());

		++pickupData;
	}
}

// Credits go to reestart entirely
void CVisuals::ManualNetwork(const StartSoundParams_t& params)
{
	if (!Vars::ESP::Main::DormantSoundESP.Value)
		return;
	if (params.soundsource <= 0)
		return;

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



void CVisuals::OverrideWorldTextures() // This is 100% pasted from spook953
{
	static KeyValues* kv = nullptr;

	if (!kv) {
		kv = new KeyValues("LightmappedGeneric");
		kv->SetString("$basetexture", "vgui/white_additive");
		kv->SetString("$color2", "[0.12 0.12 0.15]");
	}

	if (Vars::Visuals::World::OverrideTextures.Value)
	{
		for (auto h = I::MaterialSystem->First(); h != I::MaterialSystem->Invalid(); h = I::MaterialSystem->Next(h))
		{
			IMaterial* pMaterial = I::MaterialSystem->Get(h);

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
	//			pMaterial->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
	//	}
	//}
	if (F::Visuals.MaterialHandleDatas.empty())
		return;

	for (const auto& material : F::Visuals.MaterialHandleDatas)
	{
		if (material.Material)
		{
			if (material.GroupType != CVisuals::MaterialHandleData::EMatGroupType::GROUP_WORLD)
				continue;
			if (material.Material->IsErrorMaterial() || !material.Material->IsPrecached())
				continue;
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
	//		pMaterial->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
	//}
	if (F::Visuals.MaterialHandleDatas.empty())
		return;

	for (const auto& material : F::Visuals.MaterialHandleDatas)
	{
		if (material.Material)
		{
			if (material.GroupType != CVisuals::MaterialHandleData::EMatGroupType::GROUP_SKY)
				continue;
			if (material.Material->IsErrorMaterial() || !material.Material->IsPrecached())
				continue;
			material.Material->ColorModulate(Color::TOFLOAT(clr.r), Color::TOFLOAT(clr.g), Color::TOFLOAT(clr.b));
		}
	}
}

void CVisuals::ModulateWorld()
{
	const bool bScreenshot = I::EngineClient->IsTakingScreenshot() && Vars::Visuals::CleanScreenshots.Value;
	const bool bWorldModulation = Vars::Visuals::World::WorldModulation.Value && !bScreenshot;
	const bool bSkyModulation = Vars::Visuals::World::SkyModulation.Value && !bScreenshot;

	static bool bLastConnectionState = I::EngineClient->IsConnected() && I::EngineClient->IsInGame();
	const bool bCurrConnectionState = I::EngineClient->IsConnected() && I::EngineClient->IsInGame();
	const bool bUnchanged = bLastConnectionState == bCurrConnectionState;

	bool bSetChanged = false;
	// check if modulation has been switched
	{
		static auto oldW = bWorldModulation;
		const auto curW = bWorldModulation;
		static auto oldS = bSkyModulation;
		const auto curS = bSkyModulation;

		if (curS != oldS || curW != oldW)
		{
			oldW = curW;
			oldS = curS;
			bSetChanged = true;
		}
	}

	bool bColorChanged = false;
	// check if colours have been changed
	{
		static auto oldW = Vars::Colors::WorldModulation.Value;
		static auto oldS = Vars::Colors::SkyModulation.Value;
		const auto curW = Vars::Colors::WorldModulation.Value;
		const auto curS = Vars::Colors::SkyModulation.Value;

		if (curW.r != oldW.r || curW.g != oldW.g || curW.b != oldW.b || curS.r != oldS.r || curS.g != oldS.g || curS.b != oldS.b)
		{
			oldW = curW;
			oldS = curS;
			bColorChanged = true;
		}
	}

	if (bSetChanged || bColorChanged || !bUnchanged)
	{
		bWorldModulation ? ApplyModulation(Vars::Colors::WorldModulation.Value) : ApplyModulation({ 255, 255, 255, 255 });
		bSkyModulation ? ApplySkyboxModulation(Vars::Colors::SkyModulation.Value) : ApplySkyboxModulation({ 255, 255, 255, 255 });
		bLastConnectionState = bCurrConnectionState;
	}
}

void CVisuals::RestoreWorldModulation() // keep this because its mentioned in @DLLMain.cpp if you find a better way to do this, remove it ig.
{
	ApplyModulation({ 255, 255, 255, 255 });
	ApplySkyboxModulation({ 255, 255, 255, 255 });
}

void CVisuals::SkyboxChanger()
{
	using LoadNamedSkysFn = bool(_cdecl*)(const char*);
	static auto fnLoadSkys = S::LoadSkys.As<LoadNamedSkysFn>();

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

	if (Vars::Visuals::World::SkyboxChanger.Value)
	{
		if (Vars::Visuals::World::SkyboxNum.Value == 0)
		{
			if (Vars::Misc::BypassPure.Value)
				fnLoadSkys(Vars::Visuals::World::SkyboxName.Value.c_str());
			else
				fnLoadSkys(I::Cvar->FindVar("sv_skyname")->GetString());
		}
		else
			fnLoadSkys(skybNames[Vars::Visuals::World::SkyboxNum.Value]);
	}
	else
		fnLoadSkys(I::Cvar->FindVar("sv_skyname")->GetString());
}

void CVisuals::StoreMaterialHandles()
{
	for (MaterialHandle_t h = I::MaterialSystem->First(); h != I::MaterialSystem->Invalid(); h = I::MaterialSystem->Next(h))
	{
		if (const auto& pMaterial = I::MaterialSystem->Get(h))
		{
			if (pMaterial->IsErrorMaterial() || !pMaterial->IsPrecached())
				continue;

			MaterialHandleData data;
			data.Handle = h;
			data.Material = pMaterial;
			auto sGroup = std::string_view(pMaterial->GetTextureGroupName());
			data.Group = sGroup;
			auto sName = std::string_view(pMaterial->GetName());
			data.Name = sName;

			if (sGroup._Starts_with("SkyBox"))
				data.GroupType = MaterialHandleData::EMatGroupType::GROUP_SKY;
			else if (sGroup._Starts_with("World"))
				data.GroupType = MaterialHandleData::EMatGroupType::GROUP_WORLD;
			else
				data.GroupType = MaterialHandleData::EMatGroupType::GROUP_OTHER;

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
				data.ShouldOverrideTextures = true;

			MaterialHandleDatas.push_back(data);
		}
	}
}

void CVisuals::ClearMaterialHandles()
{
	MaterialHandleDatas.clear();
}