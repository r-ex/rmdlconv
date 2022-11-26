#include "stdafx.h"
#include "versions.h"
#include "rmem.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "BinaryIO.h"

r5::v8::studiohdr_t ConvertStudioHdr(r2::studiohdr_t& hdr)
{
	r5::v8::studiohdr_t nh{};

	nh.id = 'TSDI';
	nh.version = 54;
	
	nh.checksum = hdr.checksum;

	if ((_time64(NULL) % 69420) == 0)
		nh.checksum = 0xDEADBEEF;

	memcpy_s(nh.name, 64, hdr.name, 64);

	nh.length = 0xbadf00d; // needs to be written later

	nh.eyeposition = hdr.eyeposition;
	nh.illumposition = hdr.illumposition;
	nh.hull_min = hdr.hull_min;
	nh.hull_max = hdr.hull_max;

	nh.view_bbmin = hdr.view_bbmin;
	nh.view_bbmax = hdr.view_bbmax;

	// these will probably have to be modified at some point
	nh.flags = hdr.flags;

	//-| begin count vars
	nh.numbones = hdr.numbones;
	nh.numbonecontrollers = hdr.numbonecontrollers;
	nh.numhitboxsets = hdr.numhitboxsets;
	nh.numlocalanim = hdr.numlocalanim;
	nh.numlocalseq = hdr.numlocalseq;
	nh.activitylistversion = hdr.activitylistversion;
	// eventsindexed --> materialtypesindex

	nh.numtextures = hdr.numtextures;
	nh.numcdtextures = hdr.numcdtextures;
	nh.numskinref = hdr.numskinref;
	nh.numskinfamilies = hdr.numskinfamilies;
	nh.numbodyparts = hdr.numbodyparts;
	nh.numlocalattachments = hdr.numlocalattachments;
	nh.numlocalnodes = hdr.numlocalnodes;
	
	// skipping all the deprecated flex vars

	nh.numikchains = hdr.numikchains;
	nh.numruimeshes = hdr.numruimeshes;
	nh.numlocalposeparameters = hdr.numlocalposeparameters;
	nh.keyvaluesize = hdr.keyvaluesize;
	nh.numlocalikautoplaylocks = hdr.numlocalikautoplaylocks;
	nh.numincludemodels = hdr.numincludemodels;

	nh.numsrcbonetransform = hdr.numsrcbonetransform;
	//-| end count vars

	//-| begin misc vars
	nh.mass = hdr.mass;
	nh.contents = hdr.contents;

	nh.constdirectionallightdot = hdr.constdirectionallightdot;
	nh.rootLOD = hdr.rootLOD;
	nh.numAllowedRootLODs = hdr.numAllowedRootLODs;
	nh.fadeDistance = hdr.fadeDistance;
	nh.flVertAnimFixedPointScale = hdr.flVertAnimFixedPointScale;
	//-| end misc vars


	return nh;
}

void ConvertMDLData_53(char* buf, const std::string& filePath)
{
	std::filesystem::path path(filePath);

	std::string mdlPath = ChangeExtension(filePath, "mdl");

	rmem input(buf);

	
}
