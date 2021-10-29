#include "modGnssReceiver.h"

namespace mod
{

tGnssReceiver::tStateError::tStateError(tGnssReceiver* obj, const std::string& value)
	:tState(obj, "StateError")
{
	std::stringstream SStr;
	SStr << "tStateError: " << value;
	m_pObj->m_LastErrorMsg = SStr.str();
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, m_pObj->m_LastErrorMsg);

	m_pObj->m_Control_Operation = false;
}

bool tGnssReceiver::tStateError::OnCmdFailed()
{
	return tState::OnCmdDone();
}

void tGnssReceiver::tStateError::OnTaskScriptDone()
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightYellow, "OnTaskScriptDone");

	ChangeState(new tStateHalt(m_pObj, "error", true));
	return;
}

void tGnssReceiver::tStateError::OnTaskScriptFailed(const std::string& msg)
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightYellow, "OnTaskScriptFailed: " + msg);

	ChangeState(new tStateHalt(m_pObj, "error", true));
	return;
}

}
