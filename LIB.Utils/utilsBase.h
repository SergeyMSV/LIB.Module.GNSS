///////////////////////////////////////////////////////////////////////////////////////////////////
// utilsBase.h
// 2014-09-24
// Standard ISO/IEC 114882, C++17
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <stdexcept>
#include <string>
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

enum class tRadix : std::uint8_t
{
	//oct = 8,//it's just for tests
	dec = 10,
	hex = 16,
};

template<typename T, typename Iterator, tRadix Radix>
struct tRead
{
	typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type operator()(Iterator first, Iterator last)
	{
		assert(false);
		return 0;//T();//T{};//[TBD] - C++20
	}
};

template<typename T, typename Iterator>
struct tRead<T, Iterator, tRadix::dec>
{
	typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type operator()(Iterator first, Iterator last)
	{
		std::string ValStr(first, last);
		ValStr.erase(ValStr.begin(), std::find_if(ValStr.begin(), ValStr.end(), [](char ch) { return std::isdigit(ch) || ch == '-'; }));
		return static_cast<T>(strtol(ValStr.c_str(), nullptr, 10));
	}
};

template<typename T, typename Iterator>
struct tRead<T, Iterator, tRadix::hex>
{
	typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type operator()(Iterator first, Iterator last)
	{
		std::string ValStr(first, last);
		ValStr.erase(ValStr.begin(), std::find_if(ValStr.begin(), ValStr.end(), [](char ch) { return std::isxdigit(ch); }));
		return static_cast<T>(strtoul(ValStr.c_str(), nullptr, 16));
	}
};

template<typename T, typename Iterator, int N = 20>
typename std::enable_if<std::is_trivially_copyable<T>::value, T>::type Read(Iterator first, Iterator last, tRadix radix)
{
	switch (radix)
	{
	//case tRadix::oct: return tRead<T, Iterator, tRadix::oct>()(first, last);
	case tRadix::dec: return tRead<T, Iterator, tRadix::dec>()(first, last);
	case tRadix::hex: return tRead<T, Iterator, tRadix::hex>()(first, last);
	default:
		assert(false);
		return 0;
	}
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

struct tVersion // 1.0.234
{
	std::uint16_t Major = 0;
	std::uint16_t Minor = 0;
	std::uint16_t Build = 0;

	tVersion() = default;
	tVersion(std::uint16_t major, std::uint16_t minor, std::uint16_t build)
		:Major(major), Minor(minor), Build(build)
	{
	}
	explicit tVersion(const std::string& strVersion)
	{
		if (!TryParse(strVersion, *this))
			throw std::runtime_error("format");
	}

	bool operator==(const tVersion& val) const
	{
		return Major == val.Major && Minor == val.Minor && Build == val.Build;
	}
	bool operator!=(const tVersion& val) const
	{
		return !operator==(val);
	}
	//bool operator==(const tVersion&)const = default;//[TBD] - C++20 set at the beginning of the file
	//bool operator!=(const tVersion&)const = default;

	bool operator<(const tVersion& val) const
	{
		if (Major == val.Major)
		{
			if (Minor == val.Minor)
			{
				return Build < val.Build;
			}
			return Minor < val.Minor;
		}
		return Major < val.Major;
	}
	bool operator>(const tVersion& val) const
	{
		if (Major == val.Major)
		{
			if (Minor == val.Minor)
			{
				return Build > val.Build;
			}
			return Minor > val.Minor;
		}
		return Major > val.Major;
	}

	static bool TryParse(const std::string& strVersion, tVersion& version)
	{
		version = tVersion{};

		auto IsNotVersionSymbol = [](char ch)->bool {return !isdigit(ch) && ch != '.'; };

		std::string Value = strVersion;
		Value.erase(std::remove_if(Value.begin(), Value.end(), IsNotVersionSymbol), Value.end());

		const std::size_t Part1Begin = 0;
		const std::size_t Part1End = Value.find('.');
		const std::size_t Part2Begin = Part1End + 1;
		const std::size_t Part2End = Value.find('.', Part2Begin);
		const std::size_t Part3Begin = Part2End + 1;
		const std::size_t Part3End = Value.size() - 1;

		if (Part1End == std::string::npos || Part2End == std::string::npos || Part2End == Value.size() - 1)
			return false;

		auto GetFigure = [&Value](std::size_t begin, std::size_t end)->long
		{
			std::string SubStr = Value.substr(begin, end);
			return std::strtol(SubStr.c_str(), nullptr, 10);
		};

		version.Major = static_cast<std::uint16_t>(GetFigure(Part1Begin, Part1End));
		version.Minor = static_cast<std::uint16_t>(GetFigure(Part2Begin, Part2End));
		version.Build = static_cast<std::uint16_t>(GetFigure(Part3Begin, Part3End));

		return true;
	}

	std::string ToString() const
	{
		return std::to_string(Major) + "." + std::to_string(Minor) + "." + std::to_string(Build);
	}
};

static std::string GetStringEnding(const std::string& pattern, const std::string& str)
{
	size_t Pos = str.find(pattern);
	if (Pos == std::string::npos)
		return {};

	return str.substr(Pos + pattern.size());
};

}
