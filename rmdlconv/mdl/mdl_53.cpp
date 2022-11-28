#include "stdafx.h"
#include "versions.h"
#include "rmem.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "BinaryIO.h"

//
// ConvertStudioHdr
// Purpose: converts the mdl v53 (Titanfall 2) studiohdr_t struct to rmdl v8 compatible (Apex Legends Season 0-6)
r5::v8::studiohdr_t ConvertStudioHdr(r2::studiohdr_t& hdr)
{
	r5::v8::studiohdr_t nh{};

	nh.id = 'TSDI';
	nh.version = 54;
	
	nh.checksum = hdr.checksum;

	// :)
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

//
// ConvertMDLData_53
// Purpose: converts mdl data from mdl v53 (Titanfall 2) to rmdl v9 (Apex Legends Season 2/3)
//
void ConvertMDLData_53(char* buf, const std::string& filePath)
{
	rmem input(buf);

	r2::studiohdr_t oldHeader = input.read<r2::studiohdr_t>();
	r5::v8::studiohdr_t newHeader = ConvertStudioHdr(oldHeader);

	std::unique_ptr<char[]> vtxBuf;
	if (oldHeader.vtxsize > 0)
	{
		vtxBuf = std::unique_ptr<char[]>(new char[oldHeader.vtxsize]);

		input.seek(oldHeader.vtxindex, rseekdir::beg);
		input.read(vtxBuf.get(), oldHeader.vtxsize);
	}

	std::unique_ptr<char[]> vvdBuf;
	if (oldHeader.vvdsize > 0)
	{
		vvdBuf = std::unique_ptr<char[]>(new char[oldHeader.vvdsize]);

		input.seek(oldHeader.vvdindex, rseekdir::beg);
		input.read(vvdBuf.get(), oldHeader.vvdsize);
	}

	std::unique_ptr<char[]> vphyBuf;
	if (oldHeader.vphysize > 0)
	{
		vphyBuf = std::unique_ptr<char[]>(new char[oldHeader.vphysize]);

		input.seek(oldHeader.vphyindex, rseekdir::beg);
		input.read(vphyBuf.get(), oldHeader.vphysize);
	}

	std::unique_ptr<char[]> vvcBuf;
	if (oldHeader.vvcsize > 0)
	{
		vvcBuf = std::unique_ptr<char[]>(new char[oldHeader.vvcsize]);

		input.seek(oldHeader.vvcindex, rseekdir::beg);
		input.read(vvcBuf.get(), oldHeader.vvcsize);
	}
}
