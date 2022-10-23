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

	//if (mdlVersion == 54)
	{
		// === RMDL -> RMDL ===

		// version is used to identify the format of the model to be converted,
		// as there are many subversions of MDL v54 which are not easily identifiable from the data alone

		std::string version = "12.1";

		// get version from command line if specified or ask for manual input
		if (!cmdline.HasParam("-version"))
		{
			std::cout << pszVersionHelpString;
			std::cin >> version;
		}
		else
			version = cmdline.GetParamValue("-version", "12.1");

		printf("input MDL is version %s. converting...\n\n", version.c_str());

		// always call ChangeExtension so we guarantee that the path is .vg
		std::string vgFilePath = ChangeExtension(mdlPath, "vg");
		char* vgInputBuf = nullptr;

		if (FILE_EXISTS(vgFilePath))
		{
			uintmax_t vgInputSize = GetFileSize(vgFilePath);

			vgInputBuf = new char[vgInputSize];

			std::ifstream ifs(vgFilePath, std::ios::in | std::ios::binary);

			ifs.read(vgInputBuf, vgInputSize);

			if (*(int*)vgInputBuf != 0x47567430) // 0tVG
				delete[] vgInputBuf;
		}

		if (version == "12.1") // handle 12.1 model conversions
		{
			if (vgInputBuf) // if vgInputBuf == nullptr, there is no valid vg file
				ConvertVGData_12_1(vgInputBuf, vgFilePath);
		}
		else if (version == "8")
		{
			CreateVGFile_v8(mdlPath);
		}
		else
		{
			Error("version is not currently supported\n");
		}
	}

}
