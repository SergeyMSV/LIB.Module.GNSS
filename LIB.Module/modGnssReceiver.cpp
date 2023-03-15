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

tGnssReceiver::tDevStatus tGnssReceiver::GetStatus() const
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

void tGnssReceiver::ChangeState(tState* state)
{
	tState* Prev = m_pState;
	m_pState = state;
	delete Prev;
}

tGnssReceiverPacketLog::tGnssReceiverPacketLog(utils::tLog* log, std::chrono::time_point<tClock>& startTime)
	:m_pLog(log), m_StartTime(startTime)
{

}

tGnssReceiverPacketLog::~tGnssReceiverPacketLog()
{

}

void tGnssReceiverPacketLog::OnReceived(size_t size)
{
	m_MsgTime = "; ";
	m_MsgTime += GetBaudrateString(m_StartTime, size);
	m_StartTime = tClock::now();
}

void tGnssReceiverPacketLog::OnReceived(const std::string& id, const tMsgGSV& msg)
{
	Write(utils::tLogColour::LightMagenta, id + " " + msg.MsgQty.ToString() + " " + msg.MsgNum.ToString() + " " + msg.SatelliteQty.ToString());
}

void tGnssReceiverPacketLog::OnReceived(const std::string& id, const tMsgRMC_Ft4& msg)
{
	Write(utils::tLogColour::LightMagenta, id + " " + msg.Date.ToString() + " " + msg.Time.ToString());
}

void tGnssReceiverPacketLog::OnReceived(const std::string& id, const tMsgRMC_Ft6& msg)
{
	Write(utils::tLogColour::LightMagenta, id + " " + msg.Date.ToString() + " " + msg.Time.ToString());
}

void tGnssReceiverPacketLog::OnReceived(const std::string& id, const utils::packet_NMEA::tPayloadPTWS_JAM_SIGNAL_VAL& msg)
{
	Write(utils::tLogColour::LightMagenta, id + " " + msg.Index.ToString() + " " + msg.Frequency.ToString());
}

void tGnssReceiverPacketLog::OnReceived(const std::string& id)
{
	Write(utils::tLogColour::Yellow, id + " - NOT PARSED");
}

void tGnssReceiverPacketLog::Write(utils::tLogColour colour, std::string msg)
{
	m_MsgTime += "; ";
	m_MsgTime += GetTimePeriodString(m_StartTime);
	m_StartTime = tClock::now();

	int Align = 26 - static_cast<int>(msg.size());
	if (Align > 0)
		msg += std::string(Align, ' ');
	m_pLog->Write(true, colour, msg);
	m_pLog->WriteLine(false, utils::tLogColour::Default, m_MsgTime);
}

std::string tGnssReceiverPacketLog::GetTimePeriodString(const std::chrono::time_point<tClock>& timePoint) const
{
	auto Time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tClock::now() - timePoint).count();
	double Time_ms = static_cast<double>(Time_ns) / 1000000;
	return std::to_string(Time_ms) + " ms";
}

std::string tGnssReceiverPacketLog::GetBaudrateString(const std::chrono::time_point<tClock>& timePoint, std::size_t sizeBytes) const
{
	auto Time_us = std::chrono::duration_cast<std::chrono::microseconds>(tClock::now() - timePoint).count();
	double Time_ms = static_cast<double>(Time_us) / 1000;

	std::stringstream Str;
	Str << sizeBytes << ", ";
	Str.setf(std::ios::fixed);
	const int SizeFract = 2;
	Str << std::setw(SizeFract + 5) << std::setprecision(SizeFract);
	Str << Time_ms << " ms, ";

	//double Time_s = Time_ms / 1000;
	//if (Time_s > 0)
	//{
	//	Str.setf(std::ios::fixed);
	//	Str << std::setw(SizeFract + 8) << std::setprecision(SizeFract);
	//	Str << (static_cast<double>(sizeBytes) * 8 / Time_s) << " bps";
	//}
	//else
	//{
	//	Str << "n/a";
	//}

	return Str.str();
}

}
