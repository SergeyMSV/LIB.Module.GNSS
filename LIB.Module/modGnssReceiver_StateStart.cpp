#include "modGnssReceiver.h"

namespace mod
{

tGnssReceiver::tStateStart::tStateStart(tGnssReceiver* obj, const std::string& value)
	:tState(obj, "StateStart")
{
	std::stringstream SStr;
	SStr << "tStateStart: " << value;
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, SStr.str());

	if (m_pObj->IsControlRestart())
	{
		m_pObj->m_Control_Restart = false;
	}
}

void tGnssReceiver::tStateStart::OnTaskScriptDone()
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightYellow, "OnTaskScriptDone");

	if (m_NextState_Stop)
	{
		ChangeState(new tStateStop(m_pObj, "start single"));
		return;
	}

	ChangeState(new tStateOperation(m_pObj));
	return;
}

void tGnssReceiver::tStateStart::OnTaskScriptFailed(const std::string& msg)
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightYellow, "OnTaskScriptFailed: " + msg);

	ChangeState(new tStateError(m_pObj, "start"));
	return;
}

}