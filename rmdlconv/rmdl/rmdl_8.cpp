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

void CreateVGFile_v8(const std::string& filePath)
{
	printf("creating VG file from v8 rmdl...\n");

	std::string rmdlPath = ChangeExtension(filePath, "rmdl");
	std::string vtxPath = ChangeExtension(filePath, "vtx");
	std::string vvdPath = ChangeExtension(filePath, "vvd");
	std::string vvcPath = ChangeExtension(filePath, "vvc");
	std::string vvwPath = ChangeExtension(filePath, "vvw");

	if (!FILE_EXISTS(vtxPath) || !FILE_EXISTS(vvdPath))
		Error("failed to convert external model components (vtx, vvd) to VG. '.vtx' and '.vvd' files are required but were not found\n");

	char* rmdlBuf = nullptr;
	{
		uintmax_t rmdlSize = GetFileSize(rmdlPath);

		rmdlBuf = new char[rmdlSize];

		std::ifstream ifs(rmdlPath, std::ios::in | std::ios::binary);

		ifs.read(rmdlBuf, rmdlSize);
	}

	char* vtxBuf = nullptr;
	{
		uintmax_t vtxSize = GetFileSize(vtxPath);

		vtxBuf = new char[vtxSize];

		std::ifstream ifs(vtxPath, std::ios::in | std::ios::binary);

		ifs.read(vtxBuf, vtxSize);
	}

	char* vvdBuf = nullptr;
	{
		uintmax_t vvdSize = GetFileSize(vvdPath);

		vvdBuf = new char[vvdSize];

		std::ifstream ifs(vvdPath, std::ios::in | std::ios::binary);

		ifs.read(vvdBuf, vvdSize);
	}

	// vertex color and uv2 buffer
	char* vvcBuf = nullptr;
	if (FILE_EXISTS(vvcPath))
	{
		{
			uintmax_t vvcSize = GetFileSize(vvcPath);

			vvcBuf = new char[vvcSize];

			std::ifstream ifs(vvcPath, std::ios::in | std::ios::binary);

			ifs.read(vvcBuf, vvcSize);
		}
	}

	// extended weight buffer
	char* vvwBuf = nullptr;
	if (FILE_EXISTS(vvwPath))
	{
		{
			uintmax_t vvwSize = GetFileSize(vvwPath);

			vvwBuf = new char[vvwSize];

			std::ifstream ifs(vvwPath, std::ios::in | std::ios::binary);

			ifs.read(vvwBuf, vvwSize);
		}
	}

	CreateVGFile(ChangeExtension(filePath, "vg"), reinterpret_cast<r5::v8::studiohdr_t*>(rmdlBuf), vtxBuf, vvdBuf, vvcBuf, vvwBuf);
}
