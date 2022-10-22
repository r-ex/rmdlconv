#include "stdafx.h"
#include "versions.h"
#include "rmem.h"
#include "BinaryIO.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"


void CreateVGFile_v8(const std::string& filePath)
{
	printf("creating VG file from v8 rmdl...\n");

	std::string vtxPath = ChangeExtension(filePath, "vtx");
	std::string vvdPath = ChangeExtension(filePath, "vvd");

	if (!FILE_EXISTS(vtxPath) || !FILE_EXISTS(vvdPath))
		Error("failed to convert vtx,vvd to VG. vtx and vvd files are required but could not be found");

	char* vtxBuf = nullptr;
	{
		uintmax_t vtxSize = GetFileSize(vtxPath);

		vtxBuf = new char[vtxSize];

		std::ifstream ifs(vtxPath, std::ios::in | std::ios::binary);

		ifs.read(vtxBuf, vtxSize);
	}

	char* vvdBuf = nullptr;
	{
		uintmax_t vvdSize = GetFileSize(vvdPath);

		vvdBuf = new char[vvdSize];

		std::ifstream ifs(vvdPath, std::ios::in | std::ios::binary);

		ifs.read(vvdBuf, vvdSize);
	}

	FileHeader_t* vtx = reinterpret_cast<FileHeader_t*>(vtxBuf);
	vertexFileHeader_t* vvd = reinterpret_cast<vertexFileHeader_t*>(vvdBuf);

	int numVertices = 0;
	int numMeshes = 0;
	int numLODs = vtx->numLODs;

	std::vector<ModelLODHeader_VG_t> lods;
	std::vector<uint16_t> indices;
	std::vector<StripHeader_t> strips;
	std::vector<VGVertex_t> vertices;
	std::vector<mstudioboneweight_t> externalWeight;
	std::vector<VGMesh> meshes;

	for (int i = 0; i < vtx->numBodyParts; ++i)
	{
		BodyPartHeader_t* bodyPart = vtx->bodyPart(i);

		for (int j = 0; j < bodyPart->numModels; ++j)
		{
			ModelHeader_t* model = bodyPart->model(j);

			for (int k = 0; k < model->numLODs; ++k)
			{
				ModelLODHeader_t* lod = model->lod(k);

				lods.push_back(ModelLODHeader_VG_t{ (short)numMeshes, (short)lod->numMeshes, lod->switchPoint });

				numMeshes += lod->numMeshes;

				for (int l = 0; l < lod->numMeshes; ++l)
				{
					MeshHeader_t* mesh = lod->mesh(l);

					VGMesh newMesh{};

					newMesh.stripsOffset = strips.size();
					newMesh.indexOffset = indices.size();
					newMesh.vertexOffset = numVertices;
					newMesh.externalWeightsOffset = numVertices;
					newMesh.flags = 0x2005A41;

					//newMesh.flags = 0;

					//newMesh.flags += 0x1; // set unpacked pos flag
					//newMesh.flags += 0x5000; // set packed weight flag

					for (int m = 0; m < mesh->numStripGroups; ++m)
					{
						StripGroupHeader_t* stripGroup = mesh->stripGroup(m);

						numVertices += stripGroup->numVerts;

						newMesh.vertexCount += stripGroup->numVerts;
						newMesh.stripsCount += stripGroup->numStrips;
						newMesh.indexCount += stripGroup->numIndices;
						newMesh.externalWeightsCount += stripGroup->numVerts;

						strips.resize(strips.size() + stripGroup->numStrips);
						std::memcpy(strips.data(), stripGroup->strip(0), stripGroup->numStrips * sizeof(StripHeader_t));

						indices.resize(indices.size() + stripGroup->numIndices);
						std::memcpy(indices.data(), stripGroup->indices(), stripGroup->numIndices * sizeof(uint16_t));
					}

					meshes.push_back(newMesh);
				}
			}
		}
	}

	for (int i = 0; i < vvd->numLODVertexes[0]; ++i)
	{
		mstudiovertex_t* vert = vvd->vertex(i);

		VGVertex_t newVert{};
		newVert.m_packedNormal = PackNormal_UINT32(vert->m_vecNormal);
		newVert.m_vecPosition = vert->m_vecPosition;
		newVert.m_vecTexCoord = vert->m_vecTexCoord;

		// default weights (pretty sure this is just typical packing float into short)
		newVert.m_packedWeights.BlendWeights[0] = (1 * 32767.5);
		newVert.m_packedWeights.BlendWeights[1] = 0;

		// set the bone(?) ids to 0 because we are using extended weights.
		for (int j = 0; j < 4; j++)
		{
			newVert.m_packedWeights.BlendIds[j] = 0;
		}

		vertices.push_back(newVert);
	}

	for (int i = 0; i < vvd->numLODVertexes[0]; ++i)
	{
		mstudiovertex_t* vert = vvd->vertex(i);

		mstudioboneweight_t newExternalWeight{};

		newExternalWeight = vert->m_BoneWeights;

		externalWeight.push_back(newExternalWeight);
	}

	BinaryIO io;
	io.open(ChangeExtension(filePath, "vg"), BinaryIOMode::Write);

	VGHeader header{};

	header.meshCount = meshes.size();
	header.indexCount = indices.size();
	header.vertexCount = vertices.size() * sizeof(VGVertex_t);
	header.lodCount = lods.size();
	header.stripsCount = strips.size();
	header.externalWeightsCount = externalWeight.size();

	io.write(header);

	header.meshOffset = io.tell();
	io.getWriter()->write((char*)meshes.data(), meshes.size() * sizeof(VGMesh));
	
	header.indexOffset = io.tell();
	io.getWriter()->write((char*)indices.data(), indices.size() * sizeof(uint16_t));
	
	header.vertexOffset = io.tell();
	io.getWriter()->write((char*)vertices.data(), vertices.size() * sizeof(VGVertex_t));

	header.lodOffset = io.tell();
	io.getWriter()->write((char*)lods.data(), lods.size() * sizeof(ModelLODHeader_VG_t));

	header.externalWeightsOffset = io.tell();
	io.getWriter()->write((char*)externalWeight.data(), externalWeight.size() * sizeof(mstudioboneweight_t));

	header.stripsOffset = io.tell();
	io.getWriter()->write((char*)strips.data(), strips.size() * sizeof(StripHeader_t));

	io.seek(0);
	io.write(header);

	io.close();
}
