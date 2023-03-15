///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsPacketNMEA.h
// 2019-01-31
// Standard ISO/IEC 114882, C++14
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "utilsCRC.h"
#include "utilsPacket.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace utils
{
	namespace packet_NMEA
	{
///////////////////////////////////////////////////////////////////////////////////////////////////
template <class TPayload, std::uint8_t stx = '$'>
struct tFormat
{
	enum : std::uint8_t { STX = stx, CTX = '*' };

protected:
	static tVectorUInt8 TestPacket(tVectorUInt8::const_iterator cbegin, tVectorUInt8::const_iterator cend)
	{
		std::size_t Size = std::distance(cbegin, cend);

		if (Size >= GetSize(0) && *cbegin == STX)
		{
			const tVectorUInt8::const_iterator Begin = cbegin + 1;
			const tVectorUInt8::const_iterator End = std::find(Begin, cend, CTX);

			if (End != cend)
			{
				const std::size_t DataSize = std::distance(Begin, End);

				if (Size >= GetSize(DataSize) && VerifyCRC(Begin, DataSize))
				{
					return tVectorUInt8(cbegin, cbegin + GetSize(DataSize));
				}
			}
		}

		return tVectorUInt8();
	}

	static bool TryParse(const tVectorUInt8& packetVector, TPayload& payload)
	{
		if (packetVector.size() >= GetSize(0) && packetVector[0] == STX)
		{
			const tVectorUInt8::const_iterator Begin = packetVector.cbegin() + 1;
			const tVectorUInt8::const_iterator End = std::find(Begin, packetVector.cend(), CTX);

			if (End != packetVector.cend())
			{
				const std::size_t DataSize = std::distance(Begin, End);

				if (packetVector.size() == GetSize(DataSize) && VerifyCRC(Begin, DataSize))
				{
					payload = TPayload(Begin, End);

					return true;
				}
			}
		}

		return false;
	}

	static std::size_t GetSize(std::size_t payloadSize) { return payloadSize + 6; };//$*xx\xd\xa

	static void Append(tVectorUInt8& dst, const TPayload& payload)
	{
		dst.reserve(GetSize(payload.size()));

		dst.push_back(STX);

		for (auto i : payload)
		{
			dst.push_back(i);
		}

		const std::uint8_t CRC = utils::crc::CRC08_NMEA(payload.begin(), payload.end());

		dst.push_back(CTX);

		std::stringstream SStream;

		SStream << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << static_cast<uint16_t>(CRC);
		SStream << "\xd\xa";

		const std::string EndStr = SStream.str();

		dst.insert(dst.end(), EndStr.cbegin(), EndStr.cend());
	}

private:
	static bool VerifyCRC(tVectorUInt8::const_iterator begin, std::size_t crcDataSize)
	{
		const std::uint8_t CRC = utils::crc::CRC08_NMEA(begin, begin + crcDataSize);

		const tVectorUInt8::const_iterator CRCBegin = begin + crcDataSize + 1;//1 for '*'

		const std::uint8_t CRCReceived = utils::Read<std::uint8_t>(CRCBegin, CRCBegin + 2, utils::tRadix::hex);

		return CRC == CRCReceived;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
template <class TPayload> struct tFormatNMEA : public tFormat<TPayload, '$'> { };
template <class TPayload> struct tFormatNMEABin : public tFormat<TPayload, '!'> { };
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tPayloadCommon
{
	typedef std::vector<std::string> value_type;

	class tIterator
	{
		friend struct tPayloadCommon;

		const tPayloadCommon* m_pObj = nullptr;

		const std::size_t m_DataSize = 0;
		std::size_t m_DataIndex = 0;
		const char* m_DataPtr = nullptr;

		tIterator() = delete;
		tIterator(const tPayloadCommon* obj, bool begin) :m_pObj(obj), m_DataSize(m_pObj->size())
		{
			if (m_DataSize > 0 && m_pObj->Value[0].size() > 0)
			{
				if (begin)
				{
					m_DataPtr = &m_pObj->Value[0][0];
				}
				else
				{
					m_DataIndex = m_DataSize;
				}
			}
		}

	public:
		tIterator& operator ++ ()
		{
			if (m_DataIndex < m_DataSize)
			{
				++m_DataIndex;
			}

			std::size_t DataIndex = m_DataIndex;

			for (const std::string& i : m_pObj->Value)
			{
				const std::size_t StrSize = i.size();

				if (DataIndex >= StrSize + 1)
				{
					DataIndex -= StrSize + 1;
				}
				else if (DataIndex == StrSize)
				{
					static char Separator = ',';
					m_DataPtr = &Separator;
					break;
				}
				else
				{
					m_DataPtr = &i[DataIndex];
					break;
				}
			}

			return *this;
		}

		bool operator != (const tIterator& val) const
		{
			return m_DataIndex != val.m_DataIndex;
		}

		const char operator * () const
		{
			assert(m_DataPtr != nullptr);

			return *m_DataPtr;
		}
	};

	typedef tIterator iterator;

	value_type Value{};

	tPayloadCommon() = default;

	explicit tPayloadCommon(const value_type& value)
		:Value(value)
	{}

	tPayloadCommon(tVectorUInt8::const_iterator cbegin, tVectorUInt8::const_iterator cend)
	{
		AppendData(cbegin, cend);
	}

	tPayloadCommon(std::string::const_iterator cbegin, std::string::const_iterator cend)
	{
		AppendData(cbegin, cend);
	}

	std::size_t size() const
	{
		std::size_t Size = 0;

		for (const auto& i : Value)
		{
			Size += i.size();
		}

		if (Value.size() > 0)//that's for ','
		{
			Size += Value.size() - 1;
		}

		return Size;
	}

	iterator begin() const
	{
		return iterator(this, true);
	}

	iterator end() const
	{
		return iterator(this, false);
	}

private:
	template <class It>
	void AppendData(It begin, It end)
	{
		std::string LocalString;

		for (It i = begin; i != end; ++i)
		{
			if (*i == ',')
			{
				Value.push_back(LocalString);
				LocalString.clear();
			}
			else
			{
				LocalString.push_back(static_cast<char>(*i));
			}
		}

		Value.push_back(LocalString);
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
struct tPayloadString
{
	typedef std::string value_type;
	typedef value_type::const_iterator iterator;

	value_type Value{};

	tPayloadString() = default;

	explicit tPayloadString(const value_type& value)
		:Value(value)
	{}

	tPayloadString(tVectorUInt8::const_iterator cbegin, tVectorUInt8::const_iterator cend)
	{
		Value.insert(Value.end(), cbegin, cend);
	}

	std::size_t size() const
	{
		return Value.size();
	}

	iterator begin() const
	{
		return Value.begin();
	}

	iterator end() const
	{
		return Value.end();
	}
};

	}
}
