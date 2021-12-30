#include <devConfig.h>

#include <devGNSS.h>
#include <devLog.h>
#include <devSettings.h>
#include <devShell.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

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

		return static_cast<int>(utils::tExitCode::EX_CONFIG);
	}

	utils::tExitCode CErr = utils::tExitCode::EX_OK;
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
			CErr = utils::tExitCode::EX_NOINPUT;
	}
	catch (std::exception & e)
	{
		std::cerr << "Exception: " << e.what() << "\n";

		g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::Exit;

		CErr = utils::tExitCode::EX_IOERR;
	}

	Thread_GNSS.join();

	if (ShellEnabled)
		Thread_Shell.detach();

	return static_cast<int>(CErr);
}
