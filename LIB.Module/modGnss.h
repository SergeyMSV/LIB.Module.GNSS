///////////////////////////////////////////////////////////////////////////////////////////////////
// modGnss.h
// 2020-04-28
// Standard ISO/IEC 114882, C++11
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <utilsPacketNMEAType.h>

#include <cstdint>
#include <ctime>
#include <deque>
#include <iomanip>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

namespace mod
{

struct tGnssTaskScriptCmd
{
	enum class tID : std::uint8_t
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

struct tGnssTaskScriptCmdREQ : public tGnssTaskScriptCmd
{
	//enum class tCase_NoRsp : std::uint8_t//C++11
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

struct tGnssTaskScriptCmdGPI : public tGnssTaskScriptCmd
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
	std::map<int, double> Jamming;

	std::string ModeIndicator;

	tGnssDataSet() = default;

	std::tm GetDateTime() const;

	template <int SizeFractTime = 3, int SizeFract = 6>
	std::string ToString() const
	{
		std::stringstream Stream;
		Stream.setf(std::ios::fixed);
		Stream.fill('0');

		Stream << std::setw(2) << static_cast<int>(Hour);
		Stream << std::setw(2) << static_cast<int>(Minute);

		Stream.precision(SizeFractTime);
		Stream << std::setw(2 + SizeFractTime + 1) << Second;

		Stream << " " << (Valid ? 'A' : 'V');
		Stream << " " << ModeIndicator;

		Stream.precision(SizeFract);
		Stream << " " << std::setw(2 + SizeFract + 1) << Latitude;
		Stream << " " << std::setw(2 + SizeFract + 1) << Longitude;
		Stream << " " << std::setw(2 + SizeFract + 1) << Altitude;

		Stream << " " << Satellite.size();
		Stream << " " << Jamming.size();

		return Stream.str();
	}

	template <int SizeFractTime = 3, int SizeFractPosition = 6>
	std::string ToJSON() const
	{
		std::stringstream Stream;
		Stream.setf(std::ios::fixed);
		Stream.fill('0');

		Stream << "{\n";
		Stream << " \"utc\": \"";
		Stream << std::setw(2) << static_cast<int>(Day) << ".";
		Stream << std::setw(2) << static_cast<int>(Month) << ".";
		Stream << std::setw(2) << static_cast<int>(Year) << " ";
		Stream << std::setw(2) << static_cast<int>(Hour) << ".";
		Stream << std::setw(2) << static_cast<int>(Minute) << ".";
		Stream.precision(SizeFractTime);
		Stream << std::setw(2 + SizeFractTime + 1) << Second;
		Stream << "\",\n";
		Stream << " \"valid\": \"" << (Valid ? 'A' : 'V') << "\",\n";
		Stream << " \"mode_indicator\": \"" << ModeIndicator << "\",\n";

		Stream.precision(SizeFractPosition);
		Stream << " \"latitude\": " << Latitude << ",\n";
		Stream << " \"longitude\": " << Longitude << ",\n";
		Stream << " \"altitude\": " << Altitude << ",\n";

		Stream.precision(2);
		Stream << " \"speed\": " << Speed << ",\n";
		Stream << " \"course\": " << Course << ",\n";

		Stream << " \"satellite\": [\n";
		for (std::size_t i = 0; i < Satellite.size(); ++i)
		{
			Stream << "  {\n";
			Stream << "   \"id\": " << static_cast<int>(Satellite[i].ID.Value) << ",\n";
			Stream << "   \"elevation\": " << static_cast<int>(Satellite[i].Elevation.Value) << ",\n";
			Stream << "   \"azimuth\": " << static_cast<int>(Satellite[i].Azimuth.Value) << ",\n";
			Stream << "   \"snr\": " << static_cast<int>(Satellite[i].SNR.Value) << "\n";
			Stream << "  }";
			if (i < Satellite.size() - 1)
			{
				Stream << ",";
			}
			Stream << "\n";
		}
		Stream << " ],\n";

		Stream.precision(6);//for jamming frequency
		Stream << " \"jamming\": [\n";
		std::size_t counter = 0;
		for (const auto& i : Jamming)
		{
			Stream << "  {\n";
			Stream << "   \"index\": " << i.first << ",\n";
			Stream << "   \"frequency\": " << i.second << "\n";
			Stream << "  }";
			if (++counter < Jamming.size())
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
	std::uint8_t LatLonFract = 0;

	bool IsWrong() const
	{
		return
			MsgLast.empty() ||
			PeriodMax == 0 ||
			(LatLonFract != 4 && LatLonFract != 6);
	}
};

}
