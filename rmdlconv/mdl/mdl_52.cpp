#include "stdafx.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "versions.h"

//
// ConvertStudioHdr
// Purpose: converts the mdl v52 (Titanfall 1) studiohdr_t struct to mdl v53 compatible (Titanfall 2)
void ConvertStudioHdrFrom52To53(r2::studiohdr_t* out, r1::studiohdr_t* hdr)
{
	out->id = 'TSDI';
	out->version = 53;

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
	out->view_bbmin = hdr->view_bbmin;
	out->view_bbmax = hdr->view_bbmax;

	// these will probably have to be modified at some point
	out->flags = hdr->flags;

	//-| begin count vars
	out->numbones = hdr->numbones;
	out->numbonecontrollers = hdr->numbonecontrollers;
	out->numhitboxsets = hdr->numhitboxsets;
	//out->numlocalanim = hdr->numlocalanim;
	//out->numlocalseq = hdr->numlocalseq;
	out->activitylistversion = hdr->activitylistversion;
	out->eventsindexed = hdr->eventsindexed;

	out->numtextures = hdr->numtextures;
	out->numcdtextures = hdr->numcdtextures;
	out->numskinref = hdr->numskinref;
	out->numskinfamilies = hdr->numskinfamilies;
	out->numbodyparts = hdr->numbodyparts;
	out->numlocalattachments = hdr->numlocalattachments;
	out->numlocalnodes = hdr->numlocalnodes;

	// skipping all the deprecated flex vars

	out->numikchains = hdr->numikchains;
	out->numlocalposeparameters = hdr->numlocalposeparameters;
	out->keyvaluesize = hdr->keyvaluesize;
	out->numlocalikautoplaylocks = hdr->numlocalikautoplaylocks; // cut?
	out->numincludemodels = hdr->numincludemodels;
	//-| end count vars

	//-| begin misc vars
	out->mass = hdr->mass;
	out->contents = hdr->contents;

	out->constdirectionallightdot = hdr->constdirectionallightdot;
	out->rootLOD = hdr->rootLOD;
	out->numAllowedRootLODs = hdr->numAllowedRootLODs;
	out->fadeDistance = hdr->fadeDistance;
	out->flVertAnimFixedPointScale = hdr->flVertAnimFixedPointScale;
	//-| end misc vars

	//-| begin studiohdr2 vars
	out->numsrcbonetransform = hdr->pStudioHdr2()->numsrcbonetransform;
	out->illumpositionattachmentindex = hdr->pStudioHdr2()->illumpositionattachmentindex;
	out->m_nPerTriAABBNodeCount = hdr->pStudioHdr2()->m_nPerTriAABBNodeCount;
	out->m_nPerTriAABBLeafCount = hdr->pStudioHdr2()->m_nPerTriAABBLeafCount;
	out->m_nPerTriAABBVertCount = hdr->pStudioHdr2()->m_nPerTriAABBVertCount;
	//-| end studiohdr2 vars
}

void ConvertBonesFrom52To53(r1::mstudiobone_t* pOldBones, int numBones)
{
	printf("converting %i bones...\n", numBones);
	std::vector<r2::mstudiobone_t*> proceduralBones;

	char* pBoneStart = g_model.pData;
	for (int i = 0; i < numBones; ++i)
	{
		r1::mstudiobone_t* oldBone = &pOldBones[i];

		r2::mstudiobone_t* newBone = reinterpret_cast<r2::mstudiobone_t*>(g_model.pData) + i;

		AddToStringTable((char*)newBone, &newBone->sznameindex, STRING_FROM_IDX(oldBone, oldBone->sznameindex));

		AddToStringTable((char*)newBone, &newBone->surfacepropidx, STRING_FROM_IDX(oldBone, oldBone->surfacepropidx));

		newBone->parent = oldBone->parent;
		memcpy(&newBone->bonecontroller, &oldBone->bonecontroller, sizeof(oldBone->bonecontroller));
		newBone->pos = oldBone->pos;
		newBone->quat = oldBone->quat;
		newBone->rot = oldBone->rot;
		newBone->scale = oldBone->scale;
		newBone->posscale = { 0.0f, 0.0f, 0.0f };
		newBone->rotscale = oldBone->rotscale;
		newBone->scalescale = oldBone->scalescale;
		newBone->poseToBone = oldBone->poseToBone;
		newBone->qAlignment = oldBone->qAlignment;
		newBone->flags = oldBone->flags;
		newBone->proctype = oldBone->proctype;
		newBone->procindex = oldBone->procindex;
		newBone->physicsbone = oldBone->physicsbone;
		newBone->contents = oldBone->contents;
		newBone->surfacepropLookup = oldBone->surfacepropLookup;

		if (oldBone->proctype != 0)
			proceduralBones.push_back(newBone);
	}
	g_model.hdrV53()->boneindex = g_model.pData - g_model.pBase;
	g_model.pData += numBones * sizeof(r2::mstudiobone_t);

	ALIGN4(g_model.pData);

	if (proceduralBones.size() > 0)
		printf("converting %lld procedural bones (jiggle bones)...\n", proceduralBones.size());

	for (auto bone : proceduralBones)
	{
		int boneid = ((char*)bone - pBoneStart) / sizeof(r2::mstudiobone_t);
		r1::mstudiobone_t* oldBone = &pOldBones[boneid];
		mstudiojigglebone_t* oldJBone = PTR_FROM_IDX(mstudiojigglebone_t, oldBone, oldBone->procindex);

		mstudiojigglebone_t* jBone = reinterpret_cast<mstudiojigglebone_t*>(g_model.pData);

		bone->procindex = (char*)jBone - (char*)bone;
		jBone->flags = oldJBone->flags;
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

		g_model.pData += sizeof(mstudiojigglebone_t);
	}

	ALIGN4(g_model.pData);
}

void ConvertHitboxesFromMDLTo53(mstudiohitboxset_t* pOldHitboxSets, int numHitboxSets)
{
	printf("converting %i hitboxsets...\n", numHitboxSets);

	g_model.hdrV53()->hitboxsetindex = g_model.pData - g_model.pBase;

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
			r2::mstudiobbox_t* newHitbox = reinterpret_cast<r2::mstudiobbox_t*>(g_model.pData);

			memcpy(g_model.pData, oldHitbox, sizeof(r2::mstudiobbox_t));

			AddToStringTable((char*)newHitbox, &newHitbox->szhitboxnameindex, STRING_FROM_IDX(oldHitbox, oldHitbox->szhitboxnameindex));
			AddToStringTable((char*)newHitbox, &newHitbox->keyvalueindex, "");

			g_model.pData += sizeof(r2::mstudiobbox_t);
		}
	}

	ALIGN4(g_model.pData);
}

void ConvertBodyPartsFrom52To53(mstudiobodyparts_t* pOldBodyParts, int numBodyParts)
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
		r1::mstudiomodel_t* oldModels = reinterpret_cast<r1::mstudiomodel_t*>((char*)oldbodypart + oldbodypart->modelindex);

		// pointer to start of new model data (in .rmdl)
		r1::mstudiomodel_t* newModels = reinterpret_cast<r1::mstudiomodel_t*>(g_model.pData);
		for (int j = 0; j < newbodypart->nummodels; ++j)
		{
			r1::mstudiomodel_t* oldModel = oldModels + j;
			r1::mstudiomodel_t* newModel = reinterpret_cast<r1::mstudiomodel_t*>(g_model.pData);

			memcpy(&newModel->name, &oldModel->name, sizeof(newModel->name));
			newModel->type = oldModel->type;
			newModel->boundingradius = oldModel->boundingradius;
			newModel->nummeshes = oldModel->nummeshes;
			newModel->numvertices = oldModel->numvertices;
			newModel->vertexindex = oldModel->vertexindex;
			newModel->tangentsindex = oldModel->tangentsindex;
			newModel->numattachments = oldModel->numattachments;
			newModel->attachmentindex = oldModel->attachmentindex;
			newModel->deprecated_numeyeballs = oldModel->deprecated_numeyeballs;
			newModel->deprecated_eyeballindex = oldModel->deprecated_eyeballindex;
			newModel->colorindex = oldModel->colorindex;
			newModel->uv2index = oldModel->uv2index;

			g_model.pData += sizeof(r1::mstudiomodel_t);
		}

		for (int j = 0; j < newbodypart->nummodels; ++j)
		{
			r1::mstudiomodel_t* oldModel = oldModels + j;
			r1::mstudiomodel_t* newModel = newModels + j;

			newModel->meshindex = g_model.pData - (char*)newModel;

			// pointer to old meshes for this model (in .mdl)
			r1::mstudiomesh_t* oldMeshes = reinterpret_cast<r1::mstudiomesh_t*>((char*)oldModel + oldModel->meshindex);

			// pointer to new meshes for this model (in .rmdl)
			r2::mstudiomesh_t* newMeshes = reinterpret_cast<r2::mstudiomesh_t*>(g_model.pData);

			for (int k = 0; k < newModel->nummeshes; ++k)
			{
				r1::mstudiomesh_t* oldMesh = oldMeshes + k;
				r2::mstudiomesh_t* newMesh = newMeshes + k;

				memcpy(newMesh, oldMesh, sizeof(r2::mstudiomesh_t));

				newMesh->modelindex = (char*)newModel - (char*)newMesh;

				g_model.pData += sizeof(r2::mstudiomesh_t);
			}
		}
	}

	ALIGN4(g_model.pData);
}

void ConvertIkChainsFromMDLTo53(mstudioikchain_t* pOldIkChains, int numIkChains)
{
	g_model.hdrV53()->ikchainindex = g_model.pData - g_model.pBase;

	printf("converting %i ikchains...\n", numIkChains);

	int currentLinkCount = 0;

	for (int i = 0; i < numIkChains; i++)
	{
		mstudioikchain_t* oldChain = &pOldIkChains[i];
		r2::mstudioikchain_t* newChain = reinterpret_cast<r2::mstudioikchain_t*>(g_model.pData);

		AddToStringTable((char*)newChain, &newChain->sznameindex, STRING_FROM_IDX(oldChain, oldChain->sznameindex));

		newChain->linktype = oldChain->linktype;
		newChain->numlinks = oldChain->numlinks;
		newChain->linkindex = (sizeof(mstudioiklink_t) * currentLinkCount) + (sizeof(r2::mstudioikchain_t) * (numIkChains - i));
		//newChain->unk = oldChain->unk;

		g_model.pData += sizeof(r2::mstudioikchain_t);

		currentLinkCount += oldChain->numlinks;
	}

	for (int i = 0; i < numIkChains; i++)
	{
		mstudioikchain_t* oldChain = &pOldIkChains[i];

		for (int linkIdx = 0; linkIdx < oldChain->numlinks; linkIdx++)
		{
			mstudioiklink_t* oldLink = PTR_FROM_IDX(mstudioiklink_t, oldChain, oldChain->linkindex + (sizeof(mstudioiklink_t) * linkIdx));
			mstudioiklink_t* newLink = reinterpret_cast<mstudioiklink_t*>(g_model.pData);

			newLink->bone = oldLink->bone;
			newLink->kneeDir = oldLink->kneeDir;

			g_model.pData += sizeof(mstudioiklink_t);
		}
	}

	ALIGN4(g_model.pData);
}

#define FILEBUFSIZE (32 * 1024 * 1024)

void ConvertMDLDataFrom52To53(char* buf, const std::string& filePath)
{
	rmem input(buf);

	r1::studiohdr_t* oldHeader = input.get<r1::studiohdr_t>();

	//-| begin phy reading  |-----
	/*std::unique_ptr<char[]> phyBuf;
	{
		std::string phyPath = ChangeExtension(filePath, "dx11.vtx");
		if (!FILE_EXISTS(phyPath))
			return;

		size_t phySize = GetFileSize(phyPath);
		phyBuf = std::unique_ptr<char[]>(new char[phySize]);

		std::ifstream vtxIn(phyPath, std::ios::in | std::ios::binary);
		vtxIn.read(phyBuf.get(), phySize);
		vtxIn.close();
	}*/
	//-| end phy reading   |-----

	//-| begin vtx reading  |-----
	std::unique_ptr<char[]> vtxBuf;
	{
		std::string vtxPath = ChangeExtension(filePath, "dx11.vtx");
		if (!FILE_EXISTS(vtxPath))
			Error("couldn't find .vtx file '%s'\n", vtxPath.c_str());

		size_t vtxSize = GetFileSize(vtxPath);
		vtxBuf = std::unique_ptr<char[]>(new char[vtxSize]);

		std::ifstream vtxIn(vtxPath, std::ios::in | std::ios::binary);
		vtxIn.read(vtxBuf.get(), vtxSize);
		vtxIn.close();
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

	//-| begin vvc reading |-----
	/*std::unique_ptr<char[]> vvcBuf;
	{
		std::string vvcPath = ChangeExtension(filePath, "vvc");
		if (!FILE_EXISTS(vvcPath))
			return;

		size_t vvcSize = GetFileSize(vvcPath);

		vvdBuf = std::unique_ptr<char[]>(new char[vvcSize]);

		std::ifstream vvdIn(vvcPath, std::ios::in | std::ios::binary);
		vvdIn.read(vvdBuf.get(), vvcSize);
		vvdIn.close();
	}*/
	//-| end vvc reading

	std::string outPath = ChangeExtension(filePath, "mdl_new");
	std::ofstream out(outPath, std::ios::out | std::ios::binary);

	// allocate temp file buffer
	g_model.pBase = new char[FILEBUFSIZE] {};
	g_model.pData = g_model.pBase;

	// convert mdl hdr
	r2::studiohdr_t* pHdr = reinterpret_cast<r2::studiohdr_t*>(g_model.pData);
	ConvertStudioHdrFrom52To53(pHdr, oldHeader);
	g_model.pHdr = pHdr;
	g_model.pData += sizeof(r2::studiohdr_t);

	// init string table so we can use 
	BeginStringTable();

	std::string modelName = oldHeader->pszName();

	memcpy_s(&pHdr->name, 64, modelName.c_str(), modelName.length());
	AddToStringTable((char*)pHdr, &pHdr->sznameindex, modelName.c_str());
	AddToStringTable((char*)pHdr, &pHdr->surfacepropindex, STRING_FROM_IDX(buf, oldHeader->surfacepropindex));
	AddToStringTable((char*)pHdr, &pHdr->unkstringindex, oldHeader->pszUnkString()); // "Titan" or empty

	// source file for lulz
	input.seek(oldHeader->sourceFilenameOffset, rseekdir::beg);
	input.read(g_model.pData, oldHeader->boneindex - oldHeader->sourceFilenameOffset);

	pHdr->sourceFilenameOffset = g_model.pData - g_model.pBase;

	g_model.pData += oldHeader->boneindex - oldHeader->sourceFilenameOffset;
	ALIGN4(g_model.pData);

	// convert bones and jigglebones
	input.seek(oldHeader->boneindex, rseekdir::beg);
	ConvertBonesFrom52To53((r1::mstudiobone_t*)input.getPtr(), oldHeader->numbones);

	// convert attachments
	input.seek(oldHeader->localattachmentindex, rseekdir::beg);
	g_model.hdrV53()->localattachmentindex = ConvertAttachmentsToMDL((mstudioattachment_t*)input.getPtr(), oldHeader->numlocalattachments);

	// convert hitboxsets and hitboxes
	input.seek(oldHeader->hitboxsetindex, rseekdir::beg);
	ConvertHitboxesFromMDLTo53((mstudiohitboxset_t*)input.getPtr(), oldHeader->numhitboxsets);

	// copy bonebyname table (bone ids sorted alphabetically by name)
	input.seek(oldHeader->bonetablebynameindex, rseekdir::beg);
	input.read(g_model.pData, g_model.hdrV53()->numbones);

	g_model.hdrV53()->bonetablebynameindex = g_model.pData - g_model.pBase;
	g_model.pData += g_model.hdrV53()->numbones;

	ALIGN4(g_model.pData);

	//ConvertAnims_49();

	// convert bodyparts, models, and meshes
	input.seek(oldHeader->bodypartindex, rseekdir::beg);
	ConvertBodyPartsFrom52To53((mstudiobodyparts_t*)input.getPtr(), oldHeader->numbodyparts);

	input.seek(oldHeader->localposeparamindex, rseekdir::beg);
	g_model.hdrV53()->localposeparamindex = ConvertPoseParams((mstudioposeparamdesc_t*)input.getPtr(), oldHeader->numlocalposeparameters, false);

	input.seek(oldHeader->ikchainindex, rseekdir::beg);
	ConvertIkChainsFromMDLTo53((mstudioikchain_t*)input.getPtr(), oldHeader->numikchains);

	//// get cdtextures pointer for converting textures
	//input.seek(oldHeader->cdtextureindex, rseekdir::beg);
	//void* pOldCDTextures = input.getPtr();

	//// convert textures
	//input.seek(oldHeader->textureindex, rseekdir::beg);
	//ConvertTextures_49((mstudiotexturedir_t*)pOldCDTextures, oldHeader->numcdtextures, (mstudiotexture_t*)input.getPtr(), oldHeader->numtextures);

	//// convert skin data
	//input.seek(oldHeader->skinindex, rseekdir::beg);
	//ConvertSkins_49((char*)input.getPtr(), oldHeader->numskinref, oldHeader->numskinfamilies);

	// write base keyvalues
	input.seek(oldHeader->keyvalueindex, rseekdir::beg);
	input.read(g_model.pData, oldHeader->keyvaluesize);

	pHdr->keyvalueindex = g_model.pData - g_model.pBase;
	pHdr->keyvaluesize = oldHeader->keyvaluesize;

	g_model.pData += oldHeader->keyvaluesize;
	ALIGN4(g_model.pData);

	// SrcBoneTransforms
	mstudiosrcbonetransform_t* pSrcBoneTransforms = oldHeader->pStudioHdr2()->pSrcBoneTransforms();
	g_model.hdrV53()->srcbonetransformindex = ConvertSrcBoneTransforms(pSrcBoneTransforms, oldHeader->pStudioHdr2()->numsrcbonetransform);

	if (oldHeader->pStudioHdr2()->linearboneindex && oldHeader->numbones > 1)
	{
		mstudiolinearbone_t* pLinearBones = oldHeader->pStudioHdr2()->pLinearBones();
		ConvertLinearBoneTableTo53(pLinearBones, (char*)pLinearBones + sizeof(mstudiolinearbone_t));
	}

	g_model.pData = WriteStringTable(g_model.pData);
	ALIGN4(g_model.pData);

	pHdr->length = g_model.pData - g_model.pBase;

	out.write(g_model.pBase, pHdr->length);

	delete[] g_model.pBase;

	printf("Done!\n");
}
