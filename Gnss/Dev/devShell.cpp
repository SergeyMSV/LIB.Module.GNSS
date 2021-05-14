#include "devShell.h"

#include "devDataSet.h"
#include "devLog.h"

#include <iostream>
#include <iomanip>

extern tDataSetMainControl g_DataSetMainControl;

namespace dev
{

static const std::vector<utils::shell::tShellCommandList> g_ShellCommandList
{
	{ (char*)"help",   (char*)"blablabla", tShell::Handler1 },
	{ (char*)"?",      (char*)"la-la-la",  tShell::Handler2 },
	{ (char*)"~2",     (char*)"bla-bla",   tShell::Handler3 },
	{ (char*)"~debug", (char*)"DEBUG",     tShell::Handler3 },
	{ (char*)"echo",   (char*)"ECHO 0-off, 1-on",      tShell::HandlerECHO },
	{ (char*)"log",    (char*)"log 0-off, 1-on",      tShell::HandlerLog },
	{ (char*)"gnss",   (char*)"",      tShell::HandlerGNSS },
	{ (char*)"exit",   (char*)"",      tShell::HandlerEXIT },
	{ 0 }
};

tShell* g_pShell = nullptr;

tShell::tShell(const utils::shell::tShellCommandList* cmdList, std::size_t cmdListSize)
	:utils::shell::tShell(cmdList, cmdListSize)
{
	g_pShell = this;

	OnShell();//
}

tShell::~tShell()
{
	g_pShell = nullptr;
}

bool tShell::Handler1(const std::vector<std::string>& data)
{
	std::cout << "Handler1" << std::endl;

	//g_DataSet.SetDataValue1("Handler1");

	ShowReceivedCmd(data);

	return true;
}

bool tShell::Handler2(const std::vector<std::string>& data)
{
	std::cout << "Handler2" << std::endl;

	//g_DataSet.SetDataValue1("Handler2");

	ShowReceivedCmd(data);

	if (data.size() == 1)
	{
		const char MenuFormat[] = " %-20s %s\n";

		for (unsigned int i = 0; i < g_ShellCommandList.size(); ++i)
		{
			if (g_ShellCommandList[i].Command == 0)
			{
				break;
			}

			if (g_ShellCommandList[i].Command[0] == '~')//Hidden line
			{
				////////////////////////////////
				//Hidden menu
				std::cout << " hidden line ---- !!!" << std::endl;
				////////////////////////////////
				////////////////////////////////
			}
			else
			{
				std::printf(MenuFormat, g_ShellCommandList[i].Command, g_ShellCommandList[i].Description);
			}
		}

		return true;
	}

	return false;
}

bool tShell::Handler3(const std::vector<std::string>& data)
{
	std::cout << "Handler3" << std::endl;

	ShowReceivedCmd(data);

	Debug = !Debug;

	return true;
}

bool tShell::HandlerECHO(const std::vector<std::string>& data)
{
	if (data.size() == 2 && g_pShell)
	{
		g_pShell->SetEcho(data[1] != "0");

		return true;
	}

	return false;
}

bool tShell::HandlerLog(const std::vector<std::string>& data)
{
	if (data.size() == 2 && data[1] == "gnss")
	{
		tLog::LogSettings.Field.GNSS = !tLog::LogSettings.Field.GNSS;
		return true;
	}
	else
	{
		std::cout << std::setw(10) << std::setfill('.') << std::left << "gnss" << std::right << std::setw(20) << tLog::LogSettings.Field.GNSS << '\n';
		return true;
	}
}

bool tShell::HandlerGNSS(const std::vector<std::string>& data)
{
	if (data.size() >= 2 && data[1] == "start")
	{
		switch (data.size())
		{
		case 2: g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::Start; break;
		case 3:
		{
			g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::UserTaskScriptStart;

			std::lock_guard<std::mutex> Lock(g_DataSetMainControl.Thread_GNSS_State_UserTaskScriptIDMtx);

			g_DataSetMainControl.Thread_GNSS_State_UserTaskScriptID = data[2];
			break;
		}
		}
	}
	else if (data.size() == 2 && data[1] == "restart")
	{
		g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::Restart;
	}
	else if (data.size() == 2 && (data[1] == "halt" || data[1] == "stop"))
	{
		g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::Halt;
	}
	else if (data.size() == 2 && data[1] == "exit")
	{
		g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::Exit;
	}
	else
	{
		std::cout << std::setw(10) << std::setfill(' ') << std::left << "start" << std::right << std::setw(20) << "comment...\n";
		std::cout << std::setw(10) << std::setfill(' ') << std::left << "restart" << std::right << std::setw(20) << "comment...\n";
		std::cout << std::setw(10) << std::setfill(' ') << std::left << "halt" << std::right << std::setw(20) << "comment...\n";
		std::cout << std::setw(10) << std::setfill(' ') << std::left << "exit" << std::right << std::setw(20) << "comment...\n";
	}
	return true;
}

bool tShell::HandlerEXIT(const std::vector<std::string>& data)
{
	g_DataSetMainControl.Thread_GNSS_State = tDataSetMainControl::tStateGNSS::Exit;

	return true;
}

void tShell::Board_Send(char data) const
{
	std::cout << data;
}

void tShell::Board_Send(const std::vector<char>& data) const
{
	for (size_t i = 0; i < data.size(); ++i)
	{
		std::cout << data[i];
	}
}

void tShell::OnShell() const
{
	std::cout << "#";
	//if (Debug)
	//{
	//	std::cout << "debug>>";
	//}
	//else
	//{
	//	std::cout << "shell>>";
	//}
}

void tShell::OnFailed(std::vector<std::string>& data) const
{
	std::cout << "OnFailed:" << std::endl;

	ShowReceivedCmd(data);
}

void tShell::ShowReceivedCmd(const std::vector<std::string>& data)
{
	for (size_t i = 0; i < data.size(); ++i)
	{
		std::cout << i << " " << data[i] << std::endl;
	}
}

bool tShell::Debug = false;

void ThreadFunShell()
{
	dev::tShell Shell(g_ShellCommandList.data(), g_ShellCommandList.size());

	while (g_DataSetMainControl.Thread_GNSS_State != tDataSetMainControl::tStateGNSS::Exit)
	{
		int Byte = getchar();

		Byte = std::tolower(Byte);

		Shell.Board_OnReceived(static_cast<char>(Byte));
	}
}

}
