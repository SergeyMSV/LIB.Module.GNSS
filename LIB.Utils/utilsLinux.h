///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsLinux.h
// 2022-09-02
// Standard ISO/IEC 114882, C++20
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>

namespace utils
{

namespace linux
{

std::string CmdLine(const std::string& cmd);

double GetUptime();
std::string GetUptimeString();

struct tCpuInfo
{
	std::string ModelName;
	double BogoMIPS = 0.0;
	std::string Hardware;

	tCpuInfo() = default;
	tCpuInfo(const std::string& modelName, double bogoMIPS, const std::string& hardware)
		: ModelName(modelName), BogoMIPS(bogoMIPS), Hardware(hardware)
	{}

	bool operator == (const tCpuInfo&) const = default;
	bool operator != (const tCpuInfo&) const = default;
};

tCpuInfo GetCpuInfo();

std::string CorrPath(const std::string& pathRaw);

template <class T>
void CorrPaths(T& pathsRaw)
{
	for (auto& i : pathsRaw)
		i = CorrPath(i);
};

std::string GetPathConfig(const std::string& fileName);
std::string GetPathConfigExc(const std::string& path);
std::string GetPath(const std::string& path);

}

}
