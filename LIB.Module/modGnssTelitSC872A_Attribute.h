///////////////////////////////////////////////////////////////////////////////////////////////////
// modGnssTelitSC872A_Attribute.h
//
// Standard ISO/IEC 114882, C++11
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "modGnss.h"

#include <mutex>
#include <string>
#include <deque>
#include <ctime>

#include <iomanip>
#include <sstream>

namespace mod
{

union tGnssTelitSC872ASettings
{
	struct tNMEA_Out
	{
		std::uint8_t GLL = 0;//max 0 - 5 ()
		std::uint8_t GGA = 1;
		std::uint8_t GSA = 1;
		std::uint8_t GSV = 1;
		std::uint8_t GRS = 0;
		std::uint8_t GST = 0;
		std::uint8_t RMC = 1;
		std::uint8_t VTG = 1;
		std::uint8_t ZDA = 1;
		std::uint8_t MCHN = 0;//PMTKCHN
		std::uint8_t DTM = 0;
		std::uint8_t GBS = 0;
	};
};

}
