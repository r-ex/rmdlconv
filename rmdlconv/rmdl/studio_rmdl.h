#pragma once
#include "structs.h"

#define MAX_NUM_LODS 8

// studiohdr flags
#define STUDIOHDR_FLAGS_COMPLEX_WEIGHTS		0x4000 // don't really know what to name this one
#define STUDIOHDR_FLAGS_USES_VERTEX_COLOR	0x1000000 // model has/uses vertex color
#define STUDIOHDR_FLAGS_USES_UV2			0x2000000 // model has/uses secondary uv layer

// vg mesh flags
#define VG_POSITION         0x1
#define VG_PACKED_POSITION  0x2
#define VG_VERTEX_COLOR     0x10 // see: STUDIOHDR_FLAGS_USES_VERTEX_COLOR
#define VG_PACKED_WEIGHTS   0x5000
#define VG_UV_LAYER2        0x200000000 // see: STUDIOHDR_FLAGS_USES_UV2

struct mstudio_meshvertexloddata_t
{
	int modelvertexdataUnusedPad; // likely has none of the funny stuff because unused

	int numLODVertexes[MAX_NUM_LODS]; // depreciated starting with rmdl v14(?)
};

struct mstudiobodyparts_t
{
	int sznameindex;
	int nummodels;
	int base;
	int modelindex; // index into models array
};

namespace r5 // apex legends
{
	namespace v8
	{
		// target studiohdr for use in season 3. compatible with v8->v12
		struct studiohdr_t
		{
			int id;          // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
			int version;     // Format version number, such as 48 (0x30,0x00,0x00,0x00)
			int checksum;    // This has to be the same in the phy and vtx files to load!
			int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
			char name[64];   // The internal name of the model, padding with null bytes.
			int length;      // Data size of MDL file in bytes.

			Vector3 eyeposition;	// ideal eye position

			Vector3 illumposition;	// illumination center

			Vector3 hull_min;		// ideal movement hull size
			Vector3 hull_max;

			Vector3 view_bbmin;		// clipping bounding box
			Vector3 view_bbmax;

			int flags;

			int numbones; // bones
			int boneindex;

			int numbonecontrollers; // bone controllers
			int bonecontrollerindex;

			int numhitboxsets;
			int hitboxsetindex;

			int numlocalanim;   // animations/poses
			int localanimindex; // animation descriptions

			int numlocalseq; // sequences
			int	localseqindex;

			int activitylistversion; // initialization flag - have the sequences been indexed?

			// mstudiotexture_t
			// short rpak path
			// raw textures
			int materialtypesindex;
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

			mstudiobodyparts_t* bodypart(int i)
			{
				return reinterpret_cast<mstudiobodyparts_t*>((char*)this + bodypartindex) + i;
			}

			int numlocalattachments;
			int localattachmentindex;

			int numlocalnodes;
			int localnodeindex;
			int localnodenameindex;

			int numunknodes;
			int unknodexindex;

			int meshindex; // offset to model meshes

			int deprecated_numflexcontrollers;
			int deprecated_flexcontrollerindex;

			int deprecated_numflexrules;
			int deprecated_flexruleindex;

			int numikchains;
			int ikchainindex;

			// this is rui meshes
			int numruimeshes;
			int ruimeshindex;

			int numlocalposeparameters;
			int localposeparamindex;

			int surfacepropindex;

			int keyvalueindex;
			int keyvaluesize;

			int numlocalikautoplaylocks;
			int localikautoplaylockindex;

			float mass;
			int contents;

			// unused for packed models
			int numincludemodels;
			int includemodelindex;

			int virtualModel; // set as int for our purposes

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

			float fadeDistance;

			float gathersize; // what. from r5r struct

			int deprecated_numflexcontrollerui;
			int deprecated_flexcontrolleruiindex;

			float flVertAnimFixedPointScale; // to be verified
			int surfacepropLookup; // saved in the file

			int sourceFilenameOffset; // doesn't actually need to be written pretty sure, only four bytes when not present.

			int numsrcbonetransform;
			int srcbonetransformindex;

			int	illumpositionattachmentindex;

			int linearboneindex;

			// related to jiggle boesn now
			int m_nBoneFlexDriverCount; // unsure if that's what it is in apex
			int m_nBoneFlexDriverIndex;
			int unkindexflex;

			// aabb tree in here maybe? definitely unused
			int unk1_v54[6];

			// always "" or "Titan"
			int unkstringindex;

			// this is now used for combined files in rpak, vtx, vvd, and vvc are all combined while vphy is separate.
			// the indexes are added to the offset in the rpak mdl_ header.
			// vphy isn't vphy, looks like a heavily modified vphy.
			int vtxindex; // VTX
			int vvdindex; // VVD / IDSV
			int vvcindex; // VVC / IDCV 
			int vphyindex; // VPHY / IVPS

			int vtxsize;
			int vvdsize;
			int vvcsize;
			int vphysize; // still used in models using vg

			// unused in apex
			int unkmemberindex1;
			int numunkmember1;

			// only seen on '_animated' suffixed models so far
			int unkcount3;
			int unkindex3;

			// Per Tri Collision AABB size
			Vector3 mins;
			Vector3 maxs; // seem to be the same as hull size

			int unk3_v54[3];

			int unkindex4; // chunk before unkindex3 sometimes

			short unk4_v54[2]; // same as unk3_v54_v121

			int weightindex;
			int weightsize;
		};

		struct mstudiobone_t
		{
			int sznameindex;

			int parent; // parent bone
			int bonecontroller[6]; // bone controller index, -1 == none

			// default values
			Vector3 pos;
			Quaternion quat;
			RadianEuler rot;
			Vector3 scale; // bone scale(?)

			matrix3x4_t poseToBone;
			Quaternion qAlignment;

			int flags;
			int proctype;
			int procindex; // procedural rule
			int physicsbone; // index into physically simulated bone

			int surfacepropidx; // index into string tablefor property name

			int contents; // See BSPFlags.h for the contents flags

			int surfacepropLookup; // written on compile in v54

			int unk;

			int unkid; // physics index (?)
		};

		struct mstudiojigglebone_t
		{
			byte flags; // looks to be.

			unsigned char bone; // id of bone, might be single byte

			short pad; // possibly unused

			// general params
			float length; // how far from bone base, along bone, is tip
			float tipMass;

			float unkfloat; // v54 adds an extra value here but otherwise the same
			// observed values are between 0-1

			// flexible params
			float yawStiffness;
			float yawDamping;
			float pitchStiffness;
			float pitchDamping;
			float alongStiffness;
			float alongDamping;

			// angle constraint
			float angleLimit; // maximum deflection of tip in radians

			// yaw constraint
			float minYaw; // in radians
			float maxYaw; // in radians
			float yawFriction;
			float yawBounce;

			// pitch constraint
			float minPitch; // in radians
			float maxPitch; // in radians
			float pitchFriction;
			float pitchBounce;

			// base spring
			float baseMass;
			float baseStiffness;
			float baseDamping;
			float baseMinLeft;
			float baseMaxLeft;
			float baseLeftFriction;
			float baseMinUp;
			float baseMaxUp;
			float baseUpFriction;
			float baseMinForward;
			float baseMaxForward;
			float baseForwardFriction;
		};
	
		struct mstudioattachment_t
		{
			int sznameindex;
			int flags;

			int localbone; // parent bone

			matrix3x4_t localmatrix; // attachment point
		};

		struct mstudiomesh_t
		{
			int material;

			int modelindex;

			int numvertices; // number of unique vertices/normals/texcoords
			int vertexoffset; // vertex mstudiovertex_t

			// Access thin/fat mesh vertex data (only one will return a non-NULL result)

			int deprecated_numflexes; // vertex animation
			int deprecated_flexindex;

			// special codes for material operations
			int deprecated_materialtype;
			int deprecated_materialparam;

			// a unique ordinal for this mesh
			int meshid;

			Vector3 center;

			mstudio_meshvertexloddata_t vertexloddata;

			int unk[2]; // these are suposed to be filled on load, however this isn't true??
		};

		struct mstudiomodel_t
		{
			char name[64];

			int unkindex2; // goes to bones sometimes

			int type;

			float boundingradius;

			int nummeshes;
			int meshindex;

			mstudiomesh_t* mesh(int i)
			{
				return reinterpret_cast<mstudiomesh_t*>((char*)this + meshindex) + i;
			}

			// cache purposes
			int numvertices; // number of unique vertices/normals/texcoords
			int vertexindex; // vertex Vector
			int tangentsindex; // tangents Vector

			int numattachments;
			int attachmentindex;

			int deprecated_numeyeballs;
			int deprecated_eyeballindex;

			int pad[4];

			int colorindex; // vertex color
			// offset by colorindex number of bytes into vvc vertex colors
			int uv2index; // vertex second uv map
			// offset by uv2index number of bytes into vvc secondary uv map
		};

		struct mstudiobbox_t
		{
			int bone;
			int group; // intersection group

			Vector3 bbmin; // bounding box
			Vector3 bbmax;

			int szhitboxnameindex; // offset to the name of the hitbox.

			int critoverride; // overrides the group to be a crit, 0 or 1. might be group override since group 1 is head.

			int keyvalueindex; // used for keyvalues, most for titans.
		};

#pragma pack(push, 1)
		struct mstudiotexture_t
		{
			int sznameindex;
			unsigned __int64 guid;
		};
#pragma pack(pop)
	}

	namespace v121
	{
		// data source struct for subversion 12.1
		struct studiohdr_t
		{
			int id;          // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
			int version;     // Format version number, such as 48 (0x30,0x00,0x00,0x00)
			int checksum;    // This has to be the same in the phy and vtx files to load!
			int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
			char name[64];   // The internal name of the model, padding with null bytes.
			int length;      // Data size of MDL file in bytes.

			Vector3 eyeposition;	// ideal eye position

			Vector3 illumposition;	// illumination center

			Vector3 hull_min;		// ideal movement hull size
			Vector3 hull_max;

			Vector3 view_bbmin;		// clipping bounding box
			Vector3 view_bbmax;

			int flags;

			int numbones; // bones
			int boneindex;

			int numbonecontrollers; // bone controllers
			int bonecontrollerindex;

			int numhitboxsets;
			int hitboxsetindex;

			int numlocalanim;   // animations/poses
			int localanimindex; // animation descriptions

			int numlocalseq; // sequences
			int	localseqindex;

			int activitylistversion; // initialization flag - have the sequences been indexed?

			// mstudiotexture_t
			// short rpak path
			// raw textures
			int materialtypesindex;
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

			// these are unknown since I don't know what they cut
			int numunk_v121;
			int unkindex_v121;

			int numikchains;
			int ikchainindex;

			// this is rui meshes
			int numruimeshes;
			int ruimeshindex;

			int numlocalposeparameters;
			int localposeparamindex;

			int surfacepropindex;

			int keyvalueindex;
			int keyvaluesize;

			int numlocalikautoplaylocks;
			int localikautoplaylockindex;

			float mass;
			int contents;

			// unused for packed models
			int numincludemodels;
			int includemodelindex;

			int virtualModel;

			int bonetablebynameindex;

			int numunk1_v121; // count is (lodCount / totalSubmeshCount)
			int unkindex1_v121; // data matches the "unknown" data in s3 VG

			int boneremapindex;
			int numboneremaps;

			int unk_v54_v121[4];

			// section before bone remaps
			int unkindex2_v121;
			int numunk2_v121;

			// section before above section
			int unkindex3_v121;
			int numunk3_v121;

			float fadedistance;

			float gathersize; // what. from r5r struct

			int unk_v54[2];

			// asset bakery strings if it has any
			int sourceFilenameOffset;

			int numsrcbonetransform;
			int srcbonetransformindex;

			int	illumpositionattachmentindex;

			int linearboneindex;

			int numboneflexdrivers; // unsure if that's what it is in apex
			int boneflexdriverindex;

			int unk3_v54_a[2]; // I think this section was split vs old v54

			// the indexes are added to the offset in the rpak mdl_ header.
			// vphy isn't vphy, looks like a heavily modified vphy.
			// something different about these now
			int vtxindex; // VTX
			int vvdindex; // VVD / IDSV
			int vvcindex; // VVC / IDCV 
			int vphyindex; // VPHY / IVPS

			int vtxsize;
			int vvdsize;
			int vvcsize;
			int vphysize;

			int unk3_v54_b; // second part of above

			int unkindex3; // index to chunk after string block

			Vector3 mins; // min/max for Something
			Vector3 maxs; // seem to be the same as hull size

			int unkindex4; // chunk before unkindex2 sometimes

			int unk4_v54[3];
		};
	}
}

// used in vg as well
struct mstudioexternalweight_t
{
	short weight;
	short bone;
};

// file extension unknown
struct vertexWeightFileHeader_t
{
	int checksum; // same as studiohdr_t, ensures sync
	int version;

	int numLODVertexes[MAX_NUM_LODS]; // maybe this but the others don't get filled?

	int weightDataStart; // index into mstudioexternalboneweight_t array

	mstudioexternalweight_t* weight(int i)
	{
		return reinterpret_cast<mstudioexternalweight_t*>((char*)this + weightDataStart) + i;
	}
};

// VG
struct VGHeaderNew
{
	int id;		// 0x47567430	'0tVG'
	int version;	// 0x1
	int padding;
	int lodCount;	// If 0x1, this IS the first and only lod, if > 0x1, MORE 0tVG headers follow PER lod count
	int unk;
	int unk1;
	int lodOffset;
	char unk3[8];
};

struct VGIndexCountPacked
{
	uint64_t Count : 56;
	uint64_t Type : 8;
};

struct VGMeshNew
{
	__int64 flags;
	int vertexSize;  // size in bytes per vertex
	int vertexCount; // number of vertices

	__int64 indexOffset;
	VGIndexCountPacked indexPacked;	// 0x2 each (uint16_t)

	__int64 vertexOffset;
	__int64 vertexBufferSize;      // TOTAL size of vertex buffer

	__int64 extendedWeightsOffset;
	__int64 extendedWeightsCount;	// Only 1 byte per count

	__int64 externalWeightsOffset;
	__int64 externalWeightsCount;	// 0x10 each

	__int64 stripsOffset;
	__int64 stripsCount;			// 0x23 each
};

struct VGLodNew
{
	char unk[4];
	unsigned int dataSize;
	unsigned short meshCount;
	char unk1; // both of these bytes line up with the LOD index
	char unk2;
	float distance;
	uint64_t meshOffset;
};

struct VertexGroupHeader_t
{
	int id = 0x47567430;		// 0x47567430	'0tVG'
	int version = 1;	    // 0x1
	int unk;	        // Usually 0
	int dataSize;	// Total size of data + header in starpak

	__int64 boneStateChangeOffset; // offset to bone remap buffer
	__int64 numBoneStateChanges;  // number of "bone remaps" (size: 1)

	__int64 meshOffset;   // offset to mesh buffer
	__int64 numMeshes;    // number of meshes (size: 0x48)

	__int64 indexOffset;     // offset to index buffer
	__int64 numIndices;      // number of indices (size: 2 (uint16_t))

	__int64 vertOffset;    // offset to vertex buffer
	__int64 numVerts;     // number of bytes in vertex buffer

	__int64 externalWeightOffset;   // offset to extended weights buffer
	__int64 externalWeightsSize;    // number of bytes in extended weights buffer

	// there is one for every LOD mesh
	// i.e, unknownCount == lod.meshCount for all LODs
	__int64 unknownOffset;   // offset to buffer
	__int64 numUnknown;    // count (size: 0x30)

	__int64 lodOffset;       // offset to LOD buffer
	__int64 numLODs;        // number of LODs (size: 0x8)

	__int64 legacyWeightOffset;	// seems to be an offset into the "external weights" buffer for this mesh
	__int64 numLegacyWeights;   // seems to be the number of "external weights" that this mesh uses

	__int64 stripOffset;    // offset to strips buffer
	__int64 numStrips;     // number of strips (size: 0x23)

	int unused[16];
};

struct mstudiopackedboneweight_t
{
	short weight[2]; // shouldn't be > 32767
	char bone[3];
	byte numbones; // number of bones - 1, number of extra 
};

#pragma pack(push, 1)
// full struct
struct Vertex_VG_t
{
	Vector3 m_vecPosition;
	Vector64 m_vecPositionPacked;
	mstudiopackedboneweight_t m_BoneWeightsPacked;
	uint32_t m_NormalTangentPacked;
	VertexColor_t m_color;
	Vector2 m_vecTexCoord;
	Vector2 m_vecTexCoord2;
};
#pragma pack(pop)

struct MeshHeader_VG_t
{
	__int64 flags;	// mesh flags

	int vertOffset;			// start offset for this mesh's vertices
	int vertCacheSize;		    // number of bytes used from the vertex buffer
	int numVerts;			// number of vertices

	int unk1; // what even

	int externalWeightOffset;	// start offset for this mesh's "extended weights"
	int externalWeightSize;    // size or count of extended weights

	int indexOffset;			// start offset for this mesh's "indices"
	int numIndices;				// number of indices

	int legacyWeightOffset;	// seems to be an offset into the "external weights" buffer for this mesh
	int numLegacyWeights;   // seems to be the number of "external weights" that this mesh uses

	int stripOffset;        // Index into the strips structs
	int numStrips;

	// might be stuff like topologies
	int unk[4];

	/*int numBoneStateChanges;
	int boneStateChangeOffset;

	// MDL Version 49 and up only
	int numTopologyIndices;
	int topologyOffset;*/
};

// slightly modified VG version of ModelLODHeader_t
struct ModelLODHeader_VG_t
{
	short meshIndex;
	short meshCount;
	float switchPoint;
};

uint32_t PackNormalTangent_UINT32(float v1, float v2, float v3, float v4);
uint32_t PackNormalTangent_UINT32(Vector3 vec, Vector4 tangent);

Vector64 PackPos_UINT64(Vector3 vec);

void CreateVGFile(const std::string& filePath, r5::v8::studiohdr_t* pHdr, char* vtxBuf, char* vvdBuf, char* vvcBuf = nullptr, char* vvwBuf = nullptr);
