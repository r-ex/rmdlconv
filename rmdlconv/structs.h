#pragma once
#include <cstdint>
#pragma pack(push, 1)

struct Vector3
{
	float x, y, z;
};


struct VGHeaderNew
{
	int id;		// 0x47567430	'0tVG'
	int version;	// 0x1
	int padding;
	uint32_t lodCount;	// If 0x1, this IS the first and only lod, if > 0x1, MORE 0tVG headers follow PER lod count
	uint32_t unk;
	uint32_t unk1;
	uint32_t lodOffset;
	char unk3[8];
};

struct VGIndexCountPacked
{
	uint64_t Count : 56;
	uint64_t Type : 8;
};

struct VGSubmeshNew
{
	uint64_t flags;
	uint32_t vertexSize;  // size in bytes per vertex
	uint32_t vertexCount; // number of vertices

	uint64_t indexOffset;
	VGIndexCountPacked indexPacked;	// 0x2 each (uint16_t)

	uint64_t vertexOffset;
	uint64_t vertexBufferSize;      // TOTAL size of vertex buffer

	uint64_t extendedWeightsOffset;
	uint64_t extendedWeightsCount;	// Only 1 byte per count

	uint64_t externalWeightsOffset;
	uint64_t externalWeightsCount;	// 0x10 each

	uint64_t stripsOffset;
	uint64_t stripsCount;			// 0x23 each
};

struct VGLodNew
{
	char unk[4];
	unsigned int dataSize;
	unsigned short submeshCount;
	char unk1; // both of these bytes line up with the LOD index
	char unk2;
	float distance;
	uint64_t submeshOffset;
};

struct VGHeader
{
	int id = 0x47567430;		// 0x47567430	'0tVG'
	uint32_t version = 1;	// 0x1
	uint32_t unk;	    // Usually 0
	uint32_t dataSize;	// Total size of data + header in starpak
		  
	uint64_t boneRemapOffset; // offset to bone remap buffer
	uint64_t boneRemapCount;  // number of "bone remaps" (size: 1)
		  
	uint64_t submeshOffset;   // offset to submesh buffer
	uint64_t submeshCount;    // number of submeshes (size: 0x48)
		  
	uint64_t indexOffset;     // offset to index buffer
	uint64_t indexCount;      // number of indices (size: 2 (uint16_t))
		  
	uint64_t vertexOffset;    // offset to vertex buffer
	uint64_t vertexCount;     // number of bytes in vertex buffer
		  
	uint64_t extendedWeightsOffset;   // offset to extended weights buffer
	uint64_t extendedWeightsCount;    // number of bytes in extended weights buffer
		  
	uint64_t unknownOffset;   // offset to buffer
	uint64_t unknownCount;    // count (size: 0x30)
		  
	uint64_t lodOffset;       // offset to LOD buffer
	uint64_t lodCount;        // number of LODs (size: 0x8)
		  
	uint64_t externalWeightsOffset;   // offset to external weights buffer
	uint64_t externalWeightsCount;     // number of external weights (size: 0x10)
		  
	uint64_t stripsOffset;    // offset to strips buffer
	uint64_t stripsCount;     // number of strips (size: 0x23)

	char unused[0x40];
};

struct VGSubmesh
{
	uint64_t flags;					// submesh flags
	uint32_t vertexOffset;			// start offset for this submesh's vertices
	uint32_t vertexSize;		    // number of bytes used from the vertex buffer
	uint32_t vertexCount;			// number of vertices
	uint32_t unk1;
	uint32_t extendedWeightsOffset;	// start offset for this submesh's "extended weights"
	uint32_t extendedWeightsSize;   // size or count of extended weights
	uint32_t indexOffset;			// start offset for this submesh's "indices"
	uint32_t indexCount;			// number of indices
	uint32_t externalWeightsOffset;	// seems to be an offset into the "external weights" buffer for this submesh
	uint32_t externalWeightsCount;	// seems to be the number of "external weights" that this submesh uses
	uint32_t stripsOffset;			// Index into the strips structs
	uint32_t stripsCount;
	uint32_t Int15;
	uint32_t Int16;
	uint32_t Int17;
	uint32_t Int18;
};

struct VGLod
{
	uint16_t SubmeshIndex;
	uint16_t SubmeshCount;
	float Distance;
};

struct VGStrip
{
	uint32_t indexCount;
	uint32_t indexOffset;

	uint32_t vertexCount;
	uint32_t vertexOffset;

	uint16_t numBones;

	char stripFlags;

	char unk[0x10];
};

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

	int numlocalattachments;
	int localattachmentindex;

	int numlocalnodes;
	int localnodeindex;
	int localnodenameindex;

	int numflexdesc;
	int flexdescindex;

	int meshindex; // SubmeshLodsOffset, might just be a mess offset

	int numflexcontrollers;
	int flexcontrollerindex;

	int numflexrules;
	int flexruleindex;

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

	// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
	// this value is used to calculate directional components of lighting 
	// on static props
	uint8_t constdirectionallightdot;

	// set during load of mdl data to track *desired* lod configuration (not actual)
	// the *actual* clamped root lod is found in studiohwdata
	// this is stored here as a global store to ensure the staged loading matches the rendering
	uint8_t rootLOD;

	// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
	// to be set as root LOD:
	//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
	//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
	uint8_t numAllowedRootLODs;

	uint8_t unused;

	float fadedistance;

	float gathersize; // what. from r5r struct

	int numunk_v54_early;
	int unkindex_v54_early;

	int unk_v54[2];

	// this is in all shipped models, probably part of their asset bakery. it should be 0x2CC.
	int mayaindex; // doesn't actually need to be written pretty sure, only four bytes when not present.

	int numsrcbonetransform;
	int srcbonetransformindex;

	int	illumpositionattachmentindex;

	int linearboneindex;

	int numboneflexdrivers; // unsure if that's what it is in apex
	int boneflexdriverindex;

	int unk1_v54[7];

	// maybe this is for the string table since it always has one byte padding it?
	// this is probably for some section I haven't seen or a string that hasn't been filled out.
	int unkindex1; // byte before string table start?

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

	// unk2_v54[3] is the chunk after following unkindex2's chunk
	int unk2_v54[3]; // the same four unks in v53 I think, the first index being unused now probably

	int unkindex3; // index to chunk after string block

	Vector3 mins; // min/max for Something
	Vector3 maxs; // seem to be the same as hull size

	int unk3_v54[3];

	int unkindex4; // chunk before unkindex3 sometimes

	int unk4_v54[3]; // same as unk3_v54_v121

	//int vgindex; // 0tVG
	//int unksize; // might be offset
	//int unksize1; // might be offset

};

// data source struct for subversion 12.1
struct studiohdr_121_t
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

	int numunk1_v121;
	int unkindex1_v121;

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
	int mayaindex;

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
#pragma pack(pop)