#include "modGnssReceiver.h"

namespace mod
{

tGnssReceiver::tState::tCmdGPO::tCmdGPO(tState* obj, std::unique_ptr<tGnssTaskScriptCmd> cmd)
	:tCmd(obj, std::move(cmd))
{

}

bool tGnssReceiver::tState::tCmdGPO::operator()()
{
	switch (m_Step)
	{
	case tStep::SetGPO:
	{
		auto Ptr = static_cast<tGnssTaskScriptCmdGPO*>(m_Cmd.get());

		m_StartTime = tClock::now();

		if (Ptr->ID == "PWR")
		{
			m_pObjState->m_pObj->Board_PowerSupply(Ptr->State);
		}
		else if (Ptr->ID == "RST")
		{
			m_pObjState->m_pObj->Board_Reset(Ptr->State);
		}
		else
		{
			m_pObjState->m_pObj->m_pLog->WriteLine(true, utils::tLogColour::LightRed, "ERR: unknown GPO");

			//[TBD] throw an exception or do nothing
			THROW_INVALID_ARGUMENT("Unknown GPO");
		}

		m_WaitTime_us = Ptr->Pause_us;

		m_Step = tStep::Pause;

		break;
	}
	case tStep::Pause:
	{
		auto Time_us = std::chrono::duration_cast<std::chrono::microseconds>(tClock::now() - m_StartTime).count();//C++11

		if (Time_us > m_WaitTime_us)
		{
			return m_pObjState->OnCmdDone();//ChangeState
		}
		break;
	}
	}

	return false;
}

}