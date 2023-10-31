#include "../Hooks.h"

MAKE_HOOK(C_BasePlayer_CalcViewModelView, S::CBasePlayer_CalcViewModelView(), void, __fastcall,
	void* ecx, void* edx, CBaseEntity* pOwner, const Vec3& vEyePosition, Vec3& vEyeAngles)
{
	if (I::EngineClient->IsTakingScreenshot() && Vars::Visuals::CleanScreenshots.Value)
		return Hook.Original<FN>()(ecx, edx, pOwner, vEyePosition, vEyeAngles);

	bool bFlip = false;
	{
		ConVar* cl_flipviewmodels = g_ConVars.cl_flipviewmodels;
		CBaseCombatWeapon* pWeapon = g_EntityCache.GetWeapon();
		if (cl_flipviewmodels && pWeapon)
		{
			if (cl_flipviewmodels->GetBool())
				bFlip = !bFlip;
			if (pWeapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW)
				bFlip = !bFlip;
		}
	}

	if (Vars::Visuals::AimbotViewmodel.Value)
	{
		static int iLastEyeTick = 0;
		static Vec3 vEyeAngDelayed;
		if (const auto& pLocal = g_EntityCache.GetLocal())
		{
			if (pLocal->IsAlive())
			{
				if (!G::AimPos.IsZero())
				{
					vEyeAngDelayed = Math::CalcAngle(vEyePosition, G::AimPos);
					iLastEyeTick = I::GlobalVars->tickcount;
				}

				// looks hot ty senator for the idea
				if (abs(iLastEyeTick - I::GlobalVars->tickcount) < 32)
				{
					Vec3 vDiff = I::EngineClient->GetViewAngles() - vEyeAngDelayed;
					if (bFlip)
						vDiff.y *= -1;
					vEyeAngles = I::EngineClient->GetViewAngles() - vDiff;
				}
				else
					vEyeAngles = I::EngineClient->GetViewAngles();
			}
		}
	}

	//VM Offsets

	Vec3 vForward = {}, vRight = {}, vUp = {};
	Math::AngleVectors(vEyeAngles, &vForward, &vRight, &vUp);

	Vec3 vNewEyePosition = vEyePosition + 
		(vRight * Vars::Visuals::VMOffsets.Value.x * (bFlip ? -1 : 1)) + 
		(vForward * Vars::Visuals::VMOffsets.Value.y) + 
		(vUp * Vars::Visuals::VMOffsets.Value.z);

	vEyeAngles.z += Vars::Visuals::VMRoll.Value * (bFlip ? -1 : 1); //VM Roll

	Hook.Original<FN>()(ecx, edx, pOwner, vNewEyePosition, vEyeAngles);
}