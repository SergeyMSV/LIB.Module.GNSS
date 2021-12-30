///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsPacketNMEAType.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 01 27  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "utilsBase.h"

#include <cmath>
#include <cstdlib>
#include <cstring>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace utils
{
	namespace packet_NMEA
	{
		namespace Type
		{
///////////////////////////////////////////////////////////////////////////////////////////////////
enum class tGNSS_State : std::uint8_t//It's like bitfield
{
	UNKNOWN = 0,
	GPS = 1,    //0000'0001
	GLONASS,    //0000'0010
	GPS_GLONASS,//0000'0011
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tGNSS
{
	tGNSS_State Value = tGNSS_State::UNKNOWN;

	tGNSS() = default;
	explicit tGNSS(tGNSS_State val) :Value(val) {}
	explicit tGNSS(const std::string& val);

	std::string ToString() const;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tValid :public tEmptyAble
{
	bool Value = false;

	tValid() = default;
	explicit tValid(bool val) :tEmptyAble(false), Value(val) {}
	explicit tValid(const std::string& val);

	std::string ToString() const;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tDate :public tEmptyAble
{
	std::uint8_t Year = 0;
	std::uint8_t Month = 0;
	std::uint8_t Day = 0;

	tDate() = default;//C++11
	tDate(std::uint8_t year, std::uint8_t month, std::uint8_t day);
	explicit tDate(const std::string& val);	

	std::string ToString() const;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template <int SizeFract>
class tTime :public tEmptyAble
{
	static_assert(SizeFract >= 0, "tTime: SizeFract");//C++11
	static const std::size_t Size = SizeFract == 0 ? 6 : 7 + SizeFract;//sizeof(hhmmss.)=7

public:
	std::uint8_t Hour = 0;
	std::uint8_t Minute = 0;
	double Second = 0;

	tTime() = default;
	tTime(std::uint8_t hour, std::uint8_t minute, double second) :tEmptyAble(false), Hour(hour), Minute(minute), Second(second) {}
	explicit tTime(const std::string& val)
	{
		if (val.size() == Size)
		{
			m_Empty = false;

			char Data[3]{};//C++11

			Data[0] = val[0];
			Data[1] = val[1];

			Hour = static_cast<std::uint8_t>(std::strtoul(Data, 0, 10));

			Data[0] = val[2];
			Data[1] = val[3];

			Minute = static_cast<std::uint8_t>(std::strtoul(Data, 0, 10));

			Second = std::strtod(val.c_str() + 4, 0);
		}
	}

	template <int SizeFract1>
	friend std::ostream& operator<< (std::ostream& out, const tTime<SizeFract1>& value);

	std::string ToString() const
	{
		std::stringstream Stream;

		Stream << *this;

		return Stream.str();
	}

	//std::string ToString() const
	//{
	//	if (!Empty && Hour < 24 && Minute < 60 && Second < 60)
	//	{
	//		std::stringstream Stream;

	//		Stream << std::setfill('0');
	//		Stream << std::setw(2) << static_cast<int>(Hour);
	//		Stream << std::setw(2) << static_cast<int>(Minute);

	//		int SizeFract = Size - 7;//sizeof(hhmmss.)=7

	//		if (SizeFract > 0)
	//		{
	//			Stream.setf(std::ios::fixed);

	//			Stream << std::setw(2 + SizeFract + 1) << std::setprecision(SizeFract) << Second;
	//		}
	//		else
	//		{
	//			Stream << std::setw(2) << static_cast<int>(Second);
	//		}

	//		return Stream.str();
	//	}

	//	return "";
	//}
};

template <int SizeFract>
std::ostream& operator<< (std::ostream& out, const tTime<SizeFract>& value)
{
	if (!value.Empty() && value.Hour < 24 && value.Minute < 60 && value.Second < 60)
	{
		out << std::setfill('0');
		out << std::setw(2) << static_cast<int>(value.Hour);
		out << std::setw(2) << static_cast<int>(value.Minute);

		if (SizeFract > 0)
		{
			out.setf(std::ios::fixed);
			out << std::setw(3 + SizeFract) << std::setprecision(SizeFract) << value.Second;
			out.unsetf(std::ios::fixed);
		}
		else
		{
			out << std::setw(2) << static_cast<int>(value.Second);
		}
	}

	return out;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
template <std::size_t SizeFract>
class tLatitude :public tEmptyAble
{
	static const std::size_t Size = 5 + SizeFract;//sizeof(ddmm.)=5

public:
	double Value = 0;

	tLatitude() = default;
	explicit tLatitude(double val) :tEmptyAble(false), Value(val) {}
	tLatitude(const std::string& val, const std::string& valSign)
	{
		if (val.size() == Size && valSign.size() == 1)
		{
			m_Empty = false;

			char Data[3]{};

			std::strncpy(Data, val.c_str(), sizeof(Data) - 1);

			Value = std::strtod(Data, 0);

			const double Rest = std::strtod(val.c_str() + 2, 0);

			Value += Rest / 60;

			if (valSign[0] == 'S')
			{
				Value = -Value;
			}
		}
	}

	std::string ToStringValue() const
	{
		if (m_Empty) return "";

		const double ValueAbs = std::abs(Value);

		const std::int8_t Deg = static_cast<std::int8_t>(ValueAbs);
		const double Min = (ValueAbs - Deg) * 60;

		std::stringstream SStream;

		if (Deg < 100)
		{
			SStream << std::setfill('0');
			SStream << std::setw(2) << static_cast<int>(Deg);
			SStream.setf(std::ios::fixed);
			SStream << std::setw(3 + SizeFract) << std::setprecision(SizeFract) << Min;
			SStream.unsetf(std::ios::fixed);
		}

		return SStream.str();
	}

	std::string ToStringHemisphere() const
	{
		if (m_Empty) return "";

		return Value < 0 ? "S" : "N";
	}

	std::string ToString() const
	{
		return ToStringValue() + ',' + ToStringHemisphere();
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template <std::size_t SizeFract>
class tLongitude :public tEmptyAble
{
	static const std::size_t Size = 6 + SizeFract;//sizeof(dddmm.)=6

public:
	double Value = 0;

	tLongitude() = default;
	explicit tLongitude(double val) :tEmptyAble(false), Value(val) { }
	tLongitude(const std::string& val, const std::string& valSign)
	{
		if (val.size() == Size && valSign.size() == 1)
		{
			m_Empty = false;

			char Data[4]{};

			std::strncpy(Data, val.c_str(), sizeof(Data) - 1);

			Value = std::strtod(Data, 0);

			double Rest = std::strtod(val.c_str() + 3, 0);

			Value += Rest / 60;

			if (valSign[0] == 'W')
			{
				Value = -Value;
			}
		}
	}

	std::string ToStringValue() const
	{
		if (m_Empty) return "";

		const double ValueAbs = std::abs(Value);

		const std::int16_t Deg = static_cast<std::int16_t>(ValueAbs);
		const double Min = (ValueAbs - Deg) * 60;

		std::stringstream SStream;

		if (Deg < 1000)
		{
			SStream << std::setfill('0');
			SStream << std::setw(3) << static_cast<int>(Deg);
			SStream.setf(std::ios::fixed);
			SStream << std::setw(3 + SizeFract) << std::setprecision(SizeFract) << Min;
			SStream.unsetf(std::ios::fixed);
		}

		return SStream.str();
	}

	std::string ToStringHemisphere() const
	{
		if (m_Empty)
			return "";

		return Value < 0 ? "W" : "E";
	}

	std::string ToString() const
	{
		return ToStringValue() + ',' + ToStringHemisphere();
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template <std::size_t SizeInt, std::size_t SizeFract>
class tFloat :public tEmptyAble
{
	static const std::size_t Size = SizeInt + SizeFract + 1;

public:
	double Value = 0;

	tFloat() = default;
	explicit tFloat(double val) :tEmptyAble(false), Value(val) {}
	tFloat(const std::string& val)
	{
		if (val.size() == Size)
		{
			m_Empty = false;

			Value = std::strtod(val.c_str(), 0);
		}
	}

	std::string ToString() const
	{
		if (m_Empty) return "";

		std::stringstream SStream;

		SStream << std::setfill('0');
		SStream.setf(std::ios::fixed);
		SStream << std::setw(SizeInt + SizeFract + 1) << std::setprecision(SizeFract) << Value;
		SStream.unsetf(std::ios::fixed);

		return SStream.str();
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template <std::size_t SizeFract>
class tFloat<0, SizeFract> :public tEmptyAble
{
	static const std::size_t SizeInt = 6;
	static const std::size_t SizeMax = SizeInt + SizeFract + 1;

public:
	double Value = 0;

	tFloat() = default;
	explicit tFloat(double val) :tEmptyAble(false), Value(val) {}
	tFloat(const std::string& val)
	{
		if (val.size() > 0 && val.size() < SizeMax)
		{
			m_Empty = false;

			Value = std::strtod(val.c_str(), 0);
		}
	}

	std::string ToString() const
	{
		if (m_Empty) return "";

		std::stringstream SStream;

		SStream.setf(std::ios::fixed);
		SStream << std::setw(SizeFract + 1) << std::setprecision(SizeFract) << Value;
		SStream.unsetf(std::ios::fixed);

		return SStream.str();
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template <> class tFloat<0, 0>;//Fractional part is just of max length (6 chars), therefore this specialisation makes no sense.
///////////////////////////////////////////////////////////////////////////////////////////////////
template <std::size_t SizeInt> class tFloat<SizeInt, 0>;
///////////////////////////////////////////////////////////////////////////////////////////////////
template <std::size_t SizeInt, std::size_t SizeFract>
class tFloatUnit :public tEmptyAble
{
	typedef tFloat<SizeInt, SizeFract> value_type;

	static const std::size_t SizeValue = SizeInt + SizeFract + 1;

public:
	value_type Value;
	char Unit = 0;

	tFloatUnit() = default;
	tFloatUnit(double val, char unit) :tEmptyAble(false), Value(val), Unit(Unit) {}
	tFloatUnit(const std::string& val, const std::string& valSign)
	{
		if (val.size() == SizeValue && valSign.size() == 1)
		{
			m_Empty = false;

			Value = value_type(val);

			Unit = valSign[0];
		}
	}

	std::string ToStringValue() const
	{
		if (m_Empty) return "";

		return Value.ToString();
	}

	std::string ToStringUnit() const
	{
		if (m_Empty) return "";

		char Str[2]{};

		Str[0] = Unit;

		return Str;
	}

	std::string ToString() const
	{
		return ToStringValue() + ',' + ToStringUnit();
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tPositioning
{
	enum class tPositioning_State : std::uint8_t
	{
		UNKNOWN = 0,
		Autonomous,
		Differential,
		Estimated,
		ManualInput,
		Simulator,
		DataNotValid,
	};

	tPositioning_State Value = tPositioning_State::UNKNOWN;

	tPositioning() = default;
	explicit tPositioning(tPositioning_State val) :Value(val) {}
	explicit tPositioning(const std::string& val);

	std::string ToString() const;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename TValue, std::size_t Size>
struct tUInt :public tEmptyAble
{
	TValue Value = static_cast<TValue>(0);

public:
	tUInt() = default;
	explicit tUInt(TValue val) :tEmptyAble(false), Value(val) {}
	tUInt(const std::string& val)
	{
		if (val.size() == Size)
		{
			m_Empty = false;

			Value = static_cast<TValue>(std::strtoul(val.c_str(), 0, 10));
		}
	}

	std::string ToString() const
	{
		if (m_Empty) return "";

		std::stringstream SStream;

		SStream << std::setfill('0');
		SStream << std::setw(Size) << static_cast<unsigned int>(Value);

		return SStream.str();
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename TValue>
struct tUInt<TValue, 0> :public tEmptyAble
{
	TValue Value = static_cast<TValue>(0);

public:
	tUInt() = default;
	explicit tUInt(TValue val) :tEmptyAble(false), Value(val) {}
	tUInt(const std::string& val)
	{
		if (val.size() > 0)
		{
			m_Empty = false;

			Value = static_cast<TValue>(std::strtoul(val.c_str(), 0, 10));
		}
	}

	std::string ToString() const
	{
		if (m_Empty) return "";

		std::stringstream SStream;

		SStream << static_cast<unsigned int>(Value);

		return SStream.str();
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tSatellite
{
	typedef tUInt<std::uint8_t, 2> id_type;
	typedef tUInt<std::uint8_t, 2> elevation_type;
	typedef tUInt<std::uint16_t, 3> azimuth_type;
	typedef tUInt<std::uint8_t, 2> snr_type;

	id_type ID;
	elevation_type Elevation;
	azimuth_type Azimuth;
	snr_type SNR;

	tSatellite() = default;//C++11
	tSatellite(std::uint8_t id, std::uint8_t elevation, std::uint16_t azimuth, std::uint8_t snr);
	explicit tSatellite(const std::string& valID, const std::string& valElevation, const std::string& valAzimuth, const std::string& valSNR);

	std::string ToStringID() const;
	std::string ToStringElevation() const;
	std::string ToStringAzimuth() const;
	std::string ToStringSNR() const;

	std::string ToString() const;
};
///////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}
}
