#pragma once

#pragma pack(push, 1)
#define MAX_NUM_BONES_PER_VERT 3
#define MAX_NUM_LODS 8

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

struct mstudiopackedweight_t
{
	short weight[3];

	short pad; // what if

	int externalweightindex;
};

struct mstudioboneweight_t
{
	union {
		mstudiopackedweight_t packedweight; // complex weights (models with 4 or more weights per vert)
		float weight[MAX_NUM_BONES_PER_VERT]; // simple weights (models with 3 or less weights per vert)
	} weights;

	char bone[MAX_NUM_BONES_PER_VERT]; // set to unsigned so we can read it
	byte numbones; // if all three above are filled and this does not equal 3 there are four(?) weights
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
	int numLODVertexes[MAX_NUM_LODS]; // num verts for desired root lod

	int numFixups; // num of vertexFileFixup_t

	int fixupTableStart; // offset from base to fixup table
	int vertexDataStart; // offset from base to vertex block
	
	mstudiovertex_t* vertex(int i)
	{
		return reinterpret_cast<mstudiovertex_t*>((char*)this + vertexDataStart) + i;
	}

	int tangentDataStart; // offset from base to tangent block

	Vector4* tangent(int i)
	{
		return reinterpret_cast<Vector4*>((char*)this + tangentDataStart) + i;
	}
};

struct vertexColorFileHeader_t
{
	int id; // MODEL_VERTEX_FILE_ID
	int version; // MODEL_VERTEX_FILE_VERSION
	int checksum; // same as studiohdr_t, ensures sync

	int numLODs; // num of valid lods
	int numLODVertexes[MAX_NUM_LODS]; // num verts for desired root lod

	int colorDataStart;

	VertexColor_t* color(int i)
	{
		return reinterpret_cast<VertexColor_t*>((char*)this + colorDataStart) + i;
	}

	int uv2DataStart;

	Vector2* uv(int i)
	{
		return reinterpret_cast<Vector2*>((char*)this + uv2DataStart) + i;
	}
};
#pragma pack(pop)

namespace r2
{
	struct studiohdr_t
	{
		int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
		int version; // Format version number, such as 48 (0x30,0x00,0x00,0x00)
		int checksum; // This has to be the same in the phy and vtx files to load!
		int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
		char name[64]; // The internal name of the model, padding with null bytes.
		// Typically "my_model.mdl" will have an internal name of "my_model"
		int length; // Data size of MDL file in bytes.

		Vector3 eyeposition;	// ideal eye position

		Vector3 illumposition;	// illumination center

		Vector3 hull_min;		// ideal movement hull size
		Vector3 hull_max;

		Vector3 view_bbmin;		// clipping bounding box
		Vector3 view_bbmax;

		int flags;

		// highest observed: 250
		// max is definitely 256 because 8bit uint limit
		int numbones; // bones
		int boneindex;

		int numbonecontrollers; // bone controllers
		int bonecontrollerindex;

		int numhitboxsets;
		int hitboxsetindex;

		int numlocalanim; // animations/poses
		int localanimindex; // animation descriptions

		int numlocalseq; // sequences
		int	localseqindex;

		int activitylistversion; // initialization flag - have the sequences been indexed? set on load
		int eventsindexed;

		// mstudiotexture_t
		// short rpak path
		// raw textures
		int numtextures; // the material limit exceeds 128, probably 256.
		int textureindex;

		// this should always only be one, unless using vmts.
		// raw textures search paths
		int numcdtextures;
		int cdtextureindex;

		// replaceable textures tables
		int numskinref;
		int numskinfamilies;
		int skinindex;

		int numbodyparts;
		int bodypartindex;

		int numlocalattachments;
		int localattachmentindex;

		int numlocalnodes;
		int localnodeindex;
		int localnodenameindex;

		int deprecated_numflexdesc;
		int deprecated_flexdescindex;

		int deprecated_numflexcontrollers;
		int deprecated_flexcontrollerindex;

		int deprecated_numflexrules;
		int deprecated_flexruleindex;

		int numikchains;
		int ikchainindex;

		int numruimeshes;
		int ruimeshindex;

		int numlocalposeparameters;
		int localposeparamindex;

		int surfacepropindex; // string index

		int keyvalueindex;
		int keyvaluesize;

		int numlocalikautoplaylocks;
		int localikautoplaylockindex;

		float mass;
		int contents;

		// external animations, models, etc.
		int numincludemodels;
		int includemodelindex;

		uint32_t virtualModel;

		// animblock is either completely cut, this is because they no longer use .ani files.

		int bonetablebynameindex;

		// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
		// this value is used to calculate directional components of lighting 
		// on static props
		byte constdirectionallightdot;

		// set during load of mdl data to track *desired* lod configuration (not actual)
		// the *actual* clamped root lod is found in studiohwdata
		// this is stored here as a global store to ensure the staged loading matches the rendering
		byte rootLOD;

		// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
		// to be set as root LOD:
		//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
		//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
		byte numAllowedRootLODs;

		byte unused;

		float fadeDistance; // set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
		// player/titan models seem to inherit this value from the first model loaded in menus.
		// works oddly on entities, probably only meant for static props

		int deprecated_numflexcontrollerui;
		int deprecated_flexcontrolleruiindex;

		float flVertAnimFixedPointScale;
		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		// this is in all shipped models, probably part of their asset bakery. it should be 0x2CC.
		// doesn't actually need to be written pretty sure, only four bytes when not present.
		// this is not completely true as some models simply have nothing, such as animation models.
		int sourceFilenameOffset;

		int numsrcbonetransform;
		int srcbonetransformindex;

		int	illumpositionattachmentindex;

		int linearboneindex;

		int m_nBoneFlexDriverCount;
		int m_nBoneFlexDriverIndex;

		// for static props (and maybe others)
		// Precomputed Per-Triangle AABB data
		int m_nPerTriAABBIndex;
		int m_nPerTriAABBNodeCount;
		int m_nPerTriAABBLeafCount;
		int m_nPerTriAABBVertCount;

		// always "" or "Titan"
		int unkstringindex; // string index

		// ANIs are no longer used and this is reflected in many structs
		// Start of interal file data
		int vtxindex; // VTX
		int vvdindex; // VVD / IDSV
		int vvcindex; // VVC / IDCV 
		int vphyindex; // VPHY / IVPS

		int vtxsize; // VTX
		int vvdsize; // VVD / IDSV
		int vvcsize; // VVC / IDCV 
		int vphysize; // VPHY / IVPS

		// this data block is related to the vphy, if it's not present the data will not be written
		// definitely related to phy, apex phy has this merged into it
		int unkmemberindex1; // section between vphy and vtx.?
		int numunkmember1; // only seems to be used when phy has one solid

		// only seen on '_animated' suffixed models so far
		int unkcount3;
		int unkindex3;

		int unused1[60];

	};
}
