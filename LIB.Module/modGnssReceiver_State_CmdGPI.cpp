#include "modGnssReceiver.h"

namespace mod
{

tGnssReceiver::tState::tCmdGPI::tCmdGPI(tState* obj, std::unique_ptr<tGnssTaskScriptCmd> cmd)
	:tCmd(obj, std::move(cmd))
{

}

bool tGnssReceiver::tState::tCmdGPI::operator()()
{
	return m_pObjState->OnCmdDone();
}

}
