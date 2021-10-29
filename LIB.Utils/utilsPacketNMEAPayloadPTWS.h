///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsPacketNMEAPayloadPTWS.h
//
// Standard ISO/IEC 114882, C++17
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 02 11  |
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
struct tPayloadPTWS_JAM_SIGNAL_VAL
{
	typedef Type::tUInt<std::uint8_t, 0> index_type;
	typedef Type::tFloat<0, 6> frequency_type;

	index_type Index;
	frequency_type Frequency;

	tPayloadPTWS_JAM_SIGNAL_VAL() = default;
	explicit tPayloadPTWS_JAM_SIGNAL_VAL(std::uint8_t index, double frequency)
		:Index(index), Frequency(frequency)
	{}
	explicit tPayloadPTWS_JAM_SIGNAL_VAL(const tPayloadCommon::value_type& val)
	{
		if (Try(val))
		{
			Index = index_type(val[5]);
			Frequency = frequency_type(val[7]);
		}
	}

	static bool Try(const tPayloadCommon::value_type& val)
	{
		return val.size() == 8 &&
			!std::strcmp(val[0].c_str(), "PTWS") &&
			!std::strcmp(val[1].c_str(), "JAM") &&
			!std::strcmp(val[2].c_str(), "SIGNAL") &&
			!std::strcmp(val[3].c_str(), "VAL") &&
			!std::strcmp(val[4].c_str(), "INDEX") &&
			!std::strcmp(val[6].c_str(), "FREQ");
	}

	tPayloadCommon::value_type GetPayload() const
	{
		tPayloadCommon::value_type Data;

		Data.push_back("PTWS");
		Data.push_back("JAM");
		Data.push_back("SIGNAL");
		Data.push_back("VAL");
		Data.push_back("INDEX");
		Data.push_back(Index.ToString());
		Data.push_back("FREQ");
		Data.push_back(Frequency.ToString());

		return Data;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tPayloadPTWS_VERSION_GET
{
	tPayloadPTWS_VERSION_GET() = default;
	explicit tPayloadPTWS_VERSION_GET(const tPayloadCommon::value_type& val)
	{
		//if (Try(val))
		//{

		//}
	}

	static bool Try(const tPayloadCommon::value_type& val)
	{
		return val.size() == 3 &&
			!std::strcmp(val[0].c_str(), "PTWS") &&
			!std::strcmp(val[1].c_str(), "VERSION") &&
			!std::strcmp(val[2].c_str(), "GET");
	}

	tPayloadCommon::value_type GetPayload() const
	{
		tPayloadCommon::value_type Data;

		Data.push_back("PTWS");
		Data.push_back("VERSION");
		Data.push_back("GET");

		return Data;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tPayloadPTWS_VERSION_VAL
{
	typedef std::string version_type;

	version_type Version;

	tPayloadPTWS_VERSION_VAL() = default;
	explicit tPayloadPTWS_VERSION_VAL(version_type version)
		:Version(version)
	{}
	explicit tPayloadPTWS_VERSION_VAL(const tPayloadCommon::value_type& val)
	{
		if (Try(val))
		{
			Version = val[3];
		}
	}

	static bool Try(const tPayloadCommon::value_type& val)
	{
		return val.size() == 4 &&
			!std::strcmp(val[0].c_str(), "PTWS") &&
			!std::strcmp(val[1].c_str(), "VERSION") &&
			!std::strcmp(val[2].c_str(), "VAL");
	}

	tPayloadCommon::value_type GetPayload() const
	{
		tPayloadCommon::value_type Data;

		Data.push_back("PTWS");
		Data.push_back("VERSION");
		Data.push_back("VAL");
		Data.push_back(Version);

		return Data;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
	}
}
