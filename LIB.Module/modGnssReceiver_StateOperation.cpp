#include "modGnssReceiver.h"

#include <utilsPacketNMEA.h>
#include <utilsPacketNMEAPayload.h>

#include <chrono>
#include <thread>

namespace mod
{

tGnssReceiver::tStateOperation::tStateOperation(tGnssReceiver* obj)
	:tState(obj, "StateOperation"), m_StartTime(tClock::now()), m_SettingsNMEA(m_pObj->GetSettingsNMEA())
{
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateOperation");
}

bool tGnssReceiver::tStateOperation::SetUserTaskScript(const std::string& taskScriptID)
{
	return SetTaskScript(taskScriptID, true);
}

void tGnssReceiver::tStateOperation::OnTaskScriptDone()
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightGreen, "OnTaskScriptDone");
}

void tGnssReceiver::tStateOperation::OnTaskScriptFailed(const std::string& msg)
{
	m_pObj->m_pLog->WriteLine(false, utils::tLogColour::LightRed, "OnTaskScriptFailed: " + msg);
}

bool tGnssReceiver::tStateOperation::Go()
{
	if (!m_pObj->IsControlOperation())
	{
		ChangeState(new tStateStop(m_pObj, "operation"));
		return true;
	}

	const auto Time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tClock::now() - m_StartTime).count();//C++11
	const double Time_us = static_cast<double>(Time_ns) / 1000;//C++11
	if (Time_us > m_SettingsNMEA.PeriodMax)
	{
		ChangeState(new tStateError(m_pObj, "operation: no data within PeriodMAX"));
		return true;
	}
	
	return true;
}

template<class T, class U>
void SetParam(T& valDst, U valSrc, bool& check)
{
	if (valSrc.Empty())
	{
		check = false;
	}
	else
	{
		valDst = valSrc.Value;
	}
}

void tGnssReceiver::tStateOperation::OnReceived(const tPacketNMEA_Template& value)
{
	std::string Payload = value.GetPayload();

	tPacketNMEA::payload_type PacketData(Payload.cbegin(), Payload.cend());

	std::stringstream StrTime;

	StrTime << "; ";
	m_pObj->SetStrBaudrate(StrTime, m_StartTime, Payload.size());
	m_StartTime = tClock::now();

	//[TBD] it's possible to use std::map instead of following statement... just reg in map new handler...
	if (tMsgGSV::Try(PacketData.Data))
	{
		tMsgGSV Msg(PacketData.Data);

		for (auto& i : Msg.Satellite)//C++11
		{
			m_DataSet.Satellite.push_back(std::forward<tGNSS_Satellite>(i));
		}

		StrTime << "; ";
		m_pObj->SetStrTimePeriod(StrTime, m_StartTime);
		m_StartTime = tClock::now();

		m_pObj->m_pLog->Write(true, utils::tLogColour::LightMagenta, PacketData.Data[0] + " " + Msg.MsgQty.ToString() + " " + Msg.MsgNum.ToString() + " " + Msg.SatelliteQty.ToString());
		m_pObj->m_pLog->WriteLine(false, utils::tLogColour::Default, StrTime.str());
	}
	else if (tMsgRMC::Try(PacketData.Data))
	{
		tMsgRMC Msg(PacketData.Data);

		if (!Msg.Date.Empty())
		{
			m_DataSet.Year = Msg.Date.Year;
			m_DataSet.Month = Msg.Date.Month;
			m_DataSet.Day = Msg.Date.Day;
		}

		if (!Msg.Time.Empty())
		{
			m_DataSet.Hour = Msg.Time.Hour;
			m_DataSet.Minute = Msg.Time.Minute;
			m_DataSet.Second = Msg.Time.Second;
		}

		m_DataSet.Check_DateTime = !Msg.Date.Empty() && !Msg.Time.Empty();

		SetParam(m_DataSet.Valid, Msg.Valid, m_DataSet.Check_Position);
		SetParam(m_DataSet.Latitude, Msg.Latitude, m_DataSet.Check_Position);
		SetParam(m_DataSet.Longitude, Msg.Longitude, m_DataSet.Check_Position);
		SetParam(m_DataSet.Speed, Msg.Speed, m_DataSet.Check_Position);
		SetParam(m_DataSet.Course, Msg.Course, m_DataSet.Check_Position);

		StrTime << "; ";
		m_pObj->SetStrTimePeriod(StrTime, m_StartTime);
		m_StartTime = tClock::now();

		m_pObj->m_pLog->Write(true, utils::tLogColour::LightMagenta, PacketData.Data[0] + " " + Msg.Date.ToString() + " " + Msg.Time.ToString());
		m_pObj->m_pLog->WriteLine(false, utils::tLogColour::Default, StrTime.str());
	}
	else
	{
		StrTime << "; ";
		m_pObj->SetStrTimePeriod(StrTime, m_StartTime);
		m_StartTime = tClock::now();

		m_pObj->m_pLog->Write(true, utils::tLogColour::Yellow, PacketData.Data[0]);
		m_pObj->m_pLog->WriteLine(false, utils::tLogColour::Default, StrTime.str());
	}

	if (!m_SettingsNMEA.MsgLast.empty() && PacketData.Data.size() > 0 && PacketData.Data[0].find(m_SettingsNMEA.MsgLast) != std::string::npos)
	{
		m_pObj->OnChanged(m_DataSet);

		ChangeState(new tStateOperation(m_pObj));
		return;
	}
}

}
