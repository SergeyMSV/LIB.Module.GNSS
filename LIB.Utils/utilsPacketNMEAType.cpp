#include "utilsPacketNMEAType.h"

namespace utils
{
	namespace packet_NMEA
	{
		namespace Type
		{
///////////////////////////////////////////////////////////////////////////////////////////////////
tGNSS::tGNSS(const std::string& val)
{
	if (val.size() >= 2 && val[0] == 'G')
	{
		switch (val[1])
		{
		case 'P': Value = tGNSS_State::GPS; break;
		case 'L': Value = tGNSS_State::GLONASS; break;
		case 'N': Value = tGNSS_State::GPS_GLONASS; break;
		}
	}
}

std::string tGNSS::ToString() const
{
	switch (Value)
	{
	case tGNSS_State::GPS: return "GP";
	case tGNSS_State::GLONASS: return "GL";
	case tGNSS_State::GPS_GLONASS: return "GN";
	default: return "--";
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
tValid::tValid(const std::string& val)
{
	if (val.size() == 1)
	{
		m_Empty = false;

		Value = val[0] == 'A' ? true : false;
	}
}

std::string tValid::ToString() const
{
	if (m_Empty) return "";

	return Value ? "A" : "V";
}
///////////////////////////////////////////////////////////////////////////////////////////////////
tDate::tDate(std::uint8_t year, std::uint8_t month, std::uint8_t day)
	:tEmptyAble(false), Year(year), Month(month), Day(day)
{

}

tDate::tDate(const std::string& val)
{
	if (val.size() == 6)//260216
	{
		m_Empty = false;

		char Data[3]{};//C++11

		Data[0] = val[0];
		Data[1] = val[1];

		Day = static_cast<std::uint8_t>(std::strtoul(Data, 0, 10));

		Data[0] = val[2];
		Data[1] = val[3];

		Month = static_cast<std::uint8_t>(std::strtoul(Data, 0, 10));

		Data[0] = val[4];
		Data[1] = val[5];

		Year = static_cast<std::uint8_t>(std::strtoul(Data, 0, 10));
	}
}

std::string tDate::ToString() const
{
	if (m_Empty) return "";

	std::stringstream SStream;
	
	if (Year < 100 && Month < 13 && Day < 32)
	{
		SStream << std::setfill('0');
		SStream << std::setw(2) << static_cast<unsigned int>(Day);
		SStream << std::setw(2) << static_cast<unsigned int>(Month);
		SStream << std::setw(2) << static_cast<unsigned int>(Year);
	}

	return SStream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
tModeIndicator::tModeIndicator(const std::string& val)
{
	if (val.size() == 1 && val[0] >= 'A' && val[0] <= 'Z')
		Value = val[0];
}

std::string tModeIndicator::ToString() const
{
	switch (Value)
	{
	case 'A': return "Autonomous";
	case 'D': return "Differential";
	case 'E': return "Estimated";
	case 'M': return "Manual input";
	case 'S': return "Simulator";
	case 'N': return "Not valid";
	}
	return "Error";
}
///////////////////////////////////////////////////////////////////////////////////////////////////
tSatellite::tSatellite(std::uint8_t id, std::uint8_t elevation, std::uint16_t azimuth, std::uint8_t snr)
	:ID(id), Elevation(elevation), Azimuth(azimuth), SNR(snr)
{

}

tSatellite::tSatellite(const std::string& valID, const std::string& valElevation, const std::string& valAzimuth, const std::string& valSNR)
{
	ID = id_type(valID);
	Elevation = elevation_type(valElevation);
	Azimuth = azimuth_type(valAzimuth);
	SNR = snr_type(valSNR);
}

std::string tSatellite::ToStringID() const
{
	return ID.ToString();
}

std::string tSatellite::ToStringElevation() const
{
	return Elevation.ToString();
}

std::string tSatellite::ToStringAzimuth() const
{
	return Azimuth.ToString();
}

std::string tSatellite::ToStringSNR() const
{
	return SNR.ToString();
}

std::string tSatellite::ToString() const
{
	return ToStringID() + ',' + ToStringElevation() + ',' + ToStringAzimuth() + ',' + ToStringSNR();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}
}
