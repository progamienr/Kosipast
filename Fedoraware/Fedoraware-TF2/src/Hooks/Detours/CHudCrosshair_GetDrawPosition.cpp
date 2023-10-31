#include "../Hooks.h"

MAKE_HOOK(CHudCrosshair_GetDrawPosition, S::CHudCrosshair_GetDrawPosition(), void, __cdecl,
	float* pX, float* pY, bool* pbBehindCamera, Vec3 angleCrosshairOffset)
{
	if (I::EngineClient->IsTakingScreenshot() && Vars::Visuals::CleanScreenshots.Value)
		return Hook.Original<FN>()(pX, pY, pbBehindCamera, angleCrosshairOffset);

	bool bSet = false;

	if (const auto& pLocal = g_EntityCache.GetLocal();
		Vars::Visuals::ThirdPerson::Crosshair.Value &&
		I::Input->CAM_IsThirdPerson())
	{
		const Vec3 viewangles = I::EngineClient->GetViewAngles();
		Vec3 vForward{};
		Math::AngleVectors(viewangles, &vForward);

		const Vec3 vStartPos = pLocal->GetEyePosition();
		const Vec3 vEndPos = (vStartPos + vForward * 8192);

		CGameTrace trace = {};
		CTraceFilterHitscan filter = {};
		filter.pSkip = pLocal;
		Utils::Trace(vStartPos, vEndPos, MASK_SHOT, &filter, &trace);

		Vec3 vScreen;
		if (Utils::W2S(trace.vEndPos, vScreen))
		{
			if (pX) *pX = vScreen.x;
			if (pY) *pY = vScreen.y;
			if (pbBehindCamera) *pbBehindCamera = false;
			bSet = true;
		}
	}

	if (const auto& pLocal = g_EntityCache.GetLocal();
		Vars::Visuals::CrosshairAimPos.Value)
	{
		static int iLastEyeTick = 0;
		static Vec3 vPosDelayed;
		if (pLocal->IsAlive())
		{
			if (!G::AimPos.IsZero())
			{
				vPosDelayed = G::AimPos;
				iLastEyeTick = I::GlobalVars->tickcount;
			}

			// looks hot ty senator for the idea
			if (abs(iLastEyeTick - I::GlobalVars->tickcount) < 32)
			{
				Vec3 vScreen;
				if (Utils::W2S(vPosDelayed, vScreen))
				{
					if (pX) *pX = vScreen.x;
					if (pY) *pY = vScreen.y;
					if (pbBehindCamera) *pbBehindCamera = false;
					bSet = true;
				}
			}
		}
	}

	if (!bSet)
		Hook.Original<FN>()(pX, pY, pbBehindCamera, angleCrosshairOffset);
}