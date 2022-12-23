// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPLv3)

#pragma once

enum MdlVersion
{
	PORTAL2     = 49,
	TITANFALL   = 52,
	TITANFALL2  = 53,
	APEXLEGENDS = 54
};

void ConvertMDLData_49(char* inputBuf, const std::string& filePath);

void ConvertMDLData_53(char* inputBuf, const std::string& filePath);

// VG
void ConvertVGData_12_1(char* inputBuf, const std::string& filePath);

void CreateVGFile_v8(const std::string& filePath);

// v53 conversion
void ConvertMDLDataFrom52To53(char* inputBuf, const std::string& filePath);