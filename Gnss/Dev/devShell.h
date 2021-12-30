///////////////////////////////////////////////////////////////////////////////////////////////////
// devShell.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 03 04  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <devConfig.h>
#include <devLog.h>

#include <utilsShell.h>

namespace dev
{

class tShell : public utils::shell::tShell
{
	tShell() = delete;

public:
	tShell(const utils::shell::tShellCommandList* cmdList, std::size_t cmdListSize);
	virtual ~tShell();

	static bool HandlerEcho(const std::vector<std::string>& data);
	static bool HandlerHelp (const std::vector<std::string>& data);
	static bool HandlerLog(const std::vector<std::string>& data);
	static bool HandlerGNSS(const std::vector<std::string>& data);
	static bool HandlerEXIT(const std::vector<std::string>& data);

protected:
	virtual void Board_Send(char data) const;
	virtual void Board_Send(const std::vector<char>& data) const;
	virtual void OnShell() const;
	virtual void OnFailed(std::vector<std::string>& data) const;

private:
	static void ShowReceivedCmd(const std::vector<std::string>& data);
};

}
