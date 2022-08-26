///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsPath.h
// 2022-04-20
// Standard ISO/IEC 114882, C++20
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace utils
{

std::string GetDateTime(tm a_DateTime);
std::string GetDateTime();
tm GetDateTime(const std::string& a_value);

	namespace linux
	{

std::string GetPathConfig(const std::string& fileName);
std::string GetPath(const std::string& path);

struct tCpuInfo
{
	std::string ModelName;
	double BogoMIPS = 0.0;
	std::string Hardware;

	tCpuInfo() = default;

	bool operator == (const tCpuInfo&) const = default;
	bool operator != (const tCpuInfo&) const = default;
};

tCpuInfo GetCpuInfo();
std::string GetHostname();
std::string GetLoadAvg();//[TBD] is not a string
double GetUptime();
std::string GetVersion();

	}

}
