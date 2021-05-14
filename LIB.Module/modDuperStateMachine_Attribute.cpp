#include "modFiniteStateMachine_Attribute.h"

namespace mod
{

bool tFiniteStateMachineDataSet::Changed()
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	if (m_Changed)
	{
		m_Changed = false;
		return true;
	}

	return false;
}

std::string tFiniteStateMachineDataSet::GetDataValue1() const
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	return m_DataValue1;
}

void tFiniteStateMachineDataSet::SetDataValue1(const std::string& value)
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	if (!m_Changed && m_DataValue1 != value)
	{
		m_Changed = true;
	}

	m_DataValue1 = value;
}

}