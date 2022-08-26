///////////////////////////////////////////////////////////////////////////////////////////////////
// devDataSet.h
// 2020-03-05
// Standard ISO/IEC 114882, C++17
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <mutex>
#include <string>

namespace dev
{

struct DataSetGNSSData
{
	double Latitude = 0;
	double Longitude = 0;
	double Altitude = 0;
	double Speed = 0;
	double Course = 0;

	DataSetGNSSData() = default;
	DataSetGNSSData(double latitude, double longitude, double altitude, double speed, double course)
	{
		Latitude = latitude;
		Longitude = longitude;
		Altitude = altitude;
		Speed = speed;
		Course = course;
	}

	bool operator == (const DataSetGNSSData&) const = default;
	bool operator != (const DataSetGNSSData&) const = default;
};

class tDataSet
{
	mutable std::mutex m_GNSSMtx;
	mutable bool m_GNSSChanged = false;
	DataSetGNSSData m_GNSSData;

	mutable std::mutex m_Mtx;
	mutable bool m_Changed = false;
	std::string m_DataValue1;

public:
	bool Changed();

	DataSetGNSSData GetGNSSData() const;
	void SetGNSSData(const DataSetGNSSData& value);

	std::string GetDataValue1() const;
	void SetDataValue1(const std::string& value);
};

}
