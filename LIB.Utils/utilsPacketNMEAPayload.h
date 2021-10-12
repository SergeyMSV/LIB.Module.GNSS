///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsPacketNMEAPayload.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 01 31  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "utilsBase.h"
#include "utilsPacketNMEA.h"
#include "utilsPacketNMEAType.h"

namespace utils
{
	namespace packet_NMEA
	{
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
	std::size_t FieldQty,
	int TimeSizeFract,
	int LatitudeSizeFract,
	int LongitudeSizeFract,
	int AltitudeIntSize,
	int AltitudeFactSize,
	int GeoidalSeparationIntSize,
	int GeoidalSeparationFactSize
>
struct tPayloadGGA
{
	typedef Type::tGNSS gnss_type;
	typedef Type::tTime<TimeSizeFract> time_type;
	typedef Type::tLatitude<LatitudeSizeFract> latitude_type;
	typedef Type::tLongitude<LongitudeSizeFract> longitude_type;
	typedef Type::tFloatUnit<AltitudeIntSize, AltitudeFactSize> altitude_type;
	typedef Type::tFloatUnit<GeoidalSeparationIntSize, GeoidalSeparationFactSize> geoidal_separation_type;

	gnss_type GNSS;
	time_type Time;
	latitude_type Latitude;
	longitude_type Longitude;
	altitude_type Altitude;
	geoidal_separation_type GeoidalSeparation;

	tPayloadGGA() = default;
	explicit tPayloadGGA(const tPayloadCommon::value_type& val)
	{
		if(Try(val))
		{
			GNSS = gnss_type(val[0]);
			Time = time_type(val[1]);
			Latitude = latitude_type(val[2], val[3]);
			Longitude = longitude_type(val[4], val[5]);
			Altitude = altitude_type(val[9],val[10]);
			GeoidalSeparation = geoidal_separation_type(val[11], val[12]);
		}
	}

	static const char* GetID() { return "GGA"; }

	static bool Try(const tPayloadCommon::value_type& val)
	{
		return val.size() == FieldQty && val[0].size() > 3 && !std::strcmp(&val[0][2], GetID());
	}

	tPayloadCommon::value_type GetPayload() const
	{
		tPayloadCommon::value_type Data;

		Data.push_back(GNSS.ToString() + GetID());
		Data.push_back(Time.ToString());
		Data.push_back(Latitude.ToStringValue());
		Data.push_back(Latitude.ToStringHemisphere());
		Data.push_back(Longitude.ToStringValue());
		Data.push_back(Longitude.ToStringHemisphere());
		Data.push_back("");
		Data.push_back("");
		Data.push_back("");
		Data.push_back(Altitude.ToStringValue());
		Data.push_back(Altitude.ToStringUnit());
		Data.push_back(GeoidalSeparation.ToStringValue());
		Data.push_back(GeoidalSeparation.ToStringUnit());
		Data.push_back("");
		Data.push_back("");

		return Data;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tPayloadGSV
{
	typedef Type::tGNSS gnss_type;
	typedef Type::tUInt<std::uint8_t, 1> counter_type;
	typedef Type::tUInt<std::uint8_t, 2> satellite_counter_type;
	typedef Type::tSatellite satellite_type;
	typedef std::vector<satellite_type> satellite_collection_type;

	gnss_type GNSS;
	counter_type MsgQty;
	counter_type MsgNum;
	satellite_counter_type SatelliteQty;
	satellite_collection_type Satellite;

	tPayloadGSV() = default;
	explicit tPayloadGSV(const tPayloadCommon::value_type& val)
	{
		if (Try(val))
		{
			GNSS = gnss_type(val[0]);
			MsgQty = counter_type(val[1]);
			MsgNum = counter_type(val[2]);
			SatelliteQty = satellite_counter_type(val[3]);

			int SatQty = static_cast<int>((val.size() - 3) / 4);

			for (int i = 1; i <= SatQty; ++i)
			{
				int Index = 4 * i;
				Satellite.push_back(satellite_type(val[Index], val[Index + 1], val[Index + 2], val[Index + 3]));
			}
		}
	}

	static const char* GetID() { return "GSV"; }

	static bool Try(const tPayloadCommon::value_type& val)
	{
		return val.size() > 3 && val[0].size() > 3 && !std::strcmp(&val[0][2], GetID());
	}

	tPayloadCommon::value_type GetPayload() const
	{
		tPayloadCommon::value_type Data;

		Data.push_back(GNSS.ToString() + GetID());
		Data.push_back(MsgQty.ToString());
		Data.push_back(MsgNum.ToString());
		Data.push_back(SatelliteQty.ToString());

		for (auto& i : Satellite)//C++11
		{
			Data.push_back(i.ToString());
		}

		return Data;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
	std::size_t FieldQty,
	int TimeSizeFract,
	int LatitudeSizeFract,
	int LongitudeSizeFract
>
struct tPayloadRMC
{
	typedef Type::tGNSS gnss_type;
	typedef Type::tValid valid_type;
	typedef Type::tDate date_type;
	typedef Type::tTime<TimeSizeFract> time_type;
	typedef Type::tLatitude<LatitudeSizeFract> latitude_type;
	typedef Type::tLongitude<LongitudeSizeFract> longitude_type;
	typedef Type::tFloat<0, 2> speed_type;
	typedef Type::tFloat<0, 2> course_type;
	typedef Type::tPositioning positioning_type;

	gnss_type GNSS;
	valid_type Valid;
	date_type Date;
	time_type Time;
	latitude_type Latitude;
	longitude_type Longitude;
	speed_type Speed;
	course_type Course;
	positioning_type Positioning;

	tPayloadRMC() = default;
	explicit tPayloadRMC(const tPayloadCommon::value_type& val)
	{
		if (Try(val))
		{
			GNSS = gnss_type(val[0]);
			Time = time_type(val[1]);
			Valid = valid_type(val[2]);
			Latitude = latitude_type(val[3], val[4]);
			Longitude = longitude_type(val[5], val[6]);
			Speed = speed_type(val[7]);
			Course = course_type(val[8]);
			Date = date_type(val[9]);
			Positioning = positioning_type(val[12]);
		}
	}

	static const char* GetID() { return "RMC"; }

	static bool Try(const tPayloadCommon::value_type& val)
	{
		return val.size() == FieldQty && val[0].size() > 3 && !std::strcmp(&val[0][2], GetID());
	}

	tPayloadCommon::value_type GetPayload() const
	{
		tPayloadCommon::value_type Data;

		Data.push_back(GNSS.ToString() + GetID());
		Data.push_back(Time.ToString());
		Data.push_back(Valid.ToString());
		Data.push_back(Latitude.ToStringValue());
		Data.push_back(Latitude.ToStringHemisphere());
		Data.push_back(Longitude.ToStringValue());
		Data.push_back(Longitude.ToStringHemisphere());
		Data.push_back(Speed.ToString());
		Data.push_back(Course.ToString());
		Data.push_back(Date.ToString());
		Data.push_back("");
		Data.push_back("");
		Data.push_back(Positioning.ToString());

		return Data;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
	}
}
