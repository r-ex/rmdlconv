#include "stdafx.h"
#include "versions.h"
#include "rmem.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "BinaryIO.h"

//
// ConvertStudioHdr
// Purpose: converts the mdl v53 (Titanfall 2) studiohdr_t struct to rmdl v8 compatible (Apex Legends Season 0-6)
void ConvertStudioHdr(r5::v8::studiohdr_t* out, r2::studiohdr_t& hdr)
{
	out->id = 'TSDI';
	out->version = 54;
	
	out->checksum = hdr.checksum;

	// :)
	if ((_time64(NULL) % 69420) == 0)
		out->checksum = 0xDEADBEEF;

	memcpy_s(out->name, 64, hdr.name, 64);

	out->length = 0xbadf00d; // needs to be written later

	out->eyeposition = hdr.eyeposition;
	out->illumposition = hdr.illumposition;
	out->hull_min = hdr.hull_min;
	out->hull_max = hdr.hull_max;

	out->view_bbmin = hdr.view_bbmin;
	out->view_bbmax = hdr.view_bbmax;

	// these will probably have to be modified at some point
	out->flags = hdr.flags;

	//-| begin count vars
	out->numbones = hdr.numbones;
	out->numbonecontrollers = hdr.numbonecontrollers;
	out->numhitboxsets = hdr.numhitboxsets;
	out->numlocalanim = hdr.numlocalanim;
	out->numlocalseq = hdr.numlocalseq;
	out->activitylistversion = hdr.activitylistversion;
	// eventsindexed --> materialtypesindex

	out->numtextures = hdr.numtextures;
	out->numcdtextures = hdr.numcdtextures;
	out->numskinref = hdr.numskinref;
	out->numskinfamilies = hdr.numskinfamilies;
	out->numbodyparts = hdr.numbodyparts;
	out->numlocalattachments = hdr.numlocalattachments;
	out->numlocalnodes = hdr.numlocalnodes;
	
	// skipping all the deprecated flex vars

	out->numikchains = hdr.numikchains;
	out->numruimeshes = hdr.numruimeshes;
	out->numlocalposeparameters = hdr.numlocalposeparameters;
	out->keyvaluesize = hdr.keyvaluesize;
	out->numlocalikautoplaylocks = hdr.numlocalikautoplaylocks;
	out->numincludemodels = hdr.numincludemodels;

	out->numsrcbonetransform = hdr.numsrcbonetransform;
	//-| end count vars

	//-| begin misc vars
	out->mass = hdr.mass;
	out->contents = hdr.contents;

	out->constdirectionallightdot = hdr.constdirectionallightdot;
	out->rootLOD = hdr.rootLOD;
	out->numAllowedRootLODs = hdr.numAllowedRootLODs;
	out->fadeDistance = hdr.fadeDistance;
	out->flVertAnimFixedPointScale = hdr.flVertAnimFixedPointScale;
	//-| end misc vars
}

void ConvertBones_53(r2::mstudiobone_t* pOldBones, int numBones)
{
	printf("converting %i bones...\n", numBones);
	std::vector<r5::v8::mstudiobone_t*> proceduralBones;

	char* pBoneStart = g_model.pData;
	for (int i = 0; i < numBones; ++i)
	{
		r2::mstudiobone_t* oldBone = &pOldBones[i];

		r5::v8::mstudiobone_t* newBone = reinterpret_cast<r5::v8::mstudiobone_t*>(g_model.pData) + i;

		AddToStringTable((char*)newBone, &newBone->sznameindex, STRING_FROM_IDX(oldBone, oldBone->sznameindex));
		//newBone.name = boneName;

		AddToStringTable((char*)newBone, &newBone->surfacepropidx, STRING_FROM_IDX(oldBone, oldBone->surfacepropidx));
		//newBone.surfaceprop = surfaceprop;

		newBone->parent = oldBone->parent;
		memcpy(&newBone->bonecontroller, &oldBone->bonecontroller, sizeof(oldBone->bonecontroller));
		newBone->pos = oldBone->pos;
		newBone->quat = oldBone->quat;
		newBone->rot = oldBone->rot;
		newBone->scale = oldBone->scale;
		newBone->poseToBone = oldBone->poseToBone;
		newBone->qAlignment = oldBone->qAlignment;
		newBone->flags = oldBone->flags;
		newBone->proctype = oldBone->proctype;
		newBone->procindex = oldBone->procindex;
		newBone->physicsbone = oldBone->physicsbone;
		newBone->contents = oldBone->contents;
		newBone->surfacepropLookup = oldBone->surfacepropLookup;

		if(oldBone->proctype != 0)
			proceduralBones.push_back(newBone);
	}
	g_model.pHdr->boneindex = g_model.pData - g_model.pBase;
	g_model.pData += numBones * sizeof(r5::v8::mstudiobone_t);

	if(proceduralBones.size() > 0)
		printf("converting %lld procedural bones (jiggle bones)...\n", proceduralBones.size());

	for (auto bone : proceduralBones)
	{
		int boneid = ((char*)bone - pBoneStart) / sizeof(r5::v8::mstudiobone_t);
		r2::mstudiobone_t* oldBone = &pOldBones[boneid];
		mstudiojigglebone_t* oldJBone = PTR_FROM_IDX(mstudiojigglebone_t, oldBone, oldBone->procindex);

		r5::v8::mstudiojigglebone_t* jBone = reinterpret_cast<r5::v8::mstudiojigglebone_t*>(g_model.pData);

		bone->procindex = (char*)jBone - (char*)bone;
		jBone->flags = oldJBone->flags;
		jBone->bone = boneid;
		jBone->length = oldJBone->length;
		jBone->tipMass = oldJBone->tipMass;
		jBone->yawStiffness = oldJBone->yawStiffness;
		jBone->yawDamping = oldJBone->yawDamping;
		jBone->pitchStiffness = oldJBone->pitchStiffness;
		jBone->pitchDamping = oldJBone->pitchDamping;
		jBone->alongStiffness = oldJBone->alongStiffness;
		jBone->alongDamping = oldJBone->alongDamping;
		jBone->angleLimit = oldJBone->angleLimit;
		jBone->minYaw = oldJBone->minYaw;
		jBone->maxYaw = oldJBone->maxYaw;
		jBone->yawFriction = oldJBone->yawFriction;
		jBone->yawBounce = oldJBone->yawBounce;
		jBone->baseMass = oldJBone->baseMass;
		jBone->baseStiffness = oldJBone->baseStiffness;
		jBone->baseDamping = oldJBone->baseDamping;
		jBone->baseMinLeft = oldJBone->baseMinLeft;
		jBone->baseMaxLeft = oldJBone->baseMaxLeft;
		jBone->baseLeftFriction = oldJBone->baseLeftFriction;
		jBone->baseMinUp = oldJBone->baseMinUp;
		jBone->baseMaxUp = oldJBone->baseMaxUp;
		jBone->baseUpFriction = oldJBone->baseUpFriction;
		jBone->baseMinForward = oldJBone->baseMinForward;
		jBone->baseMaxForward = oldJBone->baseMaxForward;
		jBone->baseForwardFriction = oldJBone->baseForwardFriction;

		g_model.pData += sizeof(r5::v8::mstudiojigglebone_t);
	}
}

void ConvertAttachments_53(mstudioattachment_t* pOldAttachments, int numAttachments)
{
	printf("converting %i attachments...\n", numAttachments);

	for (int i = 0; i < numAttachments; ++i)
	{
		mstudioattachment_t* oldAttach = &pOldAttachments[i];

		r5::v8::mstudioattachment_t* attach = reinterpret_cast<r5::v8::mstudioattachment_t*>(g_model.pData) + i;

		AddToStringTable((char*)attach, &attach->sznameindex, STRING_FROM_IDX(oldAttach, oldAttach->sznameindex));
		attach->flags = oldAttach->flags;
		attach->localbone = oldAttach->localbone;
		memcpy(&attach->localmatrix, &oldAttach->localmatrix, sizeof(oldAttach->localmatrix));
	}
	g_model.pHdr->localattachmentindex = g_model.pData - g_model.pBase;
	g_model.pData += numAttachments * sizeof(r5::v8::mstudioattachment_t);
}

#define FILEBUFSIZE (32 * 1024 * 1024)

//
// ConvertMDLData_53
// Purpose: converts mdl data from mdl v53 (Titanfall 2) to rmdl v9 (Apex Legends Season 2/3)
//
void ConvertMDLData_53(char* buf, const std::string& filePath)
{
	rmem input(buf);

	r2::studiohdr_t oldHeader = input.read<r2::studiohdr_t>();

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

	std::string rmdlPath = ChangeExtension(filePath, "rmdl");
	std::ofstream out(rmdlPath, std::ios::out | std::ios::binary);


	// === convert old file structs to new mem structs ===
	// 
	// TODO[rexx]: investigate whether these can be written directly into the buf below
	//             instead of being converted to mem structs first
	//

	// allocate temp file buffer
	g_model.pBase = new char[FILEBUFSIZE]{};
	g_model.pData = g_model.pBase;

	// convert mdl hdr
	r5::v8::studiohdr_t* pHdr = (r5::v8::studiohdr_t*)g_model.pData;
	ConvertStudioHdr(pHdr, oldHeader);
	g_model.pHdr = pHdr;
	g_model.pData += sizeof(r5::v8::studiohdr_t);

	// init string table so we can use 
	BeginStringTable();

	AddToStringTable((char*)pHdr, &pHdr->sznameindex, STRING_FROM_IDX(buf, oldHeader.sznameindex));
	AddToStringTable((char*)pHdr, &pHdr->surfacepropindex, STRING_FROM_IDX(buf, oldHeader.surfacepropindex));
	AddToStringTable((char*)pHdr, &pHdr->unkstringindex, STRING_FROM_IDX(buf, oldHeader.unkstringindex));

	input.seek(oldHeader.boneindex, rseekdir::beg);
	ConvertBones_53((r2::mstudiobone_t*)input.getPtr(), oldHeader.numbones);

	input.seek(oldHeader.localattachmentindex, rseekdir::beg);
	ConvertAttachments_53((mstudioattachment_t*)input.getPtr(), oldHeader.numlocalattachments);

	g_model.pData = WriteStringTable(g_model.pData);

	pHdr->length = g_model.pData - g_model.pBase;

	out.write(g_model.pBase, pHdr->length);


	delete[] g_model.pBase;
	printf("Done!\n");
}
