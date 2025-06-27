// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include <pch.h>
#include <studio/studio.h>
#include <studio/versions.h>
#include <studio/common.h>


/*
	Type:    RMDL
	Version: 12.1
	Game:    Apex Legends Season 7

	Files: .rmdl, .vg
*/

void ConvertVGData_12_1(char* buf, const std::string& filePath, const std::string& pathOut)
{
	//std::filesystem::path /*path*/(outPath);

	// this needs to be changed to put the file in the right output dir
	//std::string pathOut = outPath;

	rmem input(buf);

	vg::rev2::VertexGroupHeader_t vghInput = input.read<vg::rev2::VertexGroupHeader_t>();

	size_t vertexBufSize = 0;
	size_t indexBufSize = 0;
	size_t extendedWeightsBufSize = 0;
	size_t externalWeightsBufSize = 0;
	size_t stripsBufSize = 0;
	size_t lodBufSize = vghInput.lodCount * sizeof(vg::rev1::ModelLODHeader_t);
	short lodSubmeshCount = 0;

	//char* lodBuf = new char[lodBufSize];
	std::unique_ptr<char[]> lodBuf(new char[lodBufSize]);
	rmem lods(lodBuf.get());

	for (int i = 0; i < vghInput.lodCount; ++i)
	{
		size_t thisLodOffset = 0x18 + (i * sizeof(vg::rev2::ModelLODHeader_t)) + vghInput.lodOffset;
		input.seek(thisLodOffset, rseekdir::beg);
		vg::rev2::ModelLODHeader_t lodInput = input.read<vg::rev2::ModelLODHeader_t>();

		vg::rev1::ModelLODHeader_t lod{ lodSubmeshCount, (short)lodInput.meshCount, lodInput.switchPoint};

		lods.write(lod);

		for (int j = 0; j < lodInput.meshCount; ++j)
		{
			size_t thisSubmeshOffset = thisLodOffset + offsetof(vg::rev2::ModelLODHeader_t, meshOffset) + lodInput.meshOffset + (j * sizeof(vg::rev2::MeshHeader_t));
			input.seek(thisSubmeshOffset, rseekdir::beg);

			vg::rev2::MeshHeader_t submesh = input.read<vg::rev2::MeshHeader_t>();
			vertexBufSize += submesh.vertBufferSize;
			indexBufSize += submesh.indexCount * 2;
			extendedWeightsBufSize += submesh.externalWeightSize;
			externalWeightsBufSize += submesh.legacyWeightCount * 0x10;
			stripsBufSize += submesh.stripCount * sizeof(OptimizedModel::StripHeader_t);
		}

		lodSubmeshCount += lodInput.meshCount;
	}

	std::unique_ptr<char[]> vertexBuf(new char[vertexBufSize]);
	std::unique_ptr<char[]> indexBuf(new char[indexBufSize]);
	std::unique_ptr<char[]> extendedWeightsBuf(new char[extendedWeightsBufSize]);
	std::unique_ptr<char[]> externalWeightsBuf(new char[externalWeightsBufSize]);
	std::unique_ptr<char[]> stripsBuf(new char[stripsBufSize]);
	std::unique_ptr<char[]> meshBuf(new char[lodSubmeshCount * sizeof(vg::rev1::MeshHeader_t)]);

	printf("VG: allocatedbuffers:\n");
	printf(
		"vertex: %lld\nindex: %lld\nextendedWeights: %lld\nexternalWeights: %lld\n",
		vertexBufSize,
		indexBufSize,
		extendedWeightsBufSize,
		externalWeightsBufSize);

	// reuse vars for size added
	vertexBufSize = 0;
	indexBufSize = 0;
	extendedWeightsBufSize = 0;
	externalWeightsBufSize = 0;
	stripsBufSize = 0;

	rmem submeshes(meshBuf.get());

	// populate buffers fr
	for (int i = 0; i < vghInput.lodCount; ++i)
	{
		size_t thisLodOffset = 0x18 + (i * sizeof(vg::rev2::ModelLODHeader_t)) + vghInput.lodOffset;
		input.seek(thisLodOffset, rseekdir::beg);
		vg::rev2::ModelLODHeader_t lodInput = input.read<vg::rev2::ModelLODHeader_t>();

		for (int j = 0; j < lodInput.meshCount; ++j)
		{
			size_t thisSubmeshOffset = thisLodOffset + offsetof(vg::rev2::ModelLODHeader_t, meshOffset) + lodInput.meshOffset + (j * sizeof(vg::rev2::MeshHeader_t));
			input.seek(thisSubmeshOffset, rseekdir::beg);

			char* thisSubmeshPointer = reinterpret_cast<char*>(input.getPtr());

			vg::rev2::MeshHeader_t submeshInput = input.read<vg::rev2::MeshHeader_t>();

			vg::rev1::MeshHeader_t submesh{};

			submesh.flags = submeshInput.flags;
			submesh.vertCacheSize = (unsigned int)submeshInput.vertCacheSize;
			submesh.vertCount = (unsigned int)submeshInput.vertCount;
			submesh.indexCount = (unsigned int)submeshInput.indexCount;
			submesh.extraBoneWeightSize = (unsigned int)submeshInput.externalWeightSize;
			submesh.legacyWeightCount = (unsigned int)submeshInput.legacyWeightCount;
			submesh.stripCount = (unsigned int)submeshInput.stripCount;

			submesh.vertOffset = (unsigned int)vertexBufSize;
			submesh.indexOffset = (unsigned int)indexBufSize / sizeof(uint16_t);
			submesh.extraBoneWeightSize = (unsigned int)extendedWeightsBufSize;
			submesh.legacyWeightOffset = (unsigned int)externalWeightsBufSize;
			submesh.stripOffset = (unsigned int)stripsBufSize / sizeof(OptimizedModel::StripHeader_t);

			submeshes.write(submesh);
			
			void* vtxPtr = (thisSubmeshPointer + offsetof(vg::rev2::MeshHeader_t, vertOffset) + submeshInput.vertOffset);
			std::memcpy(vertexBuf.get() + vertexBufSize, vtxPtr, submeshInput.vertBufferSize);
			vertexBufSize += submeshInput.vertBufferSize;

			void* indexPtr = (thisSubmeshPointer + offsetof(vg::rev2::MeshHeader_t, indexOffset) + submeshInput.indexOffset);
			std::memcpy(indexBuf.get() + indexBufSize, indexPtr, submeshInput.indexCount * 2);
			indexBufSize += submeshInput.indexCount * 2;

			void* extendedWeightsPtr = (thisSubmeshPointer + offsetof(vg::rev2::MeshHeader_t, externalWeightOffset) + submeshInput.externalWeightOffset);
			std::memcpy(extendedWeightsBuf.get() + extendedWeightsBufSize, extendedWeightsPtr, submeshInput.externalWeightSize);
			extendedWeightsBufSize += submeshInput.externalWeightSize;

			void* externalWeightsPtr = (thisSubmeshPointer + offsetof(vg::rev2::MeshHeader_t, legacyWeightOffset) + submeshInput.legacyWeightOffset);
			std::memcpy(externalWeightsBuf.get() + externalWeightsBufSize, externalWeightsPtr, submeshInput.legacyWeightCount * 0x10);
			externalWeightsBufSize += submeshInput.legacyWeightCount * 0x10;

			void* stripsPtr = (thisSubmeshPointer + offsetof(vg::rev2::MeshHeader_t, stripOffset) + submeshInput.stripOffset);
			std::memcpy(stripsBuf.get() + stripsBufSize, stripsPtr, submeshInput.stripCount * sizeof(OptimizedModel::StripHeader_t));
			stripsBufSize += submeshInput.stripCount * sizeof(OptimizedModel::StripHeader_t);
		}
	}

	std::string rmdlPath = ChangeExtension(filePath, "rmdl");

	char* boneRemapBuf = nullptr;
	unsigned int boneRemapCount = 0;

	char* unkDataBuf = nullptr;

	if (std::filesystem::exists(rmdlPath) && GetFileSize(rmdlPath) > sizeof(r5::v121::studiohdr_t))
	{
		// grab bone remaps from rmdl
		std::ifstream ifs(rmdlPath, std::ios::in | std::ios::binary);

		r5::v121::studiohdr_t hdr;

		ifs.read((char*)&hdr, sizeof(hdr));

		if (hdr.boneStateCount > 0)
		{
			ifs.seekg(offsetof(r5::v121::studiohdr_t, boneStateOffset) + hdr.boneStateOffset, std::ios::beg);

			boneRemapCount = hdr.boneStateCount;

			boneRemapBuf = new char[boneRemapCount];
			ifs.read(boneRemapBuf, boneRemapCount);
		}

		if (hdr.vgMeshCount > 0)
		{
			ifs.seekg(offsetof(r5::v121::studiohdr_t, vgMeshOffset) + hdr.vgMeshOffset, std::ios::beg);

			unkDataBuf = new char[hdr.vgMeshCount * 0x30];
			ifs.read(unkDataBuf, hdr.vgMeshCount * 0x30);
		}

		// close rmdl stream once we are done with it
		ifs.close();
	}

	vg::rev1::VertexGroupHeader_t vgh{};
	vgh.id = 'GVt0';
	vgh.version = 1;
	vgh.boneStateChangeCount = boneRemapCount;
	vgh.meshCount = lodSubmeshCount;
	vgh.indexCount = indexBufSize / 2;
	vgh.vertBufferSize = vertexBufSize;
	vgh.extraBoneWeightSize = extendedWeightsBufSize;
	vgh.lodCount = vghInput.lodCount;
	vgh.unknownCount = vgh.meshCount / vgh.lodCount;
	vgh.legacyWeightOffset = externalWeightsBufSize / 0x10;
	vgh.stripCount = stripsBufSize / sizeof(OptimizedModel::StripHeader_t);

	BinaryIO out;

	out.open(pathOut, BinaryIOMode::Write);

	out.write(vgh);

	vgh.boneStateChangeOffset = out.tell();
	if(boneRemapCount)
		out.getWriter()->write(boneRemapBuf, boneRemapCount);

	vgh.meshOffset = out.tell();
	out.getWriter()->write(meshBuf.get(), lodSubmeshCount * sizeof(vg::rev1::MeshHeader_t));

	vgh.indexOffset = out.tell();
	out.getWriter()->write(indexBuf.get(), indexBufSize);

	vgh.vertOffset = out.tell();
	out.getWriter()->write(vertexBuf.get(), vertexBufSize);

	vgh.extraBoneWeightOffset = out.tell();
	out.getWriter()->write(extendedWeightsBuf.get(), extendedWeightsBufSize);

	// if this data hasn't been retrieved from .rmdl, write it as null bytes
	if(!unkDataBuf)
		unkDataBuf = new char[vgh.unknownCount * 0x30]{};

	vgh.unknownOffset = out.tell();
	out.getWriter()->write(unkDataBuf, vgh.unknownCount * 0x30);

	vgh.lodOffset = out.tell();
	out.getWriter()->write(lodBuf.get(), lodBufSize);

	vgh.legacyWeightOffset = out.tell();
	out.getWriter()->write(externalWeightsBuf.get(), externalWeightsBufSize);

	vgh.stripOffset = out.tell();
	out.getWriter()->write(stripsBuf.get(), stripsBufSize);

	vgh.dataSize = (unsigned int)out.tell();

	out.seek(0, std::ios::beg);

	out.write(vgh);

	out.close();

	printf("done! freeing buffers\n");

	if (boneRemapBuf)
		delete[] boneRemapBuf;

	delete[] unkDataBuf;
}

//
// ConvertStudioHdr
void ConvertStudioHdr(r5::v8::studiohdr_t* out, r5::v121::studiohdr_t* hdr)
{
	out->id = 'TSDI';
	out->version = 54;

	out->checksum = hdr->checksum;

	// :)
	//if ((_time64(NULL) % 69420) == 0)
	//	out->checksum = 0xDEADBEEF;

	memcpy_s(out->name, 64, hdr->name, 64);

	out->length = 0xbadf00d; // needs to be written later

	out->eyeposition = hdr->eyeposition;
	out->illumposition = hdr->illumposition;
	out->hull_min = hdr->hull_min;
	out->hull_max = hdr->hull_max;

	out->mins = hdr->hull_min;
	out->maxs = hdr->hull_max;

	out->view_bbmin = hdr->view_bbmin;
	out->view_bbmax = hdr->view_bbmax;

	// these will probably have to be modified at some point
	out->flags = hdr->flags;

	//-| begin count vars
	out->numbones = hdr->numbones;
	out->numbonecontrollers = hdr->numbonecontrollers;
	out->numhitboxsets = hdr->numhitboxsets;
	out->numlocalanim = 0; // this is no longer used, force set to 0
	out->numlocalseq = hdr->numlocalseq;
	out->activitylistversion = hdr->activitylistversion;
	// eventsindexed --> materialtypesindex

	out->numtextures = hdr->numtextures;
	out->numcdtextures = hdr->numcdtextures;
	out->numskinref = hdr->numskinref;
	out->numskinfamilies = hdr->numskinfamilies;
	out->numbodyparts = hdr->numbodyparts;
	out->numlocalattachments = hdr->numlocalattachments;

	// next few comments are mostly for rigs

	//out->numlocalnodes = hdr->numlocalnodes;

	// skipping all the deprecated flex vars

	//out->numikchains = hdr->numikchains;
	//out->numruimeshes = hdr->numruimeshes;
	//out->numlocalposeparameters = hdr->numlocalposeparameters;
	out->keyvaluesize = hdr->keyvaluesize;
	//out->numlocalikautoplaylocks = hdr->numlocalikautoplaylocks; // cut?

	out->numincludemodels = -1;

	// why did i add this?
	//out->numincludemodels = hdr->numincludemodels;

	out->numsrcbonetransform = hdr->numsrcbonetransform;
	//-| end count vars

	//-| begin misc vars
	out->mass = hdr->mass;
	out->contents = hdr->contents;

	//out->constdirectionallightdot = hdr->constdirectionallightdot;
	//out->rootLOD = hdr->rootLOD;
	//out->numAllowedRootLODs = hdr->numAllowedRootLODs;
	out->defaultFadeDist = hdr->defaultFadeDist;
	out->flVertAnimFixedPointScale = hdr->flVertAnimFixedPointScale;
	//-| end misc vars

	//-| begin for giggles
	/*out->vtxindex = -1;
	out->vvdindex = hdr->vtxSize;
	out->vvcindex = hdr->vtxSize + hdr->vvdSize;
	out->vphyindex = -123456;*/

	out->phyOffset = hdr->phyOffset;
	out->vtxSize = hdr->vtxSize;
	out->vvdSize = hdr->vvdSize;
	out->vvcSize = hdr->vvcSize;
	out->phySize = hdr->phySize;
	out->vvwSize = hdr->vvwSize;
	//-| end for giggles
}

void GenerateRigHdr(r5::v8::studiohdr_t* out, r5::v121::studiohdr_t* hdr)
{
	out->id = 'TSDI';
	out->version = 54;

	memcpy_s(out->name, 64, hdr->name, 64);

	out->numbones = hdr->numbones;
	out->numbonecontrollers = hdr->numbonecontrollers;
	out->numhitboxsets = hdr->numhitboxsets;
	out->numlocalattachments = hdr->numlocalattachments;
	out->numlocalnodes = hdr->numlocalnodes;
	out->numikchains = hdr->numikchains;
	out->numlocalposeparameters = hdr->numlocalposeparameters;

	out->mass = hdr->mass;
	out->contents = hdr->contents;

	// hard to tell if the first three are required
	//out->constdirectionallightdot = hdr->constdirectionallightdot;
	//out->rootLOD = hdr->rootLOD;
	//out->numAllowedRootLODs = hdr->numAllowedRootLODs;
	out->defaultFadeDist = hdr->defaultFadeDist;
}

void ConvertBones_121(r5::v121::mstudiobone_t* pOldBones, int numBones, bool isRig)
{
	printf("converting %i bones...\n", numBones);
	std::vector<r5::v8::mstudiobone_t*> proceduralBones;

	char* pBoneStart = g_model.pData;
	for (int i = 0; i < numBones; ++i)
	{
		r5::v121::mstudiobone_t* oldBone = &pOldBones[i];

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
		newBone->flags = oldBone->flags; // rigs should only have certain flags
		//newBone->proctype = oldBone->proctype;
		//newBone->procindex = oldBone->procindex;
		//newBone->physicsbone = oldBone->physicsbone;
		newBone->contents = oldBone->contents;
		newBone->surfacepropLookup = oldBone->surfacepropLookup;

		if (!isRig)
		{
			newBone->proctype = oldBone->proctype;
			newBone->procindex = oldBone->procindex;
			newBone->physicsbone = oldBone->physicsbone;

			if (oldBone->proctype != 0)
				proceduralBones.push_back(newBone);
		}
	}
	g_model.hdrV54()->boneindex = g_model.pData - g_model.pBase;
	g_model.pData += numBones * sizeof(r5::v8::mstudiobone_t);

	ALIGN4(g_model.pData);

	// rigs do not have proc bones
	if (isRig)
		return;

	if (proceduralBones.size() > 0)
		printf("converting %lld procedural bones (jiggle bones)...\n", proceduralBones.size());

	std::map<uint8_t, uint8_t> linearprocbones;

	for (auto bone : proceduralBones)
	{
		int boneid = ((char*)bone - pBoneStart) / sizeof(r5::v8::mstudiobone_t);
		r5::v121::mstudiobone_t* oldBone = &pOldBones[boneid];
		mstudiojigglebone_t* oldJBone = PTR_FROM_IDX(mstudiojigglebone_t, oldBone, oldBone->procindex);

		r5::v8::mstudiojigglebone_t* jBone = reinterpret_cast<r5::v8::mstudiojigglebone_t*>(g_model.pData);

		if (oldJBone->flags & JIGGLE_IS_RIGID)
		{
			Error("Apex Legends does not support 'is_rigid' type jigglebones");
		}

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

		jBone->minPitch = oldJBone->minPitch;
		jBone->maxPitch = oldJBone->maxPitch;
		jBone->pitchFriction = oldJBone->pitchFriction;
		jBone->pitchBounce = oldJBone->pitchBounce;

		jBone->flags |= JIGGLE_UNK; // this is required for a lot of jigglbone checks, they no longer check for 'JIGGLE_IS_FLEXIBLE'

		linearprocbones.emplace(jBone->bone, linearprocbones.size());

		g_model.pData += sizeof(r5::v8::mstudiojigglebone_t);
	}

	ALIGN4(g_model.pData);

	if (linearprocbones.size() == 0)
		return;

	g_model.hdrV54()->procBoneCount = linearprocbones.size();
	g_model.hdrV54()->procBoneTableOffset = g_model.pData - g_model.pBase;

	for (auto& it : linearprocbones)
	{
		*g_model.pData = it.first;
		g_model.pData += sizeof(uint8_t);
	}

	g_model.hdrV54()->linearProcBoneOffset = g_model.pData - g_model.pBase;

	for (int i = 0; i < numBones; i++)
	{
		*g_model.pData = linearprocbones.count(i) ? linearprocbones.find(i)->second : 0xff;
		g_model.pData += sizeof(uint8_t);
	}

	ALIGN4(g_model.pData);
}

void ConvertHitboxes_121(mstudiohitboxset_t* pOldHitboxSets, int numHitboxSets)
{
	printf("converting %i hitboxsets...\n", numHitboxSets);

	g_model.hdrV54()->hitboxsetindex = g_model.pData - g_model.pBase;

	mstudiohitboxset_t* hboxsetStart = reinterpret_cast<mstudiohitboxset_t*>(g_model.pData);
	for (int i = 0; i < numHitboxSets; ++i)
	{
		mstudiohitboxset_t* oldhboxset = &pOldHitboxSets[i];
		mstudiohitboxset_t* newhboxset = reinterpret_cast<mstudiohitboxset_t*>(g_model.pData);

		memcpy(g_model.pData, oldhboxset, sizeof(mstudiohitboxset_t));

		AddToStringTable((char*)newhboxset, &newhboxset->sznameindex, STRING_FROM_IDX(oldhboxset, oldhboxset->sznameindex));

		g_model.pData += sizeof(mstudiohitboxset_t);
	}

	for (int i = 0; i < numHitboxSets; ++i)
	{
		mstudiohitboxset_t* oldhboxset = &pOldHitboxSets[i];
		mstudiohitboxset_t* newhboxset = hboxsetStart + i;

		newhboxset->hitboxindex = g_model.pData - (char*)newhboxset;

		r5::v8::mstudiobbox_t* oldHitboxes = reinterpret_cast<r5::v8::mstudiobbox_t*>((char*)oldhboxset + oldhboxset->hitboxindex);

		for (int j = 0; j < newhboxset->numhitboxes; ++j)
		{
			r5::v8::mstudiobbox_t* oldHitbox = oldHitboxes + j;
			r5::v8::mstudiobbox_t* newHitbox = reinterpret_cast<r5::v8::mstudiobbox_t*>(g_model.pData);

			memcpy(g_model.pData, oldHitbox, sizeof(r5::v8::mstudiobbox_t));

			AddToStringTable((char*)newHitbox, &newHitbox->szhitboxnameindex, STRING_FROM_IDX(oldHitbox, oldHitbox->szhitboxnameindex));
			AddToStringTable((char*)newHitbox, &newHitbox->hitdataGroupOffset, STRING_FROM_IDX(oldHitbox, oldHitbox->hitdataGroupOffset));

			g_model.pData += sizeof(r5::v8::mstudiobbox_t);
		}
	}

	ALIGN4(g_model.pData);
}

void ConvertBodyParts_121(mstudiobodyparts_t* pOldBodyParts, int numBodyParts)
{
	printf("converting %i bodyparts...\n", numBodyParts);

	g_model.hdrV54()->bodypartindex = g_model.pData - g_model.pBase;

	mstudiobodyparts_t* bodypartStart = reinterpret_cast<mstudiobodyparts_t*>(g_model.pData);
	for (int i = 0; i < numBodyParts; ++i)
	{
		mstudiobodyparts_t* oldbodypart = &pOldBodyParts[i];
		mstudiobodyparts_t* newbodypart = reinterpret_cast<mstudiobodyparts_t*>(g_model.pData);

		memcpy(g_model.pData, oldbodypart, sizeof(mstudiobodyparts_t));

		printf("%s\n", STRING_FROM_IDX(oldbodypart, oldbodypart->sznameindex));
		AddToStringTable((char*)newbodypart, &newbodypart->sznameindex, STRING_FROM_IDX(oldbodypart, oldbodypart->sznameindex));

		g_model.pData += sizeof(mstudiobodyparts_t);
	}

	for (int i = 0; i < numBodyParts; ++i)
	{
		mstudiobodyparts_t* oldbodypart = &pOldBodyParts[i];
		mstudiobodyparts_t* newbodypart = bodypartStart + i;

		newbodypart->modelindex = g_model.pData - (char*)newbodypart;

		// pointer to old models (in .mdl)
		r5::v121::mstudiomodel_t* oldModels = reinterpret_cast<r5::v121::mstudiomodel_t*>((char*)oldbodypart + oldbodypart->modelindex);

		// pointer to start of new model data (in .rmdl)
		r5::v8::mstudiomodel_t* newModels = reinterpret_cast<r5::v8::mstudiomodel_t*>(g_model.pData);
		for (int j = 0; j < newbodypart->nummodels; ++j)
		{
			r5::v121::mstudiomodel_t* oldModel = oldModels + j;
			r5::v8::mstudiomodel_t* newModel = reinterpret_cast<r5::v8::mstudiomodel_t*>(g_model.pData);

			memcpy(&newModel->name, &oldModel->name, sizeof(newModel->name));
			newModel->type = oldModel->type;
			newModel->boundingradius = oldModel->boundingradius;
			newModel->nummeshes = oldModel->nummeshes;
			newModel->numvertices = oldModel->numvertices;
			newModel->vertexindex = oldModel->vertexindex;
			newModel->tangentsindex = oldModel->tangentsindex;
			newModel->numattachments = oldModel->numattachments;
			newModel->attachmentindex = oldModel->attachmentindex;
			newModel->deprecated_numeyeballs = 0;// oldModel->deprecated_numeyeballs;
			newModel->deprecated_eyeballindex = 0;// oldModel->deprecated_eyeballindex;
			newModel->colorindex = oldModel->colorindex;
			newModel->uv2index = oldModel->uv2index;

			g_model.pData += sizeof(r5::v8::mstudiomodel_t);
		}

		for (int j = 0; j < newbodypart->nummodels; ++j)
		{
			r5::v121::mstudiomodel_t* oldModel = oldModels + j;
			r5::v8::mstudiomodel_t* newModel = newModels + j;

			newModel->meshindex = g_model.pData - (char*)newModel;

			// pointer to old meshes for this model (in .mdl)
			r5::v121::mstudiomesh_t* oldMeshes = reinterpret_cast<r5::v121::mstudiomesh_t*>((char*)oldModel + oldModel->meshindex);

			// pointer to new meshes for this model (in .rmdl)
			r5::v8::mstudiomesh_t* newMeshes = reinterpret_cast<r5::v8::mstudiomesh_t*>(g_model.pData);

			for (int k = 0; k < newModel->nummeshes; ++k)
			{
				r5::v121::mstudiomesh_t* oldMesh = oldMeshes + k;
				r5::v8::mstudiomesh_t* newMesh = newMeshes + k;

				newMesh->material = oldMesh->material;
				newMesh->numvertices = oldMesh->numvertices;
				newMesh->vertexoffset = oldMesh->vertexoffset;
				newMesh->meshid = oldMesh->meshid;
				newMesh->center = oldMesh->center;
				newMesh->vertexloddata = oldMesh->vertexloddata;

				newMesh->modelindex = (char*)newModel - (char*)newMesh;

				g_model.pData += sizeof(r5::v8::mstudiomesh_t);
			}
		}
	}

	ALIGN4(g_model.pData);
}

void ConvertIkChains_121(r5::v8::mstudioikchain_t* pOldIkChains, int numIkChains, bool isRig)
{
	g_model.hdrV54()->ikchainindex = g_model.pData - g_model.pBase;

	if (!isRig)
		return;

	printf("converting %i ikchains...\n", numIkChains);

	int currentLinkCount = 0;
	std::vector<r5::v8::mstudioiklink_t> ikLinks;

	for (int i = 0; i < numIkChains; i++)
	{
		r5::v8::mstudioikchain_t* oldChain = &pOldIkChains[i];
		r5::v8::mstudioikchain_t* newChain = reinterpret_cast<r5::v8::mstudioikchain_t*>(g_model.pData);



		AddToStringTable((char*)newChain, &newChain->sznameindex, STRING_FROM_IDX(oldChain, oldChain->sznameindex));

		newChain->linktype = oldChain->linktype;
		newChain->numlinks = oldChain->numlinks;
		newChain->linkindex = (sizeof(r5::v8::mstudioiklink_t) * currentLinkCount) + (sizeof(r5::v8::mstudioikchain_t) * (numIkChains - i));
		newChain->unk = oldChain->unk;

		g_model.pData += sizeof(r5::v8::mstudioikchain_t);

		currentLinkCount += oldChain->numlinks;
	}

	for (int i = 0; i < numIkChains; i++)
	{
		r5::v8::mstudioikchain_t* oldChain = &pOldIkChains[i];

		for (int linkIdx = 0; linkIdx < oldChain->numlinks; linkIdx++)
		{
			mstudioiklink_t* oldLink = PTR_FROM_IDX(mstudioiklink_t, oldChain, oldChain->linkindex + (sizeof(mstudioiklink_t) * linkIdx));
			r5::v8::mstudioiklink_t* newLink = reinterpret_cast<r5::v8::mstudioiklink_t*>(g_model.pData);

			newLink->bone = oldLink->bone;
			newLink->kneeDir = oldLink->kneeDir;

			g_model.pData += sizeof(r5::v8::mstudioiklink_t);
		}
	}

	ALIGN4(g_model.pData);
}

static void ConvertUIPanelMeshes(const r5::v121::studiohdr_t* const oldHeader, rmem& input)
{
	if (oldHeader->uiPanelCount == 0)
		return;

	g_model.hdrV54()->uiPanelCount = oldHeader->uiPanelCount;
	input.seek(oldHeader->uiPanelOffset, rseekdir::beg);

	const size_t totalHeaderBufSize = oldHeader->uiPanelCount * sizeof(r5::v8::mstudiorruiheader_t);
	input.read(g_model.pData, totalHeaderBufSize, true);

	g_model.hdrV54()->uiPanelOffset = static_cast<int>(g_model.pData - g_model.pBase);

	r5::v8::mstudiorruiheader_t* const ruiHeaders = reinterpret_cast<r5::v8::mstudiorruiheader_t*>(g_model.pData);
	g_model.pData += totalHeaderBufSize;

	// The RUI mesh data itself must be aligned to 16 bytes.
	ALIGN16(g_model.pData);

	for (int i = 0; i < oldHeader->uiPanelCount; i++)
	{
		r5::v8::mstudiorruiheader_t* ruiHeader = &ruiHeaders[i];
		const size_t seekOffset = (oldHeader->uiPanelOffset + (i * sizeof(r5::v8::mstudiorruiheader_t))) + ruiHeader->ruimeshindex;

		input.seek(seekOffset, rseekdir::beg);
		input.read(g_model.pData, sizeof(r5::v8::mstudioruimesh_t), true);

		// Update the mesh index as it can be different due to header alignment.
		ruiHeader->ruimeshindex = static_cast<int>(g_model.pData - reinterpret_cast<const char*>(ruiHeader));

		const r5::v8::mstudioruimesh_t* const header = reinterpret_cast<r5::v8::mstudioruimesh_t*>(g_model.pData);
		g_model.pData += sizeof(r5::v8::mstudioruimesh_t);

		//size_t uiPanelNameBufLength = 0; // Includes the null.

		// Read UI mesh name string and padding, this is within the
		// space between our cursor and the parent index.
		input.read(g_model.pData, header->parentindex, true);
		g_model.pData += header->parentindex;

		// Parents.
		const size_t parentBytes = header->numparents * sizeof(short);
		input.read(g_model.pData, parentBytes, true);

		g_model.pData += parentBytes;

		// Vertex maps.
		const size_t vertMapBytes = header->numfaces * sizeof(r5::v8::mstudioruivertmap_t);
		input.read(g_model.pData, vertMapBytes, true);

		g_model.pData += vertMapBytes;

		// Fourth vertices.
		const size_t fourthVertBytes = header->numfaces * sizeof(r5::v8::mstudioruifourthvert_t);
		input.read(g_model.pData, fourthVertBytes, true);

		g_model.pData += fourthVertBytes;

		// Vertices.
		const size_t vertBytes = header->numvertices * sizeof(r5::v8::mstudioruivert_t);
		input.read(g_model.pData, vertBytes, true);

		g_model.pData += vertBytes;

		// Faces.
		const size_t faceBytes = header->numfaces * sizeof(r5::v8::mstudioruimeshface_t);
		input.read(g_model.pData, faceBytes, true);

		g_model.pData += faceBytes;
	}

	ALIGN4(g_model.pData);
}

void ConvertTextures_121(mstudiotexturedir_t* pCDTextures, int numCDTextures, r5::v8::mstudiotexture_t* pOldTextures, int numTextures, const MaterialShaderType_t* const shaderTypes)
{
	// TODO[rexx]: maybe add old cdtexture parsing here if available, or give the user the option to manually set the material paths
	printf("converting %i textures...\n", numTextures);

	g_model.hdrV54()->textureindex = g_model.pData - g_model.pBase;
	for (int i = 0; i < numTextures; ++i)
	{
		r5::v8::mstudiotexture_t* oldTexture = &pOldTextures[i];

		r5::v8::mstudiotexture_t* newTexture = reinterpret_cast<r5::v8::mstudiotexture_t*>(g_model.pData);

		const char* textureName = STRING_FROM_IDX(oldTexture, oldTexture->sznameindex);
		AddToStringTable((char*)newTexture, &newTexture->sznameindex, textureName);

		newTexture->textureGuid = oldTexture->textureGuid;

		g_model.pData += sizeof(r5::v8::mstudiotexture_t);
	}

	if (shaderTypes)
	{
		ALIGN4(g_model.pData);

		// Material Shader Types
		// Used for the CMaterialSystem::FindMaterial call in CModelLoader::Studio_LoadModel
		// Must be set properly otherwise the materials will not be found
		g_model.hdrV54()->materialtypesindex = g_model.pData - g_model.pBase;

		memcpy(g_model.pData, shaderTypes, numTextures);
		g_model.pData += numTextures;
	}
	else
	{
		// TEMP, COMMENT OUT FOR NON V13
		// V13 seems to have these removed while its using RGDP materials.
		// needs further research to decide how to convert these.
		g_model.hdrV54()->materialtypesindex = g_model.pData - g_model.pBase;

		memset(g_model.pData, RGDP, numTextures);
		g_model.pData += numTextures;
	}

	ALIGN4(g_model.pData); // align data to 4 bytes

	// Write static cdtexture data
	g_model.hdrV54()->cdtextureindex = g_model.pData - g_model.pBase;

	// i think cdtextures are mostly unused in r5 so use empty string
	AddToStringTable(g_model.pBase, (int*)g_model.pData, "");
	g_model.pData += sizeof(int);
}

void ConvertSkins_121(char* pOldModelBase, char* pOldSkinData, int numSkinRef, int numSkinFamilies)
{
	// TODO[rexx]: maybe add old cdtexture parsing here if available, or give the user the option to manually set the material paths
	printf("converting %i skins (%i skinrefs)...\n", numSkinFamilies, numSkinRef);

	g_model.hdrV54()->skinindex = g_model.pData - g_model.pBase;

	int skinIndexDataSize = sizeof(__int16) * numSkinRef * numSkinFamilies;
	memcpy(g_model.pData, pOldSkinData, skinIndexDataSize);

	g_model.pData += skinIndexDataSize;
	pOldSkinData += skinIndexDataSize;

	ALIGN4(g_model.pData);
	ALIGN4(pOldSkinData);

	int* oldSkinData = reinterpret_cast<int*>(pOldSkinData);
	// write skin names
	// skin 0 is unnamed
	for (int i = 0; i < numSkinFamilies - 1; ++i)
	{
		AddToStringTable(g_model.pBase, (int*)g_model.pData, STRING_FROM_IDX(pOldModelBase, oldSkinData[i]));
		g_model.pData += sizeof(int);
	}

	ALIGN4(g_model.pData);
}

template <typename mstudioanimdesc_type_t>
static void CopyAnimDesc(const r5::v8::mstudioseqdesc_t* const curOldSeqDesc, r5::v8::mstudioseqdesc_t* const curNewSeqDesc,
						 const int* const oldBlendGroups, int* const newBlendGroups, const int numAnims)
{
	for (int i = 0; i < numAnims; i++)
	{
		const mstudioanimdesc_type_t* const oldAnimDesc = PTR_FROM_IDX(mstudioanimdesc_type_t, curOldSeqDesc, oldBlendGroups[i]);
		r5::v8::mstudioanimdesc_t* const newAnimDesc = reinterpret_cast<r5::v8::mstudioanimdesc_t*>(g_model.pData);

		newBlendGroups[i] = g_model.pData - reinterpret_cast<const char*>(curNewSeqDesc);
		g_model.pData += sizeof(r5::v8::mstudioanimdesc_t);

		newAnimDesc->baseptr = oldAnimDesc->baseptr;
		AddToStringTable((char*)newAnimDesc, &newAnimDesc->sznameindex, STRING_FROM_IDX(oldAnimDesc, oldAnimDesc->sznameindex));
		newAnimDesc->fps = oldAnimDesc->fps;
		newAnimDesc->flags = oldAnimDesc->flags;
		newAnimDesc->numframes = oldAnimDesc->numframes;

		newAnimDesc->animindex = ConvertAnimation(PTR_FROM_IDX(char, oldAnimDesc, oldAnimDesc->animindex), newAnimDesc, g_model.hdrV54()->numbones);
	}
}

template <typename mstudioanimdesc_type_t>
void ConvertAnims_121(const char* const oldData, const int numlocalseq)
{
	g_model.hdrV54()->localseqindex = g_model.pData - g_model.pBase;
	g_model.hdrV54()->numlocalseq = numlocalseq;

	CopyAnimRefData(oldData, g_model.pData, numlocalseq);

	const r5::v8::mstudioseqdesc_t* const oldSeqDescBase = reinterpret_cast<const r5::v8::mstudioseqdesc_t*>(oldData);
	r5::v8::mstudioseqdesc_t* const newSeqDescBase = reinterpret_cast<r5::v8::mstudioseqdesc_t*>(g_model.pData);

	g_model.pData += numlocalseq * sizeof(r5::v8::mstudioseqdesc_t);

	for (int i = 0; i < numlocalseq; i++)
	{
		const r5::v8::mstudioseqdesc_t* const curOldSeqDesc = &oldSeqDescBase[i];
		r5::v8::mstudioseqdesc_t* const curNewSeqDesc = &newSeqDescBase[i];

		const int numAnims = curOldSeqDesc->groupsize[0] + curOldSeqDesc->groupsize[1];

		if (numAnims)
		{
			const size_t copyCount = numAnims * sizeof(int);

			const int* const oldBlendGroups = PTR_FROM_IDX(int, curOldSeqDesc, curOldSeqDesc->animindexindex);
			int* const newBlendGroups = reinterpret_cast<int*>(g_model.pData);

			curNewSeqDesc->animindexindex = g_model.pData - reinterpret_cast<const char*>(curNewSeqDesc);
			g_model.pData += copyCount;

			CopyAnimDesc<mstudioanimdesc_type_t>(curOldSeqDesc, curNewSeqDesc, oldBlendGroups, newBlendGroups, numAnims);
		}

		if (curOldSeqDesc->weightlistindex)
		{
			const size_t copyCount = g_model.hdrV54()->numbones * sizeof(float);
			memcpy(g_model.pData, PTR_FROM_IDX(char, curOldSeqDesc, curOldSeqDesc->weightlistindex), copyCount);

			curNewSeqDesc->weightlistindex = g_model.pData - reinterpret_cast<const char*>(curNewSeqDesc);
			g_model.pData += copyCount;
		}

		if (curOldSeqDesc->posekeyindex) // todo: verify if curOldSeqDesc->cycleposeindex is always 0.
		{
			const size_t copyCount = numAnims * sizeof(float);
			memcpy(g_model.pData, PTR_FROM_IDX(char, curOldSeqDesc, curOldSeqDesc->posekeyindex), copyCount);

			curNewSeqDesc->posekeyindex = g_model.pData - reinterpret_cast<const char*>(curNewSeqDesc);
			g_model.pData += copyCount;
		}
	}

	ALIGN4(g_model.pData);
}


static int CopyAttachmentsData(r5::v8::mstudioattachment_t* pOldAttachments, int numAttachments)
{
	int index = g_model.pData - g_model.pBase;

	printf("converting %i attachments...\n", numAttachments);

	for (int i = 0; i < numAttachments; ++i)
	{
		r5::v8::mstudioattachment_t* oldAttach = &pOldAttachments[i];

		r5::v8::mstudioattachment_t* attach = reinterpret_cast<r5::v8::mstudioattachment_t*>(g_model.pData) + i;

		AddToStringTable((char*)attach, &attach->sznameindex, STRING_FROM_IDX(oldAttach, oldAttach->sznameindex));
		attach->flags = oldAttach->flags;
		attach->localbone = oldAttach->localbone;
		memcpy(&attach->localmatrix, &oldAttach->localmatrix, sizeof(oldAttach->localmatrix));
	}
	g_model.pData += numAttachments * sizeof(r5::v8::mstudioattachment_t);

	return index;

	ALIGN4(g_model.pData);
}
#define FILEBUFSIZE (32 * 1024 * 1024)

//
// ConvertRMDL121To10
// Purpose: converts mdl data from rmdl v53 subversion 12.1 (Season 8) to rmdl v9 (Apex Legends Season 2/3)
//
void ConvertRMDL121To10(char* pMDL, const std::string& pathIn, const std::string& pathOut)
{
	std::string rawModelName = std::filesystem::path(pathIn).filename().u8string();

	printf("Converting model '%s' from version 54 (subversion 12.1) to version 54 (subversion 10)...\n", rawModelName.c_str());

	TIME_SCOPE(__FUNCTION__);

	rmem input(pMDL);

	r5::v121::studiohdr_t* oldHeader = input.get<r5::v121::studiohdr_t>();

	std::string rmdlPath = ChangeExtension(pathOut, "rmdl_conv");
	std::ofstream out(rmdlPath, std::ios::out | std::ios::binary);

	// allocate temp file buffer
	g_model.pBase = new char[FILEBUFSIZE] {};
	g_model.pData = g_model.pBase;

	// convert mdl hdr
	r5::v8::studiohdr_t* pHdr = reinterpret_cast<r5::v8::studiohdr_t*>(g_model.pData);
	ConvertStudioHdr(pHdr, oldHeader);
	g_model.pHdr = pHdr;
	g_model.pData += sizeof(r5::v8::studiohdr_t);

	// Copy the source file name if it exists.
	if (oldHeader->sourceFilenameOffset != 0 && oldHeader->boneindex > oldHeader->sourceFilenameOffset)
	{
		input.seek(oldHeader->sourceFilenameOffset, rseekdir::beg);
		const int sourceNameSize = oldHeader->boneindex - oldHeader->sourceFilenameOffset;

		input.read(g_model.pData, sourceNameSize);
		g_model.hdrV54()->sourceFilenameOffset = static_cast<int>(g_model.pData - g_model.pBase);

		g_model.pData += sourceNameSize;
	}

	// init string table so we can use
	BeginStringTable();

	std::string originalModelName = STRING_FROM_IDX(pMDL, oldHeader->sznameindex);

	std::string modelName = originalModelName;

	if (modelName.rfind("mdl/", 0) != 0)
		modelName = "mdl/" + modelName;
	if (EndsWith(modelName, ".mdl"))
	{
		modelName = modelName.substr(0, modelName.length() - 4);
		modelName += ".rmdl";
	}

	memcpy_s(&pHdr->name, 64, modelName.c_str(), min(modelName.length(), 64));
	AddToStringTable((char*)pHdr, &pHdr->sznameindex, modelName.c_str());
	AddToStringTable((char*)pHdr, &pHdr->surfacepropindex, STRING_FROM_IDX(pMDL, oldHeader->surfacepropindex));
	AddToStringTable((char*)pHdr, &pHdr->unkStringOffset, "");

	// convert bones and jigglebones
	input.seek(oldHeader->boneindex, rseekdir::beg);
	ConvertBones_121((r5::v121::mstudiobone_t*)input.getPtr(), oldHeader->numbones, false);

	// convert attachments
	input.seek(oldHeader->localattachmentindex, rseekdir::beg);
	g_model.hdrV54()->localattachmentindex = CopyAttachmentsData((r5::v8::mstudioattachment_t*)input.getPtr(), oldHeader->numlocalattachments);

	// convert hitboxsets and hitboxes
	input.seek(oldHeader->hitboxsetindex, rseekdir::beg);
	ConvertHitboxes_121((mstudiohitboxset_t*)input.getPtr(), oldHeader->numhitboxsets);

	// copy bonebyname table (bone ids sorted alphabetically by name)
	input.seek(oldHeader->bonetablebynameindex, rseekdir::beg);
	input.read(g_model.pData, g_model.hdrV54()->numbones);

	g_model.hdrV54()->bonetablebynameindex = g_model.pData - g_model.pBase;
	g_model.pData += g_model.hdrV54()->numbones;

	ALIGN4(g_model.pData);

	input.seek(oldHeader->localseqindex, rseekdir::beg);
	ConvertAnims_121<r5::v121::mstudioanimdesc_t>((const char*)input.getPtr(), oldHeader->numlocalseq);

	// convert bodyparts, models, and meshes
	input.seek(oldHeader->bodypartindex, rseekdir::beg);
	ConvertBodyParts_121((mstudiobodyparts_t*)input.getPtr(), oldHeader->numbodyparts);

	input.seek(oldHeader->localposeparamindex, rseekdir::beg);
	g_model.hdrV54()->localposeparamindex = ConvertPoseParams((mstudioposeparamdesc_t*)input.getPtr(), oldHeader->numlocalposeparameters, false);

	input.seek(oldHeader->ikchainindex, rseekdir::beg);
	ConvertIkChains_121((r5::v8::mstudioikchain_t*)input.getPtr(), oldHeader->numikchains, false);

	ConvertUIPanelMeshes(oldHeader, input);

	// get cdtextures pointer for converting textures
	input.seek(oldHeader->cdtextureindex, rseekdir::beg);
	void* pOldCDTextures = input.getPtr();

	MaterialShaderType_t* matTypes = nullptr;

	if (oldHeader->materialtypesindex > 0)
		matTypes = reinterpret_cast<MaterialShaderType_t*>(&pMDL[oldHeader->materialtypesindex]);
	// [amos]: if the model has no materialtypesindex, the game does the following
	// at the offset [r5apex.exe + 0x45600A] (r5reloaded):
	/*
		MaterialShaderType_t fallbackValue = RGDC;

		if (oldHeader->numbones > 1)
			fallbackValue = SKNC;

		if (oldHeader->numtextures > 0)
			memset(fallBackShaderTypes, fallbackValue, oldHeader->numtextures);
	*/
	// So I think in these cases we should just keep materialtypesindex 0.
	// ConvertTextures_121 will keep it 0 if matTypes is nullptr.

	// convert textures
	input.seek(oldHeader->textureindex, rseekdir::beg);
	ConvertTextures_121((mstudiotexturedir_t*)pOldCDTextures, oldHeader->numcdtextures, (r5::v8::mstudiotexture_t*)input.getPtr(), oldHeader->numtextures, matTypes);

	// convert skin data
	input.seek(oldHeader->skinindex, rseekdir::beg);
	ConvertSkins_121(pMDL, (char*)input.getPtr(), oldHeader->numskinref, oldHeader->numskinfamilies);

	// write base keyvalues
	std::string keyValues = "mdlkeyvalue{prop_data{base \"\"}}\n";
	strcpy_s(g_model.pData, keyValues.length() + 1, keyValues.c_str());

	pHdr->keyvalueindex = g_model.pData - g_model.pBase;
	pHdr->keyvaluesize = IALIGN2(keyValues.length() + 1);

	g_model.pData += keyValues.length() + 1;
	ALIGN4(g_model.pData);

	// SrcBoneTransforms
	input.seek(oldHeader->srcbonetransformindex, rseekdir::beg);
	g_model.hdrV54()->srcbonetransformindex = ConvertSrcBoneTransforms((mstudiosrcbonetransform_t*)input.getPtr(), oldHeader->numsrcbonetransform);

	if (oldHeader->linearboneindex && oldHeader->numbones > 1)
	{
		input.seek(oldHeader->linearboneindex, rseekdir::beg);
		CopyLinearBoneTableTo54(reinterpret_cast<const r5::v8::mstudiolinearbone_t* const>(input.getPtr()));
	}

	g_model.pData = WriteStringTable(g_model.pData);
	ALIGN64(g_model.pData);

	if (oldHeader->bvhOffset)
	{
		g_model.hdrV54()->bvhOffset = g_model.pData - g_model.pBase;

		input.seek(oldHeader->bvhOffset, rseekdir::beg);
		ConvertCollisionData_V120(oldHeader, (char*)input.getPtr());
	}

	pHdr->length = g_model.pData - g_model.pBase;

	out.write(g_model.pBase, pHdr->length);

	// now that rmdl is fully converted, convert vtx/vvd/vvc to VG
	//CreateVGFile(ChangeExtension(pathOut, "vg"), pHdr, vtxBuf.get(), vvdBuf.get(), vvcBuf.get(), nullptr);

					// convert v12.1 vg to v9 vg
	std::string vgFilePath = ChangeExtension(pathIn, "vg");

	if (FILE_EXISTS(vgFilePath))
	{
		uintmax_t vgInputSize = GetFileSize(vgFilePath);

		char* vgInputBuf = new char[vgInputSize];

		std::ifstream ifs(vgFilePath, std::ios::in | std::ios::binary);

		ifs.read(vgInputBuf, vgInputSize);

		// if 0tVG magic
		if (*(int*)vgInputBuf == 'GVt0')
			ConvertVGData_12_1(vgInputBuf, vgFilePath, ChangeExtension(pathOut, "vg_conv"));

		delete[] vgInputBuf;
	}

	// now delete rmdl buffer so we can write the rig
	delete[] g_model.pBase;

	///////////////
	// ANIM RIGS //
	///////////////
	// TODO[rexx]: this ought to be moved to a separate function when possible

	std::string rigName = originalModelName;
	if (rigName.rfind("animrig/", 0) != 0)
		rigName = "animrig/" + rigName;
	if (EndsWith(rigName, ".mdl"))
	{
		rigName = rigName.substr(0, rigName.length() - 4);
		rigName += ".rrig";
	}

	printf("Creating rig from model...\n");

	std::string rrigPath = ChangeExtension(pathOut, "rrig");
	std::ofstream rigOut(rrigPath, std::ios::out | std::ios::binary);

	g_model.pBase = new char[FILEBUFSIZE] {};
	g_model.pData = g_model.pBase;

	// generate rig
	pHdr = reinterpret_cast<r5::v8::studiohdr_t*>(g_model.pData);
	GenerateRigHdr(pHdr, oldHeader);
	g_model.pHdr = pHdr;
	g_model.pData += sizeof(r5::v8::studiohdr_t);

	// reset and recreate the string table for rig	
	BeginStringTable();

	memcpy_s(&pHdr->name, 64, rigName.c_str(), min(rigName.length(), 64));
	AddToStringTable((char*)pHdr, &pHdr->sznameindex, rigName.c_str());
	AddToStringTable((char*)pHdr, &pHdr->surfacepropindex, STRING_FROM_IDX(pMDL, oldHeader->surfacepropindex));
	AddToStringTable((char*)pHdr, &pHdr->unkStringOffset, "");// STRING_FROM_IDX(pMDL, oldHeader->unkStringOffset));

	// convert bones and jigglebones
	input.seek(oldHeader->boneindex, rseekdir::beg);
	ConvertBones_121((r5::v121::mstudiobone_t*)input.getPtr(), oldHeader->numbones, true);

	// convert attachments
	input.seek(oldHeader->localattachmentindex, rseekdir::beg);
	g_model.hdrV54()->localattachmentindex = CopyAttachmentsData((r5::v8::mstudioattachment_t*)input.getPtr(), oldHeader->numlocalattachments);

	// convert hitboxsets and hitboxes
	input.seek(oldHeader->hitboxsetindex, rseekdir::beg);
	ConvertHitboxes_121((mstudiohitboxset_t*)input.getPtr(), oldHeader->numhitboxsets);

	// copy bonebyname table (bone ids sorted alphabetically by name)
	input.seek(oldHeader->bonetablebynameindex, rseekdir::beg);
	input.read(g_model.pData, g_model.hdrV54()->numbones);

	g_model.hdrV54()->bonetablebynameindex = g_model.pData - g_model.pBase;
	g_model.pData += g_model.hdrV54()->numbones;

	ALIGN4(g_model.pData);

	input.seek(oldHeader->localposeparamindex, rseekdir::beg);
	g_model.hdrV54()->localposeparamindex = ConvertPoseParams((mstudioposeparamdesc_t*)input.getPtr(), oldHeader->numlocalposeparameters, true);

	input.seek(oldHeader->ikchainindex, rseekdir::beg);
	ConvertIkChains_121((r5::v8::mstudioikchain_t*)input.getPtr(), oldHeader->numikchains, true);
	ALIGN4(g_model.pData);

	g_model.pData = WriteStringTable(g_model.pData);

	pHdr->length = g_model.pData - g_model.pBase;

	rigOut.write(g_model.pBase, pHdr->length);

	delete[] g_model.pBase;
	//printf("Done!\n");


	g_model.stringTable.clear(); // cleanup string table

	printf("Finished converting model '%s', proceeding...\n\n", rawModelName.c_str());
}

