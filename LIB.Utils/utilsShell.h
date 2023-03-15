///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsShell.h
//
// Standard ISO/IEC 114882, C++14
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |            |   2018 02 09  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "utilsBase.h"

#include <deque>
#include <vector>
#include <string>

namespace utils
{
	namespace shell
	{

struct tShellCommandList
{
	char *Command = nullptr;
	char *Description = nullptr;
	bool(*Handler)(const std::vector<std::string>& data);
};

class tShell
{
	std::deque<char> m_CharQueue;

	const std::size_t m_CharQueueSize = 50;

	std::vector<std::string> m_CmdLine;

	const tShellCommandList *p_CmdList = nullptr;

	const std::size_t m_CmdListSize = 0;

	bool m_EchoEnabled = false;

	tShell() { };

public:
	tShell(const tShellCommandList* cmdList, std::size_t cmdListSize, bool echo = false);
	virtual ~tShell();

	void SetEcho(bool enabled);

protected:
	virtual void Board_Send(char data) const = 0;
	virtual void Board_Send(const std::vector<char>& data) const = 0;

public:
	void Board_OnReceived(char data);
	void Board_OnReceived(const std::vector<char>& data);

private:
	void OnReceivedCmd(const std::vector<std::string>& data) const;

protected:
	virtual void OnShell() const = 0;
	virtual void OnFailed(const std::vector<std::string>& data) const { }
};

	}
}
