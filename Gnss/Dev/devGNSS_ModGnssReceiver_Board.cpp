#include "devGNSS.h"
#include "devSettings.h"

namespace dev
{

tGNSS::tModGnssReceiver::tBoard::tBoard(tModGnssReceiver* obj, boost::asio::io_context& io)
	:m_pObj(obj), tCommunication(io, g_Settings.SerialPort.ID, g_Settings.SerialPort.BR)
{

}

tGNSS::tModGnssReceiver::tBoard::~tBoard()
{

}

void tGNSS::tModGnssReceiver::tBoard::OnReceived(utils::tVectorUInt8& data)
{
	m_pObj->OnReceived(data);
}

}
