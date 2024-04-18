#include "../Hooks.h"

MAKE_HOOK(C_BasePlayer_CalcViewModelView, S::CBasePlayer_CalcViewModelView(), void, __fastcall,
	void* ecx, void* edx, CBaseEntity* pOwner, const Vec3& vEyePosition, Vec3& vEyeAngles)
{
	if (Vars::Visuals::UI::CleanScreenshots.Value && I::EngineClient->IsTakingScreenshot())
		return Hook.Original<FN>()(ecx, edx, pOwner, vEyePosition, vEyeAngles);

	bool bFlip = false;
	{
		static auto cl_flipviewmodels = g_ConVars.FindVar("cl_flipviewmodels");
		auto pWeapon = g_EntityCache.GetWeapon();
		if (cl_flipviewmodels ? cl_flipviewmodels->GetBool() : false)
			bFlip = !bFlip;
		if (pWeapon && pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)
			bFlip = !bFlip;
	}

	auto pLocal = g_EntityCache.GetLocal();
	if (Vars::Visuals::Viewmodel::ViewmodelAim.Value && pLocal && pLocal->IsAlive())
	{
		static Vec3 vAng = {};
		static int iTick = 0;

		if (!G::AimPos.IsZero())
		{
			vAng = Math::CalcAngle(vEyePosition, G::AimPos);
			iTick = I::GlobalVars->tickcount;
		}

		if (abs(iTick - I::GlobalVars->tickcount) < 32)
		{
			Vec3 vDiff = I::EngineClient->GetViewAngles() - vAng;
			if (bFlip)
				vDiff.y *= -1;
			vEyeAngles = I::EngineClient->GetViewAngles() - vDiff;
		}
	}

	Vec3 vNewEyePosition = vEyePosition;
	Vec3 vOffset = { float(Vars::Visuals::Viewmodel::OffsetX.Value), float(Vars::Visuals::Viewmodel::OffsetY.Value), float(Vars::Visuals::Viewmodel::OffsetZ.Value) };
	if (!vOffset.IsZero())
	{
		Vec3 vForward = {}, vRight = {}, vUp = {};
		Math::AngleVectors(vEyeAngles, &vForward, &vRight, &vUp);
		vNewEyePosition += (vRight * vOffset.x * (bFlip ? -1 : 1)) + (vForward * vOffset.y) + (vUp * vOffset.z);
	}
	if (Vars::Visuals::Viewmodel::Roll.Value)
		vEyeAngles.z += Vars::Visuals::Viewmodel::Roll.Value * (bFlip ? -1 : 1);

	Hook.Original<FN>()(ecx, edx, pOwner, vNewEyePosition, vEyeAngles);
}