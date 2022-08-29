#pragma once
#include <cstdarg>
#include <cstdio>
#include <string>
#include <filesystem>
#include <iostream>

#define FILE_EXISTS(path) std::filesystem::exists(path)

static void Error(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	std::string msg = "ERROR: " + std::string(fmt);

	vprintf(msg.c_str(), args);

	va_end(args);

	exit(EXIT_FAILURE);
}

static uintmax_t GetFileSize(std::string filename)
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