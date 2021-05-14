#include "modDuperStateMachine.h"

namespace mod
{

tFiniteStateMachine::tState::tState(tObjectState* obj)
	:tStateBase(obj)
{

}

bool tFiniteStateMachine::tState::Halt()
{
	ChangeState(new tStateStop(GetObject<utils::pattern_State::tObjectState>(), "halt"));
	return true;
}

}
