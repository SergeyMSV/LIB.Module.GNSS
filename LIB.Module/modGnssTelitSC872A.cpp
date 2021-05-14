#include "modGnssTelitSC872A.h"

#include <iostream>

namespace mod
{

tGnssTelitSC872A::tGnssTelitSC872A(utils::tLog* log, const tGnssTelitSC872ASettings& settings, bool start)
	:m_pLog(log)
{
	m_Control_Operation = start;

	if (m_Control_Operation)
	{
		ChangeState(new tStateStart(this, "the very start"));
		return;
	}
	else
	{
		ChangeState(new tStateHalt(this, "the very start"));
		return;
	}
}

tGnssError tGnssTelitSC872A::operator()()
{
	//std::lock_guard<std::mutex> Lock(m_Mtx);
	while ((*m_pState)());

	return tGnssError::OK;
}

void tGnssTelitSC872A::Start()
{
	m_Control_Operation = true;
}

void tGnssTelitSC872A::Restart()
{
	m_Control_Restart = true;
}

void tGnssTelitSC872A::Halt()
{
	m_Control_Operation = false;
}

void tGnssTelitSC872A::Exit()
{
	m_Control_Exit = true;
	m_Control_Operation = false;
}

tGnssStatus tGnssTelitSC872A::GetStatus()
{
	//std::lock_guard<std::mutex> Lock(m_Mtx);

	return m_pState->GetStatus();
}

tGnssTelitSC872ASettings tGnssTelitSC872A::GetSettings()
{
	std::lock_guard<std::mutex> Lock(m_MtxSettings);

	return m_Settings;
}

void tGnssTelitSC872A::SetSettings(const tGnssTelitSC872ASettings& settings)
{
	std::lock_guard<std::mutex> Lock(m_MtxSettings);

	m_Settings = settings;

	//[TBD] - put - need restart is needed....
}

void tGnssTelitSC872A::Board_OnReceived(utils::tVectorUInt8& data)
{
	std::lock_guard<std::mutex> Lock(m_MtxReceivedData);

	m_ReceivedData.push(data);
}

bool tGnssTelitSC872A::IsReceivedData()
{
	std::lock_guard<std::mutex> Lock(m_MtxReceivedData);

	return m_ReceivedData.size() > 0;
}

utils::tVectorUInt8 tGnssTelitSC872A::GetReceivedDataChunk()
{
	std::lock_guard<std::mutex> Lock(m_MtxReceivedData);

	utils::tVectorUInt8 Data(std::forward<utils::tVectorUInt8>(m_ReceivedData.front()));

	m_ReceivedData.pop();

	return Data;
}

void tGnssTelitSC872A::ClearReceivedData()
{
	while (!m_ReceivedData.empty())
	{
		m_ReceivedData.pop();
	}
}

void tGnssTelitSC872A::SetStrTimePeriod(std::stringstream& stream, const std::chrono::time_point<tClock>& timePoint) const
{
	auto Time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tClock::now() - timePoint).count();//C++11
	double Time_ms = static_cast<double>(Time_ns) / 1000000;//C++11

	stream << Time_ms << " ms";
}

void tGnssTelitSC872A::SetStrBaudrate(std::stringstream& stream, const std::chrono::time_point<tClock>& timePoint, std::size_t sizeBytes) const
{
	auto Time_us = std::chrono::duration_cast<std::chrono::microseconds>(tClock::now() - timePoint).count();//C++11
	double Time_ms = static_cast<double>(Time_us) / 1000;//C++11

	//stream << "; ";
	stream << sizeBytes << ", ";
	stream.setf(std::ios::fixed);
	int SizeFract = 2;
	stream << std::setw(4 + SizeFract + 1) << std::setprecision(SizeFract);
	stream << Time_ms << " ms, ";

	double Time_s = Time_ms / 1000;
	if (Time_s > 0)
	{
		stream.setf(std::ios::fixed);
		SizeFract = 2;
		stream << std::setw(4 + SizeFract + 1) << std::setprecision(SizeFract);
		stream << (static_cast<double>(sizeBytes) * 8 / Time_s) << " bps";
	}
	else
	{
		stream << "n/a";
	}
}

void tGnssTelitSC872A::ChangeState(tState* state)
{
	tState* Prev = m_pState;

	m_pState = state;

	delete Prev;
}

}
