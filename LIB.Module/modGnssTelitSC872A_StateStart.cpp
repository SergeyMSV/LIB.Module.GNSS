#include "modGnssTelitSC872A.h"

#include <chrono>
#include <thread>

namespace mod
{

tGnssTelitSC872A::tStateStart::tStateStart(tGnssTelitSC872A* obj, const std::string& value)
	:tState(obj)
{
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateStart: %s", value.c_str());

	if (m_pObj->IsControlRestart())
	{
		m_pObj->m_Control_Restart = false;
	}

	m_pObj->ClearReceivedData();
}

bool tGnssTelitSC872A::tStateStart::operator()()
{
	auto TimeStart = std::chrono::high_resolution_clock::now();

	m_pObj->m_pLog->Write(false, utils::tLogColour::Yellow, "[");

	while (true)//Step 1
	{
		//do some work...
		{
			m_pObj->m_pLog->Write(false, utils::tLogColour::LightYellow, ".");
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

	m_pObj->m_pLog->Write(false, utils::tLogColour::Yellow, "]");

	if (++m_Counter > 10)
	{
		m_pObj->m_pLog->WriteLine();

		ChangeState(new tStateOperation(m_pObj));
		return true;
	}

	m_pObj->m_pLog->Write(false, utils::tLogColour::LightRed, "s");

	return true;
}

}