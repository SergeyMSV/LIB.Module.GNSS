#include "utilsBase.h"
#include "utilsPath.h"

#include <cstdlib>

#include <array>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace utils
{

std::string GetDateTime(tm a_DateTime)
{
	std::ostringstream oss;
	oss << std::put_time(&a_DateTime, "%Y-%m-%d_%H-%M-%S");
	return oss.str();
}

std::string GetDateTime()
{
	time_t TimeNow = std::time(nullptr);
	tm* Time = std::localtime(&TimeNow);
	return GetDateTime(*Time);
}

tm GetDateTime(const std::string& a_value)
{
	tm DateTime{};
	std::istringstream iss(a_value);
	iss >> std::get_time(&DateTime, "%Y-%m-%d_%H-%M-%S");
	return DateTime;
}

namespace path
{

std::string GetAppName(const std::filesystem::path& path)
{
	std::filesystem::path PathFileName = path.filename();
	if (PathFileName.has_extension())
		PathFileName.replace_extension();
	return PathFileName.string();
}

std::string GetAppNameMain(const std::filesystem::path& path)
{
	std::string MainPart = GetAppName(path);
	// Main part of application name: mfrc522_xxx
	std::size_t Pos = MainPart.find_last_of('_');
	if (Pos != std::string::npos)
		MainPart = MainPart.substr(0, Pos);
	return MainPart;
}

std::filesystem::path GetPathNormal(const std::filesystem::path& pathRaw)
{
	if (pathRaw.empty())
		return {};
	std::error_code ErrCode;//In order to avoid exceptions
	std::filesystem::path Path;
#if defined(_WIN32)
	Path = "test_root_fs";
#endif
	//path("foo") / ""      // the result is "foo/" (appends)
	//path("foo") / "/bar"; // the result is "/bar" (replaces)
	if (pathRaw.string()[0] == '/')
	{
		Path += pathRaw.lexically_normal();
	}
	else if (pathRaw.string()[0] == '~')
	{
#if defined(_WIN32)
		std::string Home = "root";
#else
		std::string Home = std::getenv("HOME");
#endif
		std::string PathRawStr = pathRaw.string();
		PathRawStr.replace(0, 1, Home);
		Path /= std::filesystem::path(PathRawStr).lexically_normal();
	}
	else
	{
		Path /= pathRaw.lexically_normal();
	}

	return std::filesystem::weakly_canonical(Path, ErrCode); // it doesn't check if the file exists
}

static std::filesystem::path TestPath(const std::filesystem::path& path, std::string filename, bool currPath, bool testDir)
{
	auto TestPathFile = [](std::filesystem::path path, const std::string& filename, std::filesystem::path& pathFull)
	{
		path.append(filename);
#if defined(LIB_UTILS_LINUX_LOG)
		std::cout << "TestFile: " << path.string() << '\n';
#endif
		std::error_code ErrCode;//In order to avoid exceptions
		pathFull = std::filesystem::canonical(path, ErrCode);
		return !pathFull.empty();
	};

	std::filesystem::path PathFull;

	// It can find itself. This statement in order to avoid that situation.
	if (!currPath && TestPathFile(path, filename, PathFull))
		return PathFull;

	if (TestPathFile(path, "." + filename, PathFull)) // hidden file
		return PathFull;
	if (TestPathFile(path, filename + "rc", PathFull))
		return PathFull;
	if (TestPathFile(path, "." + filename + "rc", PathFull)) // hidden file
		return PathFull;
	if (TestPathFile(path, filename + ".conf", PathFull))
		return PathFull;
	if (TestPathFile(path, filename + ".conf.json", PathFull))
		return PathFull;

	if (!testDir)
		return {};
	std::filesystem::path PathDir = path;
	PathDir.append(filename);
	return TestPath(PathDir, filename, false, false);
}

std::filesystem::path GetPathConfig(const std::string& filename)
{
	if (filename.empty())
		return {};

	constexpr std::array PathConfig =
	{
		".",
		"../etc",
		"/etc",
		"~/",
		"/etc/default",
		"/usr/local/etc",
	#if defined(_WIN32)
		"..", // $(ProjectDir)
	#endif
	};

	for (const auto& i : PathConfig)
	{
		bool CurrPath = i == ".";
#if defined(_WIN32)
		if (!CurrPath)
			CurrPath = i == ".."; // $(ProjectDir)
#endif
		std::filesystem::path PathItem = GetPathNormal(i);
		if (PathItem.empty())
			continue;
		std::filesystem::path Path = TestPath(PathItem, filename, CurrPath, true);
		if (!Path.empty())
			return Path;
	}

	return {};
}

std::filesystem::path GetPathConfigExc(const std::string& filename)
{
	std::filesystem::path Str = GetPathConfig(filename);
	if (Str.empty())
		THROW_RUNTIME_ERROR("File not found: " + filename);
	return Str;
}

}

}
