#include <iostream>
#include <cstdarg>
#include <filesystem>
#include <fstream>

#include "vg.h"
#include "utils.h"
#include "CommandLine.h"

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

	printf("input RMDL is version %s. Continuing to conversion\n\n", version.c_str());

	// always call ChangeExtension so we guarantee that the path is .vg
	std::string vgFilePath = ChangeExtension(argv[1], "vg");
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
	else
	{
		Error("version %s is not currently supported\n", version.c_str());
	}
}
