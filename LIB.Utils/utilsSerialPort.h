#include <mutex>
#include <queue>
#include <vector>

#include <boost/asio.hpp>

namespace utils
{
	namespace serial_port
	{
using tCharSize = boost::asio::serial_port_base::character_size;
using tStopBits = boost::asio::serial_port_base::stop_bits::type;
using tParity = boost::asio::serial_port_base::parity::type;
using tFlowControl = boost::asio::serial_port_base::flow_control::type;

template<int DataSizeRecv = 1024>
class tSerialPort
{
    boost::asio::serial_port m_Port;

	std::uint8_t m_Data[DataSizeRecv];

	mutable std::mutex m_Mtx;//[TBD] maybe scoped_mutex
	std::queue<std::vector<std::uint8_t>> m_DataSent;

public:
	tSerialPort(boost::asio::io_context& io, const std::string& id, std::uint32_t portBR, tCharSize charSize, tStopBits stopBits, tParity parity, tFlowControl flowControl)
        : m_Port(io)
    {
        m_Port.open(id);

        if (m_Port.is_open())
        {
            m_Port.set_option(boost::asio::serial_port_base::baud_rate(portBR));
            m_Port.set_option(boost::asio::serial_port_base::character_size(charSize));
            m_Port.set_option(boost::asio::serial_port_base::stop_bits(stopBits));
            m_Port.set_option(boost::asio::serial_port_base::parity(parity));
            m_Port.set_option(boost::asio::serial_port_base::flow_control(flowControl));
        }

        Receive();
    }

	tSerialPort(boost::asio::io_context& io, const std::string& id, std::uint32_t portBR)
		:tSerialPort(io, id, portBR, tCharSize(8), tStopBits::one, tParity::none, tFlowControl::none)
	{
	}

	bool Send(const std::vector<std::uint8_t>& data)
	{
		std::lock_guard<std::mutex> Lock(m_Mtx);

		bool DataSentEmpty = m_DataSent.empty();//[TBD]it shall limit incoming data for sending

		m_DataSent.push(data);

		if (DataSentEmpty)
		{
			Send();
		}

		return true;
	}

private:
    void Receive()
    {
        m_Port.async_read_some(boost::asio::buffer(m_Data, sizeof(m_Data)),
            [this](boost::system::error_code ec, std::size_t bytes_recvd)
            {
                if (!ec && bytes_recvd > 0)
                {
					std::vector<std::uint8_t> Data(m_Data, m_Data + bytes_recvd);
					OnReceived(Data);
                }
                //else
                //{
                //}

				Receive();
            });
    }

protected:
    virtual void OnReceived(std::vector<std::uint8_t>& data) = 0;

	void Send()
	{
		m_Port.async_write_some(boost::asio::buffer(m_DataSent.front(), m_DataSent.front().size()),
			[this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
			{
				std::lock_guard<std::mutex> Lock(m_Mtx);

				m_DataSent.pop();

				if (!m_DataSent.empty())
				{
					Send();
				}
			});
	}

//    void do_receive()
//    {
//        socket_.async_receive_from(
//            boost::asio::buffer(data_, max_length), sender_endpoint_,
//            [this](boost::system::error_code ec, std::size_t bytes_recvd)
//            {
//                if (!ec && bytes_recvd > 0)
//                {
//                    do_send(bytes_recvd);
//                }
//                else
//                {
//                    do_receive();
//                }
//            });
//    }
//
//    void do_send(std::size_t length)
//    {
//        socket_.async_send_to(
//            boost::asio::buffer(data_, length), sender_endpoint_,
//            [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/)
//            {
//                do_receive();
//            });
//    }
//
//private:
//    udp::socket socket_;
//    udp::endpoint sender_endpoint_;
//    enum { max_length = 1024 };
//    char data_[max_length];
};

}
}
