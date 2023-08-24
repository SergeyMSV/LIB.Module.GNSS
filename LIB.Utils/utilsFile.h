///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsFile.h
// 2022-04-20
// Standard ISO/IEC 114882, C++20
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <ctime>

#include <deque>
#include <string>
#include <optional>

namespace utils
{
	namespace file
	{

std::optional<std::string> ReplaceFileNamePrefix(const std::string& path, const std::string& from, const std::string& to);

std::deque<std::string> GetFilesLatest(const std::string& path, const std::string& prefix, size_t qtyFilesLatest);
void RemoveFilesOutdated(const std::string& path, const std::string& prefix, size_t qtyFilesLatest);
void RemoveFilesWithPrefix(const std::string& path, const std::string& prefix);

	}
}
