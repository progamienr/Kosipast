#include "../Hooks.h"
#include "../../Features/TickHandler/TickHandler.h"

//	"NetMsg"
//	@net_chan.cpp L2524
MAKE_HOOK(CNetChan_SendNetMsg, S::CNetChan_SendNetMsg(), bool, __fastcall,
	CNetChannel* netChannel, void* edi, INetMessage& msg, bool bForceReliable, bool bVoice)
{
	/*
	bool retn = false, runfurther = false;
	if (Vars::NoSpread::Hitscan.Value) {
		if (F::NoSpread.SendNetMessage(&msg)) {
			bForceReliable = true;
			runfurther = true;
		}
	}
	*/

	switch (msg.GetType())
	{
		case clc_VoiceData:
		{
			// stop lag with voice chat
			bVoice = true;
			break;
		}

		case clc_FileCRCCheck:
		{
			// whitelist
			if (Vars::Misc::BypassPure.Value)
			{
				return false;
			}
			break;
		}

		case clc_RespondCvarValue:
		{
			//	causes b1g crash
			if (Vars::Visuals::RemoveConvarQueries.Value)
			{
				if (const auto respondMsg = reinterpret_cast<DWORD*>(&msg))
				{
					if (const auto cvarName = reinterpret_cast<const char*>(respondMsg[6]))
					{
						if (const auto convarC = g_ConVars.FindVar(cvarName))
						{
							if (const char* defaultValue = convarC->GetDefault())
							{
								respondMsg[7] = reinterpret_cast<DWORD>(defaultValue);
								I::Cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "%s\n", msg.ToString()); //	mt everest
								break;
							}
						}
						return true; //	if we failed to manipulate the data, don't send it.
					}
				}
			}
			break;
		}

		case clc_Move:
		{
			/*
			// Charge
			if (G::Recharge && G::ShiftedTicks < G::MaxShift)
			{
				auto moveMsg = (CLC_Move*)&msg;

				moveMsg->m_nBackupCommands = 0;
				moveMsg->m_nNewCommands = 0;
				moveMsg->m_DataOut.Reset();
				moveMsg->m_DataOut.m_nDataBits = 0;
				moveMsg->m_DataOut.m_nDataBytes = 0;
				moveMsg->m_DataOut.m_iCurBit = 0;

				G::ShiftedTicks++;
				G::WaitForShift = G::ShiftedTicks;
			}
			else
			*/
			{
				const int iAllowedNewCommands = fmax(g_ConVars.sv_maxusrcmdprocessticks->GetInt() - G::ShiftedTicks, 0); // g_ConVars.sv_maxusrcmdprocessticks->GetInt();
				const auto& moveMsg = reinterpret_cast<CLC_Move&>(msg);
				const int iCmdCount = moveMsg.m_nNewCommands + moveMsg.m_nBackupCommands - 3;
				if (iCmdCount > iAllowedNewCommands)
				{
					Utils::ConLog("clc_Move", std::format("{:d} sent <{:d} | {:d}>, max was {:d}.", iCmdCount, moveMsg.m_nNewCommands, moveMsg.m_nBackupCommands, iAllowedNewCommands).c_str(), { 0, 222, 255, 255 }, Vars::Debug::Logging.Value);
					G::ShiftedTicks -= iCmdCount - iAllowedNewCommands;
					F::Ticks.iDeficit = iCmdCount - iAllowedNewCommands;
				}
			}
			break;
		}
	}

	/*
	retn = Hook.Original<FN>()(netChannel, edi, msg, bForceReliable, bVoice);

	if (runfurther)
		F::NoSpread.SendNetMessagePost();

	return retn;
	*/

	return Hook.Original<FN>()(netChannel, edi, msg, bForceReliable, bVoice);
}