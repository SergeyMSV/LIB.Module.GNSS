#include "utilsLog.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#ifdef LIB_UTILS_LOG

namespace utils
{

tLog::tLog(bool colourEnabled)
	:m_ColourEnabled(colourEnabled)
{

}

void tLog::ColourEnabled(bool state)
{
	m_ColourEnabled = state;
}

void tLog::Write(bool timestamp, tLogColour textColour, const std::string& msg)
{
	WriteLog(timestamp, false, textColour, msg);
}

void tLog::WriteLine()
{
	WriteLog(false, true, tLogColour::Default, "");
}

void tLog::WriteLine(bool timestamp, tLogColour textColour, const std::string& msg)
{
	WriteLog(timestamp, true, textColour, msg);
}

void tLog::WriteHex(bool timestamp, tLogColour textColour, const std::string& msg, const tVectorUInt8& data)
{
	std::stringstream Stream;

	Stream << msg + '\n';

	std::string Substr;

	for (std::size_t i = 0; i < data.size(); ++i)
	{
		Stream << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(data[i]) << ' ';

		if (data[i] <= 0x20 || data[i] == 0x25)
		{
			Substr += '.';
		}
		else
		{
			Substr += data[i];
		}

		if (((i + 1) % 16) == 0)
		{
			Stream << "  " + Substr;

			Substr.clear();

			if (i < data.size() - 1)//It's not needed for the last string
			{
				Stream << '\n';
			}
		}
		else if (((i + 1) % 8) == 0)
		{
			Stream << "  ";
		}
	}

	const std::size_t Remains = data.size() % 16;

	if (Remains)
	{
		std::size_t Empty = (16 - Remains) * 3 + (Remains > 8 ? 2 : 4);

		for (std::size_t i = 0; i < Empty; ++i)
		{
			Stream << ' ';
		}

		Stream << Substr;
	}

	WriteLog(timestamp, true, textColour, Stream.str().c_str());
}

void tLog::WriteLog(bool timestamp, bool endl, tLogColour textColour, const std::string& msg)
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	std::stringstream Stream;

	if (timestamp)
	{
		const auto TimeNow = std::chrono::system_clock::now();
		const auto Time_us = std::chrono::time_point_cast<std::chrono::microseconds>(TimeNow);
		const auto TimeFract = static_cast<unsigned int>(Time_us.time_since_epoch().count() % 1000000);

		const std::time_t Time = std::chrono::system_clock::to_time_t(TimeNow);

		Stream << '[';
		Stream << std::put_time(std::localtime(&Time), "%T") << '.';
		Stream << std::setfill('0');
		Stream << std::setw(6) << TimeFract;

		const char* Sign = GetSign();
		if (Sign)
		{
			Stream << ' ';
			Stream << std::setfill(' ');
			Stream << std::setw(4) << Sign;
		}

		Stream << ']';
	}

	if (m_ColourEnabled)
	{
		Stream << "\x1b[";

		switch (textColour)
		{
		case tLogColour::Black: Stream << "30"; break;
		case tLogColour::Red: Stream << "31"; break;
		case tLogColour::Green: Stream << "32"; break;
		case tLogColour::Yellow: Stream << "33"; break;
		case tLogColour::Blue: Stream << "34"; break;
		case tLogColour::Magenta: Stream << "35"; break;
		case tLogColour::Cyan: Stream << "36"; break;
		case tLogColour::White: Stream << "37"; break;
		case tLogColour::Default: Stream << "39"; break;
		case tLogColour::LightGray: Stream << "90"; break;
		case tLogColour::LightRed: Stream << "91"; break;
		case tLogColour::LightGreen: Stream << "92"; break;
		case tLogColour::LightYellow: Stream << "93"; break;
		case tLogColour::LightBlue: Stream << "94"; break;
		case tLogColour::LightMagenta: Stream << "95"; break;
		case tLogColour::LightCyan: Stream << "96"; break;
		case tLogColour::LightWhite: Stream << "97"; break;
		default: Stream << "39"; break;
		}

		Stream << "m" + msg + "\x1b[0m";

		if (endl)
		{
			Stream << '\n';
		}	
	}
	else
	{
		Stream << msg;

		if (endl)
		{
			Stream << '\n';
		}
	}

	WriteLog(Stream.str().c_str());
}

}

#endif//LIB_UTILS_LOG
