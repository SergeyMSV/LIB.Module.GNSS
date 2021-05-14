#include "modDuperStateMachine.h"

namespace mod
{

tFiniteStateMachine::tStateError::tStateError(tObjectState* obj, const std::string& value)
	:tState(obj)
{
	GetObject<tFiniteStateMachine>()->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateError: %s", value.c_str());

	GetObject<tFiniteStateMachine>()->m_pDataSet->SetDataValue1("tState-Operation");
}

void tFiniteStateMachine::tStateError::operator()()
{
	if (++m_Counter < 10)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	else
	{
		GetObject<tFiniteStateMachine>()->m_pLog->WriteLine();

		ChangeState(new tStateStop(GetObject<utils::pattern_State::tObjectState>(), "lalala"));
		return;
	}

	GetObject<tFiniteStateMachine>()->m_pLog->Write(false, utils::tLogColour::LightRed, "e");
}

}
