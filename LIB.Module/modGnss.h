///////////////////////////////////////////////////////////////////////////////////////////////////
// modGnss.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 04 28  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <utilsPacketNMEAType.h>

#include <cstdint>
#include <ctime>
#include <deque>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace mod
{

struct tGnssTaskScriptCmd
{
	enum class tID :std::uint8_t
	{
		GPI,
		GPO,
		REQ,
	};

private:
	tID m_ID = tID::REQ;

	tGnssTaskScriptCmd() = delete;

protected:
	explicit tGnssTaskScriptCmd(tID id) :m_ID(id) {}

public:
	virtual ~tGnssTaskScriptCmd() = default;

	tID GetID() const { return m_ID; }
};

struct tGnssTaskScriptCmdREQ :public tGnssTaskScriptCmd
{
	//enum class tCase_NoRsp: unsigned char//C++11
	//{
	//	Continue,
	//	Restart,
	//	Error = 0xFF,
	//};

	std::string Msg;
	std::string RspHead;
	std::string RspBody;
	std::string CaseRspWrong;
	int RspWait_us = 0;
	int Pause_us = 0;
	//tCase_NoRsp Case_NoRsp = tCase_NoRsp::Continue;

	tGnssTaskScriptCmdREQ() :tGnssTaskScriptCmd(tGnssTaskScriptCmd::tID::REQ) {}
};

struct tGnssTaskScriptCmdGPI :public tGnssTaskScriptCmd
{
	std::string ID;
	bool State = 0;
	int Wait_us = 0;

	tGnssTaskScriptCmdGPI() :tGnssTaskScriptCmd(tGnssTaskScriptCmd::tID::GPI) {}
};

struct tGnssTaskScriptCmdGPO :public tGnssTaskScriptCmd
{
	std::string ID;
	bool State = 0;
	int Pause_us = 0;

	tGnssTaskScriptCmdGPO() :tGnssTaskScriptCmd(tGnssTaskScriptCmd::tID::GPO) {}
};

typedef std::deque<std::unique_ptr<tGnssTaskScriptCmd>> tGnssTaskScript;

typedef utils::packet_NMEA::Type::tGNSS_State tGNSS_State;
typedef utils::packet_NMEA::Type::tSatellite tGNSS_Satellite;

enum class tGnssStatus :std::uint8_t
{
	Init,
	Operation,
	Deinit,
	Halted,
	Error,
	Unknown = 0xFF,
};

//enum class tGnssError : std::uint8_t
//{
//	OK,
//
//	State_ErrTimer,
//
//	Unknown = 0xFF,
//};

struct tGnssDataSet
{
	tGNSS_State GNSS = tGNSS_State::UNKNOWN;

	bool Check_DateTime = false;
	std::uint8_t Year = 0;
	std::uint8_t Month = 0;
	std::uint8_t Day = 0;
	std::uint8_t Hour = 0;
	std::uint8_t Minute = 0;
	double Second = 0;

	bool Check_Position = false;
	bool Valid = false;
	double Latitude = 0;
	double Longitude = 0;
	double Altitude = 0;
	double Speed = 0;
	double Course = 0;

	std::deque<tGNSS_Satellite> Satellite;

	tGnssDataSet() = default;

	std::tm GetDateTime() const;

	template <int SizeFractTime = 3, int SizeFract = 6>
	std::string ToString() const
	{
		std::stringstream Stream;

		Stream << std::setfill('0');
		Stream << std::setw(2) << static_cast<int>(Hour);
		Stream << std::setw(2) << static_cast<int>(Minute);

		Stream.setf(std::ios::fixed);

		Stream << std::setw(2 + SizeFractTime + 1) << std::setprecision(SizeFractTime) << Second;

		Stream << " " << (Valid ? 'A' : 'V');


		Stream.setf(std::ios::fixed);

		Stream << " " << std::setw(2 + SizeFract + 1) << std::setprecision(SizeFract) << Latitude;
		Stream << " " << std::setw(2 + SizeFract + 1) << std::setprecision(SizeFract) << Longitude;
		Stream << " " << std::setw(2 + SizeFract + 1) << std::setprecision(SizeFract) << Altitude;


		Stream << " " << Satellite.size();

		return Stream.str();
	}

	template <int SizeFractTime = 3, int SizeFractPosition = 6>
	std::string ToJSON() const
	{
		std::stringstream Stream;
		Stream << "{\n";
		//Stream << " \"gnss\": \"" <<utils::packet_NMEA::Type::tGNSS(GNSS).ToString() << "\",\n";
		Stream << " \"utc\": \"";
		Stream << std::setfill('0');
		Stream << std::setw(2) << static_cast<int>(Day) << ".";
		Stream << std::setw(2) << static_cast<int>(Month) << ".";
		Stream << std::setw(2) << static_cast<int>(Year) << " ";
		Stream << std::setw(2) << static_cast<int>(Hour) << ".";
		Stream << std::setw(2) << static_cast<int>(Minute) << ".";
		Stream.setf(std::ios::fixed);
		Stream << std::setw(2 + SizeFractTime + 1) << std::setprecision(SizeFractTime) << Second;
		Stream << "\",\n";
		Stream << " \"valid\": \"" << (Valid ? '1' : '0') << "\",\n";

		Stream.setf(std::ios::fixed);

		Stream << " \"latitude\": \"";
		Stream << std::setprecision(SizeFractPosition) << Latitude;
		Stream << "\",\n";
		Stream << " \"longitude\": \"";
		Stream << std::setprecision(SizeFractPosition) << Longitude;
		Stream << "\",\n";
		Stream << " \"altitude\": \"";
		Stream << std::setprecision(SizeFractPosition) << Altitude;
		Stream << "\",\n";

		const int FixedFract = 2;
		Stream << " \"speed\": \"";
		Stream << std::setprecision(FixedFract) << Speed;
		Stream << "\",\n";
		Stream << " \"course\": \"";
		Stream << std::setprecision(FixedFract) << Course;
		Stream << "\",\n";
		
		Stream << " \"satellite\": [\n";
		for (std::size_t i = 0; i < Satellite.size(); ++i)
		{
			Stream << "  {\n";
			Stream << "   \"id\": \"" << static_cast<int>(Satellite[i].ID.Value) << "\",\n";
			Stream << "   \"elevation\": \"" << static_cast<int>(Satellite[i].Elevation.Value) << "\",\n";
			Stream << "   \"azimuth\": \"" << static_cast<int>(Satellite[i].Azimuth.Value) << "\",\n";
			Stream << "   \"snr\": \"" << static_cast<int>(Satellite[i].SNR.Value) << "\"\n";
			Stream << "  }";
			if (i < Satellite.size() - 1)
			{
				Stream << ",";
			}
			Stream << "\n";
		}
		Stream << " ]\n";

		Stream << "}";
		return Stream.str();
	}
};

struct tGnssSettingsNMEA
{
	std::string MsgLast;
	std::uint32_t PeriodMax = 0;
};

}
