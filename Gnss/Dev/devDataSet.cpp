#include "devDataSet.h"

namespace dev
{

bool tDataSet::Changed()
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	if (m_Changed)
	{
		m_Changed = false;
		return true;
	}

	return false;
}

DataSetGNSSData tDataSet::GetGNSSData() const
{
	std::lock_guard<std::mutex> Lock(m_GNSSMtx);

	return m_GNSSData;
}

void tDataSet::SetGNSSData(const DataSetGNSSData& value)
{
	std::lock_guard<std::mutex> Lock(m_GNSSMtx);

	if (!m_GNSSChanged && m_GNSSData != value)
	{
		m_GNSSChanged = true;
	}

	m_GNSSData = value;
}

std::string tDataSet::GetDataValue1() const
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	return m_DataValue1;
}

void tDataSet::SetDataValue1(const std::string& value)
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	if (!m_Changed && m_DataValue1 != value)
	{
		m_Changed = true;
	}

	m_DataValue1 = value;
}

//tDataSet g_DataSet;

}
