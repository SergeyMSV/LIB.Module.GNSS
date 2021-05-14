#include "modDuperStateMachine.h"

namespace mod
{

tFiniteStateMachine::tStateHalt::tStateHalt(tObjectState* obj, const std::string& value)
	:tState(obj)
{
	GetObject<tFiniteStateMachine>()->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateHalt: %s", value.c_str());

	GetObject<tFiniteStateMachine>()->m_pDataSet->SetDataValue1("tState-Halt");
}

}
