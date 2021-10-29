#include "modGnssReceiver.h"

namespace mod
{

tGnssReceiver::tStateStop::tStateStop(tGnssReceiver* obj, const std::string& value)
	:tState(obj, "StateStop")
{
	std::stringstream SStr;
	SStr << "tStateStop: " << value;
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, SStr.str());
}

void tGnssReceiver::tStateStop::OnTaskScriptDone()
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightYellow, "OnTaskScriptDone");

	ChangeState(new tStateHalt(m_pObj, "stop"));
	return;
}

void tGnssReceiver::tStateStop::OnTaskScriptFailed(const std::string& msg)
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightYellow, "OnTaskScriptFailed: " + msg);

	ChangeState(new tStateError(m_pObj, "stop"));
	return;
}

}