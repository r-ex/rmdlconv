#pragma once
#include <cstdarg>
#include <cstdio>
#include <string>
#include <filesystem>
#include <iostream>

#define FILE_EXISTS(path) std::filesystem::exists(path)

#define IALIGN2( a ) ((a + 1) & ~ 1)
#define IALIGN4( a ) ((a + 3) & ~ 3)


// pointer alignment
#define ALIGN4( a ) a = (byte *)((__int64)((byte *)a + 3) & ~ 3)
#define ALIGN16( a ) a = (byte *)((__int64)((byte *)a + 15) & ~ 15)
#define ALIGN32( a ) a = (byte *)((__int64)((byte *)a + 31) & ~ 31)
#define ALIGN64( a ) a = (byte *)((__int64)((byte *)a + 63) & ~ 63)
#define ALIGN512( a ) a = (byte *)((__int64)((byte *)a + 511) & ~ 511)

static void Error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	std::string msg = "ERROR: " + std::string(fmt);

	vprintf(msg.c_str(), args);

	va_end(args);

	exit(EXIT_FAILURE);
}

static uintmax_t GetFileSize(const std::string& filename)
{
	try {
		return std::filesystem::file_size(filename);
	}
	catch (std::filesystem::filesystem_error& e) {
		std::cout << e.what() << '\n';
		exit(0);
	}
}

static std::string ChangeExtension(const std::string& path, const std::string& extension="")
{
	return std::filesystem::path(path).replace_extension(extension).u8string();
}

inline bool EndsWith(std::string const& value, std::string const& ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

static std::uint64_t __fastcall HashString(const char* pData)
{
	std::uint32_t* v1; // r8
	std::uint64_t         v2; // r10
	int                   v3; // er11
	std::uint32_t         v4; // er9
	std::uint32_t          i; // edx
	std::uint64_t         v6; // rcx
	int                   v7; // er9
	int                   v8; // edx
	int                   v9; // eax
	std::uint32_t        v10; // er8
	int                  v12; // ecx
	std::uint32_t* a1 = (std::uint32_t*)pData;

	v1 = a1;
	v2 = 0i64;
	v3 = 0;
	v4 = (*a1 - 45 * ((~(*a1 ^ 0x5C5C5C5Cu) >> 7) & (((*a1 ^ 0x5C5C5C5Cu) - 0x1010101) >> 7) & 0x1010101)) & 0xDFDFDFDF;
	for (i = ~*a1 & (*a1 - 0x1010101) & 0x80808080; !i; i = v8 & 0x80808080)
	{
		v6 = v4;
		v7 = v1[1];
		++v1;
		v3 += 4;
		v2 = ((((std::uint64_t)(0xFB8C4D96501i64 * v6) >> 24) + 0x633D5F1 * v2) >> 61) ^ (((std::uint64_t)(0xFB8C4D96501i64 * v6) >> 24)
			+ 0x633D5F1 * v2);
		v8 = ~v7 & (v7 - 0x1010101);
		v4 = (v7 - 45 * ((~(v7 ^ 0x5C5C5C5Cu) >> 7) & (((v7 ^ 0x5C5C5C5Cu) - 0x1010101) >> 7) & 0x1010101)) & 0xDFDFDFDF;
	}
	v9 = -1;
	v10 = (i & -(signed)i) - 1;
	if (_BitScanReverse((unsigned long*)&v12, v10))
	{
		v9 = v12;
	}
	return 0x633D5F1 * v2 + ((0xFB8C4D96501i64 * (std::uint64_t)(v4 & v10)) >> 24) - 0xAE502812AA7333i64 * (std::uint32_t)(v3 + v9 / 8);
}