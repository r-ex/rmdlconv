// Copyright (c) 2023, rexx
// See LICENSE.txt for licensing information (GPL v3)

#pragma once


#define MAX_NUM_BONES_PER_STRIP 512
#define OPTIMIZED_MODEL_FILE_VERSION 7

#define MAX_NUM_BONES_PER_VERT 3
#define MAX_NUM_EXTRA_BONE_WEIGHTS	16 // for apex legends


#pragma pack(push, 1)
namespace OptimizedModel
{
	struct BoneStateChangeHeader_t
	{
		int hardwareID;
		int newBoneID;
	};

	struct Vertex_t
	{
		// these index into the mesh's vert[origMeshVertID]'s bones
		unsigned char boneWeightIndex[MAX_NUM_BONES_PER_VERT];
		unsigned char numBones;

		unsigned short origMeshVertID;

		// for sw skinned verts, these are indices into the global list of bones
		// for hw skinned verts, these are hardware bone indices
		char boneID[MAX_NUM_BONES_PER_VERT];
	};

	enum StripHeaderFlags_t
	{
		STRIP_IS_TRILIST = 0x01,
		STRIP_IS_QUADLIST_REG = 0x02,		// Regular sub-d quads
		STRIP_IS_QUADLIST_EXTRA = 0x04		// Extraordinary sub-d quads
	};


	// A strip is a piece of a stripgroup which is divided by bones 
	struct StripHeader_t
	{
		int numIndices;
		int indexOffset;

		int numVerts;
		int vertOffset;

		short numBones;

		unsigned char flags;

		int numBoneStateChanges;
		int boneStateChangeOffset;

		BoneStateChangeHeader_t* pBoneStateChange(int i)
		{
			return reinterpret_cast<BoneStateChangeHeader_t*>((char*)this + boneStateChangeOffset) + i;
		};

		// MDL Version 49 and up only
		int numTopologyIndices;
		int topologyOffset;
	};

	enum StripGroupFlags_t
	{
		STRIPGROUP_IS_HWSKINNED = 0x02,
		STRIPGROUP_IS_DELTA_FLEXED = 0x04,
		STRIPGROUP_SUPPRESS_HW_MORPH = 0x08,	// NOTE: This is a temporary flag used at run time.
	};

	struct StripGroupHeader_t
	{
		// These are the arrays of all verts and indices for this mesh.  strips index into this.
		int numVerts;
		int vertOffset;

		Vertex_t* pVertex(int i)
		{
			return reinterpret_cast<Vertex_t*>((char*)this + vertOffset) + i;
		}

		int numIndices;
		int indexOffset;

		unsigned short* pIndex(int i) { return reinterpret_cast<unsigned short*>((char*)this + indexOffset) + i; }
		int Index(int i) { return static_cast<int>(*pIndex(i)); }

		// used for copying indices 1:1
		unsigned short* indices()
		{
			return reinterpret_cast<unsigned short*>((char*)this + indexOffset);
		}


		int numStrips;
		int stripOffset;

		StripHeader_t* pStrip(int i)
		{
			return reinterpret_cast<StripHeader_t*>((char*)this + stripOffset) + i;
		}

		unsigned char flags;

		// The following fields are only present if MDL version is >=49
		// Points to an array of unsigned shorts (16 bits each)
		int numTopologyIndices;
		int topologyOffset;

		unsigned short* pTopologyIndex(int i)
		{
			return reinterpret_cast<unsigned short*>((char*)this + topologyOffset) + i;
		}
	};

	// likely unused in Respawn games as mouths and eyes are cut.
	enum MeshFlags_t {
		// these are both material properties, and a mesh has a single material.
		MESH_IS_TEETH = 0x01,
		MESH_IS_EYES = 0x02
	};


	struct MeshHeader_t
	{
		int numStripGroups;
		int stripGroupHeaderOffset;

		unsigned char flags; // never read these as eyeballs and mouths are depreciated

		StripGroupHeader_t* pStripGroup(int i)
		{
			return reinterpret_cast<StripGroupHeader_t*>((char*)this + stripGroupHeaderOffset) + i;
		}
	};

	struct ModelLODHeader_t
	{
		//Mesh array
		int numMeshes;
		int meshOffset;

		float switchPoint;

		MeshHeader_t* pMesh(int i)
		{
			return reinterpret_cast<MeshHeader_t*>((char*)this + meshOffset) + i;
		}
	};

	struct ModelHeader_t
	{
		//LOD mesh array
		int numLODs;   //This is also specified in FileHeader_t
		int lodOffset;

		ModelLODHeader_t* pLOD(int i)
		{
			return reinterpret_cast<ModelLODHeader_t*>((char*)this + lodOffset) + i;
		}
	};

	struct BodyPartHeader_t
	{
		// Model array
		int numModels;
		int modelOffset;

		ModelHeader_t* pModel(int i)
		{
			return reinterpret_cast<ModelHeader_t*>((char*)this + modelOffset) + i;
		}
	};

	struct MaterialReplacementHeader_t
	{
		short materialID;
		int replacementMaterialNameOffset;
		char* pMaterialReplacementName()
		{
			return ((char*)this + replacementMaterialNameOffset);
		}
	};

	struct MaterialReplacementListHeader_t
	{
		int numReplacements;
		int replacementOffset;

		MaterialReplacementHeader_t* pMaterialReplacement(int i)
		{
			return reinterpret_cast<MaterialReplacementHeader_t*>((char*)this + replacementOffset) + i;
		}
	};

	struct FileHeader_t
	{
		// file version as defined by OPTIMIZED_MODEL_FILE_VERSION (currently 7)
		int version;

		// hardware params that affect how the model is to be optimized.
		int vertCacheSize;
		unsigned short maxBonesPerStrip;
		unsigned short maxBonesPerFace;
		int maxBonesPerVert; // max of 16 in apex

		// must match checkSum in the .mdl
		int checkSum;

		int numLODs; // Also specified in ModelHeader_t's and should match

		// Offset to materialReplacementList Array. one of these for each LOD, 8 in total
		int materialReplacementListOffset;

		MaterialReplacementListHeader_t* pMaterialReplacementList(int lodID)
		{
			return reinterpret_cast<MaterialReplacementListHeader_t*>((char*)this + materialReplacementListOffset) + lodID;
		}

		// Defines the size and location of the body part array
		int numBodyParts;
		int bodyPartOffset;

		BodyPartHeader_t* pBodyPart(int i)
		{
			return reinterpret_cast<BodyPartHeader_t*>((char*)this + bodyPartOffset) + i;
		}
	};
}
#pragma pack(pop)
