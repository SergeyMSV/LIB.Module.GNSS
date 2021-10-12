///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsBase.h
//
// Standard ISO/IEC 114882, C++17
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |     ...    |   2014 09 24  |
// |      1     |   2019 08 18  |
// |            |               | 
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <vector>

namespace utils
{

typedef std::vector<std::uint8_t> tVectorUInt8;

template<typename T>
typename std::enable_if<std::is_trivially_copyable<T>::value, void>::type Append(tVectorUInt8& dst, const T& value)
{
	const std::uint8_t* Begin = reinterpret_cast<const std::uint8_t*>(&value);

	dst.insert<const std::uint8_t*>(dst.end(), Begin, Begin + sizeof(value));
}

template<typename T>
typename std::enable_if<std::is_trivially_copyable<T>::value, tVectorUInt8>::type ToVector(const T& value)
{
	tVectorUInt8 Data;

	Data.reserve(sizeof(value));

	const std::uint8_t* Begin = reinterpret_cast<const std::uint8_t*>(&value);

	Data.insert(Data.end(), Begin, Begin + sizeof(value));

	return Data;
}

template<typename T, typename Iterator>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Read(Iterator first, Iterator last)
{
	T Data = 0;

	auto Size = std::distance(first, last);

	if (Size > 0 && Size <= static_cast<std::size_t>(sizeof(T)))
	{
		std::copy(first, last, reinterpret_cast<std::uint8_t*>(&Data));
	}

	return Data;
}

template<typename T>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Read(const char* data, std::size_t dataSize)
{
	return Read<T, const char*>(data, data + dataSize);
}

template<typename T>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Read(const unsigned char* data, std::size_t dataSize)
{
	const char* Begin = reinterpret_cast<const char*>(data);

	return Read<T, const char*>(Begin, Begin + dataSize);
}

enum tRadix
{
	tRadix_10 = 10,
	tRadix_16 = 16,
};

template<typename T, typename Iterator, int N = 20>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Read(Iterator first, Iterator last, tRadix radix)
{
	char Str[N];//[#] and +/- and 0x00

	unsigned int StrIndex = 0;

	for (; first != last && StrIndex < sizeof(Str) - 1; ++first)
	{
		char Byte = static_cast<char>(*first);

		if ((Byte >= '0' && Byte <= '9') ||
			(radix == tRadix_10 && Byte == '-' && StrIndex == 0) ||
			(radix == tRadix_16 && ((Byte >= 'A' && Byte <= 'F') || (Byte >= 'a' && Byte <= 'f'))))
		{
			Str[StrIndex++] = Byte;
		}
		else if (StrIndex != 0)
		{
			break;
		}
	}

	Str[StrIndex] = 0;

	if (Str[0] == '-' && radix == tRadix_10)
	{
		return static_cast<T>(strtol(Str, 0, radix));
	}

	return static_cast<T>(strtoul(Str, 0, radix));
}

template<typename T>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Read(const char* data, tRadix radix)
{
	std::size_t DataSize = strlen(data);

	return Read<T, const char*>(data, data + DataSize, radix);
}

template<typename T>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Reverse(T value)
{
	std::uint8_t* Begin = reinterpret_cast<std::uint8_t*>(&value);

	std::reverse<std::uint8_t*>(Begin, Begin + sizeof(value));

	return value;
}

namespace type
{

template <unsigned int size>
struct tArray1
{
	enum { Size = size };
	std::uint8_t Value[size];

	//tArray1() in union it's deleted by default
	//{
	//	std::memset(Value, 0, Size);
	//}

	std::uint8_t& operator [] (std::size_t i)
	{
		assert(i < Size);

		return Value[i];
	}

	bool operator == (const tArray1& value)
	{
		return std::memcmp(Value, value.Value, Size) == 0;
	}

	bool operator != (const tArray1& value)
	{
		return std::memcmp(Value, value.Value, Size) != 0;
	}
};

template <unsigned int size>
struct tArray2 : public tArray1<size>
{
	tArray2()
	{
		std::memset(this->Value, 0, this->Size);
	}
};

}

class tEmptyAble
{
protected:
	bool m_Empty = true;

public:
	tEmptyAble() = default;
	explicit tEmptyAble(bool empty) :m_Empty(empty) {}

	bool Empty() const { return m_Empty; }

protected:
	~tEmptyAble() {}
};

//char FromBCD(char dataBCD); [TBD]
//char ToBCD(char dataBCD); [TBD]
}
