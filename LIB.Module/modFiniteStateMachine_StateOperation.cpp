#include "modFiniteStateMachine.h"

//#include <utilsPacketNMEA.h>
//#include <utilsPacketNMEAPayload.h>

#include <chrono>
#include <thread>

namespace mod
{

tFiniteStateMachine::tStateOperation::tStateOperation(tObjectState* obj, const std::string& value)
	:tState(obj)
{
	GetObject<tFiniteStateMachine>()->m_pLog->WriteLine(true, utils::tLogColour::Default, "tStateOperation: %s", value.c_str());

	GetObject<tFiniteStateMachine>()->m_pDataSet->SetDataValue1("tState-Operation");
}

void tFiniteStateMachine::tStateOperation::operator()()
{
	if (++m_Counter < 10)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		//std::this_thread::sleep_for(std::chrono::duration<unsigned long, std::milli>(1000));
	}
	else
	{
		GetObject<tFiniteStateMachine>()->m_pLog->WriteLine();

		ChangeState(new tStateError(GetObject<utils::pattern_State::tObjectState>(), "lalala"));
		//ChangeState(new tStateStop(GetObject<utils::pattern_State::tObjectState>(), "lalala"));
		return;
	}

	std::string Data = "$GNRMC,090210.000,A,5539.564975,N,03732.411956,E,0.03,274.40,120517,,,A*71\xd\xa";

	utils::tVectorUInt8 DataVector(Data.cbegin(), Data.cend());

	tPacketNMEA Packet;

	if (tPacketNMEA::Find(DataVector, Packet))
	{
		std::vector<std::string> PacketData = Packet.GetPayload();

		struct DataSet1
		{
			double A = 0;
			double B = 0;
		}ds1;

		auto Handle1 = [&ds1](const std::vector<std::string>& packetData)->bool
		{
			if (packetData.size() == 13 && packetData[0] == "GNRMC")
			{
				if (packetData[9].size() > 0)
				{
					//utils::packet_NMEA::Payload::tDate Date(packetData[9]);
					ds1.A = 123.02;
				}
				else
				{
					ds1.A = 1;
				}

				return true;
			}

			return false;
		};

		Handle1(PacketData);

		//auto f = []<typename ...Ts>(Ts && ...ts) {
		//	return foo(std::forward<Ts>(ts)...);
		//};
		//auto glambda = []<class T>(T a, auto && b) { return a < b; };


		GetObject<tFiniteStateMachine>()->m_pLog->Write(true, utils::tLogColour::LightYellow, "");

		for (std::string i : PacketData)
		{
			GetObject<tFiniteStateMachine>()->m_pLog->Write(false, utils::tLogColour::LightYellow, i + " ");
		}

		GetObject<tFiniteStateMachine>()->m_pLog->WriteLine();
	}
	else
	{
		GetObject<tFiniteStateMachine>()->m_pLog->Write(false, utils::tLogColour::LightRed, "o");
	}
}

}

