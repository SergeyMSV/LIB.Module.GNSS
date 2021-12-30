///////////////////////////////////////////////////////////////////////////////////////////////////
// devShared.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 03 05  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <mutex>
#include <string>//[TBD]TEST

namespace dev
{

//typedef tGNSS_State utils::

struct DataSetGNSSData
{
	//date time
	double Latitude = 0;
	double Longitude = 0;
	double Altitude = 0;
	double Speed = 0;
	double Course = 0;

	DataSetGNSSData(double latitude, double longitude, double altitude, double speed, double course)
	{
		Latitude = latitude;
		Longitude = longitude;
		Altitude = altitude;
		Speed = speed;
		Course = course;
	}

	bool operator == (const DataSetGNSSData& value)
	{
		return
			Latitude == value.Latitude &&
			Longitude == value.Longitude &&
			Altitude == value.Altitude &&
			Speed == value.Speed &&
			Course == value.Course;
	}

	bool operator != (const DataSetGNSSData& value)
	{
		return !operator==(value);
	}
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

//extern tDataSet g_DataSet;

}
