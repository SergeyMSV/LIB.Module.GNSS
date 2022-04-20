///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsBase.h
//
// Standard ISO/IEC 114882, C++17
//
// |   version  |    release    | Description
// |------------|---------------|---------------------------------
// |            |   2014 09 24  |
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

	const auto Size = std::distance(first, last);

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

enum class tRadix : unsigned char
{
	dec = 10,
	hex = 16,
};

template<typename T, typename Iterator, int N = 20>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Read(Iterator first, Iterator last, tRadix radix)
{
	char Str[N];//[#] and +/- and 0x00

	std::size_t StrIndex = 0;

	for (; first != last && StrIndex < sizeof(Str) - 1; ++first)
	{
		char Byte = static_cast<char>(*first);

		if ((Byte >= '0' && Byte <= '9') ||
			(radix == tRadix::dec && Byte == '-' && StrIndex == 0) ||
			(radix == tRadix::hex && ((Byte >= 'A' && Byte <= 'F') || (Byte >= 'a' && Byte <= 'f'))))
		{
			Str[StrIndex++] = Byte;
		}
		else if (StrIndex != 0)
		{
			break;
		}
	}

	Str[StrIndex] = 0;

	if (Str[0] == '-' && radix == tRadix::dec)
	{
		return static_cast<T>(strtol(Str, 0, static_cast<int>(radix)));
	}

	return static_cast<T>(strtoul(Str, 0, static_cast<int>(radix)));
}

template<typename T>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Read(const char* data, tRadix radix)
{
	const std::size_t DataSize = strlen(data);

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

template <std::size_t size>
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

template <std::size_t size>
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

enum class tDevStatus : std::uint8_t
{
	Init,
	Operation,
	Deinit,
	Halted,
	Error,
	Unknown = 0xFF,
};

enum class tGNSSCode : std::uint8_t
{
	Unknown = 0,
	GLONASS,
	GPS,
	GALILEO,
	BEIDOU,
	WAAS = 20,
	//END = 31, in this case it's possible to use this GNSSCode as an index for a bitfield when navigation solution based on two or more GNSS systems
};

//from /usr/include/sysexits.h
enum class tExitCode : int
{
	EX_OK = 0,			// successful termination
	EX__BASE = 64,		// base value for error messages
	EX_USAGE = 64,		// command line usage error
	EX_DATAERR = 65,	// data format error
	EX_NOINPUT = 66,	// cannot open input
	EX_NOUSER = 67,		// addressee unknown
	EX_NOHOST = 68,		// host name unknown
	EX_UNAVAILABLE = 69,// service unavailable
	EX_SOFTWARE = 70,	// internal software error
	EX_OSERR = 71,		// system error (e.g., can't fork)
	EX_OSFILE = 72,		// critical OS file missing
	EX_CANTCREAT = 73,	// can't create (user) output file
	EX_IOERR = 74,		// input/output error
	EX_TEMPFAIL = 75,	// temp failure; user is invited to retry
	EX_PROTOCOL = 76,	// remote error in protocol
	EX_NOPERM = 77,		// permission denied
	EX_CONFIG = 78,		// configuration error
	EX__MAX = 78,		// maximum listed value
};

}
