#include "devGNSS.h"

namespace dev
{

tGNSS::tGNSS(utils::tLog* log, boost::asio::io_context& io)
	:m_pLog(log), m_pIO(&io)
{
	m_pMod = new tModGnssReceiver(this);
}

tGNSS::~tGNSS()
{
	delete m_pMod;
}

void tGNSS::operator()()
{
	if (m_pMod)
	{
		if (m_StartAuto)
		{
			m_StartAuto = false;

			m_pMod->Start(true);
		}

		(*m_pMod)();
	}
}

void tGNSS::Start()
{
	if (m_pMod)
	{
		m_pMod->Start();
	}
}

void tGNSS::Restart()
{
	if (m_pMod)
	{
		m_pMod->Restart();
	}
}

void tGNSS::Halt()
{
	if (m_pMod)
	{
		m_pMod->Halt();
	}
}

void tGNSS::Exit()
{
	if (m_pMod)
	{
		m_pMod->Exit();
	}
}

bool tGNSS::StartUserTaskScript(const std::string& taskScriptID)
{
	if (m_pMod)
	{
		return m_pMod->StartUserTaskScript(taskScriptID);
	}

	return false;
}

utils::tDevStatus tGNSS::GetStatus() const
{
	if (m_pMod)
	{
		return m_pMod->GetStatus();
	}

	return utils::tDevStatus::Unknown;
}

std::string tGNSS::GetLastErrorMsg() const
{
	if (m_pMod)
	{
		return m_pMod->GetLastErrorMsg();
	}

	return {};
}

}
