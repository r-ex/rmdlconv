// Copyright (c) 2023, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include "stdafx.h"
#include <versions.h>
#include <mdl/studio.h>

// fetch all the valid models from a given path
// todo: this should support sequences
void GetStudioModelFromPath(std::string& pathIn, std::vector<std::string>& pathOut)
{
	std::filesystem::path pathInAsPath(pathIn);
	std::unique_ptr<char[]> extension(new char[_MAX_PATH]); // :D

	if (pathInAsPath.has_extension())
	{
		strncpy_s(extension.get(), _MAX_PATH, pathInAsPath.extension().u8string().c_str(), _MAX_PATH);

		if (!strncmp(extension.get(), ".mdl", _MAX_PATH) || !strncmp(extension.get(), ".rmdl", _MAX_PATH))
			pathOut.push_back(pathIn);
	}
	else
	{
		for (auto& dir : std::filesystem::recursive_directory_iterator(pathIn))
		{
			std::filesystem::path path(dir.path());

			if (!path.has_extension())
				continue;

			strncpy_s(extension.get(), _MAX_PATH, path.extension().u8string().c_str(), _MAX_PATH);

			std::string pathStr = path.u8string();

			if (!strncmp(extension.get(), ".mdl", _MAX_PATH) || !strncmp(extension.get(), ".rmdl", _MAX_PATH))
				pathOut.push_back(pathStr);
		}
	}

	return;
}

// set our output path
std::string SetStudioModelOutput(const std::string& pathIn, const std::string& pathModel, const char* pathOut)
{
	std::filesystem::path out{}; // export to a new directory for ease

	// should only be done if not nullptr, to be tested
	// this whole section is REALLY BAD
	if (pathOut)
	{
		size_t pathOffset = std::filesystem::path(pathIn).parent_path().u8string().length(); // this is ass

		std::filesystem::path pathCustom = pathModel.substr(pathOffset, pathModel.length() - pathOffset);

		out = std::filesystem::path(pathOut).append(pathCustom.relative_path().u8string());
	}
	else
	{
		out = std::filesystem::path(pathModel).parent_path().append("rmdlconv_out");
		out.append(std::filesystem::path(pathModel).filename().u8string());
	}

	std::string pathOutAsString = out.u8string(); // for passing to funcs

	std::filesystem::create_directories(std::filesystem::path(out).parent_path());

	return out.string();
}

__forceinline int GetStudioVersionFromBuffer(char* pMDL)
{
	studiohdrshort_t* pHdr = reinterpret_cast<studiohdrshort_t*>(pMDL);
	return pHdr->id == IDSTUDIOHEADER ? pHdr->version : -1; // return -1 if invalid model
}

// upgrade model funcs, varies per target version. func will parse models existing version and choose a function accordingly.
void UpgradeStudioModelTo53(std::string& modelPath, const char* outputDir)
{
	std::vector<std::string> modelPaths;

	GetStudioModelFromPath(modelPath, modelPaths);

	for (auto& path : modelPaths)
	{
		if (!std::filesystem::exists(path))
		{
			printf("skipped %s because it didn't exist.\n", path.c_str());
			continue;
		}

		BinaryIO studioModel;
		studioModel.open(path, BinaryIOMode::Read);
		studioModel.seek(0, std::ios::beg);

		int fileSize = GetFileSize(path); // should not exceed the side of a signed int. literally not possible.
		std::unique_ptr<char[]> pMDL(new char[fileSize]);

		studioModel.getReader()->read(pMDL.get(), fileSize);

		std::string pathOut = SetStudioModelOutput(modelPath, path, outputDir);

		int studioVersion = GetStudioVersionFromBuffer(pMDL.get());

		switch (studioVersion)
		{
		case MdlVersion::TITANFALL:
			ConvertMDL52To53(pMDL.get(), path, pathOut);
			break;
		default:
			printf("Model '%s' has an unsupported verion, skipping...\n", path.c_str());
			break;
		}

	}

	printf("\nFinished converting %zi models, exiting...\n", modelPaths.size());
}

void UpgradeStudioModelTo54(std::string& modelPath, const char* outputDir)
{
	std::vector<std::string> modelPaths;

	GetStudioModelFromPath(modelPath, modelPaths);

	for (auto& path : modelPaths)
	{
		if (!std::filesystem::exists(path))
		{
			printf("skipped %s because it didn't exist.\n", path.c_str());
			continue;
		}

		BinaryIO studioModel;
		studioModel.open(path, BinaryIOMode::Read);
		studioModel.seek(0, std::ios::beg);

		int fileSize = GetFileSize(path); // should not exceed the side of a signed int. literally not possible.
		std::unique_ptr<char[]> pMDL(new char[fileSize]);

		studioModel.getReader()->read(pMDL.get(), fileSize);

		std::string pathOut = SetStudioModelOutput(modelPath, path, outputDir);

		int studioVersion = GetStudioVersionFromBuffer(pMDL.get());

		switch (studioVersion)
		{
		case MdlVersion::GARRYSMOD:
			ConvertMDL48To54(pMDL.get(), path, pathOut);
			break;
		case MdlVersion::PORTAL2:
			ConvertMDL49To54(pMDL.get(), path, pathOut);
			break;
		case MdlVersion::TITANFALL:
			// no func yet
			break;
		case MdlVersion::TITANFALL2:
			ConvertMDL53To54(pMDL.get(), path, pathOut);
			break;
		case MdlVersion::APEXLEGENDS:
			ConvertRMDL8To10(pMDL.get(), path, pathOut);
			break;
		default:
			printf("Model '%s' has an unsupported verion, skipping...\n", path.c_str());
			break;
		}

	}

	printf("\nFinished converting %zi models, exiting...\n", modelPaths.size());
}

// handle target version input
void UpgradeStudioModel(std::string& modelPath, int targetVersion, const char* outputDir)
{
	switch (targetVersion)
	{
	case MdlVersion::TITANFALL2:
		UpgradeStudioModelTo53(modelPath, outputDir);
		break;
	case MdlVersion::APEXLEGENDS:
		UpgradeStudioModelTo54(modelPath, outputDir);
		break;
	default:
		Error("Unsupported studio version!!! Exiting...\n");
		break;
	}
}