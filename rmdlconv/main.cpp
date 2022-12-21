#include "stdafx.h"

#include "versions.h"
#include "utils.h"
#include "CommandLine.h"
#include "BinaryIO.h"

const char* pszVersionHelpString = {
	"Please input the version of your model:\n"
	"-- OLD --\n"
	"8:    s0,1\n"
	"9:    s2\n"
	"10:   s3,4\n"
	"11:   s5\n"
	"12:   s6\n"
	"-- NEW --\n"
	"12.1: s7,8,9\n"
	"12.2: s10,11\n"
	"13:   s12\n"
	"14:   s13.1\n"
	"14.1: s14\n"
	"> "
};

int main(int argc, char** argv)
{
	CommandLine cmdline(argc, argv);

    if (argc < 2)
        Error("invalid usage\n");

    if (!FILE_EXISTS(argv[1]))
        Error("couldn't find input file\n");

	std::string mdlPath(argv[1]);

	BinaryIO mdlIn;
	mdlIn.open(mdlPath, BinaryIOMode::Read);

	if (mdlIn.read<int>() != 'TSDI')
		Error("invalid input file. must be a valid .(r)mdl file with magic 'IDST'\n");

	int mdlVersion = mdlIn.read<int>();

	switch(mdlVersion)
	{
	case 49:
	{
		uintmax_t mdlFileSize = GetFileSize(mdlPath);

		mdlIn.seek(0, std::ios::beg);

		char* mdlBuf = new char[mdlFileSize];

		mdlIn.getReader()->read(mdlBuf, mdlFileSize);

		ConvertMDLData_49(mdlBuf, mdlPath);

		delete[] mdlBuf;

		break;
	}
	case 52:
	{
		uintmax_t mdlFileSize = GetFileSize(mdlPath);

		mdlIn.seek(0, std::ios::beg);

		char* mdlBuf = new char[mdlFileSize];

		mdlIn.getReader()->read(mdlBuf, mdlFileSize);

		ConvertMDLDataFrom52To53(mdlBuf, mdlPath);

		delete[] mdlBuf;

		break;
	}
	case 53: // Titanfall 2
	{
		uintmax_t mdlFileSize = GetFileSize(mdlPath);

		mdlIn.seek(0, std::ios::beg);

		char* mdlBuf = new char[mdlFileSize];

		mdlIn.getReader()->read(mdlBuf, mdlFileSize);

		ConvertMDLData_53(mdlBuf, mdlPath);

		delete[] mdlBuf;

		break;
	}
	case 54:
	{
		// rmdl subversion
		std::string version = "12.1";

		if (cmdline.HasParam("-version"))
			version = cmdline.GetParamValue("-version", "12.1");
		else
		{
			std::cout << pszVersionHelpString;
			std::cin >> version;
		}

		printf("Input file is RMDL v%s. attempting conversion...\n", version.c_str());

		if (version == "12.1") // handle 12.1 model conversions
		{
			// convert v12.1 vg to v9 vg
			std::string vgFilePath = ChangeExtension(mdlPath, "vg");

			if (FILE_EXISTS(vgFilePath))
			{
				uintmax_t vgInputSize = GetFileSize(vgFilePath);

				char* vgInputBuf = new char[vgInputSize];

				std::ifstream ifs(vgFilePath, std::ios::in | std::ios::binary);

				ifs.read(vgInputBuf, vgInputSize);

				// if 0tVG magic
				if (*(int*)vgInputBuf == 'GVt0')
					ConvertVGData_12_1(vgInputBuf, vgFilePath);
				else
					delete[] vgInputBuf;
			}
		}
		else if (version == "8")
		{
			CreateVGFile_v8(mdlPath);
		}
		else
		{
			Error("version is not currently supported\n");
		}
		break;
	}
	}

	if(!cmdline.HasParam("-nopause"))
		std::system("pause");
}
