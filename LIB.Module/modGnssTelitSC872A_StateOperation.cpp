#include "modGnssTelitSC872A.h"

#include <utilsPacketNMEA.h>
#include <utilsPacketNMEAPayload.h>

#include <chrono>
#include <thread>

namespace mod
{

tGnssTelitSC872A::tStateOperation::tStateOperation(tGnssTelitSC872A* obj, const std::chrono::time_point<tClock>& startTime, bool noData)
	:tState(obj), m_StartTime(startTime), m_NoData(noData)
{
	m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateOperation");
}

tGnssTelitSC872A::tStateOperation::tStateOperation(tGnssTelitSC872A* obj, const std::chrono::time_point<tClock>& startTime)
	: tStateOperation(obj, startTime, false)
{
}

tGnssTelitSC872A::tStateOperation::tStateOperation(tGnssTelitSC872A* obj)
	:tStateOperation(obj, tClock::now(), false)
{

}

template<class T, class U>
void SetParam(T& valDst, U valSrc, bool& check)
{
	if (valSrc.Absent)
	{
		check = false;
	}
	else
	{
		valDst = valSrc.Value;
	}
}

bool tGnssTelitSC872A::tStateOperation::operator()()
{
	if (!m_pObj->IsControlOperation())
	{
		ChangeState(new tStateStop(m_pObj, "operation"));
		return true;
	}

	if (m_pObj->IsReceivedData())
	{
		if (m_NoData)
		{
			m_NoData = false;

			std::stringstream StrTime;
			m_pObj->SetStrTimePeriod(StrTime, m_StartTime);
			m_StartTime = tClock::now();

			m_pObj->m_pLog->WriteLine(true, utils::tLogColour::Default, StrTime.str());
		}

		utils::tVectorUInt8 DataChunk = m_pObj->GetReceivedDataChunk();

		m_ReceivedData.insert(m_ReceivedData.end(), DataChunk.cbegin(), DataChunk.cend());//C++14

		while (m_pObj->IsControlOperation())
		{
			tPacketNMEA Packet;

			std::size_t PacketSize = tPacketNMEA::Find(m_ReceivedData, Packet);

			if (PacketSize)
			{
				std::stringstream StrTime;

				StrTime << "; ";
				m_pObj->SetStrBaudrate(StrTime, m_StartTime, PacketSize);
				m_StartTime = tClock::now();

				utils::packet_NMEA::tPayloadCommon::value_type PacketData = Packet.GetPayload();

				if (tMsgGSV::Try(PacketData))
				{
					tMsgGSV Msg(PacketData);
					

					for (auto& i : Msg.Satellite)//C++11
					{
						m_DataSet.Satellite.push_back(std::forward<tGNSS_Satellite>(i));
					}

					StrTime << "; ";
					m_pObj->SetStrTimePeriod(StrTime, m_StartTime);
					m_StartTime = tClock::now();

					m_pObj->m_pLog->Write(true, utils::tLogColour::LightMagenta, PacketData[0] + " " + Msg.MsgQty.ToString() + " " + Msg.MsgNum.ToString() + " " + Msg.SatelliteQty.ToString());
					m_pObj->m_pLog->WriteLine(false, utils::tLogColour::Default, StrTime.str());
				}
				else if (tMsgRMC::Try(PacketData))
				{
					tMsgRMC Msg(PacketData);

					if (!Msg.Date.Absent)
					{
						m_DataSet.Year = Msg.Date.Year;
						m_DataSet.Month = Msg.Date.Month;
						m_DataSet.Day = Msg.Date.Day;
					}

					if (!Msg.Time.Absent)
					{
						m_DataSet.Hour = Msg.Time.Hour;
						m_DataSet.Minute = Msg.Time.Minute;
						m_DataSet.Second = Msg.Time.Second;
					}

					m_DataSet.Check_DateTime = !Msg.Date.Absent && !Msg.Time.Absent;

					SetParam(m_DataSet.Valid, Msg.Valid, m_DataSet.Check_Position);
					SetParam(m_DataSet.Latitude, Msg.Latitude, m_DataSet.Check_Position);
					SetParam(m_DataSet.Longitude, Msg.Longitude, m_DataSet.Check_Position);
					SetParam(m_DataSet.Speed, Msg.Speed, m_DataSet.Check_Position);
					SetParam(m_DataSet.Course, Msg.Course, m_DataSet.Check_Position);

					StrTime << "; ";
					m_pObj->SetStrTimePeriod(StrTime, m_StartTime);
					m_StartTime = tClock::now();

					m_pObj->m_pLog->Write(true, utils::tLogColour::LightMagenta, PacketData[0] + " " + Msg.Date.ToString() + " " + Msg.Time.ToString());
					m_pObj->m_pLog->WriteLine(false, utils::tLogColour::Default, StrTime.str());

					m_pObj->OnChanged(m_DataSet);//TEST
					m_DataSet = tGnssDataSet();//TEST
				}
				else
				{
					StrTime << "; ";
					m_pObj->SetStrTimePeriod(StrTime, m_StartTime);
					m_StartTime = tClock::now();

					m_pObj->m_pLog->Write(true, utils::tLogColour::Yellow, PacketData[0]);
					m_pObj->m_pLog->WriteLine(false, utils::tLogColour::Default, StrTime.str());

					m_DataSet.Satellite.clear();//[TEST]
				}

				m_StartTime = tClock::now();
			}
			else
			{
				break;
			}
		}

		if (m_ReceivedData.size() == 0)
		{
			m_StartTime = tClock::now();
		}

		if (m_ReceivedData.size() > 1024)//[#]
		{
			ChangeState(new tStateError(m_pObj, "buffer overrun"));
			return true;
		}		
	}
	else
	{
		if (m_ReceivedData.size() == 0 && !m_NoData)
		{
			m_NoData = true;

			m_StartTime = tClock::now();
		}

		if (m_NoData)
		{
			auto Time_us = std::chrono::duration_cast<std::chrono::microseconds>(tClock::now() - m_StartTime).count();//C++11
			if (Time_us > 100000)//100ms
			{
				ChangeState(new tStateOperationNoData(m_pObj, m_StartTime));
				return true;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		//if (!m_ReceivedData.size() && ++m_TimeCounter > 3)//if pause more than 300 ms - reboot tStateOperation
		//{
		//	ChangeState(new tStateOperation(m_pObj, "transaction"));
		//	return true;
		//}
	}

	return true;
}

}
//[904564.367289]182539.000 A 55.924115 37.751517 00.000000 23
//[904564.398420]GNVTG; 39, 29 ms, 10758.6 bps
//
//[904564.750419]GNGGA; 74, 351 ms, 1686.61 bps
//[904564.813433]GPGSA; 62, 62 ms, 8000 bps
//[904564.861628]GLGSA; 52, 47 ms, 8851.06 bps
//[904564.943497]GPGSV 4 1 13; 70, 80 ms, 7000 bps
//[904565.006606]GPGSV 4 2 13; 70, 62 ms, 9032.26 bps
//[904565.085678]GPGSV 4 3 13; 70, 78 ms, 7179.49 bps
//[904565.118262]GPGSV 4 4 13; 31, 31 ms, 8000 bps
//[904565.183581]GLGSV 3 1 10; 66, 64 ms, 8250 bps
//[904565.261211]GLGSV 3 2 10; 70, 76 ms, 7368.42 bps
//[904565.310384]GLGSV 3 3 10; 42, 48 ms, 7000 bps
//[904565.374099]GNRMC 270320 182540.000; 71, 62 ms, 9161.29 bps
//[904565.376238]182540.000 A 55.924115 37.751517 00.000000 23
//[904565.421514]GNVTG; 38, 44 ms, 6909.09 bps
//
//[904565.742402]GNGGA; 74, 319 ms, 1855.8 bps
//[904565.805637]GPGSA; 62, 62 ms, 8000 bps
//[904565.870403]GLGSA; 52, 64 ms, 6500 bps
//[904565.933491]GPGSV 4 1 13; 70, 62 ms, 9032.26 bps
//[904566.014563]GPGSV 4 2 13; 70, 80 ms, 7000 bps
//[904566.093698]GPGSV 4 3 13; 70, 78 ms, 7179.49 bps
//[904566.125105]GPGSV 4 4 13; 31, 29 ms, 8551.72 bps
//[904566.192269]GLGSV 3 1 10; 66, 66 ms, 8000 bps
//[904566.269514]GLGSV 3 2 10; 70, 76 ms, 7368.42 bps
//[904566.302351]GLGSV 3 3 10; 42, 32 ms, 10500 bps
//[904566.382479]GNRMC 270320 182541.000; 72, 79 ms, 7291.14 bps
//[904566.384920]182541.000 A 55.924115 37.751517 00.000000 23
//[904566.430253]GNVTG; 39, 44 ms, 7090.91 bps
//
//[904566.736418]GNGGA; 74, 305 ms, 1940.98 bps
//[904566.814487]GPGSA; 62, 76 ms, 6526.32 bps
//[904566.860946]GLGSA; 52, 45 ms, 9244.44 bps
//[904566.944678]GPGSV 4 1 13; 70, 82 ms, 6829.27 bps
//[904567.006375]GPGSV 4 2 13; 70, 60 ms, 9333.33 bps
//[904567.085770]GPGSV 4 3 13; 70, 78 ms, 7179.49 bps
//[904567.119279]GPGSV 4 4 13; 31, 32 ms, 7750 bps
//[904567.181617]GLGSV 3 1 10; 66, 61 ms, 8655.74 bps
//[904567.262112]GLGSV 3 2 10; 70, 79 ms, 7088.61 bps
//[904567.309546]GLGSV 3 3 10; 42, 46 ms, 7304.35 bps
//[904567.373535]GNRMC 270320 182542.000; 71, 62 ms, 9161.29 bps
//[904567.376852]182542.000 A 55.924115 37.751517 00.000000 23
