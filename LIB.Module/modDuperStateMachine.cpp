#include "modDuperStateMachine.h"

#include <iostream>

namespace mod
{

tFiniteStateMachine::tFiniteStateMachine(utils::tLog* log, const tFiniteStateMachineSettings& settings, tFiniteStateMachineDataSet* dataSet)
	:m_pLog(log), m_pDataSet(dataSet)
{
	ChangeState(new tStateStart(this, "the very start"));
	return;
}

tFiniteStateMachineError tFiniteStateMachine::operator()()
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	(*GetState<tState>())();

	return tFiniteStateMachineError::OK;
}

void tFiniteStateMachine::Start()
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	ChangeState(new tStateStart(this, "the very start"));
	return;
}

void tFiniteStateMachine::Halt()
{
	GetState<tState>()->Halt();
	return;
}

tFiniteStateMachineStatus tFiniteStateMachine::GetStatus()
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	return GetState<tState>()->GetStatus();
}

tFiniteStateMachineSettings tFiniteStateMachine::GetSettings()
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	return m_Settings;
}

void tFiniteStateMachine::SetSettings(const tFiniteStateMachineSettings& settings)
{
	std::lock_guard<std::mutex> Lock(m_Mtx);

	m_Settings = settings;
}

}
