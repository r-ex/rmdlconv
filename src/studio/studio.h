// Copyright (c) 2023, rexx
// See LICENSE.txt for licensing information (GPL v3)

#pragma once

#include <studio/optimize.h>


#define IDSTUDIOHEADER				(('T'<<24)+('S'<<16)+('D'<<8)+'I') // little-endian "IDST"
#define IDSTUDIOANIMGROUPHEADER		(('G'<<24)+('A'<<16)+('D'<<8)+'I') // little-endian "IDAG"

#define MODEL_VERTEX_FILE_ID			(('V'<<24)+('S'<<16)+('D'<<8)+'I') // little-endian "IDSV"
#define MODEL_VERTEX_FILE_VERSION		4
#define MODEL_VERTEX_COLOR_FILE_ID		(('V'<<24)+('C'<<16)+('D'<<8)+'I') // little-endian "IDCV"
#define MODEL_VERTEX_COLOR_FILE_VERSION 1

#define MODEL_VERTEX_HWDATA_FILE_ID			(('G'<<24)+('V'<<16)+('t'<<8)+'0')
#define MODEL_VERTEX_HWDATA_FILE_VERSION	1

//#define MAX_NUM_BONES_PER_VERT 3
#define MAX_NUM_LODS 8


//===================
// STUDIO VERTEX DATA
//===================

namespace vvd
{
	struct mstudioweightextra_t
	{
		short weight[3]; // value divided by 32767.0

		short pad; // likely alignment

		int extraweightindex; // base index for vvw, add (weightIdx - 3)
	};

	struct mstudioboneweight_t
	{
		union
		{
			float	weight[MAX_NUM_BONES_PER_VERT];
			mstudioweightextra_t weightextra; // only in apex (v54)
		};

		unsigned char bone[MAX_NUM_BONES_PER_VERT]; // set to unsigned so we can read it
		char	numbones;
	};

	struct mstudiovertex_t
	{
		mstudioboneweight_t	m_BoneWeights;
		Vector m_vecPosition;
		Vector m_vecNormal;
		Vector2D m_vecTexCoord;
	};

	struct vertexFileFixup_t
	{
		int		lod;				// used to skip culled root lod
		int		sourceVertexID;		// absolute index from start of vertex/tangent blocks
		int		numVertexes;
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
		int tangentDataStart; // offset from base to tangent block

		const vertexFileFixup_t* const GetFixupData(int i) const
		{
			return reinterpret_cast<vertexFileFixup_t*>((char*)this + fixupTableStart) + i;
		}

		const mstudiovertex_t* const GetVertexData(int i) const
		{
			return reinterpret_cast<mstudiovertex_t*>((char*)this + vertexDataStart) + i;
		}

		const Vector4D* const GetTangentData(int i) const
		{
			return reinterpret_cast<Vector4D*>((char*)this + tangentDataStart) + i;
		}
	};
}

namespace vvc
{
	struct vertexColorFileHeader_t
	{
		int id; // IDCV
		int version; // 1
		int checksum; // same as studiohdr_t, ensures sync

		int numLODs; // num of valid lods
		int numLODVertexes[MAX_NUM_LODS]; // num verts for desired root lod

		int colorDataStart;
		int uv2DataStart;

		const Color32* const GetColorData(int i) const
		{
			return reinterpret_cast<Color32*>((char*)this + colorDataStart) + i;
		}

		const Vector2D* const GetUVData(int i) const
		{
			return reinterpret_cast<Vector2D*>((char*)this + uv2DataStart) + i;
		}
	};
}

namespace vvw
{
	struct mstudioboneweightextra_t
	{
		short	weight; // weight = this / 32767.0
		short   bone;
	};

	struct vertexBoneWeightsExtraFileHeader_t
	{
		int checksum; // same as studiohdr_t, ensures sync
		int version; // should be 1

		int numLODVertexes[MAX_NUM_LODS]; // maybe this but the others don't get filled?

		int weightDataStart; // index into mstudioboneweightextra_t array

		const mstudioboneweightextra_t* const GetWeightData(int i) const
		{
			return reinterpret_cast<mstudioboneweightextra_t*>((char*)this + weightDataStart) + i;
		}
	};
}

namespace vg
{
	// mesh flags, these are the same across all versions
	#define VERTEX_HAS_POSITION         0x1 // size of 12
	#define VERTEX_HAS_POSITION_PACKED  0x2 // size of 8
	// 0x4 size of 0
	// 0x8 size of 0
	#define VERTEX_HAS_COLOR			0x10 // size of 4
	// 0x20 size of 0
	#define VERTEX_HAS_UNK				0x40 // size of 8??? flag gets ignored, is this flag for having mesh data?
	// 0x80 size of 0
	// 0x100 size of 12, unpacked nml?
	#define VERTEX_HAS_NORMAL_PACKED	0x200
	// 0x400 size of 12, unpacked nml?
	#define VERTEX_HAS_UNK2				0x800 // size of 16, unpacked tangent? legacy weight? used but size doesn't factor in
	#define VERTEX_HAS_WEIGHT_BONES		0x1000 // size of 4, bone idxs?
	#define VERTEX_HAS_WEIGHT_VALUES_N	0x2000	// size of 8, more packed weight fields?
	#define VERTEX_HAS_WEIGHT_VALUES_2	0x4000 // size of 4, presumably this is 'packed' weights
	// 0x8000 size of 0
	// to
	// 0x800000 size of 0
	// 0x1000000 // repeating pattern start (unused flags)
	#define VERTEX_HAS_UV1				0x2000000 // size of 8, this is likely uv1
	#define VERTEX_HAS_UV2				0x200000000 // size of 8

#pragma pack(push, 1)
	struct mstudiopackedweights_t
	{
		unsigned short weight[2];	// packed weight with a max value of 32767, divide value by 32767 to get weight. weights will always correspond to first and second bone
		// if the mesh has extra bone weights the second value will be used as an index into the array of extra bone weights, max value of 65535.
	};

	struct mstudiopackedbones_t
	{
		unsigned char bones[3];	// when the model doesn't have extra bone weights all three are used for bone indices, otherwise in order they will be used for: first bone, last bone (assumes vvd->vg), unused.
		byte numbones;	// number of bones this vertex is weighted to excluding the base weight (value of 0 if only one weight, max of 15 with 16 weights)
	};

	struct Vertex_t
	{
		Vector m_vecPosition;
		Vector64 m_vecPositionPacked;
		mstudiopackedweights_t m_WeightsPacked;
		mstudiopackedbones_t m_BonesPacked;
		uint32_t m_NormalTangentPacked;
		Color32 m_color;
		Vector2D m_vecTexCoord;
		Vector2D m_vecTexCoord2;

		__int64 parentMeshIndex; // index of the mesh that owns this vertex, used for writing, not part of actual struct
	};
#pragma pack(pop)

	// rmdl versions 9-12
	namespace rev1
	{
		struct MeshHeader_t
		{
			__int64 flags;	// mesh flags

			// uses dynamic sized struct, similar to RLE animations
			unsigned int vertOffset;			    // start offset for this mesh's vertices
			unsigned int vertCacheSize;		    // size of the vertex structure
			unsigned int vertCount;			    // number of vertices

			int unk1;

			// vvw::mstudioboneweightextra_t
			int extraBoneWeightOffset;	// start offset for this mesh's "extended weights"
			int extraBoneWeightSize;    // size or count of extended weights

			// unsigned short
			int indexOffset;			// start offset for this mesh's "indices"
			int indexCount;				// number of indices

			// vvd::mstudioboneweight_t
			int legacyWeightOffset;	// seems to be an offset into the "external weights" buffer for this mesh
			int legacyWeightCount;   // seems to be the number of "external weights" that this mesh uses

			// vtx::StripHeader_t
			int stripOffset;        // Index into the strips structs
			int stripCount;

			// might be stuff like topologies and or bonestates, definitely unused
			int unk[4];

			/*int numBoneStateChanges;
			int boneStateChangeOffset;
			// MDL Version 49 and up only
			int numTopologyIndices;
			int topologyOffset;*/
		};

		struct ModelLODHeader_t
		{
			//Mesh array
			unsigned short meshOffset;
			unsigned short meshCount;

			float switchPoint;
		};

		struct UnkVgData_t
		{
			__int64 unk;
			float unk1;

			char data[0x24];
		};

		struct VertexGroupHeader_t
		{
			int id;			// 0x47567430	'0tVG'
			int version;	// 1
			int unk;		// Usually 0, checksum?
			int dataSize;	// Total size of data + header in starpak

			// unsigned char
			__int64 boneStateChangeOffset; // offset to bone remap buffer
			__int64 boneStateChangeCount;  // number of "bone remaps" (size: 1)

			// MeshHeader_t
			__int64 meshOffset;   // offset to mesh buffer
			__int64 meshCount;    // number of meshes (size: 0x48)

			// unsigned short
			__int64 indexOffset;     // offset to index buffer
			__int64 indexCount;      // number of indices (size: 2 (uint16_t))

			// uses dynamic sized struct, similar to RLE animations
			__int64 vertOffset;    // offset to vertex buffer
			__int64 vertBufferSize;     // number of chars in vertex buffer

			// vvw::mstudioboneweightextra_t
			__int64 extraBoneWeightOffset;   // offset to extended weights buffer
			__int64 extraBoneWeightSize;    // number of chars in extended weights buffer

			// there is one for every LOD mesh
			// i.e, unknownCount == lod.meshCount for all LODs
			__int64 unknownOffset;   // offset to buffer
			__int64 unknownCount;    // count (size: 0x30)

			// ModelLODHeader_t
			__int64 lodOffset;       // offset to LOD buffer
			__int64 lodCount;        // number of LODs (size: 0x8)

			// vvd::mstudioboneweight_t
			__int64 legacyWeightOffset;	// seems to be an offset into the "external weights" buffer for this mesh
			__int64 legacyWeightCount;   // seems to be the number of "external weights" that this mesh uses

			// vtx::StripHeader_t
			__int64 stripOffset;    // offset to strips buffer
			__int64 stripCount;     // number of strips (size: 0x23)

			__int64 unused[8];
		};
	}

	// seasons 7-13(?)
	// starts of the trend of offsets starting from their variable
	namespace rev2
	{
		struct MeshHeader_t // 96
		{
			__int64 flags;

			int vertCacheSize;		        // number of bytes used from the vertex buffer
			int vertCount;			        // number of vertices

			__int64 indexOffset;			// start offset for this mesh's "indices"
			__int64 indexCount : 56;        // number of indices
			__int64 indexType : 8;			// "type", is this tris vs quads? very odd

			__int64 vertOffset;             // start offset for this mesh's vertices
			__int64 vertBufferSize;         // TOTAL size of vertex buffer

			__int64 externalWeightOffset;	// start offset for this mesh's "extended weights"
			__int64 externalWeightSize;     // size or count of extended weights

			__int64 legacyWeightOffset;	    // seems to be an offset into the "external weights" buffer for this mesh
			__int64 legacyWeightCount;       // seems to be the number of "external weights" that this mesh uses

			__int64 stripOffset;            // Index into the strips structs
			__int64 stripCount;
		};

		struct ModelLODHeader_t
		{
			int dataOffset; // stolen from rmdl
			int dataSize;

			// this is like the section in rmdl, but backwards for some reason
			unsigned char meshCount;
			unsigned char meshIndex;	// for lod, probably 0 in most cases
			unsigned char lodLevel;
			unsigned char lodLevelUnk;	// set to 0 if there is no extra headers?

			float switchPoint;

			__int64 meshOffset; // from the start of this value, why?
		};

		// potential for several per file
		struct VertexGroupHeader_t
		{
			int id;			// 0x47567430	'0tVG'
			int version;	// 1
			int lodLevel;    // from RMDL struct
			int lodCount;	// total number of lods, them being based of the main header or sub headers.
			int unk;
			int unkCount; // the one in the main 0tVG header is the total of across all VG LODs.
			int lodOffset;
		};
	}

	namespace rev3
	{
		// This seems to be the only changed struct in S14 (v14.1?) vg's.
		// 16 new bytes at the end of the struct, unknown which type yet,
		// but the first 4 bytes seems to be filled a lot while the rest
		// of the 12 bytes not so much.
		struct MeshHeader_t
		{
			__int64 flags;

			int vertCacheSize;		        // number of bytes used from the vertex buffer
			int vertCount;			        // number of vertices

			__int64 indexOffset;			// start offset for this mesh's "indices"
			__int64 indexCount : 56;        // number of indices
			__int64 indexType : 8;			// "type", is this tris vs quads? very odd

			__int64 vertOffset;             // start offset for this mesh's vertices
			__int64 vertBufferSize;         // TOTAL size of vertex buffer

			__int64 externalWeightOffset;	// start offset for this mesh's "extended weights"
			__int64 externalWeightSize;     // size or count of extended weights

			__int64 legacyWeightOffset;	    // seems to be an offset into the "external weights" buffer for this mesh
			__int64 legacyWeightCount;       // seems to be the number of "external weights" that this mesh uses

			__int64 stripOffset;            // Index into the strips structs
			__int64 stripCount;

			// New in rev3.
			__int64 unk1;
			__int64 unk2;
		};
	}
}


//=============
// STUDIO MODEL
//=============

/* STUDIO STRUCT FLAGS END */

#define BONE_CALCULATE_MASK			0x1F
#define BONE_PHYSICALLY_SIMULATED	0x01	// bone is physically simulated when physics are active
#define BONE_PHYSICS_PROCEDURAL		0x02	// procedural when physics is active
#define BONE_ALWAYS_PROCEDURAL		0x04	// bone is always procedurally animated
#define BONE_SCREEN_ALIGN_SPHERE	0x08	// bone aligns to the screen, not constrained in motion.
#define BONE_SCREEN_ALIGN_CYLINDER	0x10	// bone aligns to the screen, constrained by it's own axis.

#define BONE_USED_BY_IKCHAIN		0x20 // bone is influenced by IK chains, added in V52 (Titanfall 1)

#define BONE_USED_MASK				0x0007FF00
#define BONE_USED_BY_ANYTHING		0x0007FF00
#define BONE_USED_BY_HITBOX			0x00000100	// bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT		0x00000200	// bone (or child) is used by an attachment point
#define BONE_USED_BY_VERTEX_MASK	0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0	0x00000400	// bone (or child) is used by the toplevel model via skinned vertex
#define BONE_USED_BY_VERTEX_LOD1	0x00000800	
#define BONE_USED_BY_VERTEX_LOD2	0x00001000  
#define BONE_USED_BY_VERTEX_LOD3	0x00002000
#define BONE_USED_BY_VERTEX_LOD4	0x00004000
#define BONE_USED_BY_VERTEX_LOD5	0x00008000
#define BONE_USED_BY_VERTEX_LOD6	0x00010000
#define BONE_USED_BY_VERTEX_LOD7	0x00020000
#define BONE_USED_BY_BONE_MERGE		0x00040000	// bone is available for bone merge to occur against it

#define BONE_FLAG_UNK				0x00080000 // where?

#define BONE_USED_BY_VERTEX_AT_LOD(lod) ( BONE_USED_BY_VERTEX_LOD0 << (lod) )
#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK ) | BONE_USED_BY_VERTEX_AT_LOD(lod) )

#define BONE_TYPE_MASK				0x00F00000
#define BONE_FIXED_ALIGNMENT		0x00100000	// bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

#define BONE_HAS_SAVEFRAME_POS		0x00200000	// Vector48
#define BONE_HAS_SAVEFRAME_ROT64	0x00400000	// Quaternion64
#define BONE_HAS_SAVEFRAME_ROT32	0x00800000	// Quaternion32

#define BONE_FLAG_UNK1				0x01000000 // where?


// mstudiojigglebone_t flags
#define JIGGLE_IS_FLEXIBLE				0x01
#define JIGGLE_IS_RIGID					0x02
#define JIGGLE_UNK						0x02 // apex
#define JIGGLE_HAS_YAW_CONSTRAINT		0x04
#define JIGGLE_HAS_PITCH_CONSTRAINT		0x08
#define JIGGLE_HAS_ANGLE_CONSTRAINT		0x10
#define JIGGLE_HAS_LENGTH_CONSTRAINT	0x20
#define JIGGLE_HAS_BASE_SPRING			0x40


// mstudiomovement_t flags
#define STUDIO_X		0x00000001
#define STUDIO_Y		0x00000002	
#define STUDIO_Z		0x00000004
#define STUDIO_XR		0x00000008
#define STUDIO_YR		0x00000010
#define STUDIO_ZR		0x00000020

#define STUDIO_LX		0x00000040
#define STUDIO_LY		0x00000080
#define STUDIO_LZ		0x00000100
#define STUDIO_LXR		0x00000200
#define STUDIO_LYR		0x00000400
#define STUDIO_LZR		0x00000800

#define STUDIO_LINEAR	0x00001000

#define STUDIO_TYPES	0x0003FFFF
#define STUDIO_RLOOP	0x00040000	// controller that wraps shortest distance


// mstudioikrule_t flags
#define IK_SELF 1
#define IK_WORLD 2
#define IK_GROUND 3
#define IK_RELEASE 4
#define IK_ATTACHMENT 5
#define IK_UNLATCH 6


// sequence and autolayer flags
// applies to: mstudioanimdesc_t, mstudioseqdesc_t, mstudioautolayer_t
#define STUDIO_LOOPING	0x0001		// ending frame should be the same as the starting frame
#define STUDIO_SNAP		0x0002		// do not interpolate between previous animation and this one
#define STUDIO_DELTA	0x0004		// this sequence "adds" to the base sequences, not slerp blends
#define STUDIO_AUTOPLAY	0x0008		// temporary flag that forces the sequence to always play
#define STUDIO_POST		0x0010		// 
#define STUDIO_ALLZEROS	0x0020		// this animation/sequence has no real animation data
#define STUDIO_FRAMEANIM 0x0040		// animation is encoded as by frame x bone instead of RLE bone x frame
#define STUDIO_ANIM_UNK3 0x0040
#define STUDIO_CYCLEPOSE 0x0080		// cycle index is taken from a pose parameter index
#define STUDIO_REALTIME	0x0100		// cycle index is taken from a real-time clock, not the animations cycle index
#define STUDIO_LOCAL	0x0200		// sequence has a local context sequence
#define STUDIO_HIDDEN	0x0400		// don't show in default selection views
#define STUDIO_OVERRIDE	0x0800		// a forward declared sequence (empty)
#define STUDIO_ACTIVITY	0x1000		// Has been updated at runtime to activity index
#define STUDIO_EVENT	0x2000		// Has been updated at runtime to event index on server
#define STUDIO_WORLD	0x4000		// sequence blends in worldspace
#define STUDIO_NOFORCELOOP 0x8000	// do not force the animation loop
#define STUDIO_EVENT_CLIENT 0x10000	// Has been updated at runtime to event index on client
#define STUDIO_ANIM_UNK		    0x20000 // actually first in v52, from where??
#define STUDIO_FRAMEMOVEMENT    0x40000 // framemovements are only read if this flag is present
#define STUDIO_ANIM_UNK2	    0x80000 // cherry blossom v53, levi in v54
#define STUDIO_BPANIM			0x100000 // bluepoint's custom animation format, unsure if this exists in the other games

// mstudioevent_t flags
#define NEW_EVENT_STYLE ( 1 << 10 )


// autolayer flags
// mstudioautolayer_t flags
//							0x0001
//							0x0002
//							0x0004
//							0x0008
#define STUDIO_AL_POST		0x0010		// 
//							0x0020
#define STUDIO_AL_SPLINE	0x0040		// convert layer ramp in/out curve is a spline instead of linear
#define STUDIO_AL_XFADE		0x0080		// pre-bias the ramp curve to compense for a non-1 weight, assuming a second layer is also going to accumulate
//							0x0100
#define STUDIO_AL_NOBLEND	0x0200		// animation always blends at 1.0 (ignores weight)
//							0x0400
//							0x0800
#define STUDIO_AL_LOCAL		0x1000		// layer is a local context sequence
#define STUDIO_AL_UNK		0x2000		// observed in v52
#define STUDIO_AL_POSE		0x4000		// layer blends using a pose parameter instead of parent cycle
#define STUDIO_AL_UNK1		0x8000		// added in v53 (probably)


// studiohdr_t flags
// This flag is set if no hitbox information was specified
#define STUDIOHDR_FLAGS_AUTOGENERATED_HITBOX	0x1

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_ENV_CUBEMAP		0x2

// Use this when there are translucent parts to the model but we're not going to sort it 
#define STUDIOHDR_FLAGS_FORCE_OPAQUE			0x4

// Use this when we want to render the opaque parts during the opaque pass
// and the translucent parts during the translucent pass
#define STUDIOHDR_FLAGS_TRANSLUCENT_TWOPASS		0x8

// This is set any time the .qc files has $staticprop in it
// Means there's no bones and no transforms
#define STUDIOHDR_FLAGS_STATIC_PROP				0x10

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_FB_TEXTURE		    0x20

// This flag is set by studiomdl.exe if a separate "$shadowlod" entry was present
//  for the .mdl (the shadow lod is the last entry in the lod list if present)
#define STUDIOHDR_FLAGS_HASSHADOWLOD			0x40

// NOTE:  This flag is set at loadtime, not mdl build time so that we don't have to rebuild
// models when we change materials.
#define STUDIOHDR_FLAGS_USES_BUMPMAPPING		0x80

// NOTE:  This flag is set when we should use the actual materials on the shadow LOD
// instead of overriding them with the default one (necessary for translucent shadows)
#define STUDIOHDR_FLAGS_USE_SHADOWLOD_MATERIALS	0x100

// NOTE:  This flag is set when we should use the actual materials on the shadow LOD
// instead of overriding them with the default one (necessary for translucent shadows)
#define STUDIOHDR_FLAGS_OBSOLETE				0x200

#define STUDIOHDR_FLAGS_UNUSED					0x400

// NOTE:  This flag is set at mdl build time
#define STUDIOHDR_FLAGS_NO_FORCED_FADE			0x800

// NOTE:  The npc will lengthen the viseme check to always include two phonemes
#define STUDIOHDR_FLAGS_FORCE_PHONEME_CROSSFADE	0x1000

// This flag is set when the .qc has $constantdirectionallight in it
// If set, we use constantdirectionallightdot to calculate light intensity
// rather than the normal directional dot product
// only valid if STUDIOHDR_FLAGS_STATIC_PROP is also set
#define STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT 0x2000

// Flag to mark delta flexes as already converted from disk format to memory format
#define STUDIOHDR_FLAGS_FLEXES_CONVERTED		0x4000 // unused since flexes are deprecated

// This flag indicates that the model has extra weights, which allows it to have 3< weights per bone.
#define STUDIOHDR_FLAGS_USES_EXTRA_BONE_WEIGHTS	0x4000

// Indicates the studiomdl was built in preview mode
#define STUDIOHDR_FLAGS_BUILT_IN_PREVIEW_MODE	0x8000

// Ambient boost (runtime flag)
#define STUDIOHDR_FLAGS_AMBIENT_BOOST			0x10000

// Don't cast shadows from this model (useful on first-person models)
#define STUDIOHDR_FLAGS_DO_NOT_CAST_SHADOWS		0x20000

// alpha textures should cast shadows in vrad on this model (ONLY prop_static!)
#define STUDIOHDR_FLAGS_CAST_TEXTURE_SHADOWS	0x40000

// Model has a quad-only Catmull-Clark SubD cage
#define STUDIOHDR_FLAGS_SUBDIVISION_SURFACE		0x80000

// flagged on load to indicate no animation events on this model
// might be a different thing on v54
#define STUDIOHDR_FLAGS_NO_ANIM_EVENTS			0x100000

// If flag is set then studiohdr_t.flVertAnimFixedPointScale contains the
// scale value for fixed point vert anim data, if not set then the
// scale value is the default of 1.0 / 4096.0.  Regardless use
// studiohdr_t::VertAnimFixedPointScale() to always retrieve the scale value
#define STUDIOHDR_FLAGS_VERT_ANIM_FIXED_POINT_SCALE	0x200000

#define STUDIOHDR_FLAGS_RESPAWN_UNK                 0x800000

// If this flag is present the model has vertex color, and by extension a VVC (IDVC) file.
#define STUDIOHDR_FLAGS_USES_VERTEX_COLOR	        0x1000000

// If this flag is present the model has a secondary UV layer, and by extension a VVC (IDVC) file.
#define STUDIOHDR_FLAGS_USES_UV2			        0x2000000

/* STUDIO STRUCT FLAGS END */


struct studiohdrshort_t
{
	int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
	int version; // Format version number, 52 to 54 for respawn models, 49 and under are valve
	int checksum; // This has to be the same in the phy and vtx files to load!
};

// Garry's Mod & Portal 2, versions 48 & 49 respectively
// not up to the same standards as respawn structs, needs cleaning up
struct mstudioanimsections_t
{
	int animblock; // index of animblock
	int animindex;
};

// used for piecewise loading of animation data
struct mstudioanimblock_t
{
	int datastart;
	int dataend;
};

struct mstudiojigglebone_t
{
	int flags;

	// general params
	float length; // how far from bone base, along bone, is tip
	float tipMass;

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

	int	unused[8];
};

struct mstudiohitboxset_t
{
	int sznameindex;
	int numhitboxes;
	int hitboxindex;
};

struct mstudioposeparamdesc_t
{
	int					sznameindex;
	int					flags;	// ????
	float				start;	// starting value
	float				end;	// ending value
	float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
};

struct mstudioiklink_t
{
	int bone;
	Vector	kneeDir; // ideal bending direction (per link, if applicable)
	Vector	unused0; // unused in v49
};

struct mstudiotexturedir_t
{
	int sznameindex;
};

struct mstudiolinearbone_t
{
	int numbones;

	int flagsindex;

	int	parentindex;

	int	posindex;

	int quatindex;

	int rotindex;

	int posetoboneindex;

	int	posscaleindex; // unused in v53

	int	rotscaleindex;

	int	qalignmentindex;

	int unused[6];
};

struct mstudiosrcbonetransform_t
{
	int sznameindex;

	matrix3x4_t	pretransform;
	matrix3x4_t	posttransform;
};

struct mstudioikchain_t
{
	int sznameindex;
	int linktype;
	int numlinks;
	int linkindex;
};

struct mstudiomodelgroup_t
{
	int					szlabelindex;	// textual name
	int					sznameindex;	// file name
};

struct mstudio_modelvertexdata_t
{
	// both void* but studiomdl is 32-bit
	int pVertexData;
	int pTangentData;
};

struct mstudio_meshvertexdata_t
{
	// indirection to this mesh's model's vertex data
	int modelvertexdata;

	// used for fixup calcs when culling top level lods
	// expected number of mesh verts at desired lod
	int numLODVertexes[MAX_NUM_LODS];
};

struct mstudiomesh_t
{
	int material;

	int modelindex;

	int numvertices;		// number of unique vertices/normals/texcoords
	int vertexoffset;		// vertex mstudiovertex_t

	int numflexes;			// vertex animation
	int flexindex;

	// special codes for material operations
	int	materialtype;
	int	materialparam;

	// a unique ordinal for this mesh
	int	meshid;

	Vector	center;

	mstudio_meshvertexdata_t vertexdata;

	int	unused[8]; // remove as appropriate
};

struct mstudiomodel_t
{
	char name[64];

	int type;

	float boundingradius;

	int	nummeshes;
	int	meshindex;
	// cache purposes
	int	numvertices;		// number of unique vertices/normals/texcoords
	int	vertexindex;		// vertex Vector
	int	tangentsindex;		// tangents Vector

	int numattachments;
	int attachmentindex;

	int numeyeballs;
	int eyeballindex;

	mstudio_modelvertexdata_t vertexdata;

	int unused[8];		// remove as appropriate
};

struct mstudiobodyparts_t
{
	int					sznameindex;
	inline char* const pszName() const { return ((char*)this + sznameindex); }

	int					nummodels;
	int					base;
	int					modelindex; // index into models array

	mstudiomodel_t* pModel(int i)
	{
		return reinterpret_cast<mstudiomodel_t*>((char*)this + modelindex) + i;
	};
};

struct mstudiotexture_t
{
	int						sznameindex;

	int						flags;
	int						used;
	int						unused1;
	int material;  // fixme: this needs to go away . .isn't used by the engine, but is used by studiomdl
	int clientmaterial;	// gary, replace with client material pointer if used

	int						unused[10];
};

struct mstudiobbox_t
{
	int					bone;
	int					group;				// intersection group
	Vector				bbmin;				// bounding box
	Vector				bbmax;
	int					szhitboxnameindex;	// offset to the name of the hitbox.
	int					unused[8];
};

struct mstudiobone_t
{
	int					sznameindex;

	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none

	// default values
	Vector				pos;
	Quaternion			quat;
	RadianEuler			rot;
	// compression scale
	Vector				posscale;
	Vector				rotscale;

	matrix3x4_t			poseToBone;
	Quaternion			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	int			physicsbone;	// index into physically simulated bone

	int					surfacepropidx;	// index into string tablefor property name

	int					contents;		// See BSPFlags.h for the contents flags
	int					surfacepropLookup;	// this index must be cached by the loader, not saved in the file
	int					unused[7];		// remove as appropriate
};

struct studiohdr2_t
{
	// NOTE: For forward compat, make sure any methods in this struct
	// are also available in studiohdr_t so no leaf code ever directly references
	// a studiohdr2_t structure
	int numsrcbonetransform;
	int srcbonetransformindex;
	inline mstudiosrcbonetransform_t* pSrcBoneTransforms() const { return (mstudiosrcbonetransform_t*)((byte*)this) + srcbonetransformindex; }

	int	illumpositionattachmentindex;

	float flMaxEyeDeflection;

	int linearboneindex;
	inline mstudiolinearbone_t* pLinearBones() const { return (linearboneindex) ? (mstudiolinearbone_t*)(((byte*)this) + linearboneindex) : NULL; }

	int sznameindex;
	inline char* pszName() const { return (char*)((byte*)this + sznameindex); };

	int m_nBoneFlexDriverCount;
	int m_nBoneFlexDriverIndex;

	int reserved[56];
};

struct studiohdr_t
{
	int					id;
	int					version;

	long				checksum;		// this has to be the same in the phy and vtx files to load!

	char				name[64];

	int					length;

	Vector				eyeposition;	// ideal eye position

	Vector				illumposition;	// illumination center

	Vector				hull_min;		// ideal movement hull size
	Vector				hull_max;

	Vector				view_bbmin;		// clipping bounding box
	Vector				view_bbmax;

	int					flags;

	int					numbones;			// bones
	int					boneindex;

	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;

	int					numhitboxsets;
	int					hitboxsetindex;

	// file local animations? and sequences
//private:
	int					numlocalanim;			// animations/poses
	int					localanimindex;		// animation descriptions

	int					numlocalseq;				// sequences
	int					localseqindex;

	//private:
	int			activitylistversion;	// initialization flag - have the sequences been indexed?
	int			eventsindexed;

	// raw textures
	int					numtextures;
	int					textureindex;

	// raw textures search paths
	int					numcdtextures;
	int					cdtextureindex;

	// replaceable textures tables
	int					numskinref;
	int					numskinfamilies;
	int					skinindex;

	int					numbodyparts;
	int					bodypartindex;

	// queryable attachable points
//private:
	int					numlocalattachments;
	int					localattachmentindex;


	// animation node to animation node transition graph
	int					numlocalnodes;
	int					localnodeindex;
	int					localnodenameindex;

	int					numflexdesc;
	int					flexdescindex;

	int					numflexcontrollers;
	int					flexcontrollerindex;

	int					numflexrules;
	int					flexruleindex;

	int					numikchains;
	int					ikchainindex;

	int					nummouths;
	int					mouthindex;

	//private:
	int					numlocalposeparameters;
	int					localposeparamindex;


	int					surfacepropindex;


	// Key values
	int					keyvalueindex;
	int					keyvaluesize;

	int					numlocalikautoplaylocks;
	int					localikautoplaylockindex;


	// The collision model mass that jay wanted
	float				mass;
	int					contents;

	// external animations, models, etc.
	int					numincludemodels;
	int					includemodelindex;

	// implementation specific call to get a named model

	// implementation specific back pointer to virtual data
	int /* mutable void* */ virtualModel;

	// for demand loaded animation blocks
	int					szanimblocknameindex;

	int					numanimblocks;
	int					animblockindex;

	int /* mutable void* */ animblockModel;

	int					bonetablebynameindex;

	// used by tools only that don't cache, but persist mdl's peer data
	// engine uses virtualModel to back link to cache pointers
	int /* void* */ pVertexBase;
	int /* void* */ pIndexBase;

	// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
	// this value is used to calculate directional components of lighting 
	// on static props
	byte				constdirectionallightdot;

	// set during load of mdl data to track *desired* lod configuration (not actual)
	// the *actual* clamped root lod is found in studiohwdata
	// this is stored here as a global store to ensure the staged loading matches the rendering
	byte				rootLOD;

	// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
	// to be set as root LOD:
	//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
	//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
	byte				numAllowedRootLODs;

	byte				unused[1];

	int					unused4;

	int					numflexcontrollerui;
	int					flexcontrolleruiindex;

	float				flVertAnimFixedPointScale;
	mutable int			surfacepropLookup;	// this index must be cached by the loader, not saved in the file

	// FIXME: Remove when we up the model version. Move all fields of studiohdr2_t into studiohdr_t.
	int					studiohdr2index;
	studiohdr2_t* pStudioHdr2() const { return (studiohdr2_t*)(((byte*)this) + studiohdr2index); }

	inline char* pszName() const { return pStudioHdr2()->pszName(); };

	// NOTE: No room to add stuff? Up the .mdl file format version 
	// [and move all fields in studiohdr2_t into studiohdr_t and kill studiohdr2_t],
	// or add your stuff to studiohdr2_t. See NumSrcBoneTransforms/SrcBoneTransform for the pattern to use.
	int					unused2[1];
};

// Titanfall 1, version '52'
namespace r1
{
	//
	// Model Per Triangle Collision (static props)
	//

	struct mstudiopertrivertex_t
	{
		// to get float value:
		// axisValue * ((float)(bbmax.axis - bbmin.axis) * 0.000015259022)
		// where axis is x, y, or z. bbmax and bbmin are from the pertri header.
		short x, y, z;
	};

	struct mstudiopertrihdr_t
	{
		short version; // game requires this to be 2 or else it errors

		short unk; // may or may not exist, version gets casted as short in ida

		Vector bbmin;
		Vector bbmax;

		int unused[8];
	};

	struct mstudiobone_t
	{
		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int parent; // parent bone
		int bonecontroller[6]; // bone controller index, -1 == none

		// default values
		Vector pos; // base bone position
		Quaternion quat;
		RadianEuler rot; // base bone rotation

		// compression scale
		Vector posscale; // scale muliplier for bone position in animations. depreciated in v53, as the posscale is stored in anim bone headers
		Vector rotscale; // scale muliplier for bone rotation in animations

		matrix3x4_t poseToBone;
		Quaternion qAlignment;

		int flags;
		int proctype;
		int procindex; // procedural rule offset
		int physicsbone; // index into physically simulated bone
		// from what I can tell this is the section that is parented to this bone, and if this bone is not the parent of any sections, it goes up the bone chain to the nearest bone that does and uses that section index
		int surfacepropidx; // index into string tablefor property name
		inline char* const pszSurfaceProp() const { return ((char*)this + surfacepropidx); }

		int contents; // See BSPFlags.h for the contents flags

		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		Vector scale; // base bone scale
		Vector scalescale; // scale muliplier for bone scale in animations

		int unused; // remove as appropriate
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiojigglebone_t
	{
		int flags;

		// general params
		float length; // how far from bone base, along bone, is tip
		float tipMass;

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

	// this struct is the same in r1 and r2
	struct mstudiolinearbone_t
	{
		int numbones;

		int flagsindex;
		inline int* pFlags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int*>((char*)this + flagsindex) + i; }
		inline int flags(int i) const { return *pFlags(i); }

		int	parentindex;
		inline int* pParent(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<int*>((char*)this + parentindex) + i;
		}

		int	posindex;
		inline const Vector* pPos(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Vector*>((char*)this + posindex) + i;
		}

		int quatindex;
		inline const Quaternion* pQuat(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Quaternion*>((char*)this + quatindex) + i;
		}

		int rotindex;
		inline const RadianEuler* pRot(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<RadianEuler*>((char*)this + rotindex) + i;
		}

		int posetoboneindex;
		inline const matrix3x4_t* pPoseToBone(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<matrix3x4_t*>((char*)this + posetoboneindex) + i;
		}

		int	posscaleindex;
		inline const Vector* pPosScale(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<Vector*>((char*)this + posscaleindex) + i; }

		int	rotscaleindex;
		inline const Vector* pRotScale(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<Vector*>((char*)this + rotscaleindex) + i; }

		int	qalignmentindex;
		inline const Quaternion* pQAlignment(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Quaternion*>((char*)this + qalignmentindex) + i;
		}

		int unused[6];
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiosrcbonetransform_t
	{
		int			sznameindex;
		matrix3x4_t	pretransform;
		matrix3x4_t	posttransform;
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudioattachment_t
	{
		int					sznameindex;
		unsigned int		flags;
		int					localbone;
		matrix3x4_t			local; // attachment point
		int					unused[8];
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiohitboxset_t
	{
		int					sznameindex;
		int					numhitboxes;
		int					hitboxindex;
	};

	// unchanged from p2
	struct mstudiobbox_t
	{
		int					bone;
		int					group;				// intersection group
		Vector				bbmin;				// bounding box
		Vector				bbmax;
		int					szhitboxnameindex;	// offset to the name of the hitbox.
		int					unused[8];
	};

	// never seen this used but we have it anyway
	struct mstudiolocalhierarchy_t
	{
		int			iBone;			// bone being adjusted
		int			iNewParent;		// the bones new parent

		float		start;			// beginning of influence
		float		peak;			// start of full influence
		float		tail;			// end of full influence
		float		end;			// end of all influence

		int			iStart;			// first frame 

		int			localanimindex;

		int			unused[4];
	};

	union mstudioanimvalue_t
	{
		struct
		{
			char	valid;
			char	total;
		} num;
		short		value;
	};

	struct mstudioanim_valueptr_t
	{
		short	offset[3];
		inline mstudioanimvalue_t* pAnimvalue(int i) const { return (offset[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset[i]) : nullptr; }
	};

	enum RleFlags_t : unsigned char
	{
		STUDIO_ANIM_RAWPOS = 0x01, // Vector48
		STUDIO_ANIM_RAWROT = 0x02, // Quaternion48
		STUDIO_ANIM_ANIMPOS = 0x04, // mstudioanim_valueptr_t
		STUDIO_ANIM_ANIMROT = 0x08, // mstudioanim_valueptr_t
		STUDIO_ANIM_DELTA = 0x10,
		STUDIO_ANIM_RAWROT2 = 0x20, // Quaternion64
		STUDIO_ANIM_RAWSCALE = 0x40, // Vector48
		STUDIO_ANIM_ANIMSCALE = 0x80  // mstudioanim_valueptr_t 
		// gravestone_01_animated.mdl and leviathan models use scale in RLE
	};

	struct mstudio_rle_anim_t
	{
		unsigned char		bone;
		unsigned char		flags;		// weighing options

		// leaving these here as there is no other way to show the data off, there likely should be scale stuff in here
		// I wanna redo these at some point so it's not copy/paste but honestly I am unsure if it can be done better

		// scale comes after pos in both cases

		// valid for animating data only
		inline char* pData(void) const { return (((char*)this) + sizeof(struct mstudio_rle_anim_t)); };
		inline mstudioanim_valueptr_t* pRotV(void) const { return (mstudioanim_valueptr_t*)(pData()); };
		inline mstudioanim_valueptr_t* pPosV(void) const { return (mstudioanim_valueptr_t*)(pData()) + ((flags & STUDIO_ANIM_ANIMROT) != 0); };

		// valid if animation unvaring over timeline
		/*inline Quaternion48* pQuat48(void) const { return (Quaternion48*)(pData()); };
		inline Quaternion64* pQuat64(void) const { return (Quaternion64*)(pData()); };
		inline Vector48* pPos(void) const { return (Vector48*)(pData() + ((flags & STUDIO_ANIM_RAWROT) != 0) * sizeof(*pQuat48()) + ((flags & STUDIO_ANIM_RAWROT2) != 0) * sizeof(*pQuat64())); };*/

		// need funcs for scale and to make this less stinky

		// points to next bone in the list
		short				nextoffset;
	};

	enum FrameFlags_t : unsigned char
	{
		STUDIO_FRAME_RAWPOS = 0x01, // Vector48 in constants
		STUDIO_FRAME_RAWROT = 0x02, // Quaternion48 in constants
		STUDIO_FRAME_RAWSCALE = 0x04, // Vector48 in constants, atlas_mp_scripted.mdl at_hotdrop_loop at_hotdrop_01
		STUDIO_FRAME_ANIMPOS = 0x08, // Quaternion48 in framedata
		STUDIO_FRAME_ANIMROT = 0x10, // Vector48 in framedata
		STUDIO_FRAME_ANIMSCALE = 0x20 // Vector48 in framedata
	};

	// per bone
	struct unkframeanimdata_t
	{
		short unknownFrame; // advances by six when bone has flags for frame data
		short unknownConstant; // advances by six when bone has flags for constant data
	};

	struct mstudio_frame_anim_t
	{
		int constantsoffset;

		int frameoffset;
		int framelength;

		int fixedOldBoneflags;

		int unknownOffset; // indexes into array of values that somewhat relate to flags

		int unused;
	};

	struct mstudioanimblock_t
	{
		int					datastart;
		int					dataend;
	};

	struct mstudioanimsections_t
	{
		int					animblock;
		int					animindex;
	};

	struct mstudiomovement_t
	{
		int					endframe;
		int					motionflags;
		float				v0;			// velocity at start of block
		float				v1;			// velocity at end of block
		float				angle;		// YAW rotation at end of this blocks movement
		Vector				vector;		// movement vector relative to this blocks initial angle
		Vector				position;	// relative to start of animation???
	};

	// new in Titanfall 1
	// translation track for origin bone, used in lots of animated scenes, requires STUDIO_FRAMEMOVEMENT
	// pos_x, pos_y, pos_z, yaw
	struct mstudioframemovement_t
	{
		float scale[4];
		short offset[4];
		inline mstudioanimvalue_t* pAnimvalue(int i) const { return (offset[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset[i]) : nullptr; }
	};

	struct mstudioikrule_t
	{
		int index;

		int type;
		int chain;

		int	bone;

		int slot;	// iktarget slot.  Usually same as chain.
		float height;
		float radius;
		float floor;
		Vector pos;
		Quaternion q;

		int compressedikerrorindex;

		int unused2;

		int iStart;
		int ikerrorindex;

		float start;	// beginning of influence
		float peak;	// start of full influence
		float tail;	// end of full influence
		float end;	// end of all influence

		float unused3;	// 
		float contact;	// frame footstep makes ground concact
		float drop;		// how far down the foot should drop when reaching for IK
		float top;		// top of the foot box

		int unused6;
		int unused7;
		int unused8;

		int szattachmentindex;		// name of world attachment

		float endHeight; // new in v52

		int unused[6];
	};

	struct mstudioikerror_t
	{
		Vector		pos;
		Quaternion	q;
	};

	struct mstudiocompressedikerror_t
	{
		float	scale[6];
		short	offset[6];
	};

	// similar to p2 struct with some unused slots filled
	struct mstudioanimdesc_t
	{
		int baseptr;

		int sznameindex;
		inline const char* pszName() const { return ((char*)this + sznameindex); }

		float fps; // frames per second	
		int flags; // looping/non-looping flags

		int numframes;

		// piecewise movement
		int	nummovements;
		int movementindex;
		inline mstudiomovement_t* const pMovement(int i) const { return reinterpret_cast<mstudiomovement_t*>((char*)this + movementindex) + i; };

		int ikrulezeroframeindex;

		int framemovementindex; // new in v52
		inline const mstudioframemovement_t* pFrameMovement() const { return reinterpret_cast<mstudioframemovement_t*>((char*)this + framemovementindex); }

		int unused1[4]; // remove as appropriate (and zero if loading older versions)	

		int animblock;
		int animindex; // non-zero when anim data isn't in sections
		mstudio_rle_anim_t* pAnim(int* piFrame) const; // returns pointer to data and new frame index

		int numikrules;
		int ikruleindex; // non-zero when IK data is stored in the mdl
		int animblockikruleindex; // non-zero when IK data is stored in animblock file

		int numlocalhierarchy;
		int localhierarchyindex;;

		int sectionindex;
		int sectionframes; // number of frames used in each fast lookup section, zero if not used
		inline const mstudioanimsections_t* pSection(int i) const { return reinterpret_cast<mstudioanimsections_t*>((char*)this + sectionindex) + i; }

		short zeroframespan; // frames per span
		short zeroframecount; // number of spans
		int zeroframeindex;

		float zeroframestalltime; // saved during read stalls
	};



	struct mstudioevent_t
	{
		float				cycle;
		int					event;
		int					type;
		char				options[64];

		int					szeventindex;
	};

	struct mstudioautolayer_t
	{
		//private:
		short				iSequence;
		short				iPose;
		//public:
		int					flags;
		float				start;	// beginning of influence
		float				peak;	// start of full influence
		float				tail;	// end of full influence
		float				end;	// end of all influence
	};

	struct mstudioiklock_t
	{
		int			chain;
		float		flPosWeight;
		float		flLocalQWeight;
		int			flags;

		int			unused[4];
	};

	struct mstudioactivitymodifier_t
	{
		int sznameindex;
		bool negate; // negate all other activity modifiers when this one is active?
	};

	struct mstudioseqdesc_t
	{
		int baseptr;

		int	szlabelindex;

		int szactivitynameindex;

		int flags; // looping/non-looping flags

		int activity; // initialized at loadtime to game DLL values
		int actweight;

		int numevents;
		int eventindex;

		Vector bbmin; // per sequence bounding box
		Vector bbmax;

		int numblends;

		// Index into array of shorts which is groupsize[0] x groupsize[1groupsize[1] in length
		int animindexindex;

		int movementindex; // [blend] float array for blended movement
		int groupsize[2];
		int paramindex[2]; // X, Y, Z, XR, YR, ZR
		float paramstart[2]; // local (0..1) starting value
		float paramend[2]; // local (0..1) ending value
		int paramparent;

		float fadeintime; // ideal cross fate in time (0.2 default)
		float fadeouttime; // ideal cross fade out time (0.2 default)

		int localentrynode; // transition node at entry
		int localexitnode; // transition node at exit
		int nodeflags; // transition rules

		float entryphase; // used to match entry gait
		float exitphase; // used to match exit gait

		float lastframe; // frame that should generation EndOfSequence

		int nextseq; // auto advancing sequences
		int pose; // index of delta animation between end and nextseq

		int numikrules;

		int numautolayers;
		int autolayerindex;

		int weightlistindex;

		int posekeyindex;

		int numiklocks;
		int iklockindex;

		// Key values
		int	keyvalueindex;
		int keyvaluesize;

		int cycleposeindex; // index of pose parameter to use as cycle index

		int activitymodifierindex;
		int numactivitymodifiers;

		int ikResetMask; // new in v52
		int unk1; // count? STUDIO_ANIMDESC_52_UNK?? ikReset (what above var is masking)

		int unused[3];
	};

	struct mstudiomesh_t
	{
		int material;

		int modelindex;

		int numvertices; // number of unique vertices/normals/texcoords
		int vertexoffset; // vertex mstudiovertex_t
		// offset by vertexoffset number of verts into vvd vertexes, relative to the models offset

// Access thin/fat mesh vertex data (only one will return a non-NULL result)

		int deprecated_numflexes; // vertex animation
		int deprecated_flexindex;

		// special codes for material operations
		int deprecated_materialtype;
		int deprecated_materialparam;

		// a unique ordinal for this mesh
		int meshid;

		Vector center;

		mstudio_meshvertexdata_t vertexloddata;

		int unused[8]; // remove as appropriate
	};

	struct mstudiomodel_t
	{
		char name[64];

		int type;

		float boundingradius;

		int nummeshes;
		int meshindex;

		mstudiomesh_t* pMesh(int i)
		{
			return reinterpret_cast<mstudiomesh_t*>((char*)this + meshindex) + i;
		}

		// cache purposes
		int numvertices; // number of unique vertices/normals/texcoords
		int vertexindex; // vertex Vector
		// offset by vertexindex number of chars into vvd verts
		int tangentsindex; // tangents Vector
		// offset by tangentsindex number of chars into vvd tangents

		int numattachments;
		int attachmentindex;

		int deprecated_numeyeballs;
		int deprecated_eyeballindex;

		int pad[4];

		int colorindex; // vertex color
		// offset by colorindex number of chars into vvc vertex colors
		int uv2index; // vertex second uv map
		// offset by uv2index number of chars into vvc secondary uv map

		int unused[4];
	};

	struct mstudiobodyparts_t
	{
		int					sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int					nummodels;
		int					base;
		int					modelindex; // index into models array

		mstudiomodel_t* pModel(int i)
		{
			return reinterpret_cast<mstudiomodel_t*>((char*)this + modelindex) + i;
		};
	};

	struct mstudioiklink_t
	{
		int		bone;
		Vector	kneeDir;	// ideal bending direction (per link, if applicable)
		Vector	unused0;	// unused
	};

	struct mstudioikchain_t
	{
		int				sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int				linktype;
		int				numlinks;
		int				linkindex;

		mstudioiklink_t* pLink(int i)
		{
			return reinterpret_cast<mstudioiklink_t*>((char*)this + linkindex) + i;
		}
	};

	struct mstudioposeparamdesc_t
	{
		int					sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int					flags;	// ????
		float				start;	// starting value
		float				end;	// ending value
		float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
	};

	struct mstudiomodelgroup_t
	{
		int					szlabelindex;	// textual name
		inline char* const pszLabel() const { return ((char*)this + szlabelindex); }

		int					sznameindex;	// file name
		inline char* const pszName() const { return ((char*)this + sznameindex); }
	};

	struct mstudiotexture_t
	{
		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int unused_flags;
		int used;
		int unused1;

		// these are turned into 64 bit ints on load and only filled in memory
		//mutable IMaterial		*material;  // fixme: this needs to go away . .isn't used by the engine, but is used by studiomdl
		//mutable void* clientmaterial;	// gary, replace with client material pointer if used
		int material_RESERVED;
		int clientmaterial_RESERVED;

		int unused[10];
	};

	struct studiohdr2_t
	{
		int numsrcbonetransform;
		int srcbonetransformindex;

		int	illumpositionattachmentindex;

		float flMaxEyeDeflection; // default to cos(30) if not set

		int linearboneindex;
		inline mstudiolinearbone_t* const pLinearBones() const { return linearboneindex ? reinterpret_cast<mstudiolinearbone_t*>((char*)this + linearboneindex) : nullptr; }

		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int m_nBoneFlexDriverCount;
		int m_nBoneFlexDriverIndex;

		// for static props (and maybe others)
		// Precomputed Per-Triangle AABB data
		int m_nPerTriAABBIndex;
		int m_nPerTriAABBNodeCount;
		int m_nPerTriAABBLeafCount;
		int m_nPerTriAABBVertCount;
		inline mstudiopertrihdr_t* const pPerTriHdr() const { return m_nPerTriAABBIndex ? reinterpret_cast<mstudiopertrihdr_t*>((char*)this + m_nPerTriAABBIndex) : nullptr; }

		// always "" or "Titan"
		int unkStringOffset;
		inline char* const pszUnkString() const { return ((char*)this + unkStringOffset); }

		int reserved[39];
	};

	struct studiohdr_t
	{
		int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
		int version; // Format version number, such as 52 (0x34,0x00,0x00,0x00)
		int checksum; // This has to be the same in the phy and vtx files to load!
		char name[64]; // The internal name of the model, padding with null chars.
		// Typically "my_model.mdl" will have an internal name of "my_model"
		int length; // Data size of MDL file in chars.

		Vector eyeposition;	// ideal eye position

		Vector illumposition;	// illumination center

		Vector hull_min;		// ideal movement hull size
		Vector hull_max;

		Vector view_bbmin;		// clipping bounding box
		Vector view_bbmax;

		int flags;

		// highest observed: 250
		// max is definitely 256 because 8bit uint limit
		int numbones; // bones
		int boneindex;
		inline mstudiobone_t* const pBone(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<mstudiobone_t*>((char*)this + boneindex) + i; }

		int numbonecontrollers; // bone controllers
		int bonecontrollerindex;

		int numhitboxsets;
		int hitboxsetindex;

		int numlocalanim; // animations/poses
		int localanimindex; // animation descriptions

		int numlocalseq; // sequences
		int	localseqindex;

		int activitylistversion; // initialization flag - have the sequences been indexed?
		int eventsindexed;

		// raw textures
		int numtextures;
		int textureindex;
		inline mstudiotexture_t* const pTexture(int i) const { assert(i >= 0 && i < numtextures); return reinterpret_cast<mstudiotexture_t*>((char*)this + textureindex) + i; }

		// raw textures search paths
		int numcdtextures;
		int cdtextureindex;

		// replaceable textures tables
		int numskinref;
		int numskinfamilies;
		int skinindex;

		int numbodyparts;
		int bodypartindex;
		inline mstudiobodyparts_t* const pBodypart(int i) const { assert(i >= 0 && i < numbodyparts); return reinterpret_cast<mstudiobodyparts_t*>((char*)this + bodypartindex) + i; }

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

		int deprecated_nummouths;
		int deprecated_mouthindex;

		int numlocalposeparameters;
		int localposeparamindex;

		int surfacepropindex;

		int keyvalueindex;
		int keyvaluesize;

		int numlocalikautoplaylocks;
		int localikautoplaylockindex;


		float mass;
		int contents;

		// external animations, models, etc.
		int numincludemodels;
		int includemodelindex;

		// implementation specific back pointer to virtual data
		int /* mutable void* */ virtualModel;

		// for demand loaded animation blocks
		int szanimblocknameindex;

		int numanimblocks;
		int animblockindex;

		int /* mutable void* */ animblockModel;

		int bonetablebynameindex;

		// used by tools only that don't cache, but persist mdl's peer data
		// engine uses virtualModel to back link to cache pointers
		int /* void* */ pVertexBase;
		int /* void* */ pIndexBase;

		// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
		// this value is used to calculate directional components of lighting 
		// on static props
		char constdirectionallightdot;

		// set during load of mdl data to track *desired* lod configuration (not actual)
		// the *actual* clamped root lod is found in studiohwdata
		// this is stored here as a global store to ensure the staged loading matches the rendering
		char rootLOD;

		// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
		// to be set as root LOD:
		//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
		//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
		char numAllowedRootLODs;

		char unused;

		float defaultFadeDist;	// set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
		// player/titan models seem to inherit this value from the first model loaded in menus.
		// works oddly on entities, probably only meant for static props

		int deprecated_numflexcontrollerui;
		int deprecated_flexcontrolleruiindex;

		float flVertAnimFixedPointScale;
		int surfacepropLookup;	// this index must be cached by the loader, not saved in the file

		// NOTE: No room to add stuff? Up the .mdl file format version 
		// [and move all fields in studiohdr2_t into studiohdr_t and kill studiohdr2_t],
		// or add your stuff to studiohdr2_t. See NumSrcBoneTransforms/SrcBoneTransform for the pattern to use.
		int studiohdr2index;
		inline studiohdr2_t* const pStudioHdr2() const { return reinterpret_cast<studiohdr2_t*>((char*)this + studiohdr2index); }

		// stored maya files from used dmx files, animation files are not added. for internal tools likely
		// in r1 this is a mixed bag, some are null with no data, some have a four byte section, and some actually have the files stored.
		int sourceFilenameOffset;
		inline char* const pszSourceFiles() const { return ((char*)this + sourceFilenameOffset); }

		inline mstudiolinearbone_t* const pLinearBones() const { return studiohdr2index ? pStudioHdr2()->pLinearBones() : nullptr; }
		inline char* const pszName() const { return studiohdr2index ? pStudioHdr2()->pszName() : nullptr; }
		inline mstudiopertrihdr_t* const pPerTriHdr() const { return studiohdr2index ? pStudioHdr2()->pPerTriHdr() : nullptr; }
		inline char* const pszUnkString() const { return studiohdr2index ? pStudioHdr2()->pszUnkString() : nullptr; }
	};
}

// Titanfall 2, version '53'
namespace r2
{

	//
	// Model Per Triangle Collision (static props)
	//

	struct mstudiopertrivertex_t
	{
		// to get float value:
		// axisValue * ((float)(bbmax.axis - bbmin.axis) * 0.000015259022)
		// where axis is x, y, or z. bbmax and bbmin are from the pertri header.
		short x, y, z;
	};

	struct mstudiopertrihdr_t
	{
		short version; // game requires this to be 2 or else it errors

		short unk; // may or may not exist, version gets casted as short in ida

		Vector bbmin;
		Vector bbmax;

		int unused[8];
	};

	struct mstudiobone_t
	{
		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int parent; // parent bone
		int bonecontroller[6]; // bone controller index, -1 == none

		// default values
		Vector pos; // base bone position
		Quaternion quat;
		RadianEuler rot; // base bone rotation
		Vector scale; // base bone scale

		// compression scale
		Vector posscale; // scale muliplier for bone position in animations. depreciated in v53, as the posscale is stored in anim bone headers
		Vector rotscale; // scale muliplier for bone rotation in animations
		Vector scalescale; // scale muliplier for bone scale in animations

		matrix3x4_t poseToBone;
		Quaternion qAlignment;

		int flags;
		int proctype;
		int procindex; // procedural rule offset
		int physicsbone; // index into physically simulated bone
		// from what I can tell this is the section that is parented to this bone, and if this bone is not the parent of any sections, it goes up the bone chain to the nearest bone that does and uses that section index
		int surfacepropidx; // index into string tablefor property name
		inline char* const pszSurfaceProp() const { return ((char*)this + surfacepropidx); }

		int contents; // See BSPFlags.h for the contents flags

		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		// unknown phy related section
		short unkIndex; // index into this section
		short unkCount; // number of sections for this bone?? see: models\s2s\s2s_malta_gun_animated.mdl

		int unused[7]; // remove as appropriate
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiojigglebone_t
	{
		int flags;

		// general params
		float length; // how far from bone base, along bone, is tip
		float tipMass;

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

	// this struct is the same in r1 and r2
	struct mstudiolinearbone_t
	{
		int numbones;

		int flagsindex;
		inline int* pFlags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int*>((char*)this + flagsindex) + i; }
		inline int flags(int i) const { return *pFlags(i); }

		int	parentindex;
		inline int* pParent(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<int*>((char*)this + parentindex) + i;
		}

		int	posindex;
		inline const Vector* pPos(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Vector*>((char*)this + posindex) + i;
		}

		int quatindex;
		inline const Quaternion* pQuat(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Quaternion*>((char*)this + quatindex) + i;
		}

		int rotindex;
		inline const RadianEuler* pRot(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<RadianEuler*>((char*)this + rotindex) + i;
		}

		int posetoboneindex;
		inline const matrix3x4_t* pPoseToBone(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<matrix3x4_t*>((char*)this + posetoboneindex) + i;
		}

		int	posscaleindex; // unused

		int	rotscaleindex;
		inline const Vector* pRotScale(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<Vector*>((char*)this + rotscaleindex) + i; }

		int	qalignmentindex;
		inline const Quaternion* pQAlignment(int i)
			const {
			assert(i >= 0 && i < numbones);
			return reinterpret_cast<Quaternion*>((char*)this + qalignmentindex) + i;
		}

		int unused[6];
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudiosrcbonetransform_t
	{
		int			sznameindex;
		matrix3x4_t	pretransform;
		matrix3x4_t	posttransform;
	};

	// this struct is the same in r1 and r2, and unchanged from p2
	struct mstudioattachment_t
	{
		int					sznameindex;
		unsigned int		flags;
		int					localbone;
		matrix3x4_t			local; // attachment point
		int					unused[8];
	};

	// this struct is the same in r1, r2, and early r5, and unchanged from p2
	struct mstudiohitboxset_t
	{
		int					sznameindex;
		int					numhitboxes;
		int					hitboxindex;
	};

	struct mstudiobbox_t
	{
		int					bone;
		int					group;				// intersection group
		Vector				bbmin;				// bounding box
		Vector				bbmax;
		int					szhitboxnameindex;	// offset to the name of the hitbox.

		int critoverride; // overrides the group to be a crit, 0 or 1. might be group override since group 1 is head.
		int keyvalueindex; // indexes into a kv group if used, mostly for titans.

		int unused[6];
	};

	union mstudioanimvalue_t
	{
		struct
		{
			char	valid;
			char	total;
		} num;
		short		value;
	};

	struct mstudioanim_valueptr_t
	{
		short	offset[3];
		inline mstudioanimvalue_t* pAnimvalue(int i) const { return (offset[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset[i]) : nullptr; }
	};

	// These work as toggles, flag enabled = raw data, flag disabled = "pointers", with rotations
	enum RleFlags_t : unsigned char
	{
		STUDIO_ANIM_DELTA = 0x01, // delta animation
		STUDIO_ANIM_RAWPOS = 0x02, // Vector48
		STUDIO_ANIM_RAWROT = 0x04, // Quaternion64
		STUDIO_ANIM_RAWSCALE = 0x08, // Vector48, drone_frag.mdl for scale track usage
		STUDIO_ANIM_NOROT = 0x10  // this flag is used to check if there is 1. no static rotation and 2. no rotation track
		// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/bone_setup.cpp#L393 used for this
		// this flag is used when the animation has no rotation data, it exists because it is not possible to check this as there are not separate flags for static/track rotation data
	};

	struct mstudio_rle_anim_t
	{
		float				posscale; // does what posscale is used for

		unsigned char		bone;
		unsigned char		flags;		// weighing options

		inline char* pData() const { return ((char*)this + sizeof(mstudio_rle_anim_t)); } // gets start of animation data, this should have a '+2' if aligned to 4
		inline mstudioanim_valueptr_t* pRotV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData()); } // returns rot as mstudioanim_valueptr_t
		inline mstudioanim_valueptr_t* pPosV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + 8); } // returns pos as mstudioanim_valueptr_t
		inline mstudioanim_valueptr_t* pScaleV() const { return reinterpret_cast<mstudioanim_valueptr_t*>(pData() + 14); } // returns scale as mstudioanim_valueptr_t

		//inline Quaternion64* pQuat64() const { return reinterpret_cast<Quaternion64*>(pData()); } // returns rot as static Quaternion64
		//inline Vector48* pPos() const { return reinterpret_cast<Vector48*>(pData() + 8); } // returns pos as static Vector48
		//inline Vector48* pScale() const { return reinterpret_cast<Vector48*>(pData() + 14); } // returns scale as static Vector48

		// points to next bone in the list
		inline int* pNextOffset() const { return reinterpret_cast<int*>(pData() + 20); }
		inline mstudio_rle_anim_t* pNext() const { return pNextOffset() ? reinterpret_cast<mstudio_rle_anim_t*>((char*)this + *pNextOffset()) : nullptr; } // untested
	};

	struct mstudioanimsections_t
	{
		int					animindex;
	};

	struct mstudiomovement_t
	{
		int					endframe;
		int					motionflags;
		float				v0;			// velocity at start of block
		float				v1;			// velocity at end of block
		float				angle;		// YAW rotation at end of this blocks movement
		Vector				vector;		// movement vector relative to this blocks initial angle
		Vector				position;	// relative to start of animation???
	};

	// new in Titanfall 1
	// translation track for origin bone, used in lots of animated scenes, requires STUDIO_FRAMEMOVEMENT
	// pos_x, pos_y, pos_z, yaw
	struct mstudioframemovement_t
	{
		float scale[4];
		short offset[4];
		inline mstudioanimvalue_t* pAnimvalue(int i) const { return (offset[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset[i]) : nullptr; }
	};

	struct mstudioikerror_t
	{
		Vector		pos;
		Quaternion	q;
	};

	struct mstudiocompressedikerror_t
	{
		float	scale[6];
		short	offset[6];
	};

	struct mstudioikrule_t
	{
		int index;
		int type;
		int chain;
		int bone;

		int slot; // iktarget slot. Usually same as chain.
		float height;
		float radius;
		float floor;
		Vector pos;
		Quaternion q;

		int compressedikerrorindex;

		int iStart;
		int ikerrorindex;

		float start; // beginning of influence
		float peak; // start of full influence
		float tail; // end of full influence
		float end; // end of all influence

		float contact; // frame footstep makes ground concact
		float drop; // how far down the foot should drop when reaching for IK
		float top; // top of the foot box

		int szattachmentindex; // name of world attachment

		float endHeight; // new in v52
		// titan_buddy_mp_core.mdl

		int unused[8];
	};

	struct mstudioanimdesc_t
	{
		int baseptr;

		int sznameindex;
		inline const char* pszName() const { return ((char*)this + sznameindex); }

		float fps; // frames per second	
		int flags; // looping/non-looping flags

		int numframes;

		// piecewise movement
		int nummovements;
		int movementindex;
		inline mstudiomovement_t* const pMovement(int i) const { return reinterpret_cast<mstudiomovement_t*>((char*)this + movementindex) + i; };

		int framemovementindex; // new in v52
		inline const mstudioframemovement_t* pFrameMovement() const { return reinterpret_cast<mstudioframemovement_t*>((char*)this + framemovementindex); }

		int animindex; // non-zero when anim data isn't in sections
		mstudio_rle_anim_t* pAnim(int* piFrame) const; // returns pointer to data and new frame index

		int numikrules;
		int ikruleindex; // non-zero when IK data is stored in the mdl

		int numlocalhierarchy;
		int localhierarchyindex;

		int sectionindex;
		int sectionframes; // number of frames used in each fast lookup section, zero if not used
		inline const mstudioanimsections_t* pSection(int i) const { return reinterpret_cast<mstudioanimsections_t*>((char*)this + sectionindex) + i; }

		int unused[8];
	};

	struct mstudioevent_t
	{
		float				cycle;
		int					event;
		int					type;
		char				options[64];

		int					szeventindex;
	};

	struct mstudioautolayer_t
	{
		//private:
		short				iSequence;
		short				iPose;
		//public:
		int					flags;
		float				start;	// beginning of influence
		float				peak;	// start of full influence
		float				tail;	// end of full influence
		float				end;	// end of all influence
	};

	struct mstudioiklock_t
	{
		int			chain;
		float		flPosWeight;
		float		flLocalQWeight;
		int			flags;

		int			unused[4];
	};

	struct mstudioactivitymodifier_t
	{
		int sznameindex;
		bool negate; // negate all other activity modifiers when this one is active?
	};

	struct mstudioseqdesc_t
	{
		int baseptr;

		int	szlabelindex;

		int szactivitynameindex;

		int flags; // looping/non-looping flags

		int activity; // initialized at loadtime to game DLL values
		int actweight;

		int numevents;
		int eventindex;

		Vector bbmin; // per sequence bounding box
		Vector bbmax;

		int numblends;

		// Index into array of shorts which is groupsize[0] x groupsize[1] in length
		int animindexindex;

		int movementindex; // [blend] float array for blended movement
		int groupsize[2];
		int paramindex[2]; // X, Y, Z, XR, YR, ZR
		float paramstart[2]; // local (0..1) starting value
		float paramend[2]; // local (0..1) ending value
		int paramparent;

		float fadeintime; // ideal cross fate in time (0.2 default)
		float fadeouttime; // ideal cross fade out time (0.2 default)

		int localentrynode; // transition node at entry
		int localexitnode; // transition node at exit
		int nodeflags; // transition rules

		float entryphase; // used to match entry gait
		float exitphase; // used to match exit gait

		float lastframe; // frame that should generation EndOfSequence

		int nextseq; // auto advancing sequences
		int pose; // index of delta animation between end and nextseq

		int numikrules;

		int numautolayers;
		int autolayerindex;

		int weightlistindex;

		int posekeyindex;

		int numiklocks;
		int iklockindex;

		// Key values
		int keyvalueindex;
		int keyvaluesize;

		int cycleposeindex; // index of pose parameter to use as cycle index

		int activitymodifierindex;
		int numactivitymodifiers;

		int ikResetMask; // new in v52
		// titan_buddy_mp_core.mdl
		// reset all ikrules with this type???
		int unk1; // count? STUDIO_ANIMDESC_52_UNK??
		// mayhaps this is the ik type applied to the mask if what's above it true

		int unused[8];
	};

	// pack here for our silly little unknown pointer
#pragma pack(push, 4)
	struct mstudiomesh_t
	{
		int material;

		int modelindex;

		int numvertices; // number of unique vertices/normals/texcoords
		int vertexoffset; // vertex mstudiovertex_t
		// offset by vertexoffset number of verts into vvd vertexes, relative to the models offset

// Access thin/fat mesh vertex data (only one will return a non-NULL result)

		int deprecated_numflexes; // vertex animation
		int deprecated_flexindex;

		// special codes for material operations
		int deprecated_materialtype;
		int deprecated_materialparam;

		// a unique ordinal for this mesh
		int meshid;

		Vector center;

		mstudio_meshvertexdata_t vertexloddata;

		void* pUnknown; // unknown memory pointer, probably one of the older vertex pointers but moved

		int unused[6]; // remove as appropriate
	};
#pragma pack(pop)

	struct mstudiomodel_t
	{
		char name[64];

		int type;

		float boundingradius;

		int nummeshes;
		int meshindex;

		mstudiomesh_t* pMesh(int i)
		{
			return reinterpret_cast<mstudiomesh_t*>((char*)this + meshindex) + i;
		}

		// cache purposes
		int numvertices; // number of unique vertices/normals/texcoords
		int vertexindex; // vertex Vector
		// offset by vertexindex number of chars into vvd verts
		int tangentsindex; // tangents Vector
		// offset by tangentsindex number of chars into vvd tangents

		int numattachments;
		int attachmentindex;

		int deprecated_numeyeballs;
		int deprecated_eyeballindex;

		int pad[4];

		int colorindex; // vertex color
		// offset by colorindex number of chars into vvc vertex colors
		int uv2index; // vertex second uv map
		// offset by uv2index number of chars into vvc secondary uv map

		int unused[4];
	};

	struct mstudiobodyparts_t
	{
		int					sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int					nummodels;
		int					base;
		int					modelindex; // index into models array

		mstudiomodel_t* pModel(int i)
		{
			return reinterpret_cast<mstudiomodel_t*>((char*)this + modelindex) + i;
		};
	};

	struct mstudioiklink_t
	{
		int		bone;
		Vector	kneeDir;	// ideal bending direction (per link, if applicable)
		Vector	unused0;	// unused
	};

	struct mstudioikchain_t
	{
		int				sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int				linktype;
		int				numlinks;
		int				linkindex;

		mstudioiklink_t* pLink(int i)
		{
			return reinterpret_cast<mstudioiklink_t*>((char*)this + linkindex) + i;
		}

		float	unk;		// no clue what this does tbh, tweaking it does nothing
		// default value: 0.707f
		// this value is similar to default source engine ikchain values

		int		unused[3];	// these get cut in apex so I can't imagine this is used
	};

	struct mstudioposeparamdesc_t
	{
		int					sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int					flags;	// ????
		float				start;	// starting value
		float				end;	// ending value
		float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
	};

	struct mstudiomodelgroup_t
	{
		int					szlabelindex;	// textual name
		inline char* const pszLabel() const { return ((char*)this + szlabelindex); }

		int					sznameindex;	// file name
		inline char* const pszName() const { return ((char*)this + sznameindex); }
	};

	struct mstudiotexture_t
	{
		int sznameindex;
		inline char* const pszName() const { return ((char*)this + sznameindex); }

		int unused_flags;
		int used;

		int unused[8];
	};

	struct studiohdr_t
	{
		int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
		int version; // Format version number, such as 53 (0x35,0x00,0x00,0x00)
		int checksum; // This has to be the same in the phy and vtx files to load!
		int sznameindex; // This has been moved from studiohdr2_t to the front of the main header.
		inline char* const pszName() const { return ((char*)this + sznameindex); }
		char name[64]; // The internal name of the model, padding with null chars.
		// Typically "my_model.mdl" will have an internal name of "my_model"
		int length; // Data size of MDL file in chars.

		Vector eyeposition;	// ideal eye position

		Vector illumposition;	// illumination center

		Vector hull_min;		// ideal movement hull size
		Vector hull_max;

		Vector view_bbmin;		// clipping bounding box
		Vector view_bbmax;

		int flags;

		// highest observed: 250
		// max is definitely 256 because 8bit uint limit
		int numbones; // bones
		int boneindex;
		inline mstudiobone_t* const pBone(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<mstudiobone_t*>((char*)this + boneindex) + i; }

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
		inline mstudiotexture_t* const pTexture(int i) const { assert(i >= 0 && i < numtextures); return reinterpret_cast<mstudiotexture_t*>((char*)this + textureindex) + i; }

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
		inline mstudiobodyparts_t* const pBodypart(int i) const { assert(i >= 0 && i < numbodyparts); return reinterpret_cast<mstudiobodyparts_t*>((char*)this + bodypartindex) + i; }

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

		int uiPanelCount;
		int uiPanelOffset;

		int numlocalposeparameters;
		int localposeparamindex;

		int surfacepropindex;

		int keyvalueindex;
		int keyvaluesize;

		int numlocalikautoplaylocks;
		int localikautoplaylockindex;

		float mass;
		int contents;

		// external animations, models, etc.
		int numincludemodels;
		int includemodelindex;

		// implementation specific back pointer to virtual data
		int /* mutable void* */ virtualModel;

		int bonetablebynameindex;

		// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
		// this value is used to calculate directional components of lighting 
		// on static props
		char constdirectionallightdot;

		// set during load of mdl data to track *desired* lod configuration (not actual)
		// the *actual* clamped root lod is found in studiohwdata
		// this is stored here as a global store to ensure the staged loading matches the rendering
		char rootLOD;

		// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
		// to be set as root LOD:
		//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
		//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
		char numAllowedRootLODs;

		char unused;

		float defaultFadeDist; // set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
		// player/titan models seem to inherit this value from the first model loaded in menus.
		// works oddly on entities, probably only meant for static props

		int deprecated_numflexcontrollerui;
		int deprecated_flexcontrolleruiindex;

		float flVertAnimFixedPointScale;
		int surfacepropLookup; // this index must be cached by the loader, not saved in the file

		// stored maya files from used dmx files, animation files are not added. for internal tools likely
		// in r1 this is a mixed bag, some are null with no data, some have a four byte section, and some actually have the files stored.
		int sourceFilenameOffset;
		inline char* const pszSourceFiles() const { return ((char*)this + sourceFilenameOffset); }

		int numsrcbonetransform;
		int srcbonetransformindex;

		int	illumpositionattachmentindex;

		int linearboneindex;
		inline mstudiolinearbone_t* const pLinearBones() const { return linearboneindex ? reinterpret_cast<mstudiolinearbone_t*>((char*)this + linearboneindex) : nullptr; }

		int m_nBoneFlexDriverCount;
		int m_nBoneFlexDriverIndex;

		// for static props (and maybe others)
		// Precomputed Per-Triangle AABB data
		int m_nPerTriAABBIndex;
		int m_nPerTriAABBNodeCount;
		int m_nPerTriAABBLeafCount;
		int m_nPerTriAABBVertCount;
		inline mstudiopertrihdr_t* const pPerTriHdr() const { return m_nPerTriAABBIndex ? reinterpret_cast<mstudiopertrihdr_t*>((char*)this + m_nPerTriAABBIndex) : nullptr; }

		// always "" or "Titan", this is probably for internal tools
		int unkStringOffset;
		inline char* const pszUnkString() const { return ((char*)this + unkStringOffset); }

		// ANIs are no longer used and this is reflected in many structs
		// start of interal file data
		int vtxOffset; // VTX
		int vvdOffset; // VVD / IDSV
		int vvcOffset; // VVC / IDCV 
		int phyOffset; // VPHY / IVPS

		int vtxSize; // VTX
		int vvdSize; // VVD / IDSV
		int vvcSize; // VVC / IDCV 
		int phySize; // VPHY / IVPS

		inline OptimizedModel::FileHeader_t* const pVTX() const { return vtxSize > 0 ? reinterpret_cast<OptimizedModel::FileHeader_t*>((char*)this + vtxOffset) : nullptr; }
		inline vvd::vertexFileHeader_t* const pVVD() const { return vvdSize > 0 ? reinterpret_cast<vvd::vertexFileHeader_t*>((char*)this + vvdOffset) : nullptr; }
		inline vvc::vertexColorFileHeader_t* const pVVC() const { return vvcSize > 0 ? reinterpret_cast<vvc::vertexColorFileHeader_t*>((char*)this + vvcOffset) : nullptr; }
		//inline ivps::phyheader_t* const pPHY() const { return phySize > 0 ? reinterpret_cast<ivps::phyheader_t*>((char*)this + phyOffset) : nullptr; }

		// this data block is related to the vphy, if it's not present the data will not be written
		// definitely related to phy, apex phy has this merged into it
		int unkOffset; // section between vphy and vtx.?
		int unkCount; // only seems to be used when phy has one solid

		// mostly seen on '_animated' suffixed models
		// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
		int boneFollowerCount;
		int boneFollowerOffset; // index only written when numbones > 1, means whatever func writes this likely checks this (would make sense because bonefollowers need more than one bone to even be useful). maybe only written if phy exists
		inline const int* const pBoneFollowers(int i) const { assert(i >= 0 && i < boneFollowerCount); return reinterpret_cast<int*>((char*)this + boneFollowerOffset) + i; }
		inline const int BoneFollower(int i) const { return *pBoneFollowers(i); }

		int unused1[60];
	};
}

// Apex Legends, version '54'
namespace r5
{
	// shared structs
	union mstudioanimvalue_t
	{
		struct
		{
			char	valid;
			char	total;
		} num;
		short		value;
	};

	#define STUDIO_ANIMPTR_Z 0x01
	#define STUDIO_ANIMPTR_Y 0x02
	#define STUDIO_ANIMPTR_X 0x04

	// to get mstudioanim_value_t for each axis:
	// if 1 flag, only read from offset
	// if 2 flags (e.g. y,z):
	// y @ offset;
	// z @ offset + (idx1 * sizeof(mstudioanimvalue_t));

	// if 3 flags:
	// x @ offset;
	// y @ offset + (idx1 * sizeof(mstudioanimvalue_t));
	// z @ offset + (idx2 * sizeof(mstudioanimvalue_t));
	struct mstudioanim_valueptr_t
	{
		short offset : 13;
		short flags : 3;
		unsigned char axisIdx1; // these two are definitely unsigned
		unsigned char axisIdx2;
	};

	// flags for the per bone animation headers
	// "mstudioanim_valueptr_t" indicates it has a set of offsets into anim tracks
	enum RleFlags_t : char
	{
		STUDIO_ANIM_ANIMSCALE = 0x01, // mstudioanim_valueptr_t
		STUDIO_ANIM_ANIMROT = 0x02, // mstudioanim_valueptr_t
		STUDIO_ANIM_ANIMPOS = 0x04, // mstudioanim_valueptr_t
	};

	// flags for the per bone array, in 4 bit sections (two sets of flags per char), aligned to two chars
	enum RleBoneFlags_t
	{
		STUDIO_ANIM_POS		= 0x1, // animation has pos values
		STUDIO_ANIM_ROT		= 0x2, // animation has rot values
		STUDIO_ANIM_SCALE	= 0x4, // animation has scale values
	};

	struct mstudio_rle_anim_t
	{
		short size : 13; // total size of all animation data, not next offset because even the last one has it
		short flags : 3;
	};

	// for unkCount/unkOffset in seqdesc
	struct unkseqdata_t
	{
		// generally 0-1
		float unkfloat;

		int unk;

		// quaternion mayhaps
		float unkfloat1;
		float unkfloat2;
		float unkfloat3;
		float unkfloat4;
	};

	// note: rrigs do not have a checksum
	// note: many things that are normally written on load are saved in file, for example 'surfacepropLookup'
	namespace v8
	{
		struct mstudiobone_t
		{
			int sznameindex;
			inline char* const pszName() const { return ((char*)this + sznameindex); }

			int parent; // parent bone
			int bonecontroller[6]; // bone controller index, -1 == none

			// default values
			Vector pos; // base bone position
			Quaternion quat;
			RadianEuler rot; // base bone rotation
			Vector scale; // base bone scale

			matrix3x4_t poseToBone;
			Quaternion qAlignment;

			int flags;
			int proctype;
			int procindex; // procedural rule offset
			int physicsbone; // index into physically simulated bone
			// from what I can tell this is the section that is parented to this bone, and if this bone is not the parent of any sections, it goes up the bone chain to the nearest bone that does and uses that section index
			int surfacepropidx; // index into string tablefor property name
			inline char* const pszSurfaceProp() const { return ((char*)this + surfacepropidx); }

			int contents; // See BSPFlags.h for the contents flags

			int surfacepropLookup; // this index must be cached by the loader, not saved in the file

			int unk_B0;

			int collisionIndex; // index into sections of collision, phy, bvh, etc. needs confirming
		};

		// apex changed this a bit, 'is_rigid' cut
		struct mstudiojigglebone_t
		{
			uint8_t flags;

			uint8_t bone; // id of bone, might be single byte

			uint8_t pad[2]; // possibly unused, possibly struct packing

			// general params
			float length; // how far from bone base, along bone, is tip
			float tipMass;
			float tipFriction; // friction applied to tip velocity, 0-1

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

		// this struct is the same in r1 and r2
		struct mstudiolinearbone_t
		{
			int numbones;

			int flagsindex;
			inline int* pFlags(int i) const { assert(i >= 0 && i < numbones); return reinterpret_cast<int*>((char*)this + flagsindex) + i; }
			inline int flags(int i) const { return *pFlags(i); }

			int	parentindex;
			inline int* pParent(int i)
				const {
				assert(i >= 0 && i < numbones);
				return reinterpret_cast<int*>((char*)this + parentindex) + i;
			}

			int	posindex;
			inline const Vector* pPos(int i)
				const {
				assert(i >= 0 && i < numbones);
				return reinterpret_cast<Vector*>((char*)this + posindex) + i;
			}

			int quatindex;
			inline const Quaternion* pQuat(int i)
				const {
				assert(i >= 0 && i < numbones);
				return reinterpret_cast<Quaternion*>((char*)this + quatindex) + i;
			}

			int rotindex;
			inline const RadianEuler* pRot(int i)
				const {
				assert(i >= 0 && i < numbones);
				return reinterpret_cast<RadianEuler*>((char*)this + rotindex) + i;
			}

			int posetoboneindex;
			inline const matrix3x4_t* pPoseToBone(int i)
				const {
				assert(i >= 0 && i < numbones);
				return reinterpret_cast<matrix3x4_t*>((char*)this + posetoboneindex) + i;
			}
		};

		// this struct is unchanged from p2
		struct mstudiosrcbonetransform_t
		{
			int			sznameindex;
			matrix3x4_t	pretransform;
			matrix3x4_t	posttransform;
		};

		struct mstudioattachment_t
		{
			int sznameindex;
			int flags;

			int localbone; // parent bone

			matrix3x4_t localmatrix; // attachment point
		};

		// this struct is the same in r1, r2, and early r5, and unchanged from p2
		struct mstudiohitboxset_t
		{
			int					sznameindex;
			int					numhitboxes;
			int					hitboxindex;
		};

		struct mstudiobbox_t
		{
			int bone;
			int group; // intersection group

			Vector bbmin; // bounding box
			Vector bbmax;

			int szhitboxnameindex; // offset to the name of the hitbox.

			int critShotOverride;	// value of one causes this to act as if it was group of 'head', may either be crit override or group override. mayhaps aligned boolean, look into this
			int hitdataGroupOffset;	// hit_data group in keyvalues this hitbox uses.
		};

		// rle stuff here

		struct mstudioanimsections_t
		{
			int					animindex;
		};

		struct mstudiomovement_t
		{
			int					endframe;
			int					motionflags;
			float				v0;			// velocity at start of block
			float				v1;			// velocity at end of block
			float				angle;		// YAW rotation at end of this blocks movement
			Vector				vector;		// movement vector relative to this blocks initial angle
			Vector				position;	// relative to start of animation???
		};

		// new in Titanfall 1
		// translation track for origin bone, used in lots of animated scenes, requires STUDIO_FRAMEMOVEMENT
		// pos_x, pos_y, pos_z, yaw
		struct mstudioframemovement_t
		{
			float scale[4];
			int sectionFrames;
			//inline mstudioanimvalue_t* pAnimvalue(int i) const { return (offset[i] > 0) ? reinterpret_cast<mstudioanimvalue_t*>((char*)this + offset[i]) : nullptr; }

			int* pSectionOffsets(int i)
			{
				return reinterpret_cast<int*>((char*)this + sizeof(mstudioframemovement_t)) + i;
			}
		};

		struct mstudiocompressedikerror_t
		{
			float scale[6]; // these values are the same as what posscale (if it was used) and rotscale are.
			int sectionFrames; // frames per section, may not match animdesc
		};

		struct mstudioikrule_t
		{
			int index;
			int type;
			int chain;
			int bone; // gets it from ikchain now pretty sure

			int slot; // iktarget slot. Usually same as chain.
			float height;
			float radius;
			float floor;
			Vector pos;
			Quaternion q;

			// apex does this oddly
			mstudiocompressedikerror_t compressedIkError;
			int compressedikerrorindex;

			int iStart;
			int ikerrorindex;

			float start; // beginning of influence
			float peak; // start of full influence
			float tail; // end of full influence
			float end; // end of all influence

			float contact; // frame footstep makes ground concact
			float drop; // how far down the foot should drop when reaching for IK
			float top; // top of the foot box

			int szattachmentindex; // name of world attachment

			float endHeight; // new in v52   
		};

		struct mstudioanimdesc_t
		{
			int baseptr;

			int sznameindex;
			inline const char* pszName() const { return ((char*)this + sznameindex); }

			float fps; // frames per second	
			int flags; // looping/non-looping flags

			int numframes;

			// piecewise movement
			int nummovements;
			int movementindex;
			inline mstudiomovement_t* const pMovement(int i) const { return reinterpret_cast<mstudiomovement_t*>((char*)this + movementindex) + i; };

			int framemovementindex; // new in v52
			inline const mstudioframemovement_t* pFrameMovement() const { return reinterpret_cast<mstudioframemovement_t*>((char*)this + framemovementindex); }

			int animindex; // non-zero when anim data isn't in sections
			mstudio_rle_anim_t* pAnim(int* piFrame) const; // returns pointer to data and new frame index

			int numikrules;
			int ikruleindex; // non-zero when IK data is stored in the mdl

			int numlocalhierarchy;
			int localhierarchyindex;

			int sectionindex;
			int sectionframes; // number of frames used in each fast lookup section, zero if not used
			inline const mstudioanimsections_t* pSection(int i) const { return reinterpret_cast<mstudioanimsections_t*>((char*)this + sectionindex) + i; }
		};

		struct mstudioevent_t
		{
			float				cycle;
			int					event;
			int					type;
			char				options[256];

			int					szeventindex;
		};

		struct mstudioautolayer_t
		{
			unsigned __int64	assetSequence; // hashed aseq guid asset, this needs to have a guid descriptor in rpak
			short				iSequence;
			short				iPose;

			int					flags;
			float				start;	// beginning of influence
			float				peak;	// start of full influence
			float				tail;	// end of full influence
			float				end;	// end of all influence
		};

		struct mstudioactivitymodifier_t
		{
			int sznameindex;
			bool negate; // negate all other activity modifiers when this one is active?
		};

		struct mstudioseqdesc_t
		{
			int baseptr;

			int	szlabelindex;

			int szactivitynameindex;

			int flags; // looping/non-looping flags

			int activity; // initialized at loadtime to game DLL values
			int actweight;

			int numevents;
			int eventindex;

			Vector bbmin; // per sequence bounding box
			Vector bbmax;

			int numblends;

			// Index into array of shorts which is groupsize[0] x groupsize[1] in length
			int animindexindex;

			int movementindex; // [blend] float array for blended movement
			int groupsize[2];
			int paramindex[2]; // X, Y, Z, XR, YR, ZR
			float paramstart[2]; // local (0..1) starting value
			float paramend[2]; // local (0..1) ending value
			int paramparent;

			float fadeintime; // ideal cross fate in time (0.2 default)
			float fadeouttime; // ideal cross fade out time (0.2 default)

			int localentrynode; // transition node at entry
			int localexitnode; // transition node at exit
			int nodeflags; // transition rules

			float entryphase; // used to match entry gait
			float exitphase; // used to match exit gait

			float lastframe; // frame that should generation EndOfSequence

			int nextseq; // auto advancing sequences
			int pose; // index of delta animation between end and nextseq

			int numikrules;

			int numautolayers;
			int autolayerindex;

			int weightlistindex;

			int posekeyindex;

			int numiklocks;
			int iklockindex;

			// Key values
			int keyvalueindex;
			int keyvaluesize;

			int cycleposeindex; // index of pose parameter to use as cycle index

			int activitymodifierindex;
			int numactivitymodifiers;

			int ikResetMask;	// new in v52
			// titan_buddy_mp_core.mdl
			// reset all ikrules with this type???
			int unk1;	// count? STUDIO_ANIMDESC_52_UNK??
			// mayhaps this is the ik type applied to the mask if what's above it true

			int unkOffset;
			int unkCount;
		};

		// pack here for our silly little unknown pointer
#pragma pack(push, 4)
		struct mstudiomesh_t
		{
			int material;

			int modelindex;

			int numvertices;	// number of unique vertices/normals/texcoords
			int vertexoffset;	// vertex mstudiovertex_t
								// offset by vertexoffset number of verts into vvd vertexes, relative to the models offset

			// Access thin/fat mesh vertex data (only one will return a non-NULL result)

			int deprecated_numflexes; // vertex animation
			int deprecated_flexindex;

			// special codes for material operations
			int deprecated_materialtype;
			int deprecated_materialparam;

			// a unique ordinal for this mesh
			int meshid;

			Vector center;

			mstudio_meshvertexdata_t vertexloddata;

			void* pUnknown; // unknown memory pointer, probably one of the older vertex pointers but moved
		};
#pragma pack(pop)

		struct mstudiomodel_t
		{
			char name[64];

			int unkStringOffset; // goes to bones sometimes

			int type;

			float boundingradius;

			int nummeshes;
			int meshindex;

			mstudiomesh_t* pMesh(int i)
			{
				return reinterpret_cast<mstudiomesh_t*>((char*)this + meshindex) + i;
			}

			// cache purposes
			int numvertices;	// number of unique vertices/normals/texcoords
			int vertexindex;	// vertex Vector
			// offset by vertexindex number of chars into vvd verts
			int tangentsindex;	// tangents Vector
			// offset by tangentsindex number of chars into vvd tangents

			int numattachments;
			int attachmentindex;

			int deprecated_numeyeballs;
			int deprecated_eyeballindex;

			int pad[4];

			int colorindex; // vertex color
			// offset by colorindex number of chars into vvc vertex colors
			int uv2index;	// vertex second uv map
			// offset by uv2index number of chars into vvc secondary uv map
		};

		struct mstudiobodyparts_t
		{
			int					sznameindex;
			inline char* const pszName() const { return ((char*)this + sznameindex); }

			int					nummodels;
			int					base;
			int					modelindex; // index into models array

			mstudiomodel_t* pModel(int i)
			{
				return reinterpret_cast<mstudiomodel_t*>((char*)this + modelindex) + i;
			};
		};

		struct mstudiorruiheader_t
		{
			int namehash; // uses rui hash algo
			int ruimeshindex; // offset to the actual rui mesh
		};

		struct mstudioruimesh_t
		{
			short numparents; // apparently you can have meshes parented to more than one bone(?)
			short numvertices; // number of verts
			short numfaces; // number of faces (quads)

			short unk; // num uvs? or this is num faces and current num faces is num uvs

			int parentindex; // this gets padding out front of it to even off the struct

			int vertexindex; // offset into smd style vertex data
			int unkindex;
			int vertmapindex; // offsets into a vertex map for each quad
			int facedataindex; // offset into uv section
		};

		// vertex map for a face
		struct mstudioruivertmap_t
		{
			// order of vertices for triangles:
			// 1st tri: 1-3-2
			// 2nd tri: 4-2-3

			// sometimes these are for a triangle instead of a quad, if that's the case then:
			// tri: 1-4-2
			// in this case the vert map will only cover 3 verts.


			// in v53 the first two are an array like such:
			// 1-3
			// while the third is the fourth vert.
			short vertid[3];
		};

		struct mstudioruivert_t
		{
			int parent; // relative to global mesh parent, assumed

			Vector vertexpos; // position of vertex relative to bone
		};

		// NOTE: 010 says this is not actually the fourth vertex, change name once the actual
		// type name is knowm or remove this comment if it is actually the fourth vertex.
		struct mstudioruifourthvert_t
		{
			// -1 if unused, dedicated slot for fourth vert
			byte vertextra;
			byte vertextra1;
		};

		struct mstudioruimeshface_t
		{
			// this might be the same as the RPak UIMG UV struct.

			// these values are for the two vertices that are not shared
			// for the other do as such:
			// vertex 2: take x from vextex 1 and y from vextex 4
			// vertex 2: take x from vextex 4 and y from vextex 1

			// normal smd uv, seems to calculate for other values
			Vector2D faceuvmin; // vertex 1
			Vector2D faceuvmax; // vertex 4

			// these could likely be calculated by doing math with a height/width scale
			// scale of the ui element
			Vector2D facescalemin; // vertex 1
			Vector2D facescalemax; // vertex 4
		};

		struct mstudioiklink_t
		{
			int		bone;
			Vector	kneeDir;	// ideal bending direction (per link, if applicable)
			// doesn't seem to be kneeDir, however, why would they keep the unused vector?
		};

		struct mstudioikchain_t
		{
			int				sznameindex;
			inline char* const pszName() const { return ((char*)this + sznameindex); }

			int				linktype;
			int				numlinks;
			int				linkindex;

			mstudioiklink_t* pLink(int i)
			{
				return reinterpret_cast<mstudioiklink_t*>((char*)this + linkindex) + i;
			}

			float	unk;		// no clue what this does tbh, tweaking it does nothing
			// default value: 0.707f
			// this value is similar to default source engine ikchain values
		};

		struct mstudioposeparamdesc_t
		{
			int					sznameindex;
			inline char* const pszName() const { return ((char*)this + sznameindex); }

			int					flags;	// ????
			float				start;	// starting value
			float				end;	// ending value
			float				loop;	// looping range, 0 for no looping, 360 for rotations, etc.
		};

		struct mstudiomodelgroup_t
		{
			int					szlabelindex;	// textual name
			inline char* const pszLabel() const { return ((char*)this + szlabelindex); }

			int					sznameindex;	// file name
			inline char* const pszName() const { return ((char*)this + sznameindex); }
		};

#pragma pack(push,4) // this caused issues
		struct mstudiotexture_t
		{
			int sznameindex;
			inline char* const pszName() const { return ((char*)this + sznameindex); }

			unsigned __int64 textureGuid; // guid/hash of this material
		};
#pragma pack(pop)

		struct mstudiocollmodel_t
		{
			int contentMasksIndex;
			int surfacePropsIndex;
			int surfaceNamesIndex;
			int headerCount;
		};

		struct mstudiocollheader_t
		{
			int unk;
			int bvhNodeIndex;
			int vertIndex;
			int bvhLeafIndex;
			float origin[3];
			float scale;
		};

		struct dsurfaceproperty_t
		{
			short unk;
			uint8_t surfacePropId;
			uint8_t contentMaskOffset;
			int surfaceNameOffset;
		};

		struct studiohdr_t
		{
			int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
			int version; // Format version number, such as 54 (0x36,0x00,0x00,0x00)
			int checksum; // This has to be the same in the phy and vtx files to load!
			int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
			char name[64]; // The internal name of the model, padding with null chars.
			// Typically "my_model.mdl" will have an internal name of "my_model"
			int length; // Data size of MDL file in chars.

			Vector eyeposition;	// ideal eye position

			Vector illumposition;	// illumination center

			Vector hull_min;		// ideal movement hull size
			Vector hull_max;

			Vector view_bbmin;		// clipping bounding box
			Vector view_bbmax;

			int flags;

			int numbones; // bones
			int boneindex;

			int numbonecontrollers; // bone controllers
			int bonecontrollerindex;

			int numhitboxsets;
			int hitboxsetindex;

			// seemingly unused now, as animations are per sequence
			int numlocalanim; // animations/poses
			int localanimindex; // animation descriptions

			int numlocalseq; // sequences
			int	localseqindex;

			int activitylistversion; // initialization flag - have the sequences been indexed?

			// mstudiotexture_t
			// short rpak path
			// raw textures
			int materialtypesindex; // index into an array of char sized material type enums for each material used by the model
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
			inline mstudiobodyparts_t* const pBodypart(int i) const { assert(i >= 0 && i < numbodyparts); return reinterpret_cast<mstudiobodyparts_t*>((char*)this + bodypartindex) + i; }

			int numlocalattachments;
			int localattachmentindex;

			int numlocalnodes;
			int localnodeindex;
			int localnodenameindex;

			int unkNodeCount; // ???
			int nodeDataOffsetsOffset; // index into an array of int sized offsets that read into the data for each node

			int meshOffset; // hard offset to the start of this models meshes

			// all flex related model vars and structs are stripped in respawn source
			int deprecated_numflexcontrollers;
			int deprecated_flexcontrollerindex;

			int deprecated_numflexrules;
			int deprecated_flexruleindex;

			int numikchains;
			int ikchainindex;

			// mesh panels for using rui on models, primarily for weapons
			int uiPanelCount;
			int uiPanelOffset;

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
			// technically still functional though I am unsure why you'd want to use it
			int numincludemodels;
			int includemodelindex;

			int /* mutable void* */ virtualModel;

			int bonetablebynameindex;

			// if STUDIOHDR_FLAGS_CONSTANT_DIRECTIONAL_LIGHT_DOT is set,
			// this value is used to calculate directional components of lighting 
			// on static props
			char constdirectionallightdot;

			// set during load of mdl data to track *desired* lod configuration (not actual)
			// the *actual* clamped root lod is found in studiohwdata
			// this is stored here as a global store to ensure the staged loading matches the rendering
			char rootLOD;

			// set in the mdl data to specify that lod configuration should only allow first numAllowRootLODs
			// to be set as root LOD:
			//	numAllowedRootLODs = 0	means no restriction, any lod can be set as root lod.
			//	numAllowedRootLODs = N	means that lod0 - lod(N-1) can be set as root lod, but not lodN or lower.
			char numAllowedRootLODs;

			char unused;

			float defaultFadeDist;	// set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
			// player/titan models seem to inherit this value from the first model loaded in menus.
			// works oddly on entities, probably only meant for static props

			float gatherSize; // what. from r5r struct. no clue what this does, seemingly for early versions of apex bsp but stripped in release apex (season 0)
			// bad name, frustum culling

			int deprecated_numflexcontrollerui;
			int deprecated_flexcontrolleruiindex;

			float flVertAnimFixedPointScale; // to be verified
			int surfacepropLookup; // saved in the file

			// this is in most shipped models, probably part of their asset bakery.
			// doesn't actually need to be written pretty sure, only four chars when not present.
			// this is not completely true as some models simply have nothing, such as animation models.
			int sourceFilenameOffset;

			int numsrcbonetransform;
			int srcbonetransformindex;

			int	illumpositionattachmentindex;

			int linearboneindex;

			// unsure what this is for but it exists for jigglbones
			int procBoneCount;
			int procBoneTableOffset;
			int linearProcBoneOffset;

			// depreciated as they are removed in 12.1
			int deprecated_m_nBoneFlexDriverCount;
			int deprecated_m_nBoneFlexDriverIndex;

			int deprecated_m_nPerTriAABBIndex;
			int deprecated_m_nPerTriAABBNodeCount;
			int deprecated_m_nPerTriAABBLeafCount;
			int deprecated_m_nPerTriAABBVertCount;

			// always "" or "Titan"
			int unkStringOffset;

			// this is now used for combined files in rpak, vtx, vvd, and vvc are all combined while vphy is separate.
			// the indexes are added to the offset in the rpak mdl_ header.
			// vphy isn't vphy, looks like a heavily modified vphy.
			// as of s2/3 these are no unused except phy
			int vtxOffset; // VTX
			int vvdOffset; // VVD / IDSV
			int vvcOffset; // VVC / IDCV 
			int phyOffset; // VPHY / IVPS

			int vtxSize;
			int vvdSize;
			int vvcSize;
			int phySize; // still used in models using vg

			// unused in apex, gets cut in 12.1
			int deprecated_unkOffset; // deprecated_imposterIndex
			int deprecated_unkCount; // deprecated_numImposters

			// mostly seen on '_animated' suffixed models
			// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
			int boneFollowerCount;
			int boneFollowerOffset;

			// BVH4 size (?)
			Vector mins;
			Vector maxs; // seem to be the same as hull size

			int unk3_v54[3];

			int bvhOffset; // bvh4 tree

			short unk4_v54[2]; // same as unk3_v54_v121, 2nd might be base for other offsets? these are related to bvh4 stuff. collision detail for bvh (?)

			// new in apex vertex weight file for verts that have more than three weights
			// vvw is a 'fake' extension name, we do not know the proper name.
			int vvwOffset; // index will come last after other vertex files
			int vvwSize;
		};
	}

	namespace v120
	{
		struct dsurfacepropertydata_t
		{
			uint8_t surfacePropId1;
			uint8_t surfacePropId2;
			uint16_t flags;
		};

		struct mstudiocollheader_t
		{
			int unk;
			int bvhNodeIndex;
			int vertIndex;
			int bvhLeafIndex;

			// A new index that indexes into a buffer that contains surfacePropId's
			// The number of surfacePropId's is determined by surfacePropCount.
			// surfacePropArrayCount determines how many of these buffers we have,
			// they are the same size but sometimes with different surfacePropId's.
			int surfacePropDataIndex;

			uint8_t surfacePropArrayCount;
			uint8_t surfacePropCount;

			short padding_maybe;

			float origin[3];
			float scale;
		};
	}

	namespace v121
	{
		// data source struct for subversion 12.1
		struct studiohdr_t
		{
			int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
			int version; // Format version number, such as 54 (0x36,0x00,0x00,0x00)
			int checksum; // This has to be the same in the phy and vtx files to load!
			int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
			char name[64]; // The internal name of the model, padding with null chars.
			// Typically "my_model.mdl" will have an internal name of "my_model"
			int length; // Data size of MDL file in chars.

			Vector eyeposition;	// ideal eye position

			Vector illumposition;	// illumination center

			Vector hull_min;		// ideal movement hull size
			Vector hull_max;

			Vector view_bbmin;		// clipping bounding box
			Vector view_bbmax;

			int flags;

			int numbones; // bones
			int boneindex;

			int numbonecontrollers; // bone controllers
			int bonecontrollerindex;

			int numhitboxsets;
			int hitboxsetindex;

			// seemingly unused now, as animations are per sequence
			int numlocalanim; // animations/poses
			int localanimindex; // animation descriptions

			int numlocalseq; // sequences
			int	localseqindex;

			int activitylistversion; // initialization flag - have the sequences been indexed?

			// mstudiotexture_t
			// short rpak path
			// raw textures
			int materialtypesindex; // index into an array of char sized material type enums for each material used by the model
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
			inline mstudiobodyparts_t* const pBodypart(int i) const { assert(i >= 0 && i < numbodyparts); return reinterpret_cast<mstudiobodyparts_t*>((char*)this + bodypartindex) + i; }

			int numlocalattachments;
			int localattachmentindex;

			int numlocalnodes;
			int localnodeindex;
			int localnodenameindex;

			int unkNodeCount; // ???
			int nodeDataOffsetsOffset; // index into an array of int sized offsets that read into the data for each node

			int numikchains;
			int ikchainindex;

			// mesh panels for using rui on models, primarily for weapons
			int uiPanelCount;
			int uiPanelOffset;

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
			// technically still functional though I am unsure why you'd want to use it
			int numincludemodels;
			int includemodelindex;

			int /* mutable void* */ virtualModel;

			int bonetablebynameindex;

			// stuff moved from vg in v12.1
			// TODO: rename to hw as vg is a shit name
			int vgMeshCount; // total number of meshes, not including LODs. better names in general
			int vgMeshOffset;

			int boneStateOffset;
			int boneStateCount;

			int unk_v54_v121; // related to vg likely. TODO: check if this is the missing vars: constdirectionallightdot, rootLOD, numAllowedRootLODs, unused

			int vgSize;

			short vgUnk; // same as padding in vg header
			short numVGLods; // same as lod count in vg header

			int vgUnknownCount; // same as unk1 in vg header

			int vgHeaderOffset;
			int vgHeaderCount;

			int vgLODOffset;
			int vgLODCount;

			float defaultFadeDist;	// set to -1 to never fade. set above 0 if you want it to fade out, distance is in feet.
			// player/titan models seem to inherit this value from the first model loaded in menus.
			// works oddly on entities, probably only meant for static props

			float gatherSize;	// what. from r5r struct. no clue what this does, seemingly for early versions of apex bsp but stripped in release apex (season 0)
			// bad name, frustum culling

			float flVertAnimFixedPointScale; // to be verified
			int surfacepropLookup; // saved in the file

			// this is in most shipped models, probably part of their asset bakery.
			// doesn't actually need to be written pretty sure, only four chars when not present.
			// this is not completely true as some models simply have nothing, such as animation models.
			int sourceFilenameOffset;

			int numsrcbonetransform;
			int srcbonetransformindex;

			int	illumpositionattachmentindex;

			int linearboneindex;

			// unsure what this is for but it exists for jigglbones
			int procBoneCount;
			int procBoneTableOffset;
			int linearProcBoneOffset;

			// always "" or "Titan"
			int unkStringOffset;

			// this is now used for combined files in rpak, vtx, vvd, and vvc are all combined while vphy is separate.
			// the indexes are added to the offset in the rpak mdl_ header.
			// vphy isn't vphy, looks like a heavily modified vphy.
			// as of s2/3 these are no unused except phy
			int vtxOffset; // VTX
			int vvdOffset; // VVD / IDSV
			int vvcOffset; // VVC / IDCV 
			int phyOffset; // VPHY / IVPS

			int vtxSize;
			int vvdSize;
			int vvcSize;
			int phySize; // still used in models using vg

			// mostly seen on '_animated' suffixed models
			// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
			int boneFollowerCount;
			int boneFollowerOffset;

			// BVH4 size (?)
			Vector mins;
			Vector maxs; // seem to be the same as hull size

			int bvhOffset; // bvh4 tree

			short unk4_v54[2]; // same as unk3_v54_v121, 2nd might be base for other offsets? these are related to bvh4 stuff. collision detail for bvh (?)

			// new in apex vertex weight file for verts that have more than three weights
			// vvw is a 'fake' extension name, we do not know the proper name.
			int vvwOffset; // index will come last after other vertex files
			int vvwSize;
		};

		struct mstudioanimdesc_t
		{
			int baseptr;

			int sznameindex;

			float fps; // frames per second	
			int flags; // looping/non-looping flags

			int numframes;

			// piecewise movement
			int nummovements;
			int movementindex;

			int framemovementindex; // new in v52

			int animindex; // non-zero when anim data isn't in sections

			int numikrules;
			int ikruleindex; // non-zero when IK data is stored in the mdl

			int sectionindex;
			int unk; // what, obviously section related as it's wedged between sectionindex and sectiom frames
			int sectionframes; // number of frames used in each fast lookup section, zero if not used

			int unk1[4]; // is this even real?

			// it seems like there's another int here but I'm unsure
		};

#pragma pack(push, 4)
		struct mstudioanimsections_t
		{
			int animindex;
			bool external;
		};
#pragma pack(pop)

		struct mstudiobone_t
		{
			int sznameindex;
			inline char* const pszName() const { return ((char*)this + sznameindex); }

			int parent; // parent bone
			int bonecontroller[6]; // bone controller index, -1 == none

			// default values
			Vector pos; // base bone position
			Quaternion quat;
			RadianEuler rot; // base bone rotation
			Vector scale; // base bone scale

			matrix3x4_t poseToBone;
			Quaternion qAlignment;

			int flags;
			int proctype;
			int procindex; // procedural rule offset
			int physicsbone; // index into physically simulated bone
			// from what I can tell this is the section that is parented to this bone, and if this bone is not the parent of any sections, it goes up the bone chain to the nearest bone that does and uses that section index
			int surfacepropidx; // index into string tablefor property name
			inline char* const pszSurfaceProp() const { return ((char*)this + surfacepropidx); }

			int contents; // See BSPFlags.h for the contents flags

			int surfacepropLookup; // this index must be cached by the loader, not saved in the file
			
			uint8_t collisionIndex; // index into sections of collision, phy, bvh, etc. needs confirming

			uint8_t unk_B1[3]; // maybe this is 'unk'?
		};

		struct mstudiomodel_t
		{
			char name[64];

			int unkindex2; // byte before string block

			// it looks like they write the entire name
			// then write over it with other values where needed
			// why.
			int type;

			float boundingradius;

			int nummeshes;
			int meshindex;

			// cache purposes
			int numvertices; // number of unique vertices/normals/texcoords
			int vertexindex; // vertex Vector
			int tangentsindex; // tangents Vector

			int numattachments;
			int attachmentindex;

			int colorindex; // vertex color
			// offset by colorindex number of bytes into vvc vertex colors
			int uv2index; // vertex second uv map
			// offset by uv2index number of bytes into vvc secondary uv map
		};

		struct mstudiomesh_t
		{
			int material;

			int modelindex;

			int numvertices; // number of unique vertices/normals/texcoords
			int vertexoffset; // vertex mstudiovertex_t

			// a unique ordinal for this mesh
			int meshid;

			Vector center;

			// deprecated in later versions?
			mstudio_meshvertexdata_t vertexloddata;

			char unk[8]; // these are suposed to be filled on load, however this isn't true??
		};
	}

	namespace v122
	{
		struct mstudioevent_t
		{
			float cycle;
			int	event;
			int type; // this will be 0 if old style I'd imagine

			int unk; // 2, 4, animseq/weapons/crypto_heirloom/ptpov_sword_crypto/draw.rseq

			char options[256]; // this is the only difference compared to normal v54

			int szeventindex;
		};

		struct studiohdr_t
		{
			int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
			int version; // Format version number, such as 54 (0x36,0x00,0x00,0x00)
			int checksum; // This has to be the same in the phy and vtx files to load!
			int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
			char name[64]; // The internal name of the model, padding with null chars.
			// Typically "my_model.mdl" will have an internal name of "my_model"
			int length; // Data size of MDL file in chars.

			Vector eyeposition;	// ideal eye position

			Vector illumposition;	// illumination center

			Vector hull_min;		// ideal movement hull size
			Vector hull_max;

			Vector view_bbmin;		// clipping bounding box
			Vector view_bbmax;

			int flags;

			int numbones; // bones
			int boneindex;

			int numbonecontrollers; // bone controllers
			int bonecontrollerindex;

			int numhitboxsets;
			int hitboxsetindex;

			// seemingly unused now, as animations are per sequence
			int numlocalanim; // animations/poses
			int localanimindex; // animation descriptions

			int numlocalseq; // sequences
			int	localseqindex;

			int activitylistversion; // initialization flag - have the sequences been indexed?

			// mstudiotexture_t
			// short rpak path
			// raw textures
			int materialtypesindex; // index into an array of char sized material type enums for each material used by the model
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
			int localNodeUnk; // used sparsely in r2, unused in apex, removed in v16 rmdl
			int localNodeDataOffset; // offset into an array of int sized offsets that read into the data for each nod

			int numikchains;
			int ikchainindex;

			// mesh panels for using rui on models, primarily for weapons
			int uiPanelCount;
			int uiPanelOffset;

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

			int /* mutable void* */ virtualModel;

			int bonetablebynameindex;

			// hw data lookup from rmdl
			int vgMeshCount; // number of meshes per lod
			int vgMeshOffset;

			int boneStateOffset;
			int boneStateCount;

			int unk_v12_1; // related to vg likely

			int hwDataSize;

			// weird hw data stuff
			// duplicates?
			// NAMES NEEDED
			short vgUnk; // same as padding in vg header
			short numVGLods; // same as lod count in vg header

			int lodMap; // lods in this group, each bit is a lod

			// sets of lods
			int groupHeaderOffset;
			int groupHeaderCount;

			int vgLODOffset;
			int vgLODCount;  // check this

			float defaultFadeDist;
			float gatherSize; // what. from r5r struct

			float flVertAnimFixedPointScale; // to be verified
			int surfacepropLookup; // saved in the file

			int unk_v12_2; // added in transition version

			// this is in most shipped models, probably part of their asset bakery.
			// doesn't actually need to be written pretty sure, only four chars when not present.
			// this is not completely true as some models simply have nothing, such as animation models.
			int sourceFilenameOffset;

			int numsrcbonetransform;
			int srcbonetransformindex;

			int	illumpositionattachmentindex;

			int linearboneindex;

			// used for adjusting weights in sequences, quick lookup into bones that have procbones, unsure what else uses this.
			int procBoneCount;
			int procBoneOffset; // in order array of procbones and their parent bone indice
			int linearProcBoneOffset; // byte per bone with indices into each bones procbone, 0xff if no procbone is present

			// always "" or "Titan"
			int unkStringOffset;

			// offsets into vertex buffer for component files, suzes are per file if course
			// unused besides phy starting in rmdl v9
			int vtxOffset; // VTX
			int vvdOffset; // VVD / IDSV
			int vvcOffset; // VVC / IDCV 
			int phyOffset; // VPHY / IVPS

			int vtxSize;
			int vvdSize;
			int vvcSize;
			int phySize; // still used in models using vg

			// mostly seen on '_animated' suffixed models
			// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
			int boneFollowerCount;
			int boneFollowerOffset;

			// BVH4 size (?)
			Vector bvhMin;
			Vector bvhMax; // seem to be the same as hull size

			int bvhOffset; // bvh4 tree

			short bvhUnk[2]; // collision detail for bvh (?)

			// new in apex vertex weight file for verts that have more than three weights
			// vvw is a 'fake' extension name, we do not know the proper name.
			int vvwOffset; // index will come last after other vertex files
			int vvwSize;
		};
	}

	namespace v130
	{
		struct mstudiomodel_t
		{
			char name[64];

			int unkindex2; // byte before string block

			// it looks like they write the entire name
			// then write over it with other values where needed
			// why.
			int type;

			float boundingradius;

			int nummeshes;
			int meshindex;

			// cache purposes
			int numvertices; // number of unique vertices/normals/texcoords
			int vertexindex; // vertex Vector
			int tangentsindex; // tangents Vector

			int numattachments;
			int attachmentindex;

			int colorindex; // vertex color
			// offset by colorindex number of bytes into vvc vertex colors
			int uv2index; // vertex second uv map
			// offset by uv2index number of bytes into vvc secondary uv map

			int unk; // same as uv2index, did they add something to vvc/0tVG?
		};

		struct studiohdr_t
		{
			int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
			int version; // Format version number, such as 48 (0x30,0x00,0x00,0x00)
			int checksum; // This has to be the same in the phy and vtx files to load!
			int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
			char name[64]; // The internal name of the model, padding with null bytes.
			// Typically "my_model.mdl" will have an internal name of "my_model"
			int length; // Data size of MDL file in bytes.

			Vector eyeposition;	// ideal eye position

			Vector illumposition;	// illumination center

			Vector hull_min;		// ideal movement hull size
			Vector hull_max;

			Vector view_bbmin;		// clipping bounding box
			Vector view_bbmax;

			int flags;

			int numbones; // bones
			int boneindex;

			int numbonecontrollers; // bone controllers
			int bonecontrollerindex;

			int numhitboxsets;
			int hitboxsetindex;

			// unused now
			int numlocalanim; // animations/poses
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

			int numunknodes;
			int nodedataindexindex;

			int numikchains;
			int ikchainindex;

			// this is rui meshes
			int uiPanelCount;
			int uiPanelOffset;

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

			uint32_t virtualModel;

			int bonetablebynameindex;

			// stuff moved from vg in v12.1
			int vgMeshCount; // total number of meshes, not including LODs
			int vgMeshOffset;

			int boneStateOffset;
			int boneStateCount;

			int unk_v54_v121; // related to vg likely

			int vgSize;

			short vgUnk; // same as padding in vg header
			short numVGLods; // same as lod count in vg header

			int vgNumUnknown; // same as unk1 in vg header

			int groupHeaderOffset;
			int groupHeaderCount;

			int vgLODOffset;
			int vgLODCount;

			float defaultFadeDist;

			float gathersize; // what. from r5r struct

			float flVertAnimFixedPointScale; // to be verified
			int surfacepropLookup; // saved in the file

			int unk_v54_v122; // added in transition version

			// asset bakery strings if it has any
			int sourceFilenameOffset;

			int numsrcbonetransform;
			int srcbonetransformindex;

			int	illumpositionattachmentindex;

			int linearboneindex;

			// unsure what this is for but it exists for jigglbones
			int numprocbonesunk;
			int procbonearrayindex;
			int procbonemaskindex;

			// always "" or "Titan"
			int unkstringindex;

			// the indexes are added to the offset in the rpak mdl_ header.
			// vphy isn't vphy, looks like a heavily modified vphy.
			// something different about these now
			int vtxOffset; // VTX
			int vvdOffset; // VVD / IDSV
			int vvcOffset; // VVC / IDCV 
			int phyOffset; // VPHY / IVPS

			int vtxSize;
			int vvdSize;
			int vvcSize;
			int phySize; // still used in models using vg

			// mostly seen on '_animated' suffixed models
			// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
			int numbonefollowers; // numBoneFollowers
			int bonefollowerindex;

			// BVH4 size (?)
			Vector mins;
			Vector maxs; // seem to be the same as hull size

			int bvhOffset; // bvh4 tree

			short unk4_v54[2]; // same as unk3_v54_v121, 2nd might be base for other offsets?

			int vvwOffset;
			int vvwSize;

			int unk1_v54_v13[3];
		};
	}

	namespace v140
	{
		struct mstudiomodel_t
		{
			char name[64];

			int unkindex2; // byte before string block

			// they write over these two when it's the default
			int type;

			float boundingradius;

			int nummeshes;

			// first is the same as nummeshes?
			int unk_v14;
			int unk1_v14;

			int meshindex;

			// most of these vtx, vvd, vvc, and vg indexes are depreciated after v14.1 (s14)

			// cache purposes
			int numvertices; // number of unique vertices/normals/texcoords
			int vertexindex; // vertex Vector
			int tangentsindex; // tangents Vector

			int numattachments;
			int attachmentindex;

			int colorindex; // vertex color
			int uv2index; // vertex second uv map
			int unk;
		};

		struct studiohdr_t
		{
			int id; // Model format ID, such as "IDST" (0x49 0x44 0x53 0x54)
			int version; // Format version number, such as 48 (0x30,0x00,0x00,0x00)
			int checksum; // This has to be the same in the phy and vtx files to load!
			int sznameindex; // This has been moved from studiohdr2 to the front of the main header.
			char name[64]; // The internal name of the model, padding with null bytes.
			// Typically "my_model.mdl" will have an internal name of "my_model"
			int length; // Data size of MDL file in bytes.

			Vector eyeposition;	// ideal eye position

			Vector illumposition;	// illumination center

			Vector hull_min;		// ideal movement hull size
			Vector hull_max;

			Vector view_bbmin;		// clipping bounding box
			Vector view_bbmax;

			int flags;

			int numbones; // bones
			int boneindex;

			int numbonecontrollers; // bone controllers
			int bonecontrollerindex;

			int numhitboxsets;
			int hitboxsetindex;

			// unused now
			int numlocalanim; // animations/poses
			int localanimindex; // animation descriptions

			int numlocalseq; // sequences
			int	localseqindex;

			int unk_v54_v14[2]; // added in v13 -> v14

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

			int numunknodes;
			int nodedataindexindex;

			int numikchains;
			int ikchainindex;

			// this is rui meshes
			int uiPanelCount;
			int uiPanelOffset;

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

			uint32_t virtualModel;

			int bonetablebynameindex;

			// stuff moved from vg in v12.1
			int vgMeshCount; // total number of meshes, not including LODs
			int vgMeshOffset;

			int boneStateOffset;
			int boneStateCount;

			int unk_v54_v121; // related to vg likely

			int vgSize;

			short vgUnk; // same as padding in vg header
			short numVGLods; // same as lod count in vg header

			int vgNumUnknown; // same as unk1 in vg header

			int groupHeaderOffset;
			int groupHeaderCount;

			int vgLODOffset;
			int vgLODCount;

			float defaultFadeDist;

			float gathersize; // what. from r5r struct

			float flVertAnimFixedPointScale; // to be verified
			int surfacepropLookup; // saved in the file

			int unk_v54_v122; // added in transition version

			// asset bakery strings if it has any
			int sourceFilenameOffset;

			int numsrcbonetransform;
			int srcbonetransformindex;

			int	illumpositionattachmentindex;

			int linearboneindex;

			// unsure what this is for but it exists for jigglbones
			int numprocbonesunk;
			int procbonearrayindex;
			int procbonemaskindex;

			// always "" or "Titan"
			int unkstringindex;

			// the indexes are added to the offset in the rpak mdl_ header.
			// vphy isn't vphy, looks like a heavily modified vphy.
			// something different about these now
			int vtxOffset; // VTX
			int vvdOffset; // VVD / IDSV
			int vvcOffset; // VVC / IDCV 
			int phyOffset; // VPHY / IVPS

			int vtxSize;
			int vvdSize;
			int vvcSize;
			int phySize; // still used in models using vg

			// mostly seen on '_animated' suffixed models
			// manually declared bone followers are no longer stored in kvs under 'bone_followers', they are now stored in an array of ints with the bone index.
			int numbonefollowers; // numBoneFollowers
			int bonefollowerindex;

			// BVH4 size (?)
			Vector mins;
			Vector maxs; // seem to be the same as hull size

			int bvhOffset; // bvh4 tree

			short unk4_v54[2]; // same as unk3_v54_v121, 2nd might be base for other offsets?

			int vvwOffset;
			int vvwSize;

			int unk1_v54_v13[3];
		};
	}
}

// for r5 materials
enum MaterialShaderType_t : unsigned __int8
{
	RGDU = 0x0,
	RGDP = 0x1,
	RGDC = 0x2,
	SKNU = 0x3,
	SKNP = 0x4,
	SKNC = 0x5,
	WLDU = 0x6,
	WLDC = 0x7,
	PTCU = 0x8,
	PTCS = 0x9,
};


//
// INTERNAL MODEL HANDLING & WRITING
//

#define STRING_FROM_IDX(base, idx) reinterpret_cast<const char*>((char*)base + idx)
#define PTR_FROM_IDX(type, base, idx) reinterpret_cast<type*>((char*)base + idx)

struct stringentry_t
{
	char* base;
	char* addr;
	int* ptr;
	const char* string;
	int dupindex;
};

struct s_modeldata_t
{
	//r5::v8::studiohdr_t* pHdr;
	void* pHdr;
	inline r5::v8::studiohdr_t* hdrV54() { return reinterpret_cast<r5::v8::studiohdr_t*>(pHdr); }
	inline r2::studiohdr_t* hdrV53() { return reinterpret_cast<r2::studiohdr_t*>(pHdr); }

	inline r5::v8::mstudioseqdesc_t* seqV7() { return reinterpret_cast<r5::v8::mstudioseqdesc_t*>(pHdr); }

	std::vector<stringentry_t> stringTable;
	char* pBase;
	char* pData;
};

inline s_modeldata_t g_model;

static void BeginStringTable()
{
	g_model.stringTable.clear();
	g_model.stringTable.emplace_back(stringentry_t{ NULL, NULL, NULL, "", -1 });
}

static void AddToStringTable(char* base, int* ptr, const char* string)
{
	if (!string)
		string = "";

	stringentry_t newString{};

	int i = 0;
	for(auto&it: g_model.stringTable)
	{
		if (!strcmp(string, it.string))
		{
			newString.base = (char*)base;
			newString.ptr = ptr;
			newString.string = string;
			newString.dupindex = i;
			g_model.stringTable.emplace_back(newString);
			return;
		}
		i++;
	}

	newString.base = (char*)base;
	newString.ptr = ptr;
	newString.string = string;
	newString.dupindex = -1;

	g_model.stringTable.emplace_back(newString);
}

static char* WriteStringTable(char* pData)
{
	auto& stringTable = g_model.stringTable;

	for (auto& it : stringTable)
	{
		// if first time the string is added to the table (unique or first version of duplicated strings)
		if (it.dupindex == -1)
		{
			it.addr = pData;

			if (it.ptr)
			{
				*it.ptr = pData - it.base;
				
				int length = strlen(it.string);
				strcpy_s(pData, length+1, it.string);

				pData += length;
			}

			*pData = '\0';

			pData++;
		}
		else
		{
			// if string is a duplicate entry
			// find the offset from the var's base ptr to the initial instance of the string
			*it.ptr = stringTable[it.dupindex].addr - it.base;
		}
	}

	return pData;
}

/* VERTEX HARDWARE DATA START */
class CVertexHardwareDataFile_V1
{
public:
	CVertexHardwareDataFile_V1() = default;
	//CVertexHardwareDataFile_V1(__int64 inLodCount) : lodCount(inLodCount) {};

	//void AddBoneStateFromDiskFile(vvd::mstudioboneweight_t* pVvdWeight, vvw::vertexBoneWeightsExtraFileHeader_t* pVVW);
	void SetupBoneStateFromDiskFile(vvd::vertexFileHeader_t* pVVD, vvw::vertexBoneWeightsExtraFileHeader_t* pVVW);

	void FillFromDiskFiles(r5::v8::studiohdr_t* pHdr, OptimizedModel::FileHeader_t* pVtx, vvd::vertexFileHeader_t* pVVD, vvc::vertexColorFileHeader_t* pVVC = nullptr, vvw::vertexBoneWeightsExtraFileHeader_t* pVVW = nullptr);
	
	void Write(const std::string& filePath);

private:
	//int numVertices;
	//int numMeshes;
	////int numLODs;
	//int vertCacheSize;
	//int boneMapIdx = 0; // replace with .size() - 1
	//short externalWeightIdx = 0;
	//int vtxVertOffset = 0; // this exists because there isn't a good way to get through lods.. what?
	//size_t vertexByteOffset = 0;

	//__int64 lodCount; // total number of lods in this vertex data, set on creation

	vg::rev1::VertexGroupHeader_t hdr;

	// remaped bones used for quick lookup
	std::vector<unsigned char> boneStates;
	std::map<unsigned char, unsigned char> boneMap;

	// mesh data
	std::vector<vg::rev1::ModelLODHeader_t> lods;
	std::vector<vg::rev1::MeshHeader_t> meshes;
	std::vector<OptimizedModel::StripHeader_t> strips;
	std::vector<unsigned short> indices;
	std::vector<vg::Vertex_t> vertices;
	std::vector<vvw::mstudioboneweightextra_t> extraBoneWeights;
	std::vector<vvd::mstudioboneweight_t> legacyBoneWeights; // we can't just use these so obviously a hold over from vvd

	size_t currentVertexBufferSize; // total size of all vertices
	__int64 defaultMeshFlags;
};

uint32_t PackNormalTangent_UINT32(const Vector& vec, const Vector4D& tangent);

void CreateVGFile(const std::string& filePath, r5::v8::studiohdr_t* pHdr, char* vtxBuf, char* vvdBuf, char* vvcBuf = nullptr, char* vvwBuf = nullptr);
/* VERTEX HARDWARE DATA end */

// for converting attachments between normal mdl versions
// used for: mdl v52/v53 conversions
static int ConvertAttachmentsToMDL(mstudioattachment_t* pOldAttachments, int numAttachments)
{
	int index = g_model.pData - g_model.pBase;

	printf("converting %i attachments...\n", numAttachments);

	for (int i = 0; i < numAttachments; ++i)
	{
		mstudioattachment_t* oldAttach = &pOldAttachments[i];

		mstudioattachment_t* attach = reinterpret_cast<mstudioattachment_t*>(g_model.pData) + i;

		AddToStringTable((char*)attach, &attach->sznameindex, STRING_FROM_IDX(oldAttach, oldAttach->sznameindex));
		attach->flags = oldAttach->flags;
		attach->localbone = oldAttach->localbone;
		memcpy(&attach->localmatrix, &oldAttach->localmatrix, sizeof(oldAttach->localmatrix));
	}
	g_model.pData += numAttachments * sizeof(mstudioattachment_t);

	return index;

	ALIGN4(g_model.pData);
}

// for converting mdl attachments to rmdl attachments
// used for: all v54 conversions
static int ConvertAttachmentTo54(mstudioattachment_t* pOldAttachments, int numAttachments)
{
	int index = g_model.pData - g_model.pBase;

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
	g_model.pData += numAttachments * sizeof(r5::v8::mstudioattachment_t);

	return index;

	ALIGN4(g_model.pData);
}

static int ConvertPoseParams(mstudioposeparamdesc_t* pOldPoseParams, int numPoseParams, bool isRig)
{
	int index = g_model.pData - g_model.pBase;

	//if (!isRig)
	//	return;

	printf("converting %i poseparams...\n", numPoseParams);

	for (int i = 0; i < numPoseParams; i++)
	{
		mstudioposeparamdesc_t* oldPose = &pOldPoseParams[i];
		mstudioposeparamdesc_t* newPose = reinterpret_cast<mstudioposeparamdesc_t*>(g_model.pData);

		AddToStringTable((char*)newPose, &newPose->sznameindex, STRING_FROM_IDX(oldPose, oldPose->sznameindex));

		newPose->flags = oldPose->flags;
		newPose->start = oldPose->start;
		newPose->end = oldPose->end;
		newPose->loop = oldPose->loop;

		g_model.pData += sizeof(mstudioposeparamdesc_t);
	}

	return index;

	ALIGN4(g_model.pData);
}

static int ConvertSrcBoneTransforms(mstudiosrcbonetransform_t* pOldBoneTransforms, int numSrcBoneTransforms)
{
	int index = g_model.pData - g_model.pBase;

	printf("converting %i bone transforms...\n", numSrcBoneTransforms);

	for (int i = 0; i < numSrcBoneTransforms; i++)
	{
		mstudiosrcbonetransform_t* oldTransform = &pOldBoneTransforms[i];

		mstudiosrcbonetransform_t* newTransform = reinterpret_cast<mstudiosrcbonetransform_t*>(g_model.pData);

		AddToStringTable((char*)newTransform, &newTransform->sznameindex, STRING_FROM_IDX(oldTransform, oldTransform->sznameindex));

		newTransform->pretransform = oldTransform->pretransform;
		newTransform->posttransform = oldTransform->posttransform;

		g_model.pData += sizeof(mstudiosrcbonetransform_t);
	}

	return index;

	ALIGN4(g_model.pData);
}

// mult by two for: flags and parrents, rot and pos.
constexpr int boneDataSize = ((sizeof(int) * 2) + (sizeof(Vector) * 2) + sizeof(Quaternion) + sizeof(matrix3x4_t));

// mdl -> rmdl
static void ConvertLinearBoneTableTo54(mstudiolinearbone_t* pOldLinearBone, char* pOldLinearBoneTable)
{
	printf("converting linear bone table...\n");

	g_model.hdrV54()->linearboneindex = g_model.pData - g_model.pBase;

	r5::v8::mstudiolinearbone_t* newLinearBone = reinterpret_cast<r5::v8::mstudiolinearbone_t*>(g_model.pData);
	g_model.pData += sizeof(r5::v8::mstudiolinearbone_t);

	newLinearBone->numbones = pOldLinearBone->numbones;
	newLinearBone->flagsindex = pOldLinearBone->flagsindex - 36;
	newLinearBone->parentindex = pOldLinearBone->parentindex - 36;
	newLinearBone->posindex = pOldLinearBone->posindex - 36;
	newLinearBone->quatindex = pOldLinearBone->quatindex - 36;
	newLinearBone->rotindex = pOldLinearBone->rotindex - 36;
	newLinearBone->posetoboneindex = pOldLinearBone->posetoboneindex - 36;

	// mult by two for: flags and parrents, rot and pos.
	int tableSize = boneDataSize * newLinearBone->numbones;

	memcpy(g_model.pData, pOldLinearBoneTable, tableSize);
	g_model.pData += tableSize;

	ALIGN4(g_model.pData);
}

// rmdl -> rmdl
static void CopyLinearBoneTableTo54(const r5::v8::mstudiolinearbone_t* const pOldLinearBone)
{
	printf("copying linear bone table...\n");

	g_model.hdrV54()->linearboneindex = g_model.pData - g_model.pBase;

	const int dataSize = sizeof(r5::v8::mstudiolinearbone_t) + (boneDataSize * pOldLinearBone->numbones);

	memcpy(g_model.pData, reinterpret_cast<const char* const>(pOldLinearBone), dataSize);
	g_model.pData += dataSize;

	ALIGN4(g_model.pData);
}

static void ConvertLinearBoneTableTo53(mstudiolinearbone_t* pOldLinearBone, char* pOldLinearBoneTable)
{
	printf("converting linear bone table...\n");

	g_model.hdrV53()->linearboneindex = g_model.pData - g_model.pBase;

	mstudiolinearbone_t* newLinearBone = reinterpret_cast<mstudiolinearbone_t*>(g_model.pData);
	g_model.pData += sizeof(mstudiolinearbone_t);

	newLinearBone->numbones = pOldLinearBone->numbones;
	newLinearBone->flagsindex = pOldLinearBone->flagsindex;
	newLinearBone->parentindex = pOldLinearBone->parentindex;
	newLinearBone->posindex = pOldLinearBone->posindex;
	newLinearBone->quatindex = pOldLinearBone->quatindex;
	newLinearBone->rotindex = pOldLinearBone->rotindex;
	newLinearBone->posetoboneindex = pOldLinearBone->posetoboneindex;

	// do it funky like this because posscale is not here in v53, and the indexes will still work
	newLinearBone->rotscaleindex = pOldLinearBone->posscaleindex;
	newLinearBone->qalignmentindex = pOldLinearBone->rotscaleindex;

	// mult by two for: flags and parrents, quat and qalignment.
	// mult by three for: pose, rot, and rotscale.
	int tableSize = ((sizeof(int) * 2) + (sizeof(Vector) * 2) + (sizeof(Quaternion) * 1) + sizeof(matrix3x4_t)) * newLinearBone->numbones;

	memcpy(g_model.pData, pOldLinearBoneTable, tableSize);
	g_model.pData += tableSize;

	memcpy(g_model.pData, pOldLinearBoneTable + tableSize + (sizeof(Vector) * newLinearBone->numbones), (sizeof(Vector) + sizeof(Quaternion)) * newLinearBone->numbones);
	g_model.pData += (sizeof(Vector) + sizeof(Quaternion)) * newLinearBone->numbones;

	ALIGN4(g_model.pData);
}

//==========
// RSEQ
//==========

//
// ConvertSequenceHdr
// Purpose: transfers data from the existing mstudioseqdesc_t struct to the new one (seasons 0-14)
//
static void ConvertSequenceHdr(r5::v8::mstudioseqdesc_t * out, r5::v8::mstudioseqdesc_t * oldDesc)
{
	out->baseptr = oldDesc->baseptr; // should alwalts be start of file

	out->flags = oldDesc->flags;
	out->actweight = oldDesc->actweight;
	out->numevents = oldDesc->numevents;

	out->bbmin = oldDesc->bbmin;
	out->bbmax = oldDesc->bbmax;

	out->numblends = oldDesc->numblends;

	out->groupsize[0] = oldDesc->groupsize[0];
	out->groupsize[1] = oldDesc->groupsize[1];
	out->paramindex[0] = oldDesc->paramindex[0];
	out->paramindex[1] = oldDesc->paramindex[1];
	out->paramstart[0] = oldDesc->paramstart[0];
	out->paramstart[1] = oldDesc->paramstart[1];
	out->paramend[0] = oldDesc->paramend[0];
	out->paramend[1] = oldDesc->paramend[1];
	out->paramparent = oldDesc->paramparent;

	out->fadeintime = oldDesc->fadeintime;
	out->fadeouttime = oldDesc->fadeouttime;

	out->localentrynode = oldDesc->localentrynode;
	out->localexitnode = oldDesc->localexitnode;
	out->nodeflags = oldDesc->nodeflags;

	out->entryphase = oldDesc->entryphase;
	out->exitphase = oldDesc->exitphase;

	out->lastframe = oldDesc->lastframe;

	out->nextseq = oldDesc->nextseq;
	out->pose = oldDesc->pose;

	out->numikrules = oldDesc->numikrules;
	out->numautolayers = oldDesc->numautolayers;
	out->numiklocks = oldDesc->numiklocks;

	//out->keyvaluesize = oldDesc.keyvaluesize; // honestly unsure how this functions overall and if it exists in respawn made sequences to begin with

	out->numactivitymodifiers = oldDesc->numactivitymodifiers;

	out->ikResetMask = oldDesc->ikResetMask;
	out->unk1 = oldDesc->unk1;

	out->unkCount = oldDesc->unkCount;
}

//
// ConvertSequenceActMods
// Purpose: transfers data from the existing mstudioactivitymodifier_t struct to the new one (seasons 0-14)
//
static int ConvertSequenceActMods(r1::mstudioactivitymodifier_t* pOldActMods, int numActMods)
{
	printf("converting %i activitymodifiers...\n", numActMods);

	int index = g_model.pData - g_model.pBase;

	for (int actModIdx = 0; actModIdx < numActMods; actModIdx++)
	{
		r1::mstudioactivitymodifier_t* oldActivityModifier = &pOldActMods[actModIdx];
		r1::mstudioactivitymodifier_t* newActivityModifier = reinterpret_cast<r1::mstudioactivitymodifier_t*>(g_model.pData);

		AddToStringTable((char*)newActivityModifier, &newActivityModifier->sznameindex, STRING_FROM_IDX(oldActivityModifier, oldActivityModifier->sznameindex));
		newActivityModifier->negate = oldActivityModifier->negate;

		g_model.pData += sizeof(r1::mstudioactivitymodifier_t);

		ALIGN4(g_model.pData);
	}

	return index;
}

//
// ConvertSequenceUnknown
// Purpose: transfer data from unkseqdata_t if used
//
static int ConvertSequenceUnknown(r5::unkseqdata_t* pOldUnknown, int numUnknown)
{
	printf("converting %i unknown data...\n", numUnknown);

	int index = g_model.pData - g_model.pBase;

	for (int unkIdx = 0; unkIdx < numUnknown; unkIdx++)
	{
		r5::unkseqdata_t* oldUnknown = &pOldUnknown[unkIdx];
		r5::unkseqdata_t* newUnknown = reinterpret_cast<r5::unkseqdata_t*>(g_model.pData);

		newUnknown->unkfloat = oldUnknown->unkfloat;
		newUnknown->unk = oldUnknown->unk;
		newUnknown->unkfloat1 = oldUnknown->unkfloat1;
		newUnknown->unkfloat2 = oldUnknown->unkfloat2;
		newUnknown->unkfloat3 = oldUnknown->unkfloat3;
		newUnknown->unkfloat4 = oldUnknown->unkfloat4;

		g_model.pData += sizeof(r5::unkseqdata_t);
	}

	return index;
}

//
// ConvertAnimation
// Purpose: copy animation from the old animation to the new animation
//
static int ConvertAnimation(char* pOldAnimIndex, r5::v8::mstudioanimdesc_t* pNewAnimDesc, int numBones)
{
	printf("converting animation...\n");

	int index = g_model.pData - (char*)pNewAnimDesc;

	if ((pNewAnimDesc->flags & STUDIO_ALLZEROS) || !(pNewAnimDesc->flags & STUDIO_ANIM_UNK))
		return index;

	int flagSize = ((4 * numBones + 7) / 8 + 1) & 0xFFFFFFFE;

	memcpy(g_model.pData, pOldAnimIndex, flagSize);
	g_model.pData += flagSize;

	ALIGN2(g_model.pData);

	int animationSize = 0;

	r5::mstudio_rle_anim_t* pOldRleAnim = (r5::mstudio_rle_anim_t*)&pOldAnimIndex[flagSize];

	for (int boneIdx = 0; boneIdx < numBones; boneIdx++)
	{
		if ((pOldAnimIndex[boneIdx / 2] >> (4 * (boneIdx % 2))) & 0x7)
		{
			animationSize += pOldRleAnim->size;
			pOldRleAnim = reinterpret_cast<r5::mstudio_rle_anim_t*>((char*)pOldRleAnim + pOldRleAnim->size);
		}
	}

	memcpy(g_model.pData, pOldAnimIndex + flagSize, animationSize);
	g_model.pData += animationSize;

	ALIGN4(g_model.pData);

	return index;
}

//
// ConvertAnimationSections
// Purpose: parses sections for 'ConvertAnimation', including pulling from an external file when needed
//
static int ConvertAnimationSections(char* pOldSeqExtBuf, r5::v121::mstudioanimdesc_t* pOldAnimDesc, r5::v8::mstudioanimdesc_t* pNewAnimDesc, int numBones)
{
	char* pSectionAnimStart = g_model.pData;

	int index = g_model.pData - (char*)pNewAnimDesc;

	int numSections = ceil(((float)pNewAnimDesc->numframes - 1) / (float)pNewAnimDesc->sectionframes) + 1;

	std::vector<r2::mstudioanimsections_t*>sectionIndexes;

	for (int sectionIdx = 0; sectionIdx < numSections; sectionIdx++)
	{
		r2::mstudioanimsections_t* sectionIndex = reinterpret_cast<r2::mstudioanimsections_t*>(g_model.pData);
		g_model.pData += sizeof(int);

		sectionIndexes.push_back(sectionIndex);
	}

	ALIGN16(g_model.pData);

	pNewAnimDesc->animindex = g_model.pData - (char*)pNewAnimDesc;

	for (auto sectionIndex : sectionIndexes)
	{
		int currentSectionIdx = ((char*)sectionIndex - pSectionAnimStart) / sizeof(r2::mstudioanimsections_t);

		r5::v121::mstudioanimsections_t* oldSectionIndex = PTR_FROM_IDX(r5::v121::mstudioanimsections_t, pOldAnimDesc, pOldAnimDesc->sectionindex + (sizeof(r5::v121::mstudioanimsections_t) * currentSectionIdx));

		ALIGN4(g_model.pData);

		if (oldSectionIndex->external && !pOldSeqExtBuf)
			Error("failed to find external file for sections when it was required!");

		if (oldSectionIndex->external)
		{
			sectionIndex->animindex = ConvertAnimation(PTR_FROM_IDX(char, pOldSeqExtBuf, oldSectionIndex->animindex), pNewAnimDesc, numBones);
		}
		else
		{
			sectionIndex->animindex = ConvertAnimation(PTR_FROM_IDX(char, pOldAnimDesc, oldSectionIndex->animindex), pNewAnimDesc, numBones);
		}
	}

	return index;
}

//
// ConvertAnimationIkRules
// Purpose: copy IkRules from the old animation to the new animation
//
static int ConvertAnimationIkRules(r5::v8::mstudioikrule_t* pOldIkRules, r5::v8::mstudioanimdesc_t* pNewAnimDesc)
{
	printf("converting %i ikrules...\n", pNewAnimDesc->numikrules);

	int index = g_model.pData - (char*)pNewAnimDesc;

	for (int ikRuleIdx = 0; ikRuleIdx < pNewAnimDesc->numikrules; ikRuleIdx++)
	{
		r5::v8::mstudioikrule_t* oldIkRule = &pOldIkRules[ikRuleIdx];
		r5::v8::mstudioikrule_t* newIkRule = reinterpret_cast<r5::v8::mstudioikrule_t*>(g_model.pData);

		newIkRule->index = oldIkRule->index;
		newIkRule->type = oldIkRule->type;
		newIkRule->chain = oldIkRule->chain;
		newIkRule->bone = oldIkRule->bone;

		newIkRule->slot = oldIkRule->slot;
		newIkRule->height = oldIkRule->height;
		newIkRule->radius = oldIkRule->radius;
		newIkRule->floor = oldIkRule->floor;
		newIkRule->pos = oldIkRule->pos;
		newIkRule->q = oldIkRule->q;

		// do this later
		//newIkRule->compressedikerror = oldIkRule->compressedikerror;
		newIkRule->compressedikerrorindex = oldIkRule->compressedikerrorindex;

		newIkRule->iStart = oldIkRule->iStart;
		//newIkRule->ikerrorindex = oldIkRule->ikerrorindex;

		newIkRule->start = oldIkRule->start;
		newIkRule->peak = oldIkRule->peak;
		newIkRule->tail = oldIkRule->tail;
		newIkRule->end = oldIkRule->end;

		newIkRule->contact = oldIkRule->contact;
		newIkRule->drop = oldIkRule->drop;
		newIkRule->top = oldIkRule->top;

		//newIkRule->szattachmentindex = oldIkRule->szattachmentindex;

		newIkRule->endHeight = oldIkRule->endHeight;

		g_model.pData += sizeof(r5::v8::mstudioikrule_t);
	}

	ALIGN4(g_model.pData);

	return index;
}

//
// ConvertAnimationFrameMovement
// Purpose: copy Frame Movement from the old animation to the new animation
//
static int ConvertAnimationFrameMovement(r5::v8::mstudioframemovement_t* pOldFrameMovement, r5::v8::mstudioanimdesc_t* pNewAnimDesc)
{
	printf("converting framemovement...\n");

	int index = g_model.pData - (char*)pNewAnimDesc;

	char* pEndPtr = (char*)pOldFrameMovement;
	r5::v8::mstudioframemovement_t* oldFrameMovement = pOldFrameMovement;
	r5::v8::mstudioframemovement_t* newFrameMovement = reinterpret_cast<r5::v8::mstudioframemovement_t*>(g_model.pData);

	int numSections = ceil(((float)pNewAnimDesc->numframes - 1) / (float)oldFrameMovement->sectionFrames);
	bool hasSectionData = false;

	// very bad
	// redo this now that I understand it
	pEndPtr = pEndPtr + *pOldFrameMovement->pSectionOffsets(numSections - 1);

	for (int sectionIdx = 3; sectionIdx > -1; sectionIdx--)
	{
		if (*(pEndPtr + (sizeof(short) * sectionIdx)))
		{
			pEndPtr = pEndPtr + *(pEndPtr + (sizeof(short) * sectionIdx));

			hasSectionData = true;

			break;
		}
	}

	if (hasSectionData)
	{
		while (*pEndPtr)
		{
			pEndPtr = pEndPtr + (*pEndPtr * sizeof(r5::mstudioanimvalue_t));
		}

		pEndPtr = pEndPtr + 1;
	}
	else
	{
		pEndPtr = pEndPtr + (4 * sizeof(short));
	}

	memcpy(g_model.pData, pOldFrameMovement, pEndPtr - (char*)pOldFrameMovement);
	g_model.pData += pEndPtr - (char*)pOldFrameMovement;

	ALIGN4(g_model.pData);

	return index;
}

//
// ConvertSequenceAnims
// Purpose: converts a rseq v7.1 animation to a rseq v7 animation
//
static void ConvertSequenceAnims(char* pOldSeq, char* pOldSeqExtBuf, char* pNewSeq, int* pOldAnimIndexes, int numAnims, int numBones)
{
	char* pAnimIndexStart = g_model.pData;

	std::vector<int*> animIndexes;

	printf("converting %i animations...\n", numAnims);

	for (int currentAnimIdx = 0; currentAnimIdx < numAnims; currentAnimIdx++)
	{
		int* animIndex = reinterpret_cast<int*>(g_model.pData);
		g_model.pData += sizeof(int);

		animIndexes.push_back(animIndex);
	}

	for (auto animIndex : animIndexes)
	{
		int currentAnimIdx = ((char*)animIndex - pAnimIndexStart) / sizeof(int);

		r5::v121::mstudioanimdesc_t* oldAnimDesc = PTR_FROM_IDX(r5::v121::mstudioanimdesc_t, pOldSeq, pOldAnimIndexes[currentAnimIdx]);
		r5::v8::mstudioanimdesc_t* newAnimDesc = reinterpret_cast<r5::v8::mstudioanimdesc_t*>(g_model.pData);

		*animIndex = (char*)newAnimDesc - (char*)pNewSeq;

		newAnimDesc->baseptr = oldAnimDesc->baseptr;

		AddToStringTable((char*)newAnimDesc, &newAnimDesc->sznameindex, STRING_FROM_IDX(oldAnimDesc, oldAnimDesc->sznameindex));

		newAnimDesc->fps = oldAnimDesc->fps;
		newAnimDesc->flags = oldAnimDesc->flags;
		newAnimDesc->numframes = oldAnimDesc->numframes;
		newAnimDesc->nummovements = oldAnimDesc->nummovements;
		newAnimDesc->numikrules = oldAnimDesc->numikrules;
		newAnimDesc->sectionframes = oldAnimDesc->sectionframes;

		g_model.pData += sizeof(r5::v8::mstudioanimdesc_t);


		if (newAnimDesc->sectionframes)
		{
			newAnimDesc->sectionindex = ConvertAnimationSections(pOldSeqExtBuf, oldAnimDesc, newAnimDesc, numBones);
		}
		else
		{
			ALIGN16(g_model.pData);
			newAnimDesc->animindex = ConvertAnimation(PTR_FROM_IDX(char, oldAnimDesc, oldAnimDesc->animindex), newAnimDesc, numBones);
		}

		// needs compressed ik error
		if (newAnimDesc->numikrules)
			newAnimDesc->ikruleindex = ConvertAnimationIkRules(PTR_FROM_IDX(r5::v8::mstudioikrule_t, oldAnimDesc, oldAnimDesc->ikruleindex), newAnimDesc);

		if (oldAnimDesc && (newAnimDesc->flags & STUDIO_FRAMEMOVEMENT))
			newAnimDesc->framemovementindex = ConvertAnimationFrameMovement(PTR_FROM_IDX(r5::v8::mstudioframemovement_t, oldAnimDesc, oldAnimDesc->framemovementindex), newAnimDesc);
	}
}