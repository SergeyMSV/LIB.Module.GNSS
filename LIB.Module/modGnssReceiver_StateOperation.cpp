#include "modGnssReceiver.h"

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

	if (m_SettingsNMEA.IsWrong())
	{
		ChangeState(new tStateError(m_pObj, "operation: config is wrong"));
		return true;
	}

	auto Time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(tClock::now() - m_StartTime).count();//C++11
	double Time_us = static_cast<double>(Time_ns) / 1000;
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

template<class T>
void SetDataSetNMEA_RMC(const T& packRMC, tGnssDataSet& dataSet)
{
	if (!packRMC.Date.Empty())
	{
		dataSet.Year = packRMC.Date.Year;
		dataSet.Month = packRMC.Date.Month;
		dataSet.Day = packRMC.Date.Day;
	}

	if (!packRMC.Time.Empty())
	{
		dataSet.Hour = packRMC.Time.Hour;
		dataSet.Minute = packRMC.Time.Minute;
		dataSet.Second = packRMC.Time.Second;
	}

	dataSet.Check_DateTime = !packRMC.Date.Empty() && !packRMC.Time.Empty();

	SetParam(dataSet.Valid, packRMC.Valid, dataSet.Check_Position);
	SetParam(dataSet.Latitude, packRMC.Latitude, dataSet.Check_Position);
	SetParam(dataSet.Longitude, packRMC.Longitude, dataSet.Check_Position);
	SetParam(dataSet.Speed, packRMC.Speed, dataSet.Check_Position);
	SetParam(dataSet.Course, packRMC.Course, dataSet.Check_Position);

	dataSet.ModeIndicator = packRMC.ModeIndicator.ToString();
}

bool tGnssReceiver::tStateOperation::OnReceived(const tPacketNMEA_Template& value)
{
	const std::string Payload = value.GetPayloadValue();

	const tPacketNMEA::payload_type PacketData(Payload.cbegin(), Payload.cend());

	tGnssReceiverPacketLog Log(m_pObj->m_pLog, m_StartTime);
	Log.OnReceived(Payload.size());

	//[TBD] it's possible to use std::map instead of following statement... just reg in map new handler...
	if (tMsgGSV::Try(PacketData.Value))
	{
		tMsgGSV Msg(PacketData.Value);

		for (auto& i : Msg.Satellite)
		{
			m_DataSet.Satellite.push_back(std::forward<tGNSS_Satellite>(i));
		}

		Log.OnReceived(PacketData.Value[0], Msg);
	}
	else if (m_SettingsNMEA.LatLonFract == 4 && tMsgRMC_Ft4::Try(PacketData.Value))
	{
		tMsgRMC_Ft4 Msg(PacketData.Value);
		SetDataSetNMEA_RMC(Msg, m_DataSet);

		Log.OnReceived(PacketData.Value[0], Msg);
	}
	else if (m_SettingsNMEA.LatLonFract == 6 && tMsgRMC_Ft6::Try(PacketData.Value))
	{
		tMsgRMC_Ft6 Msg(PacketData.Value);
		SetDataSetNMEA_RMC(Msg, m_DataSet);

		Log.OnReceived(PacketData.Value[0], Msg);
	}
	else if (utils::packet_NMEA::tPayloadPTWS_JAM_SIGNAL_VAL::Try(PacketData.Value))
	{
		utils::packet_NMEA::tPayloadPTWS_JAM_SIGNAL_VAL Msg(PacketData.Value);
		m_DataSet.Jamming.insert({ Msg.Index.Value, Msg.Frequency.Value });

		Log.OnReceived(PacketData.Value[0], Msg);
	}
	else
	{
		Log.OnReceived(PacketData.Value[0]);
	}

	if (!m_SettingsNMEA.MsgLast.empty() && PacketData.size() > 0 && PacketData.Value[0].find(m_SettingsNMEA.MsgLast) != std::string::npos)
	{
		m_pObj->OnChanged(m_DataSet);

		ChangeState(new tStateOperation(m_pObj));
		return true;
	}
	return false;
}

}
