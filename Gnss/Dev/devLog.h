///////////////////////////////////////////////////////////////////////////////////////////////////
// devLog.h
//
// Standard ISO/IEC 114882, C++14
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 01 15  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <utilsLog.h>

#include <cstdint>

namespace dev
{

class tLog : public utils::tLog
{
public:
	enum class tID : std::uint32_t
	{
		NoID = 0,
		Enabled = (1 << 0),
		GNSS = (1 << 1),
	};

	union tSettings
	{
		struct
		{
			std::uint32_t Enabled : 1;//if it's equal to 0 no logs are shown
			std::uint32_t GNSS : 1;

			std::uint32_t : 29;
		}Field;

		std::uint32_t Value = 0;
	};

	static tSettings LogSettings;

private:
	tID m_ID = tID::NoID;
	const char* m_Sign = nullptr;

	tLog() = delete;

public:
	explicit tLog(dev::tLog::tID id, const char* sign);

protected:
	const char* GetSign() const override;

	void WriteLog(const std::string& msg) override;
};

}
