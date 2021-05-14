#include "modGnssTelitSC872A.h"

namespace mod
{

tGnssTelitSC872A::tState::tState(tGnssTelitSC872A* obj)
	:m_pObj(obj)
{

}

bool tGnssTelitSC872A::tState::Halt()
{
	ChangeState(new tStateStop(m_pObj, "halt"));
	return true;
}

}
