#include "stdafx.h"
#include "versions.h"
#include "rmem.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "BinaryIO.h"

void ConvertVGData_12_1(char* buf, const std::string& filePath)
{
	std::filesystem::path path(filePath);

	std::string outputPath = ChangeExtension(filePath, "vg_conv");

	rmem input(buf);

	VGHeaderNew vghInput = input.read<VGHeaderNew>();

	size_t vertexBufSize = 0;
	size_t indexBufSize = 0;
	size_t extendedWeightsBufSize = 0;
	size_t externalWeightsBufSize = 0;
	size_t stripsBufSize = 0;
	size_t lodBufSize = vghInput.lodCount * sizeof(ModelLODHeader_VG_t);
	uint16_t lodSubmeshCount = 0;

	//char* lodBuf = new char[lodBufSize];
	std::unique_ptr<char[]> lodBuf(new char[lodBufSize]);
	rmem lods(lodBuf.get());

	for (unsigned int i = 0; i < vghInput.lodCount; ++i)
	{
		size_t thisLodOffset = 0x18 + (i * sizeof(VGLodNew)) + vghInput.lodOffset;
		input.seek(thisLodOffset, rseekdir::beg);
		VGLodNew lodInput = input.read<VGLodNew>();

		ModelLODHeader_VG_t lod{ lodSubmeshCount, lodInput.meshCount, lodInput.distance};

		lods.write(lod);

		for (int j = 0; j < lodInput.meshCount; ++j)
		{
			size_t thisSubmeshOffset = thisLodOffset + offsetof(VGLodNew, meshOffset) + lodInput.meshOffset + (j * sizeof(VGMeshNew));
			input.seek(thisSubmeshOffset, rseekdir::beg);

			VGMeshNew submesh = input.read<VGMeshNew>();
			vertexBufSize += submesh.vertexBufferSize;
			indexBufSize += submesh.indexPacked.Count * 2;
			extendedWeightsBufSize += submesh.extendedWeightsCount;
			externalWeightsBufSize += submesh.externalWeightsCount * 0x10;
			stripsBufSize += submesh.stripsCount * sizeof(StripHeader_t);
		}

		lodSubmeshCount += lodInput.meshCount;
	}

	std::unique_ptr<char[]> vertexBuf(new char[vertexBufSize]);
	std::unique_ptr<char[]> indexBuf(new char[indexBufSize]);
	std::unique_ptr<char[]> extendedWeightsBuf(new char[extendedWeightsBufSize]);
	std::unique_ptr<char[]> externalWeightsBuf(new char[externalWeightsBufSize]);
	std::unique_ptr<char[]> stripsBuf(new char[stripsBufSize]);
	std::unique_ptr<char[]> meshBuf(new char[lodSubmeshCount * sizeof(MeshHeader_VG_t)]);

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
	for (unsigned int i = 0; i < vghInput.lodCount; ++i)
	{
		size_t thisLodOffset = 0x18 + (i * sizeof(VGLodNew)) + vghInput.lodOffset;
		input.seek(thisLodOffset, rseekdir::beg);
		VGLodNew lodInput = input.read<VGLodNew>();

		for (int j = 0; j < lodInput.meshCount; ++j)
		{
			size_t thisSubmeshOffset = thisLodOffset + offsetof(VGLodNew, meshOffset) + lodInput.meshOffset + (j * sizeof(VGMeshNew));
			input.seek(thisSubmeshOffset, rseekdir::beg);

			char* thisSubmeshPointer = reinterpret_cast<char*>(input.getPtr());

			VGMeshNew submeshInput = input.read<VGMeshNew>();

			MeshHeader_VG_t submesh{};

			submesh.flags = submeshInput.flags;
			submesh.vertCacheSize = (unsigned int)submeshInput.vertexSize;
			submesh.numVerts = (unsigned int)submeshInput.vertexCount;
			submesh.numIndices = (unsigned int)submeshInput.indexPacked.Count;
			submesh.externalWeightSize = (unsigned int)submeshInput.extendedWeightsCount;
			submesh.legacyWeightOffset = (unsigned int)submeshInput.externalWeightsCount;
			submesh.numStrips = (unsigned int)submeshInput.stripsCount;

			submesh.vertOffset = (unsigned int)vertexBufSize;
			submesh.indexOffset = (unsigned int)indexBufSize;
			submesh.externalWeightOffset = (unsigned int)extendedWeightsBufSize;
			submesh.legacyWeightOffset = (unsigned int)externalWeightsBufSize;
			submesh.stripOffset = (unsigned int)stripsBufSize / sizeof(StripHeader_t);

			submeshes.write(submesh);
			
			void* vtxPtr = (thisSubmeshPointer + offsetof(VGMeshNew, vertexOffset) + submeshInput.vertexOffset);
			std::memcpy(vertexBuf.get() + vertexBufSize, vtxPtr, submeshInput.vertexBufferSize);
			vertexBufSize += submeshInput.vertexBufferSize;

			void* indexPtr = (thisSubmeshPointer + offsetof(VGMeshNew, indexOffset) + submeshInput.indexOffset);
			std::memcpy(indexBuf.get() + indexBufSize, indexPtr, submeshInput.indexPacked.Count * 2);
			indexBufSize += submeshInput.indexPacked.Count * 2;

			void* extendedWeightsPtr = (thisSubmeshPointer + offsetof(VGMeshNew, extendedWeightsOffset) + submeshInput.extendedWeightsOffset);
			std::memcpy(extendedWeightsBuf.get() + extendedWeightsBufSize, extendedWeightsPtr, submeshInput.extendedWeightsCount);
			extendedWeightsBufSize += submeshInput.extendedWeightsCount;

			void* externalWeightsPtr = (thisSubmeshPointer + offsetof(VGMeshNew, externalWeightsOffset) + submeshInput.externalWeightsOffset);
			std::memcpy(externalWeightsBuf.get() + externalWeightsBufSize, externalWeightsPtr, submeshInput.externalWeightsCount * 0x10);
			externalWeightsBufSize += submeshInput.externalWeightsCount * 0x10;

			void* stripsPtr = (thisSubmeshPointer + offsetof(VGMeshNew, stripsOffset) + submeshInput.stripsOffset);
			std::memcpy(stripsBuf.get() + stripsBufSize, stripsPtr, submeshInput.stripsCount * sizeof(StripHeader_t));
			stripsBufSize += submeshInput.stripsCount * sizeof(StripHeader_t);
		}
	}

	std::string rmdlPath = ChangeExtension(filePath, "rmdl");

	char* boneRemapBuf = nullptr;
	unsigned int boneRemapCount = 0;

	char* unkDataBuf = nullptr;

	if (std::filesystem::exists(rmdlPath) && GetFileSize(rmdlPath) > sizeof(studiohdr_121_t))
	{
		// grab bone remaps from rmdl
		std::ifstream ifs(rmdlPath, std::ios::in | std::ios::binary);

		studiohdr_121_t hdr;

		ifs.read((char*)&hdr, sizeof(hdr));

		if (hdr.numboneremaps > 0)
		{
			ifs.seekg(offsetof(studiohdr_121_t, boneremapindex) + hdr.boneremapindex, std::ios::beg);

			boneRemapCount = hdr.numboneremaps;

			boneRemapBuf = new char[boneRemapCount];
			ifs.read(boneRemapBuf, boneRemapCount);
		}

		if (hdr.numunk1_v121 > 0)
		{
			ifs.seekg(offsetof(studiohdr_121_t, unkindex1_v121) + hdr.unkindex1_v121, std::ios::beg);

			unkDataBuf = new char[hdr.numunk1_v121 * 0x30];
			ifs.read(unkDataBuf, hdr.numunk1_v121 * 0x30);
		}

		// close rmdl stream once we are done with it
		ifs.close();
	}

	VertexGroupHeader_t vgh{};
	vgh.numBoneStateChanges = boneRemapCount;
	vgh.numMeshes = lodSubmeshCount;
	vgh.numIndices = indexBufSize / 2;
	vgh.numVerts = vertexBufSize;
	vgh.externalWeightsSize = extendedWeightsBufSize;
	vgh.numLODs = vghInput.lodCount;
	vgh.numUnknown = vgh.numLODs / vgh.numMeshes;
	vgh.legacyWeightOffset = externalWeightsBufSize / 0x10;
	vgh.numStrips = stripsBufSize / sizeof(StripHeader_t);

	BinaryIO out;

	out.open(outputPath, BinaryIOMode::Write);

	out.write(vgh);

	vgh.boneStateChangeOffset = out.tell();
	if(boneRemapCount)
		out.getWriter()->write(boneRemapBuf, boneRemapCount);

	vgh.meshOffset = out.tell();
	out.getWriter()->write(meshBuf.get(), lodSubmeshCount * sizeof(MeshHeader_VG_t));

	vgh.indexOffset = out.tell();
	out.getWriter()->write(indexBuf.get(), indexBufSize);

	vgh.vertOffset = out.tell();
	out.getWriter()->write(vertexBuf.get(), vertexBufSize);

	vgh.externalWeightOffset = out.tell();
	out.getWriter()->write(extendedWeightsBuf.get(), extendedWeightsBufSize);

	// if this data hasn't been retrieved from .rmdl, write it as null bytes
	if(!unkDataBuf)
		unkDataBuf = new char[vgh.numUnknown * 0x30]{};

	vgh.unknownOffset = out.tell();
	out.getWriter()->write(unkDataBuf, vgh.numUnknown * 0x30);

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
