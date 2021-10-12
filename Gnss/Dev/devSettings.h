///////////////////////////////////////////////////////////////////////////////////////////////////
// devSettings.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 04 29  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <devConfig.h>

#include <modGnss.h>

#include <utilsBase.h>

#include <string>

namespace dev
{

struct tSettings
{
	struct
	{
		std::string Model;
		std::string ID;
	}Main;

	struct
	{
		std::string ID;
		std::uint32_t BR = 0;
	}SerialPort;

	struct
	{
		std::string Path;
		std::string FileName;
	}Output;

private:
	std::string m_ConfigFileName;

public:
	tSettings() {}
	explicit tSettings(const std::string& fileName);

	std::string GetConfigFileName() const { return m_ConfigFileName; }

	mod::tGnssTaskScript GetTaskScript(const std::string& id, bool userTaskScript) const;
	mod::tGnssSettingsNMEA GetSettingsNMEA() const;
};

extern tSettings g_Settings;

}