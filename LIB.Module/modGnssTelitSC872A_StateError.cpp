#include "modGnssTelitSC872A.h"

namespace mod
{

tGnssTelitSC872A::tStateError::tStateError(tGnssTelitSC872A* obj, const std::string& value)
	:tState(obj)
{
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateError: %s", value.c_str());

	//m_pObj->m_pDataSet->SetDataValue1("tState-Operation");
}

bool tGnssTelitSC872A::tStateError::operator()()
{
	if (++m_Counter < 10)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	else
	{
		m_pObj->m_pLog->WriteLine();

		ChangeState(new tStateStop(m_pObj, "lalala"));
		return true;
	}

	m_pObj->m_pLog->Write(false, utils::tLogColour::LightRed, "e");

	return true;
}

}
