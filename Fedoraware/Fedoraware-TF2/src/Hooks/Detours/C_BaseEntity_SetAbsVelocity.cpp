#include "../Hooks.h"
#include <intrin.h>

namespace S
{
	MAKE_SIGNATURE(CBasePlayer_PostDataUpdate_SetAbsVelocityCall, CLIENT_DLL, "E8 ? ? ? ? 53 8B CF E8 ? ? ? ? 8D 47 F8 39 05", 0x5);
}

MAKE_HOOK(C_BaseEntity_SetAbsVelocity, S::CBaseEntity_SetAbsVelocity(), void, __fastcall,
	void* ecx, void* edx, const Vec3& vecAbsVelocity)
{
	static const auto dwSetAbsVelocityCall = S::CBasePlayer_PostDataUpdate_SetAbsVelocityCall();
	const auto dwRetAddr = reinterpret_cast<DWORD>(_ReturnAddress());

	if (dwRetAddr == dwSetAbsVelocityCall)
	{
		if (const auto pBasePlayer = static_cast<CBaseEntity*>(ecx))
		{
			if (G::VelFixRecords.contains(pBasePlayer))
			{
				const auto& record = G::VelFixRecords[pBasePlayer];

				const float flSimTimeDelta = pBasePlayer->m_flSimulationTime() - record.m_flSimulationTime;
				if (flSimTimeDelta > 0.f)
				{
					Vec3 vOldOrigin = record.m_vecOrigin;

					const int nCurFlags = pBasePlayer->m_fFlags();
					const int nOldFlags = record.m_nFlags;

					if (!(nCurFlags & FL_ONGROUND) && !(nOldFlags & FL_ONGROUND))
					{
						bool bCorrected = false;

						if ((nCurFlags & FL_DUCKING) && !(nOldFlags & FL_DUCKING))
						{
							vOldOrigin.z += 20.0f;
							bCorrected = true;
						}

						if (!(nCurFlags & FL_DUCKING) && (nOldFlags & FL_DUCKING))
						{
							vOldOrigin.z -= 20.0f;
							bCorrected = true;
						}

						if (bCorrected)
						{
							Vec3 vNewVelocity = vecAbsVelocity;
							vNewVelocity.z = (pBasePlayer->m_vecOrigin().z - vOldOrigin.z) / flSimTimeDelta;
							Hook.Original<FN>()(ecx, edx, vNewVelocity);
						}
					}
				}
			}
		}
	}

	Hook.Original<FN>()(ecx, edx, vecAbsVelocity);
}