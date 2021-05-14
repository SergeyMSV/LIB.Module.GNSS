#include "modGnss.h"

namespace mod
{

std::tm tGnssDataSet::GetDateTime() const
{
	std::tm Time{};

	if (Check_DateTime)
	{
		Time.tm_year = Year + 100;
		Time.tm_mon = Month - 1;
		Time.tm_mday = Day;
		Time.tm_hour = Hour;
		Time.tm_min = Minute;
		Time.tm_sec = static_cast<int>(Second);
	}
	else//01.01.1970 - Unix Epoch6
	{
		Time.tm_year = 70;
		Time.tm_mday = 1;
	}
	return Time;
}

}