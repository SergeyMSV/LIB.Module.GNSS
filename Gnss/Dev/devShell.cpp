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
	{ (char*)"?",      (char*)"help",				tShell::HandlerHelp },
	{ (char*)"help",   (char*)"help",				tShell::HandlerHelp },
	{ (char*)"echo",   (char*)"ECHO 0-off, 1-on",	tShell::HandlerEcho },
	{ (char*)"log",    (char*)"log  0-off, 1-on",	tShell::HandlerLog },
	{ (char*)"gnss",   (char*)"",					tShell::HandlerGNSS },
	{ (char*)"exit",   (char*)"halt the engine and close the program", tShell::HandlerEXIT },
	{ 0 }
};

const std::uint8_t g_ShellCommandListCol2Pos = 10;

tShell* g_pShell = nullptr;

tShell::tShell(const utils::shell::tShellCommandList* cmdList, std::size_t cmdListSize)
	:utils::shell::tShell(cmdList, cmdListSize)
{
	g_pShell = this;

	OnShell();
}

tShell::~tShell()
{
	g_pShell = nullptr;
}

bool tShell::HandlerEcho(const std::vector<std::string>& data)
{
	if (data.size() == 2 && g_pShell)
	{
		g_pShell->SetEcho(data[1] != "0");

		return true;
	}

	return false;
}

bool tShell::HandlerHelp(const std::vector<std::string>& data)
{
	if (data.size() == 1)
	{
		for (std::size_t i = 0; i < g_ShellCommandList.size(); ++i)
		{
			if (g_ShellCommandList[i].Command == 0)
				break;

			std::cout << std::setw(g_ShellCommandListCol2Pos) << std::setfill(' ') << std::left << g_ShellCommandList[i].Command << g_ShellCommandList[i].Description << '\n';
		}

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
		std::cout << std::setw(g_ShellCommandListCol2Pos) << std::setfill('.') << std::left << "gnss" << tLog::LogSettings.Field.GNSS << '\n';
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
		std::cout << std::setw(g_ShellCommandListCol2Pos) << std::setfill(' ') << std::left << "start" << "start the engine\n";
		std::cout << std::setw(g_ShellCommandListCol2Pos) << std::setfill(' ') << std::left << "restart" << "restart the engine\n";
		std::cout << std::setw(g_ShellCommandListCol2Pos) << std::setfill(' ') << std::left << "halt" << "halt the engine\n";
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
	for (std::size_t i = 0; i < data.size(); ++i)
	{
		std::cout << data[i];
	}
}

void tShell::OnShell() const
{
	std::cout << "# ";
}

void tShell::OnFailed(std::vector<std::string>& data) const
{
	std::cout << "OnFailed:" << std::endl;

	ShowReceivedCmd(data);
}

void tShell::ShowReceivedCmd(const std::vector<std::string>& data)
{
	for (std::size_t i = 0; i < data.size(); ++i)
	{
		std::cout << i << " " << data[i] << std::endl;
	}
}

void ThreadFunShell()
{
	dev::tShell Shell(g_ShellCommandList.data(), g_ShellCommandList.size());

	while (g_DataSetMainControl.Thread_GNSS_State != tDataSetMainControl::tStateGNSS::Exit)
	{
		int Byte = getchar();

		Byte = tolower(Byte);

		Shell.Board_OnReceived(static_cast<char>(Byte));
	}
}

}
