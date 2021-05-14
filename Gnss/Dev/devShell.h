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

#include "utilsShell.h"

namespace dev
{

class tShell : public utils::shell::tShell
{
	tShell() = delete;

public:
	explicit tShell(const utils::shell::tShellCommandList* cmdList, std::size_t cmdListSize);
	virtual ~tShell();

	static bool Handler1(const std::vector<std::string>& data);
	static bool Handler2(const std::vector<std::string>& data);

	static bool Debug;//TEST shall be like m_Exit

	static bool Handler3(const std::vector<std::string>& data);
	static bool HandlerECHO(const std::vector<std::string>& data);
	static bool HandlerLog(const std::vector<std::string>& data);
	static bool HandlerGNSS(const std::vector<std::string>& data);
	static bool HandlerDB(const std::vector<std::string>& data);
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
