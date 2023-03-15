#include "utilsShell.h"

namespace utils
{
	namespace shell
	{

tShell::tShell(const tShellCommandList *cmdList, std::size_t cmdListSize, bool echo)
	:p_CmdList(cmdList), m_CmdListSize(cmdListSize), m_EchoEnabled(echo)
{

}

tShell::~tShell()
{

}

void tShell::SetEcho(bool enabled)
{
	m_EchoEnabled = enabled;
}

void tShell::Board_OnReceived(char data)
{
	if (data == 0x08 || data == 0x7F)//Backspace
	{
		if (m_CharQueue.size() > 0)
		{
			if (m_EchoEnabled)
			{
				char Erase[] = { 0x08, ' ', 0x08 };

				Board_Send(std::vector<char>(Erase, Erase + sizeof(Erase)));
			}

			m_CharQueue.pop_back();
		}
	}
	else if (data == 0x0D || data == 0x0A)//Enter
	{
		if (m_EchoEnabled)
		{
			Board_Send(0x0D);
			Board_Send(0x0A);
		}

		if (m_CharQueue.size() > 0)
		{
			std::string SubString;

			for (std::size_t i = 0; i < m_CharQueue.size(); ++i)
			{
				if (m_CharQueue[i] != ' ')
				{
					SubString.push_back(m_CharQueue[i]);
				}
				else if (SubString.size() > 0)
				{
					m_CmdLine.push_back(SubString);

					SubString.clear();
				}
			}

			if (SubString.size() > 0)
			{
				m_CmdLine.push_back(SubString);
			}

			m_CharQueue.clear();
		}

		OnReceivedCmd(m_CmdLine);

		m_CmdLine.clear();
	}
	else if (data >= 0x20 && data <= 0x7E)
	{
		if (m_CharQueue.size() < m_CharQueueSize)
		{
			m_CharQueue.push_back(data);

			if (m_EchoEnabled)
			{
				Board_Send(data);
			}
		}
	}
}

void tShell::Board_OnReceived(const std::vector<char>& data)
{
	for (std::size_t i = 0; i < data.size(); ++i)
	{
		Board_OnReceived(data[i]);
	}
}

void tShell::OnReceivedCmd(const std::vector<std::string>& data) const
{
	if (p_CmdList && data.size() > 0)
	{
		for (std::size_t i = 0; i < m_CmdListSize; ++i)
		{
			if (p_CmdList[i].Command == nullptr)
			{
				OnFailed(data);

				break;
			}

			if (p_CmdList[i].Command == data[0])
			{
				if ((p_CmdList[i].Handler)(data) == false)
				{
					OnFailed(data);
				}

				break;
			}
		}
	}

	OnShell();
}

	}
}
