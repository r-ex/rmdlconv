#include "stdafx.h"
#include "versions.h"
#include "rmem.h"
#include "BinaryIO.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"

#include <map>


void CreateVGFile_v8(const std::string& filePath)
{
	printf("creating VG file from v8 rmdl...\n");
	;
	std::string rmdlPath = ChangeExtension(filePath, "rmdl");
	std::string vtxPath = ChangeExtension(filePath, "vtx");
	std::string vvdPath = ChangeExtension(filePath, "vvd");
	std::string vvcPath = ChangeExtension(filePath, "vvc");

	if (!FILE_EXISTS(vtxPath) || !FILE_EXISTS(vvdPath))
		Error("failed to convert vtx,vvd to VG. vtx and vvd files are required but could not be found \n");

	if (FILE_EXISTS(vvcPath))
		printf("%s", "vvc was there \n");

	char* rmdlBuf = nullptr;
	{
		uintmax_t rmdlSize = GetFileSize(rmdlPath);

		rmdlBuf = new char[rmdlSize];

		std::ifstream ifs(rmdlPath, std::ios::in | std::ios::binary);

		ifs.read(rmdlBuf, rmdlSize);
	}

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

	vertexFileHeader_t* vvc = nullptr;

	if (FILE_EXISTS(vvcPath))
	{
		char* vvcBuf = nullptr;
		{
			uintmax_t vvcSize = GetFileSize(vvcPath);

			vvcBuf = new char[vvcSize];

			std::ifstream ifs(vvcPath, std::ios::in | std::ios::binary);

			ifs.read(vvcBuf, vvcSize);
		}

		vvc = reinterpret_cast<vertexFileHeader_t*>(vvcBuf);
	}

	studiohdr_v54_t* rmdl = reinterpret_cast<studiohdr_v54_t*>(rmdlBuf);
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
	//std::vector<mstudiomesh_t_v54*> rmdlMeshes;
	std::vector<uint8_t> boneremaps;
	std::map<uint8_t, uint8_t> boneMap;

	int boneMapIdx = 0;

	std::vector<int> badVertices;

	/*for (int i = 0; i < rmdl->numbodyparts; ++i)
	{
		mstudiobodyparts_t* rmdlBodyPart = rmdl->bodypart(i);

		for (int j = 0; j < rmdlBodyPart->nummodels; ++j)
		{
			mstudiomodel_t_v54* rmdlModel = rmdlBodyPart->model(j);

			for (int k = 0; k < rmdlModel->nummeshes; k++)
			{
				mstudiomesh_t_v54* rmdlMesh = rmdlModel->mesh(k);

				rmdlMeshes.push_back(rmdlMesh);
			}

		}
	}*/

	int vtxVertOffset = 0;

	for (int i = 0; i < vtx->numBodyParts; ++i)
	{
		BodyPartHeader_t* bodyPart = vtx->bodyPart(i);
		mstudiobodyparts_t* rmdlBodyPart = rmdl->bodypart(i);

		for (int j = 0; j < bodyPart->numModels; ++j)
		{
			ModelHeader_t* model = bodyPart->model(j);
			mstudiomodel_t_v54* rmdlModel = rmdlBodyPart->model(j);

			for (int k = 0; k < model->numLODs; ++k)
			{
				ModelLODHeader_t* lod = model->lod(k);

				lods.push_back(ModelLODHeader_VG_t{ (short)numMeshes, (short)lod->numMeshes, lod->switchPoint });

				numMeshes += lod->numMeshes;

				for (int l = 0; l < lod->numMeshes; ++l)
				{
					MeshHeader_t* mesh = lod->mesh(l);
					mstudiomesh_t_v54* rmdlMesh = rmdlModel->mesh(l);

					VGMesh newMesh{};

					newMesh.stripsOffset = strips.size();
					newMesh.indexOffset = indices.size();
					newMesh.vertexOffset = numVertices * sizeof(VGVertex_t);
					newMesh.externalWeightsOffset = numVertices;
					newMesh.flags = 0x2005A42;

					//if (vvc)
						//newMesh.flags += 0x200000000;

					//newMesh.flags = 0;

					//newMesh.flags += 0x1; // set unpacked pos flag
					//newMesh.flags += 0x5000; // set packed weight flag

					for (int m = 0; m < mesh->numStripGroups; ++m)
					{
						StripGroupHeader_t* stripGroup = mesh->stripGroup(m);
						//mstudiomesh_t_v54* rmdlMesh = rmdlMeshes.at(m);

						int prevTotalVerts = numVertices;
						numVertices += stripGroup->numVerts;

						newMesh.vertexCount += stripGroup->numVerts;
						newMesh.stripsCount += stripGroup->numStrips;
						newMesh.indexCount += stripGroup->numIndices;
						newMesh.externalWeightsCount += stripGroup->numVerts;

						int lastVertId = -1;
						for (int v = 0; v < stripGroup->numVerts; ++v)
						{
							Vertex_t* vertVtx = stripGroup->vert(v);

							mstudiovertex_t* vertVvd = vvd->vertex(vtxVertOffset + vertVtx->origMeshVertID);

							//printf("vertex %i \n", vtxVertOffset + vertVtx->origMeshVertID);

							VGVertex_t newVert{};
							newVert.m_packedNormal = PackNormal_UINT32(vertVvd->m_vecNormal);
							newVert.m_vecPositionPacked = PackPos_UINT64(vertVvd->m_vecPosition);
							//newVert.m_vecPosition = vertVvd->m_vecPosition;
							newVert.m_vecTexCoord = vertVvd->m_vecTexCoord;

							for (int n = 0; n < vertVvd->m_BoneWeights.numbones; n++)
							{
								if (!boneMap.count(vertVvd->m_BoneWeights.bone[0]))
								{
									boneMap.insert(std::pair<uint8_t, uint8_t>(vertVvd->m_BoneWeights.bone[0], boneMapIdx));
									printf("added bone %i at idx %i\n", boneMap.find(vertVvd->m_BoneWeights.bone[0])->first, boneMap.find(vertVvd->m_BoneWeights.bone[0])->second);
									boneMapIdx++;

									boneremaps.push_back(boneMap.find(vertVvd->m_BoneWeights.bone[0])->first);
								}
							}

							// default weights (pretty sure this is just typical packing float into short)
							newVert.m_packedWeights.BlendWeights[0] = (vertVvd->m_BoneWeights.weight[0] * 32767.5);
							newVert.m_packedWeights.BlendWeights[1] = (vertVvd->m_BoneWeights.weight[1] * 32767.5);

							newVert.m_packedWeights.BlendIds[0] = boneMap.find(vertVvd->m_BoneWeights.bone[0])->second;

							// set the bone(?) ids to 0 because we are using extended weights.
							for (int j = 1; j < 4; j++)
							{
								newVert.m_packedWeights.BlendIds[j] = 0;
							}

							vertices.push_back(newVert);

							/*if (vvc)
							{
								Vector2* uvlayer = vvc->uv(i);

								Vector2 newUV{};

								newUV.x = uvlayer->x;
								newUV.y = uvlayer->y;

								//vertices.push_back(newUV);
							}*/

							mstudioboneweight_t newExternalWeight{};

							newExternalWeight = vertVvd->m_BoneWeights;

							externalWeight.push_back(newExternalWeight);
						}

						int stripOffset = strips.size();
						strips.resize(strips.size() + stripGroup->numStrips);
						std::memcpy(strips.data() + stripOffset, stripGroup->strip(0), stripGroup->numStrips * sizeof(StripHeader_t));

						int indicesOffset = indices.size();
						indices.resize(indices.size() + stripGroup->numIndices);
						std::memcpy(indices.data() + indicesOffset, stripGroup->indices(), stripGroup->numIndices * sizeof(uint16_t));
					}

					vtxVertOffset += rmdlMesh->numvertices;

					//printf("mesh verts %i \n", rmdlMesh->numvertices);

					meshes.push_back(newMesh);
				}
			}
		}
	}

	std::vector<ModelLODHeader_VG_t> newLods;

	ModelLODHeader_VG_t tempLod{};
	for (int i = 0; i < lods.size(); ++i)
	{
		if (lods[i].switchPoint != tempLod.switchPoint)
		{
			if(tempLod.meshCount > 0)
				newLods.push_back(tempLod);

			tempLod.meshIndex = tempLod.meshCount;
			tempLod.switchPoint = lods[i].switchPoint;
			tempLod.meshCount = 0;
		}
		if (lods[i].meshCount == 0)
			continue;

		tempLod.meshCount += lods[i].meshCount;
	}

	if (tempLod.meshCount > 0)
		newLods.push_back(tempLod);

	lods = newLods;


	BinaryIO io;
	io.open(ChangeExtension(filePath, "vg"), BinaryIOMode::Write);

	VGHeader header{};

	header.meshCount = meshes.size();
	header.indexCount = indices.size();
	header.vertexCount = vertices.size() * sizeof(VGVertex_t);
	header.lodCount = lods.size();
	header.stripsCount = strips.size();
	header.externalWeightsCount = externalWeight.size();
	header.unknownCount = header.meshCount / header.lodCount;
	header.boneRemapCount = boneremaps.size();

	io.write(header);

	header.boneRemapOffset = io.tell();
	io.getWriter()->write((char*)boneremaps.data(), boneremaps.size() * sizeof(uint8_t));

	header.meshOffset = io.tell();
	io.getWriter()->write((char*)meshes.data(), meshes.size() * sizeof(VGMesh));
	
	header.indexOffset = io.tell();
	io.getWriter()->write((char*)indices.data(), indices.size() * sizeof(uint16_t));
	
	header.vertexOffset = io.tell();
	io.getWriter()->write((char*)vertices.data(), vertices.size() * sizeof(VGVertex_t));

	char* unknownBuf = new char[header.unknownCount * 0x30] {};
	header.unknownOffset = io.tell();
	io.getWriter()->write((char*)unknownBuf, header.unknownCount * 0x30);

	header.lodOffset = io.tell();
	io.getWriter()->write((char*)lods.data(), lods.size() * sizeof(ModelLODHeader_VG_t));

	header.externalWeightsOffset = io.tell();
	io.getWriter()->write((char*)externalWeight.data(), externalWeight.size() * sizeof(mstudioboneweight_t));

	header.stripsOffset = io.tell();
	io.getWriter()->write((char*)strips.data(), strips.size() * sizeof(StripHeader_t));

	header.dataSize = io.tell();

	io.seek(0);
	io.write(header);

	io.close();
}
