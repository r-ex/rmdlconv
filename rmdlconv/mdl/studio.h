#pragma once

#pragma pack(push, 1)
#define MAX_NUM_BONES_PER_VERT 3
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

struct StripHeader_t
{
	int numIndices;
	int indexOffset;

	int numVerts;
	int vertexOffset;

	short numBones;

	char stripFlags; // StripHeaderFlags_t

	int numBoneStateChanges;
	int boneStateChangeOffset;
	int numTopologyIndices;
	int topologyOffset;
};

struct StripGroupHeader_t
{
	// These are the arrays of all verts and indices for this mesh.  strips index into this.
	int numVerts;
	int vertOffset;

	Vertex_t* vert( int n) { return reinterpret_cast<Vertex_t*>((char*)this + vertOffset) + n; }

	int numIndices;
	int indexOffset;

	uint16_t* indices()
	{
		return reinterpret_cast<uint16_t*>((char*)this + indexOffset);
	}

	int numStrips;
	int stripOffset;

	StripHeader_t* strip(int i)
	{
		return reinterpret_cast<StripHeader_t*>((char*)this + stripOffset) + i;
	}

	unsigned char flags;

	int numTopologyIndices;
	int topologyOffset;
};

struct MeshHeader_t
{
	int numStripGroups;
	int stripGroupHeaderOffset;

	StripGroupHeader_t* stripGroup(int i)
	{
		return reinterpret_cast<StripGroupHeader_t*>((char*)this + stripGroupHeaderOffset) + i;
	}

	unsigned char flags;
};

struct ModelLODHeader_t
{
	int numMeshes;
	int meshOffset;
	float switchPoint;

	MeshHeader_t* mesh(int i)
	{
		return reinterpret_cast<MeshHeader_t*>((char*)this + meshOffset) + i;
	}
};

struct ModelHeader_t
{
	int numLODs;
	int lodOffset;

	ModelLODHeader_t* lod(int i)
	{
		return reinterpret_cast<ModelLODHeader_t*>((char*)this + lodOffset) + i;
	}
};

struct BodyPartHeader_t
{
	int numModels;
	int modelOffset;

	ModelHeader_t* model(int i)
	{
		return reinterpret_cast<ModelHeader_t*>((char*)this + modelOffset) + i;
	}
};


struct FileHeader_t
{
	// file version as defined by OPTIMIZED_MODEL_FILE_VERSION (currently 7)
	int version;

	// hardware params that affect how the model is to be optimized.
	int vertCacheSize;
	short maxBonesPerStrip;
	short maxBonesPerTri;
	int maxBonesPerVert;

	// must match checkSum in the .mdl
	int checksum;

	int numLODs; // Also specified in ModelHeader_t's and should match

	// Offset to materialReplacementList Array. one of these for each LOD, 8 in total
	int materialReplacementListOffset;

	// Defines the size and location of the body part array
	int numBodyParts;
	int bodyPartOffset;

	BodyPartHeader_t* bodyPart(int i)
	{
		return reinterpret_cast<BodyPartHeader_t*>((char*)this + bodyPartOffset) + i;
	}
};

struct mstudioboneweight_t
{
	float	weight[3];
	char	bone[3];
	char	numbones;

};

struct mstudiovertex_t
{
	mstudioboneweight_t	m_BoneWeights;
	Vector3			m_vecPosition;
	Vector3			m_vecNormal;
	Vector2			m_vecTexCoord;
};

struct vertexFileHeader_t
{
	int id; // MODEL_VERTEX_FILE_ID
	int version; // MODEL_VERTEX_FILE_VERSION
	int checksum; // same as studiohdr_t, ensures sync

	int numLODs; // num of valid lods
	int numLODVertexes[8]; // num verts for desired root lod

	int numFixups; // num of vertexFileFixup_t

	int fixupTableStart; // offset from base to fixup table

	// vvc
	Vector2* uv(int i)
	{
		return reinterpret_cast<Vector2*>((char*)this + fixupTableStart) + i;
	}

	int vertexDataStart; // offset from base to vertex block
	
	// vvd
	mstudiovertex_t* vertex(int i)
	{
		return reinterpret_cast<mstudiovertex_t*>((char*)this + vertexDataStart) + i;
	}

	int tangentDataStart; // offset from base to tangent block
};

#pragma pack(pop)
