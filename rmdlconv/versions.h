// Copyright (c) 2023, rexx
// See LICENSE.txt for licensing information (GPL v3)

#pragma once

enum MdlVersion : int
{
	GARRYSMOD   = 48,
	PORTAL2     = 49,
	TITANFALL   = 52,
	TITANFALL2  = 53,
	APEXLEGENDS = 54
};

// conversion to rmdl v10 (studio version 54)
void ConvertMDL48To54(char* pMDL, const std::string& pathIn, const std::string& pathOut);
void ConvertMDL49To54(char* pMDL, const std::string& pathIn, const std::string& pathOut);
void ConvertMDL53To54(char* pMDL, const std::string& pathIn, const std::string& pathOut);
void ConvertRMDL8To10(char* pMDL, const std::string& pathIn, const std::string& pathOut);

// conversion to mdl v53
void ConvertMDL52To53(char* pMDL, const std::string& pathIn, const std::string& pathOut);

// VG
void ConvertVGData_12_1(char* inputBuf, const std::string& filePath);

// deprecated
//void CreateVGFile_v8(const std::string& filePath);


// rseq conversion
void ConvertRSEQFrom71To7(char* inputBuf, char* inputExternalBuf, const std::string& filePath);
void ConvertRSEQFrom10To7(char* inputBuf, char* inputExternalBuf, const std::string& filePath);

// model conversion handlers
void UpgradeStudioModelTo53(std::string& modelPath, const char* outputDir);
void UpgradeStudioModelTo54(std::string& modelPath, const char* outputDir);
void UpgradeStudioModel(std::string& modelPath, int targetVersion, const char* outputDir);