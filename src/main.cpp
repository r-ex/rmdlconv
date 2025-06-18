// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include <pch.h>
#include <core/CommandLine.h>
#include <studio/versions.h>
#include <core/utils.h>

const char* pszVersionHelpString = {
	"Please input the version of your model:\n"
	"-- OLD --\n"
	"8:    s0,1\n"
	"9:    s2\n"
	"10:   s3,4\n"
	"11:   s5\n"
	"12:   s6\n"
	"-- NEW --\n"
	"12.1: s7,8\n"
	"12.2: s9,10,11\n"
	"13:   s12\n"
	"14:   s13.1\n"
	"14.1: s14\n"
	"> "
};

const char* pszRSeqVersionHelpString = {
	"Please input the version of your sequence : \n"
	"7:    s0,1,3,4,5,6\n"
	"7.1:  s7,8\n"
	"10:   s9,10,11,12,13,14\n"
	"11:   s15\n"
	"> "
};

// move on from this
void LegacyConversionHandling(CommandLine& cmdline)
{
	// using command args
	if (cmdline.argc > 2)
		return;

	if (!FILE_EXISTS(cmdline.argv[1]))
		Error("couldn't find input file\n");

	std::string mdlPath(cmdline.argv[1]);

	BinaryIO mdlIn;
	mdlIn.open(mdlPath, BinaryIOMode::Read);

	if (mdlIn.read<int>() == 'TSDI')
	{
		int mdlVersion = mdlIn.read<int>();

		switch (mdlVersion)
		{
		case MdlVersion::GARRYSMOD:
		{
			uintmax_t mdlFileSize = GetFileSize(mdlPath);

			mdlIn.seek(0, std::ios::beg);

			char* mdlBuf = new char[mdlFileSize];

			mdlIn.getReader()->read(mdlBuf, mdlFileSize);

			ConvertMDL48To54(mdlBuf, mdlPath, mdlPath);

			delete[] mdlBuf;

			break;
		}
		case MdlVersion::PORTAL2:
		{
			uintmax_t mdlFileSize = GetFileSize(mdlPath);

			mdlIn.seek(0, std::ios::beg);

			char* mdlBuf = new char[mdlFileSize];

			mdlIn.getReader()->read(mdlBuf, mdlFileSize);

			ConvertMDL49To54(mdlBuf, mdlPath, mdlPath);

			delete[] mdlBuf;

			break;
		}
		case MdlVersion::TITANFALL:
		{
			uintmax_t mdlFileSize = GetFileSize(mdlPath);

			mdlIn.seek(0, std::ios::beg);

			char* mdlBuf = new char[mdlFileSize];

			mdlIn.getReader()->read(mdlBuf, mdlFileSize);

			ConvertMDL52To53(mdlBuf, mdlPath, mdlPath);

			delete[] mdlBuf;

			break;
		}
		case MdlVersion::TITANFALL2:
		{
			uintmax_t mdlFileSize = GetFileSize(mdlPath);

			mdlIn.seek(0, std::ios::beg);

			char* mdlBuf = new char[mdlFileSize];

			mdlIn.getReader()->read(mdlBuf, mdlFileSize);

			ConvertMDL53To54(mdlBuf, mdlPath, mdlPath);

			delete[] mdlBuf;

			break;
		}
		case MdlVersion::APEXLEGENDS:
		{
			// rmdl subversion
			std::string version = "12.1";

			if (cmdline.HasParam("-version"))
			{
				version = cmdline.GetParamValue("-version", "12.1");
			}
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
						ConvertVGData_12_1(vgInputBuf, vgFilePath, ChangeExtension(mdlPath, "vg_conv"));
					else
						delete[] vgInputBuf;
				}
			}
			else if (version == "8")
			{
				intmax_t mdlFileSize = GetFileSize(mdlPath);

				mdlIn.seek(0, std::ios::beg);

				char* mdlBuf = new char[mdlFileSize];

				mdlIn.getReader()->read(mdlBuf, mdlFileSize);

				ConvertRMDL8To10(mdlBuf, mdlPath, mdlPath);

				delete[] mdlBuf;

				break;
			}
			else
			{
				Error("version is not currently supported\n");
			}

			break;
		}
		default:
		{
			Error("MDL version %i is currently unsupported\n", mdlVersion);
			break;
		}
		}
	}
	else if (mdlPath.find(".rseq"))
	{
		printf("seq gaming\n");

		std::string version = "7.1";

		if (cmdline.HasParam("-version"))
		{
			version = cmdline.GetParamValue("-version", "7.1");
		}
		else
		{
			std::cout << pszRSeqVersionHelpString;
			std::cin >> version;
		}

		uintmax_t seqFileSize = GetFileSize(mdlPath);

		mdlIn.seek(0, std::ios::beg);

		char* seqBuf = new char[seqFileSize];

		mdlIn.getReader()->read(seqBuf, seqFileSize);


		std::string rseqExtPath = ChangeExtension(mdlPath, "rseq_ext");
		char* seqExternalBuf = nullptr;
		if (FILE_EXISTS(rseqExtPath))
		{
			int seqExtFileSize = GetFileSize(rseqExtPath);

			seqExternalBuf = new char[seqExtFileSize];

			std::ifstream ifs(rseqExtPath, std::ios::in | std::ios::binary);

			ifs.read(seqExternalBuf, seqExtFileSize);
		}

		if (version == "7.1")
		{
			//printf("converting rseq version 7.1 to version 7\n");

			ConvertRSEQFrom71To7(seqBuf, seqExternalBuf, mdlPath);
		}
		else if (version == "10")
		{
			ConvertRSEQFrom10To7(seqBuf, seqExternalBuf, mdlPath);
		}

		delete[] seqBuf;
	}
	else
	{
		Error("invalid input file. must be a valid .(r)mdl file with magic 'IDST'\n");
	}
}

int main(int argc, char** argv)
{

	printf("rmdlconv - Copyright (c) %s, rexx\n", &__DATE__[7]);

	CommandLine cmdline(argc, argv);

    if (argc < 2)
        Error("invalid usage\n");

	if (cmdline.HasParam("-convertmodel"))
	{
		if (!cmdline.HasParam("-targetversion"))
			Error("no '-targetversion' param found while trying to convert model(s)!!!\n required for proper conversion, exiting...\n");

		std::string modelPath = cmdline.GetParamValue("-convertmodel");
		int modelVersionTarget = atoi(cmdline.GetParamValue("-targetversion"));

		const char* customDir = nullptr; // custom base folder for models

		if (cmdline.HasParam("-outputdir"))
			customDir = cmdline.GetParamValue("-outputdir");

		UpgradeStudioModel(modelPath, modelVersionTarget, customDir);
	}

	if (cmdline.HasParam("-convertsequence"))
	{
		// todo
	}

	LegacyConversionHandling(cmdline); // this should be cut eventually

	if(!cmdline.HasParam("-nopause"))
		std::system("pause");

	return 0;
}
