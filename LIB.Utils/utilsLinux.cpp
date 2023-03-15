#include "utilsBase.h"
#include "utilsLinux.h"
#include "utilsPath.h"

#include <cerrno>
#include <cstdio>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

//#define LIB_UTILS_LINUX_LOG

#if defined(_WIN32)
#include <filesystem>
#endif

#if defined(LIB_UTILS_LINUX_LOG)
#include <iostream>
#endif

namespace utils
{

namespace linux
{

#if defined(_WIN32)
static std::string CmdLineWinTest(const std::string& cmd);
#endif

std::string CmdLine(const std::string& cmd)
{
#if defined(_WIN32)
	return CmdLineWinTest(cmd);
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

const std::vector<std::string> g_PathConfig =
{
	{"."},
	{"../etc"},
	{"/etc"},
	{"~/"},
	{"/etc/default"},
	{"/usr/local/etc"},
#if defined(_WIN32)
	{".."}, // $(ProjectDir)
#endif
};

static bool TestFile(const std::string& fileName)
{
#if defined(LIB_UTILS_LINUX_LOG)
	std::cout << "TestFile: " << fileName << '\n';
#endif
	std::fstream File = std::fstream(fileName, std::ios::in);
	if (!File.good())
		return false;

	File.close();
	return true;
}

static std::string TestPath(const std::string& path, std::string fileName, bool currPath, bool testDir)
{
	std::string FilePath;

	if (!currPath)
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
	FilePath = TestPath(FilePath, fileName, false, false);
	if (!FilePath.empty())
		return FilePath;

	return {};
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////

std::string CorrPath(const std::string& pathRaw)
{
	std::string Path = pathRaw;
	std::replace(Path.begin(), Path.end(), '\\', '/');
	Path.erase(std::unique(Path.begin(), Path.end(), [](char a, char b) { return a == '/' && b == '/'; }), Path.end());
	return Path;
}

std::string GetPathConfig(const std::string& fileName)
{
	if (fileName.empty())
		return {};

	for (auto& i : PathConfig::g_PathConfig)
	{
		bool CurrPath = CorrPath(i) == ".";

#if defined(_WIN32)
		if (!CurrPath)
			CurrPath = CorrPath(i) == ".."; // $(ProjectDir)
#endif

		std::string PathBase = GetPathReal(i) +'/';
		std::string Path = PathConfig::TestPath(PathBase, fileName, CurrPath, true);
		if (!Path.empty())
			return Path;
	}

	return {};
}

std::string GetPathConfigExc(const std::string& path)
{
	std::string Str = GetPathConfig(path);
	if (Str.empty())
		throw std::runtime_error("File not found: " + path);
	return Str;
};

std::string GetPath(const std::string& path)
{
	return GetPathReal(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions for tests
#if defined(_WIN32)
static std::string CmdLineWinTest(const std::string& cmd)
{
	if (cmd == "free")
	{
		return
			"               total        used        free      shared  buff / cache   available\n\
Mem:          492116       33240      129108        1184      329768      445984\n\
Swap:              0           0           0";
	}

	std::string CmdValue = GetStringEnding("cat", cmd);
	if (!CmdValue.empty())
	{
		std::string Path = GetPath(CmdValue);
		if (Path.empty())
			return {};

		std::fstream File(Path, std::ios::in);
		if (!File.good())
			return {};

		std::string Data;

		while (!File.eof())
		{
			std::string Line;
			std::getline(File, Line);
			Data += Line + "\n";
		}

		File.close();

		return Data;
	}

	CmdValue = GetStringEnding("realpath", cmd);
	if (!CmdValue.empty())
	{
		auto CorrPathReal = [](const std::string& pathRaw)->std::string
		{
			std::string Path = CorrPath(pathRaw);
			if (Path.back() == '/')
				Path.pop_back();
			return Path;
		};

		std::string PathCurr = std::filesystem::current_path().string();
		std::string PathMain = GetStringEnding(":", PathCurr);
		PathMain = CorrPath(PathMain);
		PathMain += "/test_root_fs/";

		CmdValue.erase(CmdValue.begin(), std::find_if(CmdValue.begin(), CmdValue.end(), [](char ch) { return !std::isspace(ch); }));
		if (CmdValue.empty())
			return "realpath: missing operand\nTry 'realpath --help' for more information.";

		if (CmdValue.size() > 0)
		{
			if (CmdValue[0] == '/')
				return CorrPathReal(PathMain + CmdValue);

			if (CmdValue[0] == '~')
			{
				CmdValue.erase(CmdValue.begin(), std::find_if(CmdValue.begin(), CmdValue.end(), [](char ch) { return ch != '~'; }));
				return CorrPathReal(PathMain + "root" + CmdValue);
			}

			if (CmdValue == "." || CmdValue == "./")
				return CorrPathReal(PathMain);

			if (CmdValue.find("..") == 0)
				return CorrPathReal(PathMain + CmdValue);
		}
		return "It seems to be a BUG";
	}

	return {};
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////
}

}
