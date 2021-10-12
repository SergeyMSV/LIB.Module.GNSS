#pragma once

#include <utilsBase.h>

#include <atomic>
#include <mutex>
#include <string>

struct tDataSetMainControl
{
	enum class tStateGNSS
	{
		Nothing,
		Halt,
		Start,
		Restart,
		Exit,
		UserTaskScriptStart,
	};

	std::atomic<tStateGNSS> Thread_GNSS_State{ tStateGNSS::Halt };

	mutable std::mutex Thread_GNSS_State_UserTaskScriptIDMtx;
	std::string Thread_GNSS_State_UserTaskScriptID;
};