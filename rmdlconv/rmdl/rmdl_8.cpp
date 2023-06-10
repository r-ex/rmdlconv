// Copyright (c) 2023, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include "stdafx.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "versions.h"

/*
	Type:    RMDL
	Version: 8
	Game:    Apex Legends Seasons 0-1

	Files: .rmdl, .vtx, .vvd, .vvc, .vvw
*/

// deprecated
//void CreateVGFile_v8(const std::string& filePath)
//{
//	printf("creating VG file from v8 rmdl...\n");
//
//	std::string rmdlPath = ChangeExtension(filePath, "rmdl");
//	std::string vtxPath = ChangeExtension(filePath, "vtx");
//	std::string vvdPath = ChangeExtension(filePath, "vvd");
//	std::string vvcPath = ChangeExtension(filePath, "vvc");
//	std::string vvwPath = ChangeExtension(filePath, "vvw");
//
//	if (!FILE_EXISTS(vtxPath) || !FILE_EXISTS(vvdPath))
//		Error("failed to convert external model components (vtx, vvd) to VG. '.vtx' and '.vvd' files are required but were not found\n");
//
//	char* rmdlBuf = nullptr;
//	{
//		uintmax_t rmdlSize = GetFileSize(rmdlPath);
//
//		rmdlBuf = new char[rmdlSize];
//
//		std::ifstream ifs(rmdlPath, std::ios::in | std::ios::binary);
//
//		ifs.read(rmdlBuf, rmdlSize);
//	}
//
//	char* vtxBuf = nullptr;
//	{
//		uintmax_t vtxSize = GetFileSize(vtxPath);
//
//		vtxBuf = new char[vtxSize];
//
//		std::ifstream ifs(vtxPath, std::ios::in | std::ios::binary);
//
//		ifs.read(vtxBuf, vtxSize);
//	}
//
//	char* vvdBuf = nullptr;
//	{
//		uintmax_t vvdSize = GetFileSize(vvdPath);
//
//		vvdBuf = new char[vvdSize];
//
//		std::ifstream ifs(vvdPath, std::ios::in | std::ios::binary);
//
//		ifs.read(vvdBuf, vvdSize);
//	}
//
//	// vertex color and uv2 buffer
//	char* vvcBuf = nullptr;
//	if (FILE_EXISTS(vvcPath))
//	{
//		{
//			uintmax_t vvcSize = GetFileSize(vvcPath);
//
//			vvcBuf = new char[vvcSize];
//
//			std::ifstream ifs(vvcPath, std::ios::in | std::ios::binary);
//
//			ifs.read(vvcBuf, vvcSize);
//		}
//	}
//
//	// extended weight buffer
//	char* vvwBuf = nullptr;
//	if (FILE_EXISTS(vvwPath))
//	{
//		{
//			uintmax_t vvwSize = GetFileSize(vvwPath);
//
//			vvwBuf = new char[vvwSize];
//
//			std::ifstream ifs(vvwPath, std::ios::in | std::ios::binary);
//
//			ifs.read(vvwBuf, vvwSize);
//		}
//	}
//
//	CreateVGFile(ChangeExtension(filePath, "vg"), reinterpret_cast<r5::v8::studiohdr_t*>(rmdlBuf), vtxBuf, vvdBuf, vvcBuf, vvwBuf);
//}

void ConvertRMDL8To10(char* pMDL, const std::string& pathIn, const std::string& pathOut)
{
	std::string modelName = std::filesystem::path(pathIn).filename().u8string();
	printf("Upgrading model '%s' to subversion 12...\n", modelName.c_str());

	std::string pathRMDL = ChangeExtension(pathIn, "rmdl");
	std::string pathVTX = ChangeExtension(pathIn, "vtx");
	std::string pathVVD = ChangeExtension(pathIn, "vvd");
	std::string pathVVC = ChangeExtension(pathIn, "vvc");
	std::string pathVVW = ChangeExtension(pathIn, "vvw");

	printf("Copying RMDL file from '%s' to '%s'...\n", std::filesystem::path(pathIn).parent_path().u8string().c_str(), std::filesystem::path(pathOut).parent_path().u8string().c_str());

	BinaryIO modelOut{};
	modelOut.open(ChangeExtension(pathOut, "rmdl_copy"), BinaryIOMode::Write);
	modelOut.getWriter()->write(pMDL, GetFileSize(pathRMDL));
	modelOut.close();

	// try this just in case
	if (!FILE_EXISTS(pathVTX))
		pathVTX = ChangeExtension(pathIn, "dx11.vtx");

	if (!FILE_EXISTS(pathVTX) || !FILE_EXISTS(pathVVD))
		Error("failed to convert external model components (vtx, vvd) to VG. '.vtx' and '.vvd' files are required but were not found\n");

	printf("Creating a VG file from RMDL...\n");

	std::unique_ptr<char[]> pVTX;
	{
		size_t sizeVTX = GetFileSize(pathVTX);

		pVTX = std::unique_ptr<char[]>(new char[sizeVTX]);

		std::ifstream ifs(pathVTX, std::ios::in | std::ios::binary);

		ifs.read(pVTX.get(), sizeVTX);
	}

	std::unique_ptr<char[]> pVVD;
	{
		size_t sizeVVD = GetFileSize(pathVVD);

		pVVD = std::unique_ptr<char[]>(new char[sizeVVD]);

		std::ifstream ifs(pathVVD, std::ios::in | std::ios::binary);

		ifs.read(pVVD.get(), sizeVVD);
	}

	// vertex color and uv2 buffer
	std::unique_ptr<char[]> pVVC;
	if (FILE_EXISTS(pathVVC))
	{
		size_t sizeVVC = GetFileSize(pathVVC);

		pVVC = std::unique_ptr<char[]>(new char[sizeVVC]);

		std::ifstream ifs(pathVVC, std::ios::in | std::ios::binary);

		ifs.read(pVVC.get(), sizeVVC);
	}

	// extended weight buffer
	std::unique_ptr<char[]> pVVW;
	if (FILE_EXISTS(pathVVW))
	{
		size_t sizeVVW = GetFileSize(pathVVW);

		pVVW = std::unique_ptr<char[]>(new char[sizeVVW]);

		std::ifstream ifs(pathVVW, std::ios::in | std::ios::binary);

		ifs.read(pVVW.get(), sizeVVW);
	}

	CreateVGFile(ChangeExtension(pathOut, "vg"), reinterpret_cast<r5::v8::studiohdr_t*>(pMDL), pVTX.get(), pVVD.get(), pVVC.get(), pVVW.get());

	printf("Finished upgrading model '%s', proceeding...\n\n", modelName.c_str());
}