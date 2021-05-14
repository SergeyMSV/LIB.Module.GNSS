#include "modGnssTelitSC872A.h"

namespace mod
{

tGnssTelitSC872A::tStateStop::tStateStop(tGnssTelitSC872A* obj, const std::string& value)
	:tState(obj)
{
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateStop: %s", value.c_str());

	//m_pObj->m_pDataSet->SetDataValue1("tState-Stop");
}

bool tGnssTelitSC872A::tStateStop::operator()()
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


		if (m_pObj->IsControlRestart())
		{
			ChangeState(new tStateStart(m_pObj, "restart"));
			return true;
		}
		if (m_pObj->IsControlHalt())
		{
			ChangeState(new tStateHalt(m_pObj, "stop"));
			return true;
		}
	}

	m_pObj->m_pLog->Write(false, utils::tLogColour::LightRed, "h");

	return true;
}

}