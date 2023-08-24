#include "modGnssReceiver.h"

namespace mod
{

tGnssReceiver::tState::tState(tGnssReceiver* obj)
	:m_pObj(obj)
{
	m_pObj->ClearReceivedData();
}

tGnssReceiver::tState::tState(tGnssReceiver* obj, const std::string& taskScriptID)
	:tState(obj)
{
	m_TaskScript = m_pObj->GetTaskScript(taskScriptID, false);
}

tGnssReceiver::tState::~tState()
{
	delete m_pCmd;
}

bool tGnssReceiver::tState::operator()()
{
	if (m_pCmd && (*m_pCmd)())
		return true;

	if (m_pObj->IsReceivedData())
	{
		utils::tVectorUInt8 DataChunk = m_pObj->GetReceivedDataChunk();

		if (DataChunk.size() > 0)
		{
			//m_pObj->m_pLog->WriteHex(true, utils::tLogColour::LightRed, "Received", DataChunk);

			m_ReceivedData.insert(m_ReceivedData.end(), DataChunk.cbegin(), DataChunk.cend());

			m_ReceivedData_Parsed = false;
		}		
	}

	if (!m_ReceivedData_Parsed)
	{
		tPacketNMEA_Template Packet;

		std::size_t PacketSize = tPacketNMEA_Template::Find(m_ReceivedData, Packet);

		//if something is parsed it's needed to try to parse the rest data because there can be one more packet
		m_ReceivedData_Parsed = PacketSize == 0;

		if (PacketSize)
		{
			if (m_pCmd && m_pCmd->OnReceived(Packet))//ChangeState
				return true;

			if (OnReceived(Packet))//ChangeState
				return true;
		}
	}
	else
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	TaskScript();

	return Go();//ChangeState
}

bool tGnssReceiver::tState::Halt()
{
	ChangeState(new tStateStop(m_pObj, "halt"));
	return true;
}

void tGnssReceiver::tState::TaskScript()
{
	if (m_pCmd == nullptr && !m_TaskScript.empty())
	{
		tGnssTaskScriptCmd* Ptr = m_TaskScript.front().get();

		if (Ptr != nullptr)
		{
			switch (Ptr->GetID())
			{
			case tGnssTaskScriptCmd::tID::GPI:
			{
				m_pCmd = new tCmdGPI(this, std::move(m_TaskScript.front()));
				break;
			}
			case tGnssTaskScriptCmd::tID::GPO:
			{
				m_pCmd = new tCmdGPO(this, std::move(m_TaskScript.front()));
				break;
			}
			case tGnssTaskScriptCmd::tID::REQ:
			{
				m_pCmd = new tCmdREQ(this, std::move(m_TaskScript.front()));
				break;
			}
			default://ERROR
			{
				m_pObj->m_pLog->WriteLine(true, utils::tLogColour::LightRed, "ERR: unknown task");

				THROW_INVALID_ARGUMENT("Unknown task");
				//[TBD] throw an exception or do nothing

				break;
			}
			}

			m_TaskScript.pop_front();
		}
	}
}

bool tGnssReceiver::tState::OnCmdDone()
{
	ResetCmd();

	if (m_TaskScript.empty())
	{
		OnTaskScriptDone();
		return true;
	}
	
	return true;
}

void tGnssReceiver::tState::OnCmdTaskScript(std::unique_ptr<tGnssTaskScriptCmd> cmd, const std::string& taskScriptID)
{
	m_TaskScript.push_front(std::move(cmd));

	if (m_OnCmdTaskScriptIDLast != taskScriptID)
	{
		m_OnCmdTaskScriptIDLast = taskScriptID;

		tGnssTaskScript Script = m_pObj->GetTaskScript(taskScriptID, false);

		for (tGnssTaskScript::reverse_iterator i = Script.rbegin(); i != Script.rend(); ++i)
		{
			m_TaskScript.push_front(std::move(*i));
		}
	}
	else
	{
		OnTaskScriptFailed(m_OnCmdTaskScriptIDLast);//ChangeState
		return;
	}
}

void tGnssReceiver::tState::ResetCmd()
{
	tCmd* Cmd = m_pCmd;

	m_pCmd = nullptr;

	delete Cmd;
}

bool tGnssReceiver::tState::SetTaskScript(const std::string& taskScriptID, bool userTaskScript)
{
	tGnssTaskScript Script = m_pObj->GetTaskScript(taskScriptID, userTaskScript);

	if (Script.empty())
	{
		m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightRed, "SetTaskScript - NO SCRIPT LOADED: " + taskScriptID);
		return false;
	}

	for (auto& i : Script)
	{
		m_TaskScript.push_back(std::move(i));
	}

	return true;
}

bool tGnssReceiver::tState::OnCmdFailed()
{
	ChangeState(new tStateError(m_pObj, "OnCmdFailed"));
	return true;
}

bool tGnssReceiver::tState::OnReceived(const tPacketNMEA_Template& value)
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightYellow, "OnReceived: " + value.GetPayloadValue());
	return false;
}

}
