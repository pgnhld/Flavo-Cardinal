#pragma once

#include <cstdint>
#include <string>

//Indication of out parameters of methods
#define OUT 

//Indication of parameters to be possibly changed
#define REF

#define SAFE_RELEASE(p)	{ if (p != nullptr) { (p)->Release(); p = nullptr; } }

#ifdef _MSC_VER
#define DEBUG_BREAK	__debugbreak();
#endif

//Annoying as hell
#define NOMINMAX
#undef min
#undef max

typedef std::string string;
typedef std::uint8_t uint8;
typedef std::int16_t int16;
typedef std::uint16_t uint16;
typedef std::int32_t int32;
typedef std::uint32_t uint32;
typedef std::int64_t int64;
typedef std::uint64_t uint64;
typedef unsigned char byte;
