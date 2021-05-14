#pragma once

#include <utilsBase.h>

#include <atomic>
#include <mutex>
#include <string>

#define DEV_DB_VERSION "DuperDBase 0.0.1"

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