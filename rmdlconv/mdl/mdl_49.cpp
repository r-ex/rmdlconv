// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include "stdafx.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "versions.h"

//
// ConvertStudioHdr
// Purpose: converts the mdl v49 (Portal 2) studiohdr_t struct to rmdl v8 compatible (Apex Legends Season 0-6)
void ConvertStudioHdr(r5::v8::studiohdr_t* out, studiohdr_t* hdr)
{
	out->id = 'TSDI';
	out->version = 54;
	
	out->checksum = hdr->checksum;

	// :)
	if ((_time64(NULL) % 69420) == 0)
		out->checksum = 0xDEADBEEF;

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

	out->numsrcbonetransform = hdr->pStudioHdr2()->numsrcbonetransform;
	//-| end count vars

	//-| begin misc vars
	out->mass = hdr->mass;
	out->contents = hdr->contents;

	out->constdirectionallightdot = hdr->constdirectionallightdot;
	out->rootLOD = hdr->rootLOD;
	out->numAllowedRootLODs = hdr->numAllowedRootLODs;
	//out->fadeDistance = hdr->fadeDistance; // [rexx]: how do we determine this?
	out->flVertAnimFixedPointScale = hdr->flVertAnimFixedPointScale;
	//-| end misc vars

	//-| begin for giggles
	/*out->vtxindex = -1;
	out->vvdindex = hdr->vtxsize;
	out->vvcindex = hdr->vtxsize + hdr->vvdsize;
	out->vphyindex = -123456;*/

	out->vtxsize  = 0;//hdr->vtxsize;
	out->vvdsize  = 0;//hdr->vvdsize;
	out->vvcsize  = 0;//hdr->vvcsize;
	out->vphysize = 0;//hdr->vphysize;
	//-| end for giggles
}

//void GenerateRigHdr(r5::v8::studiohdr_t* out, studiohdr_t& hdr)
//{
//	out->id = 'TSDI';
//	out->version = 54;
//
//	memcpy_s(out->name, 64, hdr.name, 64);\
//
//	out->numbones = hdr.numbones;
//	out->numbonecontrollers = hdr.numbonecontrollers;
//	out->numhitboxsets = hdr.numhitboxsets;
//	out->numlocalattachments = hdr.numlocalattachments;
//	out->numlocalnodes = hdr.numlocalnodes;
//	out->numikchains = hdr.numikchains;
//	out->numlocalposeparameters = hdr.numlocalposeparameters;
//
//	out->mass = hdr.mass;
//	out->contents = hdr.contents;
//
//	// hard to tell if the first three are required
//	out->constdirectionallightdot = hdr.constdirectionallightdot;
//	out->rootLOD = hdr.rootLOD;
//	out->numAllowedRootLODs = hdr.numAllowedRootLODs;
//	out->fadeDistance = hdr.fadeDistance;
//}

void ConvertBones_49(mstudiobone_t* pOldBones, int numBones, bool isRig)
{
	printf("converting %i bones...\n", numBones);
	std::vector<r5::v8::mstudiobone_t*> proceduralBones;

	char* pBoneStart = g_model.pData;
	for (int i = 0; i < numBones; ++i)
	{
		mstudiobone_t* oldBone = &pOldBones[i];

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
		newBone->scale = Vector3{ 1.f, 1.f, 1.f };
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

	for (auto bone : proceduralBones)
	{
		int boneid = ((char*)bone - pBoneStart) / sizeof(r5::v8::mstudiobone_t);
		mstudiobone_t* oldBone = &pOldBones[boneid];
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

		jBone->minPitch = oldJBone->minPitch;
		jBone->maxPitch = oldJBone->maxPitch;
		jBone->pitchFriction = oldJBone->pitchFriction;
		jBone->pitchBounce = oldJBone->pitchBounce;

		g_model.pData += sizeof(r5::v8::mstudiojigglebone_t);
	}

	ALIGN4(g_model.pData);
}

void ConvertHitboxes_49(mstudiohitboxset_t* pOldHitboxSets, int numHitboxSets)
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

		mstudiobbox_t* oldHitboxes = reinterpret_cast<mstudiobbox_t*>((char*)oldhboxset + oldhboxset->hitboxindex);

		for (int j = 0; j < newhboxset->numhitboxes; ++j)
		{
			mstudiobbox_t* oldHitbox = oldHitboxes + j;
			r5::v8::mstudiobbox_t* newHitbox = reinterpret_cast<r5::v8::mstudiobbox_t*>(g_model.pData);

			memcpy(g_model.pData, oldHitbox, sizeof(r5::v8::mstudiobbox_t));

			AddToStringTable((char*)newHitbox, &newHitbox->szhitboxnameindex, STRING_FROM_IDX(oldHitbox, oldHitbox->szhitboxnameindex));
			AddToStringTable((char*)newHitbox, &newHitbox->keyvalueindex, "");// STRING_FROM_IDX(oldHitbox, oldHitbox->keyvalueindex));

			g_model.pData += sizeof(r5::v8::mstudiobbox_t);
		}
	}

	ALIGN4(g_model.pData);
}

void ConvertBodyParts_49(mstudiobodyparts_t* pOldBodyParts, int numBodyParts)
{
	printf("converting %i bodyparts...\n", numBodyParts);

	g_model.hdrV54()->bodypartindex = g_model.pData - g_model.pBase;

	mstudiobodyparts_t* bodypartStart = reinterpret_cast<mstudiobodyparts_t*>(g_model.pData);
	for (int i = 0; i < numBodyParts; ++i)
	{
		mstudiobodyparts_t* oldbodypart = &pOldBodyParts[i];
		mstudiobodyparts_t* newbodypart = reinterpret_cast<mstudiobodyparts_t*>(g_model.pData);

		memcpy(g_model.pData, oldbodypart, sizeof(mstudiobodyparts_t));

		AddToStringTable((char*)newbodypart, &newbodypart->sznameindex, STRING_FROM_IDX(oldbodypart, oldbodypart->sznameindex));

		g_model.pData += sizeof(mstudiobodyparts_t);
	}

	for (int i = 0; i < numBodyParts; ++i)
	{
		mstudiobodyparts_t* oldbodypart = &pOldBodyParts[i];
		mstudiobodyparts_t* newbodypart = bodypartStart + i;

		newbodypart->modelindex = g_model.pData - (char*)newbodypart;

		// pointer to old models (in .mdl)
		mstudiomodel_t* oldModels = reinterpret_cast<mstudiomodel_t*>((char*)oldbodypart + oldbodypart->modelindex);

		// pointer to start of new model data (in .rmdl)
		r5::v8::mstudiomodel_t* newModels = reinterpret_cast<r5::v8::mstudiomodel_t*>(g_model.pData);
		for (int j = 0; j < newbodypart->nummodels; ++j)
		{
			mstudiomodel_t* oldModel = oldModels + j;
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
			newModel->deprecated_numeyeballs = 0; //oldModel->numeyeballs;
			newModel->deprecated_eyeballindex = 0; //oldModel->eyeballindex;
			newModel->colorindex = 0;// oldModel->colorindex;
			newModel->uv2index = 0;// oldModel->uv2index;

			g_model.pData += sizeof(r5::v8::mstudiomodel_t);
		}

		for (int j = 0; j < newbodypart->nummodels; ++j)
		{
			mstudiomodel_t* oldModel = oldModels + j;
			r5::v8::mstudiomodel_t* newModel = newModels + j;

			newModel->meshindex = g_model.pData - (char*)newModel;

			// pointer to old meshes for this model (in .mdl)
			mstudiomesh_t* oldMeshes = reinterpret_cast<mstudiomesh_t*>((char*)oldModel + oldModel->meshindex);

			// pointer to new meshes for this model (in .rmdl)
			r5::v8::mstudiomesh_t* newMeshes = reinterpret_cast<r5::v8::mstudiomesh_t*>(g_model.pData);

			for (int k = 0; k < newModel->nummeshes; ++k)
			{
				mstudiomesh_t* oldMesh = oldMeshes + k;
				r5::v8::mstudiomesh_t* newMesh = newMeshes + k;

				memcpy(newMesh, oldMesh, sizeof(r5::v8::mstudiomesh_t));

				newMesh->modelindex = (char*)newModel - (char*)newMesh;

				g_model.pData += sizeof(r5::v8::mstudiomesh_t);
			}
		}
	}

	ALIGN4(g_model.pData);
}

void ConvertIkChains_49(mstudioikchain_t* pOldIkChains, int numIkChains, bool isRig)
{
	g_model.hdrV54()->ikchainindex = g_model.pData - g_model.pBase;

	if (!isRig)
		return;

	printf("converting %i ikchains...\n", numIkChains);

	int currentLinkCount = 0;
	std::vector<r5::v8::mstudioiklink_t> ikLinks;

	for (int i = 0; i < numIkChains; i++)
	{
		mstudioikchain_t* oldChain = &pOldIkChains[i];
		r5::v8::mstudioikchain_t* newChain = reinterpret_cast<r5::v8::mstudioikchain_t*>(g_model.pData);

		AddToStringTable((char*)newChain, &newChain->sznameindex, STRING_FROM_IDX(oldChain, oldChain->sznameindex));

		newChain->linktype = oldChain->linktype;
		newChain->numlinks = oldChain->numlinks;
		newChain->linkindex = (sizeof(r5::v8::mstudioiklink_t) * currentLinkCount) + (sizeof(r5::v8::mstudioikchain_t) * (numIkChains - i));
		//newChain->unk = oldChain->unk;

		g_model.pData += sizeof(r5::v8::mstudioikchain_t);

		currentLinkCount += oldChain->numlinks;
	}

	for (int i = 0; i < numIkChains; i++)
	{
		mstudioikchain_t* oldChain = &pOldIkChains[i];

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

void ConvertTextures_49(mstudiotexturedir_t* pCDTextures, int numCDTextures, mstudiotexture_t* pOldTextures, int numTextures)
{
	// TODO[rexx]: maybe add old cdtexture parsing here if available, or give the user the option to manually set the material paths
	printf("converting %i textures...\n", numTextures);

	g_model.hdrV54()->textureindex = g_model.pData - g_model.pBase;
	for (int i = 0; i < numTextures; ++i)
	{
		mstudiotexture_t* oldTexture = &pOldTextures[i];

		r5::v8::mstudiotexture_t* newTexture = reinterpret_cast<r5::v8::mstudiotexture_t*>(g_model.pData);

		const char* textureName = STRING_FROM_IDX(oldTexture, oldTexture->sznameindex);
		AddToStringTable((char*)newTexture, &newTexture->sznameindex, textureName);

		std::string texName = "material/" + std::string(textureName) + ".rpak";
		newTexture->guid = HashString(texName.c_str());

		g_model.pData += sizeof(r5::v8::mstudiotexture_t);
	}

	ALIGN4(g_model.pData);

	// Material Shader Types
	// Used for the CMaterialSystem::FindMaterial call in CModelLoader::Studio_LoadModel
	// Must be set properly otherwise the materials will not be found
	g_model.hdrV54()->materialtypesindex = g_model.pData - g_model.pBase;

	MaterialShaderType_t materialType = MaterialShaderType_t::SKNP;
	if (g_model.hdrV54()->flags & STUDIOHDR_FLAGS_STATIC_PROP)
		materialType = MaterialShaderType_t::RGDP;

	memset(g_model.pData, materialType, numTextures);
	g_model.pData += numTextures;

	ALIGN4(g_model.pData); // align data to 4 bytes

	// Write static cdtexture data
	g_model.hdrV54()->cdtextureindex = g_model.pData - g_model.pBase;

	// i think cdtextures are mostly unused in r5 so use empty string
	AddToStringTable(g_model.pBase, (int*)g_model.pData, "");
	g_model.pData += sizeof(int);
}

void ConvertSkins_49(char* pOldSkinData, int numSkinRef, int numSkinFamilies)
{
	// TODO[rexx]: maybe add old cdtexture parsing here if available, or give the user the option to manually set the material paths
	printf("converting %i skins (%i skinrefs)...\n", numSkinFamilies, numSkinRef);

	g_model.hdrV54()->skinindex = g_model.pData - g_model.pBase;

	int skinIndexDataSize = sizeof(__int16) * numSkinRef * numSkinFamilies;
	memcpy(g_model.pData, pOldSkinData, skinIndexDataSize);
	g_model.pData += skinIndexDataSize;

	// write skin names
	// skin 0 is unnamed
	for (int i = 0; i < numSkinFamilies-1; ++i)
	{
		char* skinNameBuf = new char[32];
		sprintf_s(skinNameBuf, 32, "skin%i", i);
		AddToStringTable(g_model.pBase, (int*)g_model.pData, skinNameBuf);

		g_model.pData += 4;
	}

	ALIGN4(g_model.pData);
}

// i lied it doesnt convert anything it just creates a default ref anim
void ConvertAnims_49()
{
	r5::v8::mstudioseqdesc_t* seqdesc = reinterpret_cast<r5::v8::mstudioseqdesc_t*>(g_model.pData);

	g_model.hdrV54()->localseqindex = g_model.pData - g_model.pBase;
	g_model.hdrV54()->numlocalseq = 1;

	seqdesc->baseptr = 0;
	AddToStringTable((char*)seqdesc, &seqdesc->szlabelindex, "ref");
	AddToStringTable((char*)seqdesc, &seqdesc->szactivitynameindex, "");

	seqdesc->activity = -1;

	seqdesc->bbmin = g_model.hdrV54()->mins;
	seqdesc->bbmax = g_model.hdrV54()->maxs;
	seqdesc->groupsize[0] = 1;
	seqdesc->groupsize[1] = 1;
	seqdesc->paramindex[0] = -1;
	seqdesc->paramindex[1] = -1;
	seqdesc->fadeintime = 0.2;
	seqdesc->fadeouttime = 0.2;

	// needs to be adjusted if adding more than one anim
	seqdesc->eventindex = sizeof(*seqdesc);
	seqdesc->autolayerindex = sizeof(*seqdesc);
	seqdesc->weightlistindex = sizeof(*seqdesc);

	g_model.pData += sizeof(r5::v8::mstudioseqdesc_t);

	// weightlist
	for (int i = 0; i < g_model.hdrV54()->numbones; ++i)
	{
		*reinterpret_cast<float*>(g_model.pData) = 1.0f;
		g_model.pData += sizeof(int);
	}

	seqdesc->animindexindex = g_model.pData - (char*)seqdesc;

	// blend
	*reinterpret_cast<int*>(g_model.pData) = seqdesc->animindexindex + sizeof(int);
	g_model.pData += sizeof(int);

	// add animdesc
	r5::v8::mstudioanimdesc_t* animdesc = reinterpret_cast<r5::v8::mstudioanimdesc_t*>(g_model.pData);

	AddToStringTable((char*)animdesc, &animdesc->sznameindex, "@ref");
	animdesc->fps = 30;
	animdesc->flags = STUDIO_ALLZEROS; // no way!!!

	g_model.pData += sizeof(r5::v8::mstudioanimdesc_t);
	ALIGN4(g_model.pData);

}

#define FILEBUFSIZE (32 * 1024 * 1024)

//
// ConvertMDLData_49
// Purpose: converts mdl data from mdl v49 (Portal 2) to rmdl v9 (Apex Legends Season 2/3)
//
void ConvertMDLData_49(char* buf, const std::string& filePath)
{
	TIME_SCOPE(__FUNCTION__);

	rmem input(buf);

	studiohdr_t* oldHeader = input.get<studiohdr_t>();

	//-| begin vtx reading  |-----
	std::unique_ptr<char[]> vtxBuf;
	{
		std::string vtxPath = ChangeExtension(filePath, "dx90.vtx");
		if (!FILE_EXISTS(vtxPath))
			Error("couldn't find .vtx file '%s'\n", vtxPath.c_str());

		size_t vtxSize = GetFileSize(vtxPath);
		vtxBuf = std::unique_ptr<char[]>(new char[vtxSize]);

		std::ifstream vvdIn(vtxPath, std::ios::in | std::ios::binary);
		vvdIn.read(vtxBuf.get(), vtxSize);
		vvdIn.close();
	}
	//-| end vtx reading   |-----

	//-| begin vvd reading |-----
	std::unique_ptr<char[]> vvdBuf;
	{
		std::string vvdPath = ChangeExtension(filePath, "vvd");
		if (!FILE_EXISTS(vvdPath))
			Error("couldn't find .vvd file '%s'\n", vvdPath.c_str());

		size_t vvdSize = GetFileSize(vvdPath);
		vvdBuf = std::unique_ptr<char[]>(new char[vvdSize]);

		std::ifstream vvdIn(vvdPath, std::ios::in | std::ios::binary);
		vvdIn.read(vvdBuf.get(), vvdSize);
		vvdIn.close();
	}
	//-| end vvd reading

	std::string rmdlPath = ChangeExtension(filePath, "rmdl");
	std::ofstream out(rmdlPath, std::ios::out | std::ios::binary);

	// allocate temp file buffer
	g_model.pBase = new char[FILEBUFSIZE]{};
	g_model.pData = g_model.pBase;

	// convert mdl hdr
	r5::v8::studiohdr_t* pHdr = reinterpret_cast<r5::v8::studiohdr_t*>(g_model.pData);
	ConvertStudioHdr(pHdr, oldHeader);
	g_model.pHdr = pHdr;
	g_model.pData += sizeof(r5::v8::studiohdr_t);

	// init string table so we can use 
	BeginStringTable();

	std::string originalModelName = oldHeader->pszName();

	std::string modelName = originalModelName;

	if (modelName.rfind("mdl/", 0) != 0)
		modelName = "mdl/" + modelName;
	if (EndsWith(modelName, ".mdl"))
	{
		modelName = modelName.substr(0, modelName.length() - 4);
		modelName += ".rmdl";
	}

	memcpy_s(&pHdr->name, 64, modelName.c_str(), modelName.length());
	AddToStringTable((char*)pHdr, &pHdr->sznameindex, modelName.c_str());
	AddToStringTable((char*)pHdr, &pHdr->surfacepropindex, STRING_FROM_IDX(buf, oldHeader->surfacepropindex));
	AddToStringTable((char*)pHdr, &pHdr->unkstringindex, ""); // "Titan" or empty

	// convert bones and jigglebones
	input.seek(oldHeader->boneindex, rseekdir::beg);
	ConvertBones_49((mstudiobone_t*)input.getPtr(), oldHeader->numbones, false);

	// convert attachments
	input.seek(oldHeader->localattachmentindex, rseekdir::beg);
	g_model.hdrV54()->localattachmentindex = ConvertAttachmentTo54((mstudioattachment_t*)input.getPtr(), oldHeader->numlocalattachments);

	// convert hitboxsets and hitboxes
	input.seek(oldHeader->hitboxsetindex, rseekdir::beg);
	ConvertHitboxes_49((mstudiohitboxset_t*)input.getPtr(), oldHeader->numhitboxsets);

	// copy bonebyname table (bone ids sorted alphabetically by name)
	input.seek(oldHeader->bonetablebynameindex, rseekdir::beg);
	input.read(g_model.pData, g_model.hdrV54()->numbones);

	g_model.hdrV54()->bonetablebynameindex = g_model.pData - g_model.pBase;
	g_model.pData += g_model.hdrV54()->numbones;

	ALIGN4(g_model.pData);

	ConvertAnims_49();

	// convert bodyparts, models, and meshes
	input.seek(oldHeader->bodypartindex, rseekdir::beg);
	ConvertBodyParts_49((mstudiobodyparts_t*)input.getPtr(), oldHeader->numbodyparts);

	input.seek(oldHeader->localposeparamindex, rseekdir::beg);
	g_model.hdrV54()->localposeparamindex = ConvertPoseParams((mstudioposeparamdesc_t*)input.getPtr(), oldHeader->numlocalposeparameters, false);

	input.seek(oldHeader->ikchainindex, rseekdir::beg);
	ConvertIkChains_49((mstudioikchain_t*)input.getPtr(), oldHeader->numikchains, false);

	// get cdtextures pointer for converting textures
	input.seek(oldHeader->cdtextureindex, rseekdir::beg);
	void* pOldCDTextures = input.getPtr();

	// convert textures
	input.seek(oldHeader->textureindex, rseekdir::beg);
	ConvertTextures_49((mstudiotexturedir_t*)pOldCDTextures, oldHeader->numcdtextures, (mstudiotexture_t*)input.getPtr(), oldHeader->numtextures);

	// convert skin data
	input.seek(oldHeader->skinindex, rseekdir::beg);
	ConvertSkins_49((char*)input.getPtr(), oldHeader->numskinref, oldHeader->numskinfamilies);

	// write base keyvalues
	std::string keyValues = "mdlkeyvalue{prop_data{base \"\"}}\n";
	strcpy_s(g_model.pData, keyValues.length() + 1, keyValues.c_str());

	pHdr->keyvalueindex = g_model.pData - g_model.pBase;
	pHdr->keyvaluesize = IALIGN2(keyValues.length() + 1);

	g_model.pData += keyValues.length() + 1;
	ALIGN4(g_model.pData);

	// SrcBoneTransforms
	input.seek(oldHeader->pStudioHdr2()->srcbonetransformindex, rseekdir::beg);
	g_model.hdrV54()->srcbonetransformindex = ConvertSrcBoneTransforms((mstudiosrcbonetransform_t*)input.getPtr(), oldHeader->pStudioHdr2()->numsrcbonetransform);

	if (oldHeader->pStudioHdr2()->linearboneindex && oldHeader->numbones > 1)
	{
		mstudiolinearbone_t* pLinearBones = oldHeader->pStudioHdr2()->pLinearBones();
		ConvertLinearBoneTableTo54(pLinearBones, (char*)pLinearBones + sizeof(mstudiolinearbone_t));
	}

	g_model.pData = WriteStringTable(g_model.pData);
	ALIGN4(g_model.pData);

	pHdr->length = g_model.pData - g_model.pBase;

	out.write(g_model.pBase, pHdr->length);

	// now that rmdl is fully converted, convert vtx/vvd/vvc to VG
	CreateVGFile(ChangeExtension(filePath, "vg"), pHdr, vtxBuf.get(), vvdBuf.get(), nullptr, nullptr);

	// now delete rmdl buffer so we can write the rig
	delete[] g_model.pBase;

	///////////////
	// ANIM RIGS //
	///////////////
	// TODO[rexx]: this ought to be moved to a separate function when possible

	/*
	std::string rigName = originalModelName;
	if (rigName.rfind("animrig/", 0) != 0)
		rigName = "animrig/" + rigName;
	if (EndsWith(rigName, ".mdl"))
	{
		rigName = rigName.substr(0, rigName.length() - 4);
		rigName += ".rrig";
	}

	std::string rrigPath = ChangeExtension(filePath, "rrig");
	std::ofstream rigOut(rrigPath, std::ios::out | std::ios::binary);

	g_model.pBase = new char[FILEBUFSIZE] {};
	g_model.pData = g_model.pBase;

	// generate rig
	pHdr = reinterpret_cast<r5::v8::studiohdr_t*>(g_model.pData);
	GenerateRigHdr(pHdr, oldHeader);
	g_model.pHdr = pHdr;
	g_model.pData += sizeof(r5::v8::studiohdr_t);

	// reset string table for rig
	g_model.stringTable.clear();

	memcpy_s(&pHdr->name, 64, rigName.c_str(), rigName.length());
	AddToStringTable((char*)pHdr, &pHdr->sznameindex, rigName.c_str());
	AddToStringTable((char*)pHdr, &pHdr->surfacepropindex, STRING_FROM_IDX(buf, oldHeader->surfacepropindex));
	AddToStringTable((char*)pHdr, &pHdr->unkstringindex, STRING_FROM_IDX(buf, oldHeader->unkstringindex));

	// convert bones and jigglebones
	input.seek(oldHeader->boneindex, rseekdir::beg);
	ConvertBones_53((mstudiobone_t*)input.getPtr(), oldHeader->numbones, true);

	// convert attachments
	input.seek(oldHeader->localattachmentindex, rseekdir::beg);
	ConvertAttachments_53((mstudioattachment_t*)input.getPtr(), oldHeader->numlocalattachments);

	// convert hitboxsets and hitboxes
	input.seek(oldHeader->hitboxsetindex, rseekdir::beg);
	ConvertHitboxes_53((mstudiohitboxset_t*)input.getPtr(), oldHeader->numhitboxsets);

	// copy bonebyname table (bone ids sorted alphabetically by name)
	input.seek(oldHeader->bonetablebynameindex, rseekdir::beg);
	input.read(g_model.pData, g_model.pHdr->numbones);

	g_model.pHdr->bonetablebynameindex = g_model.pData - g_model.pBase;
	g_model.pData += g_model.pHdr->numbones;

	ALIGN4(g_model.pData);

	input.seek(oldHeader->localposeparamindex, rseekdir::beg);
	ConvertPoseParams((mstudioposeparamdesc_t*)input.getPtr(), oldHeader->numlocalposeparameters, true);

	input.seek(oldHeader->ikchainindex, rseekdir::beg);
	ConvertIkChains_53((mstudioikchain_t*)input.getPtr(), oldHeader->numikchains, true);
	ALIGN4(g_model.pData);

	g_model.pData = WriteStringTable(g_model.pData);
	ALIGN4(g_model.pData);

	pHdr->length = g_model.pData - g_model.pBase;

	rigOut.write(g_model.pBase, pHdr->length);

	delete[] g_model.pBase;

	*/
	printf("Done!\n");
}
