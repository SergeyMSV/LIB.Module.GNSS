#include "modFiniteStateMachine.h"

namespace mod
{

tFiniteStateMachine::tStateStop::tStateStop(tObjectState* obj, const std::string& value)
	:tState(obj)
{
	GetObject<tFiniteStateMachine>()->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateStop: %s", value.c_str());

	GetObject<tFiniteStateMachine>()->m_pDataSet->SetDataValue1("tState-Stop");
}

void tFiniteStateMachine::tStateStop::operator()()
{
	auto TimeStart = std::chrono::high_resolution_clock::now();

	GetObject<tFiniteStateMachine>()->m_pLog->Write(false, utils::tLogColour::Yellow, "[");

	while (true)//Step 1
	{
		//do some work...
		{
			GetObject<tFiniteStateMachine>()->m_pLog->Write(false, utils::tLogColour::LightYellow, ".");
		}

		auto TimeNow = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double, std::milli> TimeSpan = TimeNow - TimeStart;

		if (TimeSpan.count() > 100)
		{
			//Exit with ERROR...
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	GetObject<tFiniteStateMachine>()->m_pLog->Write(false, utils::tLogColour::Yellow, "]");

	if (++m_Counter > 10)
	{
		GetObject<tFiniteStateMachine>()->m_pLog->WriteLine();

		ChangeState(new tStateHalt(GetObject<utils::pattern_State::tObjectState>(), "eyeyeyeyey"));
		return;
	}

	GetObject<tFiniteStateMachine>()->m_pLog->Write(false, utils::tLogColour::LightRed, "h");
}

}