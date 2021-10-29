#include "devLog.h"

#include <iostream>

namespace dev
{

tLog::tSettings tLog::LogSettings;

tLog::tLog(dev::tLog::tID id, const char* sign)
	:utils::tLog(true), m_ID(id), m_Sign(sign)
{

}

const char* tLog::GetSign() const
{
	return m_Sign;
}

void tLog::WriteLog(const std::string& msg)
{
	if (LogSettings.Field.Enabled && LogSettings.Value & static_cast<std::uint32_t>(m_ID))
	{
		std::cout << msg;
	}
}

}
