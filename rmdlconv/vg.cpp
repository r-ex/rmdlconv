#include "vg.h"
#include "rmem.h"
#include "structs.h"
#include "BinaryIO.h"
#include <stdio.h>
#include <cstddef>
#include <string>
#include <fstream>
#include <filesystem>
#include "utils.h"

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
	size_t lodBufSize = vghInput.lodCount * sizeof(VGLod);
	unsigned short lodSubmeshCount = 0;

	char* lodBuf = new char[lodBufSize];
	rmem lods(lodBuf);

	for (unsigned int i = 0; i < vghInput.lodCount; ++i)
	{
		size_t thisLodOffset = 0x18 + (i * sizeof(VGLodNew)) + vghInput.lodOffset;
		input.seek(thisLodOffset, rseekdir::beg);
		VGLodNew lodInput = input.read<VGLodNew>();

		VGLod lod{ lodSubmeshCount, lodInput.submeshCount, lodInput.distance};

		lods.write(lod);

		for (int j = 0; j < lodInput.submeshCount; ++j)
		{
			size_t thisSubmeshOffset = thisLodOffset + offsetof(VGLodNew, submeshOffset) + lodInput.submeshOffset + (j * sizeof(VGSubmeshNew));
			input.seek(thisSubmeshOffset, rseekdir::beg);

			VGSubmeshNew submesh = input.read<VGSubmeshNew>();
			vertexBufSize += submesh.vertexBufferSize;
			indexBufSize += submesh.indexPacked.Count * 2;
			extendedWeightsBufSize += submesh.extendedWeightsCount;
			externalWeightsBufSize += submesh.externalWeightsCount * 0x10;
			stripsBufSize += submesh.stripsCount * sizeof(VGStrip);
		}

		lodSubmeshCount += lodInput.submeshCount;
	}

	char* vertexBuf = new char[vertexBufSize];
	char* indexBuf = new char[indexBufSize];
	char* extendedWeightsBuf = new char[extendedWeightsBufSize];
	char* externalWeightsBuf = new char[externalWeightsBufSize];
	char* stripsBuf = new char[stripsBufSize];
	char* submeshBuf = new char[lodSubmeshCount * sizeof(VGSubmesh)];

	printf("allocated buffers:\n");
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

	rmem submeshes(submeshBuf);

	// populate buffers fr
	for (unsigned int i = 0; i < vghInput.lodCount; ++i)
	{
		size_t thisLodOffset = 0x18 + (i * sizeof(VGLodNew)) + vghInput.lodOffset;
		input.seek(thisLodOffset, rseekdir::beg);
		VGLodNew lodInput = input.read<VGLodNew>();

		for (int j = 0; j < lodInput.submeshCount; ++j)
		{
			size_t thisSubmeshOffset = thisLodOffset + offsetof(VGLodNew, submeshOffset) + lodInput.submeshOffset + (j * sizeof(VGSubmeshNew));
			input.seek(thisSubmeshOffset, rseekdir::beg);

			char* thisSubmeshPointer = reinterpret_cast<char*>(input.getPtr());

			VGSubmeshNew submeshInput = input.read<VGSubmeshNew>();

			VGSubmesh submesh{};

			submesh.flags = submeshInput.flags;
			submesh.vertexSize = (unsigned int)submeshInput.vertexSize;
			submesh.vertexCount = (unsigned int)submeshInput.vertexCount;
			submesh.indexCount = (unsigned int)submeshInput.indexPacked.Count;
			submesh.extendedWeightsSize = (unsigned int)submeshInput.extendedWeightsCount;
			submesh.externalWeightsCount = (unsigned int)submeshInput.externalWeightsCount;
			submesh.stripsCount = (unsigned int)submeshInput.stripsCount;

			submesh.vertexOffset = (unsigned int)vertexBufSize;
			submesh.indexOffset = (unsigned int)indexBufSize;
			submesh.extendedWeightsOffset = (unsigned int)extendedWeightsBufSize;
			submesh.externalWeightsOffset = (unsigned int)externalWeightsBufSize;
			submesh.stripsOffset = (unsigned int)stripsBufSize / sizeof(VGStrip);

			submeshes.write(submesh);
			
			void* vtxPtr = (thisSubmeshPointer + offsetof(VGSubmeshNew, vertexOffset) + submeshInput.vertexOffset);
			std::memcpy(vertexBuf + vertexBufSize, vtxPtr, submeshInput.vertexBufferSize);
			vertexBufSize += submeshInput.vertexBufferSize;

			void* indexPtr = (thisSubmeshPointer + offsetof(VGSubmeshNew, indexOffset) + submeshInput.indexOffset);
			std::memcpy(indexBuf + indexBufSize, indexPtr, submeshInput.indexPacked.Count * 2);
			indexBufSize += submeshInput.indexPacked.Count * 2;

			void* extendedWeightsPtr = (thisSubmeshPointer + offsetof(VGSubmeshNew, extendedWeightsOffset) + submeshInput.extendedWeightsOffset);
			std::memcpy(extendedWeightsBuf + extendedWeightsBufSize, extendedWeightsPtr, submeshInput.extendedWeightsCount);
			extendedWeightsBufSize += submeshInput.extendedWeightsCount;

			void* externalWeightsPtr = (thisSubmeshPointer + offsetof(VGSubmeshNew, externalWeightsOffset) + submeshInput.externalWeightsOffset);
			std::memcpy(externalWeightsBuf + externalWeightsBufSize, externalWeightsPtr, submeshInput.externalWeightsCount * 0x10);
			externalWeightsBufSize += submeshInput.externalWeightsCount * 0x10;

			void* stripsPtr = (thisSubmeshPointer + offsetof(VGSubmeshNew, stripsOffset) + submeshInput.stripsOffset);
			std::memcpy(stripsBuf + stripsBufSize, stripsPtr, submeshInput.stripsCount * sizeof(VGStrip));
			stripsBufSize += submeshInput.stripsCount * sizeof(VGStrip);
		}
	}

	std::string rmdlPath = ChangeExtension(filePath, "rmdl");

	char* boneRemapBuf = nullptr;
	unsigned int boneRemapCount = 0;

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
	}

	VGHeader vgh{};
	vgh.boneRemapCount = boneRemapCount;
	vgh.submeshCount = lodSubmeshCount;
	vgh.indexCount = indexBufSize / 2;
	vgh.vertexCount = vertexBufSize;
	vgh.extendedWeightsCount = extendedWeightsBufSize;
	vgh.lodCount = vghInput.lodCount;
	vgh.unknownCount = vgh.lodCount / vgh.submeshCount;
	vgh.externalWeightsCount = externalWeightsBufSize / 0x10;
	vgh.stripsCount = stripsBufSize / sizeof(VGStrip);

	BinaryIO out;

	out.open(outputPath, BinaryIOMode::Write);

	out.write(vgh);

	vgh.boneRemapOffset = out.tell();
	if(boneRemapCount)
		out.getWriter()->write(boneRemapBuf, boneRemapCount);

	vgh.submeshOffset = out.tell();
	out.getWriter()->write(submeshBuf, lodSubmeshCount * sizeof(VGSubmesh));

	vgh.indexOffset = out.tell();
	out.getWriter()->write(indexBuf, indexBufSize);

	vgh.vertexOffset = out.tell();
	out.getWriter()->write(vertexBuf, vertexBufSize);

	vgh.extendedWeightsOffset = out.tell();
	out.getWriter()->write(extendedWeightsBuf, extendedWeightsBufSize);

	// not sure what this data is supposed to do, but it doesn't seem to break anything if it's nulled
	char* unkbuf = new char[vgh.unknownCount * 0x30] {};
	vgh.unknownOffset = out.tell();
	out.getWriter()->write(unkbuf, vgh.unknownCount * 0x30);

	vgh.lodOffset = out.tell();
	out.getWriter()->write(lodBuf, lodBufSize);

	vgh.externalWeightsOffset = out.tell();
	out.getWriter()->write(externalWeightsBuf, externalWeightsBufSize);

	vgh.stripsOffset = out.tell();
	out.getWriter()->write(stripsBuf, stripsBufSize);

	vgh.dataSize = (unsigned int)out.tell();

	out.seek(0, std::ios::beg);

	out.write(vgh);

	out.close();
	printf("done!\n");
}
