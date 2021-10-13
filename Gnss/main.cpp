#include <devConfig.h>

#include <devDB.h>
#include <devGNSS.h>
#include <devSettings.h>

#include <atomic>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

//from /usr/include/sysexits.h
enum class tExitCode : int
{
	EX_OK = 0,    // successful termination
	EX__BASE = 64,    // base value for error messages
	EX_USAGE = 64,    // command line usage error
	EX_DATAERR = 65,    // data format error
	EX_NOINPUT = 66,    // cannot open input
	EX_NOUSER = 67,    // addressee unknown
	EX_NOHOST = 68,    // host name unknown
	EX_UNAVAILABLE = 69,    // service unavailable
	EX_SOFTWARE = 70,    // internal software error
	EX_OSERR = 71,    // system error (e.g., can't fork)
	EX_OSFILE = 72,    // critical OS file missing
	EX_CANTCREAT = 73,    // can't create (user) output file
	EX_IOERR = 74,    // input/output error
	EX_TEMPFAIL = 75,    // temp failure; user is invited to retry
	EX_PROTOCOL = 76,    // remote error in protocol
	EX_NOPERM = 77,    // permission denied
	EX_CONFIG = 78,    // configuration error
	EX__MAX = 78,    // maximum listed value
};

namespace dev
{
	void ThreadFunShell();
}

tDataSetMainControl g_DataSetMainControl;

void Thread_GNSS_Handler(std::promise<bool>& promise)
{
	dev::tLog Log(dev::tLog::tID::GNSS, "GNSS");

	Log.LogSettings.Field.GNSS = 1;

	boost::asio::io_context IO;

	try
	{
		dev::tGNSS Dev(&Log, IO);

		std::thread Thread_IO([&]() { IO.run(); });

		bool Thread_Dev_Exists = true;
		bool Thread_Dev_ExistOnError = false;
		std::thread Thread_Dev([&]()
			{
				try
				{
					Dev();
					Thread_Dev_Exists = false;
					const std::string ErrMsg = Dev.GetLastErrorMsg();
					if (!ErrMsg.empty())
					{
						std::cerr << ErrMsg << "\n";
						Thread_Dev_ExistOnError = true;
					}
				}
				catch (...)
				{
					Thread_Dev_Exists = false;
					promise.set_exception(std::current_exception());
				}
			});

		tDataSetMainControl::tStateGNSS StateGNSSPrev = g_DataSetMainControl.Thread_GNSS_State;

		while (true)
		{
			if (!Thread_Dev_Exists)
				break;

			if (g_DataSetMainControl.Thread_GNSS_State != tDataSetMainControl::tStateGNSS::Nothing)
			{
				switch (g_DataSetMainControl.Thread_GNSS_State)
				{
				case tDataSetMainControl::tStateGNSS::Start: Dev.Start(); break;
				case tDataSetMainControl::tStateGNSS::Halt: Dev.Halt(); break;
				case tDataSetMainControl::tStateGNSS::Restart: Dev.Restart(); break;
				case tDataSetMainControl::tStateGNSS::Exit: Dev.Exit(); break;
				case tDataSetMainControl::tStateGNSS::UserTaskScriptStart:
				{
					std::lock_guard<std::mutex> Lock(g_DataSetMainControl.Thread_GNSS_State_UserTaskScriptIDMtx);

					if (!g_DataSetMainControl.Thread_GNSS_State_UserTaskScriptID.empty())
					{
						Dev.StartUserTaskScript(g_DataSetMainControl.Thread_GNSS_State_UserTaskScriptID);

						g_DataSetMainControl.Thread_GNSS_State_UserTaskScriptID.clear();
					}
					break;
				}
				}

				if (g_DataSetMainControl.Thread_GNSS_State == tDataSetMainControl::tStateGNSS::Exit)
					break;

				g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::Nothing;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		Thread_Dev.join();

		IO.stop();

		Thread_IO.join();

		if (!Thread_Dev_Exists)
			promise.set_value(Thread_Dev_ExistOnError);
	}
	catch (...)
	{
		promise.set_exception(std::current_exception());
	}
}

int main(int argc, char* argv[])
{
	const bool ShellEnabled = argc >= 2 && !strcmp(argv[1], "shell");

	dev::tLog::LogSettings.Value = 0;
	dev::tLog::LogSettings.Field.Enabled = ShellEnabled ? 1 : 0;

	try
	{
		const boost::filesystem::path Path{ argv[0] };
		boost::filesystem::path PathFile = Path.filename();
		if (PathFile.has_extension())
			PathFile.replace_extension();

		const std::string FileNameConf = PathFile.string() + ".conf";
		dev::g_Settings = dev::tSettings(FileNameConf);
	}
	catch (std::exception & e)
	{
		std::cerr << "Exception: " << e.what() << "\n";

		return static_cast<int>(tExitCode::EX_CONFIG);
	}

	try
	{
		dev::db::Open();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";

		return static_cast<int>(tExitCode::EX_IOERR);
	}

	int CErr = static_cast<int>(tExitCode::EX_OK);
	////////////////////////////////
	std::thread Thread_Shell;

	if (ShellEnabled)
		Thread_Shell = std::thread(dev::ThreadFunShell);
	////////////////////////////////

	std::promise<bool> Thread_GNSS_Promise;
	auto Thread_GNSS_Future = Thread_GNSS_Promise.get_future();

	std::thread Thread_GNSS(Thread_GNSS_Handler, std::ref(Thread_GNSS_Promise));//C++11

	try
	{
		if (Thread_GNSS_Future.get())
			CErr = static_cast<int>(tExitCode::EX_NOINPUT);
	}
	catch (std::exception & e)
	{
		std::cerr << "Exception: " << e.what() << "\n";

		g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::Exit;

		CErr = static_cast<int>(tExitCode::EX_IOERR);
	}

	dev::db::Close();

	Thread_GNSS.join();

	if (ShellEnabled)
		Thread_Shell.detach();

	return CErr;
}
