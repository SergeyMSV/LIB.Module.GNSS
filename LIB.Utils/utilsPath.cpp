#include "utilsShell.h"

#include <fstream>
#include <string>
#include <vector>

namespace utils
{

const std::vector<std::string> g_PathConfig =
{
	{""},
	{"../etc/"},
	{"/etc/default/"},
};

std::string GetPathConfig(const std::string& fileName)
{
	if (fileName.empty())
		return {};

	std::string FileNameConfig = fileName + ".conf";

	for (auto& i : g_PathConfig)
	{
		std::string FullPath = i + FileNameConfig;
		std::fstream File = std::fstream(FullPath, std::ios::in);
		if (File.is_open())
		{
			File.close();
			return FullPath;
		}
	}

	return {};
}

}
