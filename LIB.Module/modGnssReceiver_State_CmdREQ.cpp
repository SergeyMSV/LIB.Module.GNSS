#include "modGnssReceiver.h"

namespace mod
{

tGnssReceiver::tState::tCmdREQ::tCmdREQ(tState* obj, std::unique_ptr<tGnssTaskScriptCmd> cmd)
	:tCmd(obj, std::move(cmd))
{

}

bool tGnssReceiver::tState::tCmdREQ::operator()()
{
	switch (m_Step)
	{
	case tStep::SendMsg:
	{
		tGnssTaskScriptCmdREQ* Ptr = static_cast<tGnssTaskScriptCmdREQ*>(m_Cmd.get());

		m_StartTime = tClock::now();

		std::string Msg = Ptr->Msg;

		if (!Msg.empty())
		{
			tPacketNMEA_Template Packet(Msg);

			m_pObjState->m_pObj->Board_Send(Packet.ToVector());
		}

		m_WaitTime_us = Ptr->RspWait_us;

		m_Step = tStep::WaitRsp;

		break;
	}
	case tStep::WaitRsp:
	{
		auto Time_us = std::chrono::duration_cast<std::chrono::microseconds>(tClock::now() - m_StartTime).count();//C++11

		if (Time_us > m_WaitTime_us)
		{
			return m_pObjState->OnCmdFailed();//ChangeState
		}
		break;
	}
	case tStep::PauseSet:
	{
		tGnssTaskScriptCmdREQ* Ptr = static_cast<tGnssTaskScriptCmdREQ*>(m_Cmd.get());

		m_StartTime = tClock::now();

		m_WaitTime_us = Ptr->Pause_us;

		m_Step = tStep::PauseWait;

		break;
	}
	case tStep::PauseWait:
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

bool tGnssReceiver::tState::tCmdREQ::OnReceived(const tPacketNMEA_Template& value)
{
	if (m_Step == tStep::WaitRsp)
	{
		tGnssTaskScriptCmdREQ* Ptr = static_cast<tGnssTaskScriptCmdREQ*>(m_Cmd.get());

		if (value.GetPayload().find(Ptr->RspHead) == 0)
		{
			////
			{//[TEST]
				auto Time_us = std::chrono::duration_cast<std::chrono::microseconds>(tClock::now() - m_StartTime).count();//C++11
				std::stringstream StrTime;
				StrTime << value.GetPayload() << " --- " << Time_us << " us";
				m_pObjState->m_pObj->m_pLog->WriteLine(true, utils::tLogColour::LightYellow, StrTime.str());
			}
			////

			if (value.GetPayload() == Ptr->RspHead + Ptr->RspBody || Ptr->CaseRspWrong.empty())
			{
				m_Step = tStep::PauseSet;
			}
			else if (!Ptr->CaseRspWrong.empty())
			{
				m_pObjState->OnCmdTaskScript(std::move(m_Cmd), Ptr->CaseRspWrong);

				return m_pObjState->OnCmdDone();//ChangeState - next cmd
			}
		}

		return true;//that doesn't mean ChangeState, it means that the message has been handled
	}

	return false;
}

}
