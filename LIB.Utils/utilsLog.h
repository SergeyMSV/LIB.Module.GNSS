///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsLog.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |    long ago   | ... before 2016 05 16
// |            |               |
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <libConfig.h>

#include "utilsBase.h"

#include <cstdint>

#include <string>
#include <mutex>

namespace utils
{

enum class tLogColour : std::uint8_t
{
	Black,
	Red,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	White,
	Default,
	LightGray,
	LightRed,
	LightGreen,
	LightYellow,
	LightBlue,
	LightMagenta,
	LightCyan,
	LightWhite,
};

#ifdef LIB_UTILS_LOG

class tLog
{
	mutable std::mutex m_Mtx;

	bool m_ColourEnabled = false;

public:
	tLog() = delete;
	explicit tLog(bool colourEnabled);
	virtual ~tLog() { }

	void ColourEnabled(bool state);

	void Write(bool timestamp, tLogColour textColour, const std::string& msg);

	void WriteLine();
	void WriteLine(bool timestamp, tLogColour textColour, const std::string& msg);

	void WriteHex(bool timestamp, tLogColour textColour, const std::string& msg, const tVectorUInt8& data);

protected:
	virtual const char* GetSign() const { return nullptr; }

	virtual void WriteLog(const std::string& msg) = 0;

private:
	virtual void WriteLog(bool timestamp, bool endl, tLogColour textColour, const std::string& msg);
};

#else//LIB_UTILS_LOG

class tLog
{
public:
	tLog() = delete;
	explicit tLog(bool colourEnabled) { }
	virtual ~tLog() { }

	void ColourEnabled(bool state) { }

	void Write(bool timestamp, tLogColour textColour, const std::string& msg) { }

	void WriteLine() { }
	void WriteLine(bool timestamp, tLogColour textColour, const std::string& msg) { }

	void WriteHex(bool timestamp, tLogColour textColour, const std::string& msg, const tVectorUInt8& data) { }

protected:
	virtual const char* GetSign() const { return nullptr; }

	virtual void WriteLog(const std::string& msg) = 0;
};

#endif//LIB_UTILS_LOG

}
