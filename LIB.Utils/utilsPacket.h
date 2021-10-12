///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsPacket.h
//
// Standard ISO/IEC 114882, C++14
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |      1     |   2019 06 20  |
// |      2     |   2020 03 26  | std::size_t Find(tVectorUInt8& receivedData, tPacket& packet) instead of bool Find(tVectorUInt8& receivedData, tPacket& packet)
// |      3     |   2020 06 10  | Added typedef typename TPayload payload_type;
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "utilsBase.h"

#include <algorithm>

namespace utils
{
	namespace packet
	{

template
<
	template <class> class TFormat,
	class TPayload
>
class tPacket : private TFormat<TPayload>, private TPayload
{
public:
	typedef TPayload payload_type;
	typedef typename TPayload::value_type payload_value_type;

	tPacket() { }
	explicit tPacket(const payload_value_type& value)
	{
		TPayload::Data = value;
	}

	static std::size_t Find(tVectorUInt8& receivedData, tPacket& packet)
	{
		tVectorUInt8::const_iterator Begin = receivedData.cbegin();

		for (;;)
		{
			Begin = std::find(Begin, receivedData.cend(), TFormat<TPayload>::STX);

			if (Begin == receivedData.cend())
			{
				break;
			}

			tVectorUInt8 PacketVector = TFormat<TPayload>::TestPacket(Begin, receivedData.cend());

			if (PacketVector.size() > 0)
			{
				if (TFormat<TPayload>::TryParse(PacketVector, packet))
				{
					std::size_t EraseSize = std::distance(receivedData.cbegin(), Begin);

					EraseSize += PacketVector.size();

					receivedData.erase(receivedData.begin(), receivedData.begin() + EraseSize);

					return PacketVector.size();
				}
			}

			Begin++;
		}

		return 0;
	}

	payload_value_type GetPayload() const
	{
		return TPayload::Data;
	}

	void SetPayload(const payload_value_type& value)
	{
		TPayload::Data = value;
	}

	tVectorUInt8 ToVector()
	{
		tVectorUInt8 PacketVector;

		TFormat<TPayload>::Append(PacketVector, *this);

		return PacketVector;
	}
};

	}
}