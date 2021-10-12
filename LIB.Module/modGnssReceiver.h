///////////////////////////////////////////////////////////////////////////////////////////////////
// modGnssReceiver.h
//
// Standard ISO/IEC 114882, C++11
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2020 03 03  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "modGnss.h"

#include <utilsLog.h>
#include <utilsPacketNMEA.h>
#include <utilsPacketNMEAPayload.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>

namespace mod
{

typedef utils::packet::tPacket<utils::packet_NMEA::tFormatNMEA, utils::packet_NMEA::tPayloadCommon> tPacketNMEA;
typedef utils::packet::tPacket<utils::packet_NMEA::tFormatNMEA, utils::packet_NMEA::tPayloadString> tPacketNMEA_Template;
typedef utils::packet_NMEA::tPayloadRMC<13, 3, LIB_MOD_GNSS_LatitudeSizeFract, LIB_MOD_GNSS_LongitudeSizeFract> tMsgRMC;
typedef utils::packet_NMEA::tPayloadGSV tMsgGSV;

class tGnssReceiver
{
	using tClock = std::chrono::high_resolution_clock;//C++11

	class tStateError;

	class tState
	{
		friend class tStateError;

		class tCmd
		{
		protected:
			tState* m_pObjState = nullptr;

			std::unique_ptr<tGnssTaskScriptCmd> m_Cmd;

			std::chrono::time_point<tClock> m_StartTime;

		public:
			tCmd() = delete;
			tCmd(tState* objState, std::unique_ptr<tGnssTaskScriptCmd> cmd) :m_pObjState(objState), m_Cmd(std::move(cmd)) {}
			tCmd(const tCmd&) = delete;
			tCmd(tCmd&&) = delete;
			virtual ~tCmd() = default;

			tCmd& operator=(const tCmd&) = delete;
			tCmd& operator=(tCmd&&) = delete;

			virtual bool operator()() = 0;
			virtual bool OnReceived(const tPacketNMEA_Template& value) { return false; };
		};

		class tCmdGPI :public tCmd
		{
		public:
			tCmdGPI(tState* objState, std::unique_ptr<tGnssTaskScriptCmd> cmd);

			bool operator()() override;
		};

		class tCmdGPO :public tCmd
		{
			enum class tStep :std::uint8_t
			{
				SetGPO,
				Pause,
			};

			tStep m_Step = tStep::SetGPO;

			int m_WaitTime_us = 0;

		public:
			tCmdGPO(tState* objState, std::unique_ptr<tGnssTaskScriptCmd> cmd);

			bool operator()() override;
		};

		class tCmdREQ :public tCmd
		{
			enum class tStep :std::uint8_t
			{
				SendMsg,
				WaitRsp,
				PauseSet,
				PauseWait,
			};

			tStep m_Step = tStep::SendMsg;

			int m_WaitTime_us = 0;

		public:
			tCmdREQ(tState* objState, std::unique_ptr<tGnssTaskScriptCmd> cmd);

			bool operator()() override;
			bool OnReceived(const tPacketNMEA_Template& value) override;
		};

		tGnssTaskScript m_TaskScript;

		std::string m_OnCmdTaskScriptIDLast;

		utils::tVectorUInt8 m_ReceivedData;
		bool m_ReceivedData_Parsed = false;

		tCmd* m_pCmd = nullptr;

	protected:
		tGnssReceiver* m_pObj = nullptr;

	public:
		explicit tState(tGnssReceiver* obj);
		tState(tGnssReceiver* obj, const std::string& taskScriptID);
		virtual ~tState();

		bool operator()();

		virtual bool Start() { return false; }
		virtual bool Halt();

		virtual tGnssStatus GetStatus() = 0;

		virtual bool SetUserTaskScript(const std::string& taskScriptID) { return false; }

	private:
		void TaskScript();

		bool OnCmdDone();//ChangeState - if ChangeState is inside then return is true
		void OnCmdTaskScript(std::unique_ptr<tGnssTaskScriptCmd> cmd, const std::string& taskScriptID);

	protected:
		void ResetCmd();

		bool SetTaskScript(const std::string& taskScriptID, bool userTaskScript);

		virtual bool OnCmdFailed();//ChangeState - if ChangeState is inside then return is true

		virtual void OnTaskScriptDone() {};//ChangeState
		virtual void OnTaskScriptFailed(const std::string& msg) {};//ChangeState

		virtual bool Go() { return true; }//ChangeState
		virtual void OnReceived(const tPacketNMEA_Template& value);// {}//ChangeState

		void ChangeState(tState* state) { m_pObj->ChangeState(state); }
	};

	class tStateError :public tState
	{
	public:
		tStateError(tGnssReceiver* obj, const std::string& value);

		tGnssStatus GetStatus() override { return tGnssStatus::Error; }

		bool Halt() override { return false; }

	protected:
		bool OnCmdFailed() override;//ChangeState - if ChangeState is inside then return is true

		void OnTaskScriptDone() override;
		void OnTaskScriptFailed(const std::string& msg) override;
	};

	class tStateHalt :public tState
	{
		const bool m_Error = false;

	public:
		tStateHalt(tGnssReceiver* obj, const std::string& value);
		tStateHalt(tGnssReceiver* obj, const std::string& value, bool error);

		bool Start() override { return false; }
		bool Halt() override { return true; }

		tGnssStatus GetStatus() override { return tGnssStatus::Halted; }

	protected:
		bool Go() override;
	};

	class tStateOperation :public tState
	{
		const tGnssSettingsNMEA m_SettingsNMEA;

		std::chrono::time_point<tClock> m_StartTime;

		tGnssDataSet m_DataSet;

	public:
		explicit tStateOperation(tGnssReceiver* obj);

		tGnssStatus GetStatus() override { return tGnssStatus::Operation; }

		bool SetUserTaskScript(const std::string& taskScriptID) override;

	protected:
		void OnTaskScriptDone() override;
		void OnTaskScriptFailed(const std::string& msg) override;

		bool Go() override;
		void OnReceived(const tPacketNMEA_Template& value) override;
	};

	class tStateStop :public tState
	{
	public:
		tStateStop(tGnssReceiver* obj, const std::string& value);

		bool Start() override { return false; }
		bool Halt() override { return true; }

		tGnssStatus GetStatus() override { return tGnssStatus::Deinit; }

	protected:
		void OnTaskScriptDone() override;
		void OnTaskScriptFailed(const std::string& msg) override;
	};

	class tStateStart :public tState
	{
		bool m_NextState_Stop = false;

	public:
		tStateStart(tGnssReceiver* obj, const std::string& value);

		tGnssStatus GetStatus() override { return tGnssStatus::Init; }

	protected:
		void OnTaskScriptDone() override;
		void OnTaskScriptFailed(const std::string& msg) override;
	};

	utils::tLog* m_pLog = nullptr;

	//mutable std::mutex m_MtxState;
	tState* m_pState = nullptr;

	std::atomic_bool m_Control_Operation{ false };
	std::atomic_bool m_Control_Restart{ false };
	std::atomic_bool m_Control_Exit{ false };
	std::atomic_bool m_Control_ExitOnError{ false };

	mutable std::mutex m_MtxReceivedData;
	std::queue<utils::tVectorUInt8> m_ReceivedData;

	std::string m_LastErrorMsg;

public:
	tGnssReceiver() = delete;
	explicit tGnssReceiver(utils::tLog* log);
	tGnssReceiver(const tGnssReceiver&) = delete;
	tGnssReceiver(tGnssReceiver&&) = delete;
	virtual ~tGnssReceiver() {};// = 0;

	void operator()();

	void Start();
	void Start(bool exitOnError);
	void Restart();
	void Halt();
	void Exit();

	bool StartUserTaskScript(const std::string& taskScriptID);

	tGnssStatus GetStatus() const;
	std::string GetLastErrorMsg() const;

protected:
	virtual tGnssTaskScript GetTaskScript(const std::string& id, bool userTaskScript) = 0;
	virtual tGnssSettingsNMEA GetSettingsNMEA() = 0;

	virtual void OnChanged(const tGnssDataSet& value) = 0;
	//virtual void OnChanged(const tGnssReceiverProperty& value) = 0;
	//virtual void OnFailed(const tGnssReceiverError& value) = 0;

	//virtual void OnStateStart() = 0;
	//virtual void OnStateResponse() = 0;
	//virtual void OnStateReady() = 0;
	//virtual void OnStateHalt() = 0;
	//virtual void OnStateRestart() = 0;

	virtual void Board_PowerSupply(bool state) = 0;
	virtual void Board_Reset(bool state) = 0;

	virtual bool Board_Send(const utils::tVectorUInt8& data) = 0;
	void Board_OnReceived(utils::tVectorUInt8& data);

private:
	bool IsReceivedData() const;
	utils::tVectorUInt8 GetReceivedDataChunk();
	bool IsControlOperation() { return m_Control_Operation && !m_Control_Restart; }
	//bool IsControlStop() { return !m_Control_Operation && m_Control_Restart; }
	bool IsControlRestart() { return m_Control_Restart; }
	bool IsControlHalt() { return !m_Control_Operation; }

	void ClearReceivedData();

	void SetStrTimePeriod(std::stringstream& stream, const std::chrono::time_point<tClock>& timePoint) const;
	void SetStrBaudrate(std::stringstream& stream, const std::chrono::time_point<tClock>& timePoint, std::size_t sizeBytes) const;

	void ChangeState(tState* state);
};

}
