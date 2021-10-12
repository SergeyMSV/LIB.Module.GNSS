#include "modGnssReceiver.h"

#include <iostream>

namespace mod
{

tGnssReceiver::tGnssReceiver(utils::tLog* log)
	:m_pLog(log)
{
	ChangeState(new tStateHalt(this, "the very start"));
	return;
}

void tGnssReceiver::operator()()
{
	while ((*m_pState)());
}

void tGnssReceiver::Start()
{
	m_Control_Operation = true;
}

void tGnssReceiver::Start(bool exitOnError)
{
	m_Control_ExitOnError = exitOnError;
	Start();
}

void tGnssReceiver::Restart()
{
	m_Control_Restart = true;
}

void tGnssReceiver::Halt()
{
	m_Control_Operation = false;
}

void tGnssReceiver::Exit()
{
	m_Control_Exit = true;
	m_Control_Operation = false;
}

bool tGnssReceiver::StartUserTaskScript(const std::string& taskScriptID)
{
	//std::lock_guard<std::mutex> Lock(m_MtxState);

	return m_pState->SetUserTaskScript(taskScriptID);
}

tGnssStatus tGnssReceiver::GetStatus() const
{
	//std::lock_guard<std::mutex> Lock(m_MtxState);

	return m_pState->GetStatus();
}

std::string tGnssReceiver::GetLastErrorMsg() const
{
	//std::lock_guard<std::mutex> Lock(m_MtxState);

	return m_LastErrorMsg;
}

void tGnssReceiver::Board_OnReceived(utils::tVectorUInt8& data)
{
	std::lock_guard<std::mutex> Lock(m_MtxReceivedData);

	m_ReceivedData.push(data);
}

bool tGnssReceiver::IsReceivedData() const
{
	std::lock_guard<std::mutex> Lock(m_MtxReceivedData);

	return m_ReceivedData.size() > 0;
}

utils::tVectorUInt8 tGnssReceiver::GetReceivedDataChunk()
{
	std::lock_guard<std::mutex> Lock(m_MtxReceivedData);

	if (m_ReceivedData.size() > 0)
	{
		utils::tVectorUInt8 Data(std::forward<utils::tVectorUInt8>(m_ReceivedData.front()));

		m_ReceivedData.pop();

		return Data;
	}

	return {};
}

void tGnssReceiver::ClearReceivedData()
{
	std::lock_guard<std::mutex> Lock(m_MtxReceivedData);

	while (!m_ReceivedData.empty())
	{
		m_ReceivedData.pop();
	}
}

void tGnssReceiver::SetStrTimePeriod(std::stringstream& stream, const std::chrono::time_point<tClock>& timePoint) const
{
	auto Time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tClock::now() - timePoint).count();//C++11
	double Time_ms = static_cast<double>(Time_ns) / 1000000;//C++11

	stream << Time_ms << " ms";
}

void tGnssReceiver::SetStrBaudrate(std::stringstream& stream, const std::chrono::time_point<tClock>& timePoint, std::size_t sizeBytes) const
{
	const auto Time_us = std::chrono::duration_cast<std::chrono::microseconds>(tClock::now() - timePoint).count();//C++11
	const double Time_ms = static_cast<double>(Time_us) / 1000;//C++11

	//stream << "; ";
	stream << sizeBytes << ", ";
	stream.setf(std::ios::fixed);
	const int SizeFract = 2;
	stream << std::setw(SizeFract + 5) << std::setprecision(SizeFract);
	stream << Time_ms << " ms, ";

	const double Time_s = Time_ms / 1000;
	if (Time_s > 0)
	{
		stream.setf(std::ios::fixed);
		stream << std::setw(SizeFract + 5) << std::setprecision(SizeFract);
		stream << (static_cast<double>(sizeBytes) * 8 / Time_s) << " bps";
	}
	else
	{
		stream << "n/a";
	}
}

void tGnssReceiver::ChangeState(tState* state)
{
	tState* Prev = m_pState;

	m_pState = state;

	delete Prev;
}

}
