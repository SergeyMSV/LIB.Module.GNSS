#include "utilsLinux.h"
#include "utilsPath.h"

#include <cerrno>
#include <cstdio>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

namespace utils
{

namespace linux
{

std::string CmdLine(const std::string& cmd)
{
#if defined(_WIN32)
	std::string Pattern = "realpath";
	size_t ValuePos = cmd.find(Pattern);
	if (ValuePos != std::string::npos)
	{
		std::string Value(cmd.begin() + ValuePos + Pattern.size(), cmd.end());
		Value.erase(Value.begin(), std::find_if(Value.begin(), Value.end(), [](char ch) { return !std::isspace(ch); }));
		if (Value.size() > 0)
		{
			if (Value[0] == '/')
				return Value;

			if (Value[0] == '~')
			{
				Value.erase(Value.begin(), std::find_if(Value.begin(), Value.end(), [](char ch) { return ch != '~'; }));
				return "/root" + Value;
			}
		}
	}

	return {};
#else
	FILE* File = popen(cmd.c_str(), "r");//File = popen("/bin/ls /etc/", "r");
	if (File == NULL)
		return {};

	std::string CmdRsp;

	char DataChunk[1035];
	while (fgets(DataChunk, sizeof(DataChunk), File) != NULL)
	{
		CmdRsp += DataChunk;
	}

	pclose(File);

	if (!CmdRsp.empty() && CmdRsp.back() == '\n')//Removes last '\n'
		CmdRsp.pop_back();

	return CmdRsp;
#endif
}

std::string GetPathReal(const std::string& path)
{
	std::string PathReal = CmdLine("realpath " + path);
	if (PathReal.size() > 0 && PathReal[0] == '/')
		return PathReal;
	return {};
}

double GetUptime()
{
	std::string FileName = GetPath("/proc/uptime");
	std::fstream File(FileName, std::ios::in);
	if (!File.good())
		return {};

	double UptimeSeconds;
	File >> UptimeSeconds;

	File.close();

	return UptimeSeconds;
}

std::string UptimeToString(double uptime)
{
	int Utime_Day = static_cast<int>(uptime / 86400);
	int UptimeRemove = Utime_Day * 86400;
	int Utime_Hour = static_cast<int>((uptime - UptimeRemove) / 3600);
	UptimeRemove += Utime_Hour * 3600;
	int Utime_Min = static_cast<int>((uptime - UptimeRemove) / 60);
	UptimeRemove += Utime_Min * 60;
	int Utime_Sec = static_cast<int>(uptime - UptimeRemove);

	std::stringstream SStr;
	SStr << Utime_Day << " days ";
	SStr << std::setfill('0') << std::setw(2) << Utime_Hour << ":";
	SStr << std::setfill('0') << std::setw(2) << Utime_Min << ":";
	SStr << std::setfill('0') << std::setw(2) << Utime_Sec;
	SStr << " (" << static_cast<int>(uptime / 3600) << " h " << Utime_Min << " min)";

	return SStr.str();
}

std::string GetUptimeString()
{
	return UptimeToString(GetUptime());
}

tCpuInfo GetCpuInfo()
{
	std::string FileName = GetPath("/proc/cpuinfo");
	std::fstream File(FileName, std::ios::in);
	if (!File.good())
		return {};

	tCpuInfo CpuInfo{};

	std::deque<std::string> Strings;

	while (!File.eof())
	{
		std::string Line;
		std::getline(File, Line);
		Line.erase(std::remove_if(Line.begin(), Line.end(), [](char ch) { return ch == '\t'; }), Line.end());

		size_t Pos = Line.find(":", 0);
		std::string PrmName = Line.substr(0, Pos);

		auto GetValueString = [&Pos](const std::string& a_line)
		{
			std::string Value = a_line.substr(Pos, a_line.size());
			Value.erase(Value.begin(), std::find_if(Value.begin(), Value.end(), [](char ch) { return ch != ' ' && ch != ':'; }));
			return Value;
		};

		if (PrmName == "model name")
		{
			CpuInfo.ModelName = GetValueString(Line);
		}
		else if (PrmName == "BogoMIPS")
		{
			errno = 0;
			std::string Value = GetValueString(Line);
			double Num = strtod(Value.c_str(), nullptr);
			if (Num > 0 && errno != ERANGE)
				CpuInfo.BogoMIPS = Num;
			errno = 0;
		}
		else if (PrmName == "Hardware")
		{
			CpuInfo.Hardware = GetValueString(Line);
		}
	}

	File.close();

	return CpuInfo;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
namespace PathConfig
{
#if defined(_WIN32)
	constexpr char g_TestDirRootFS[] = "test_root_fs";
#endif

	const std::vector<std::string> g_PathConfig =
	{
		{""},
		{"../etc/"},
		{"/etc/"},
		{"~/"},
		{"/etc/default/"},
	#if defined(_WIN32)
		{"../"}, // $(ProjectDir)
	#endif
	};

	static bool TestFile(const std::string& fileName)
	{
		std::fstream File = std::fstream(fileName, std::ios::in);
		if (!File.good())
			return false;

		File.close();
		return true;
	}

	static std::string TestPath(const std::string& path, std::string fileName, bool testDir)
	{
		std::string FilePath;

		if (!path.empty())
		{
			FilePath = path + fileName;
			if (TestFile(FilePath))
				return FilePath;
		}

		FilePath = path + "." + fileName; // hidden file
		if (TestFile(FilePath))
			return FilePath;

		FilePath = path + fileName + "rc";
		if (TestFile(FilePath))
			return FilePath;

		FilePath = path + "." + fileName + "rc"; // hidden file
		if (TestFile(FilePath))
			return FilePath;

		FilePath = path + fileName + ".conf";
		if (TestFile(FilePath))
			return FilePath;

		FilePath = path + fileName + ".conf.json";
		if (TestFile(FilePath))
			return FilePath;

		if (!testDir)
			return {};

		FilePath = path + fileName + "/";
		FilePath = TestPath(FilePath, fileName, false);
		if (!FilePath.empty())
			return FilePath;

		return {};
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::string GetPathConfig(const std::string& fileName)
{
	if (fileName.empty())
		return {};

	for (auto& i : PathConfig::g_PathConfig)
	{
		std::string PathBase = GetPathReal(i);

#if defined(_WIN32)
		if (i == "../") // $(ProjectDir)
			PathBase = "/../";

		PathBase = PathConfig::g_TestDirRootFS + PathBase;
#endif

		std::string Path = PathConfig::TestPath(PathBase, fileName, true);
		if (!Path.empty())
			return Path;
	}

	return {};
}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::string GetPath(const std::string& path)
{
	std::string PathReal = GetPathReal(path);

#if defined(_WIN32)
	if (PathReal.empty())
		return {};

	return PathConfig::g_TestDirRootFS + PathReal;
#else
	return PathReal;
#endif
}

}

}
