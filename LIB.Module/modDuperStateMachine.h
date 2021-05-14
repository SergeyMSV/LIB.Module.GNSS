///////////////////////////////////////////////////////////////////////////////////////////////////
// modDuperStateMachine.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 01 14  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <modFiniteStateMachine_Attribute.h>

#include <utilsLog.h>
#include <utilsPacketNMEA.h>
#include <utilsPatternState.h>

#include <mutex>

namespace mod
{

typedef utils::packet::tPacket<utils::packet_NMEA::tFormatNMEA, utils::packet_NMEA::tPayloadCommon> tPacketNMEA;

class tFiniteStateMachine :public utils::pattern_State::tObjectState
{
	class tState :public utils::pattern_State::tObjectState::tStateBase
	{
	protected:
		int m_Counter = 0;

	public:
		tState(tObjectState* obj);

		virtual void operator()() { }

		virtual bool Start() { return false; }
		virtual bool Halt();

		virtual tFiniteStateMachineStatus GetStatus() = 0;
	};

	class tStateError :public tState
	{
	public:
		tStateError(tObjectState* obj, const std::string& value);

		void operator()() override;

		tFiniteStateMachineStatus GetStatus() override { return tFiniteStateMachineStatus::Error; }
	};

	class tStateHalt :public tState
	{
	public:
		tStateHalt(tObjectState* obj, const std::string& value);

		bool Start() override { return false; }
		bool Halt() override { return true; }

		tFiniteStateMachineStatus GetStatus() override { return tFiniteStateMachineStatus::Halted; }
	};

	class tStateOperation :public tState
	{
	public:
		tStateOperation(tObjectState* obj, const std::string& value);
		
		void operator()() override;

		tFiniteStateMachineStatus GetStatus() override { return tFiniteStateMachineStatus::Operation; }
	};

	class tStateStart :public tState
	{

	public:
		tStateStart(tObjectState* obj, const std::string& value);

		void operator()() override;

		tFiniteStateMachineStatus GetStatus() override { return tFiniteStateMachineStatus::Init; }
	};

	class tStateStop :public tState
	{
	public:
		tStateStop(tObjectState* obj, const std::string& value);

		void operator()() override;

		bool Start() override { return false; }
		bool Halt() override { return true; }

		tFiniteStateMachineStatus GetStatus() override { return tFiniteStateMachineStatus::Deinit; }
	};

	mutable std::mutex m_Mtx;

	utils::tLog* m_pLog = nullptr;

	tFiniteStateMachineSettings m_Settings;
	tFiniteStateMachineProperty m_Property;

	tFiniteStateMachineDataSet* m_pDataSet = nullptr;

public:
	tFiniteStateMachine() = delete;
	explicit tFiniteStateMachine(utils::tLog* log, const tFiniteStateMachineSettings& settings, tFiniteStateMachineDataSet* dataSet);
	tFiniteStateMachine(const tFiniteStateMachine&) = delete;
	tFiniteStateMachine(tFiniteStateMachine&&) = delete;

	tFiniteStateMachineError operator()();

	void Start();
	void Halt();

	tFiniteStateMachineStatus GetStatus();

	tFiniteStateMachineSettings GetSettings();
	void SetSettings(const tFiniteStateMachineSettings& settings);

	//virtual void OnChanged(const tFiniteStateMachineProperty& value) = 0;
	//virtual void OnFailed(const tFiniteStateMachineError& value) = 0;

	//virtual void OnStateStart() = 0;
	//virtual void OnStateResponse() = 0;
	//virtual void OnStateReady() = 0;
	//virtual void OnStateHalt() = 0;
	//virtual void OnStateRestart() = 0;

	//virtual void OnGGA(GnssMTK::PacketNMEA::tMsgGGA value) { }
	//virtual void OnGSV(GnssMTK::PacketNMEA::tMsgGSV value) { }
	//virtual void OnRMC(GnssMTK::PacketNMEA::tMsgRMC value) { }

//protected:
//	virtual void Board_PowerSupply(bool state) = 0;
//	virtual void Board_Reset(bool state) = 0;
//
//	virtual bool Board_Send(std::vector<char>& data) = 0;
//	void Board_OnReceived(std::vector<char>& data);
};

}
