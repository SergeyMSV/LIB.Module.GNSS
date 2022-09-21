///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsPath.h
// 2022-04-20
// Standard ISO/IEC 114882, C++20
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <ctime>

#include <deque>
#include <string>
#include <vector>

namespace utils
{

std::string GetDateTime(tm a_DateTime);
std::string GetDateTime();
tm GetDateTime(const std::string& a_value);

std::deque<std::string> GetFilesLatest(const std::string& path, const std::string& prefix, size_t qtyFilesLatest);
void RemoveFilesOutdated(const std::string& path, const std::string& prefix, size_t qtyFilesLatest);

}
