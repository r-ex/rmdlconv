// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include "stdafx.h"
#include "rmdl/studio_rmdl.h"
#include "mdl/studio.h"
#include "versions.h"

uint32_t PackNormalTangent_UINT32(const Vector3& normal, const Vector4& tangent)
{
	// normal 1 and normal 2
	int16_t normal1, normal2;
	uint8_t sign = 0, droppedComponent = 0;
	float s;

	Vector3 absNml = normal.Abs();

	if (absNml.x >= absNml.y && absNml.x >= absNml.z)
		droppedComponent = 0;
	else if (absNml.y >= absNml.x && absNml.y >= absNml.z)
		droppedComponent = 1;
	else
		droppedComponent = 2;

	switch (droppedComponent)
	{
	case 0:
		sign = absNml.x < 0 ? 1 : 0;
		s = absNml.x / (sign ? -255 : 255);
		normal2 = (int16_t)std::roundf((absNml.y / s) + 256.f);
		normal1 = (int16_t)std::roundf((absNml.z / s) + 256.f);
		break;
	case 1:
		sign = absNml.y < 0 ? 1 : 0;
		s = absNml.y / (sign ? -255 : 255);
		normal1 = (int16_t)std::roundf((absNml.x / s) + 256.f);
		normal2 = (int16_t)std::roundf((absNml.z / s) + 256.f);
		break;
	case 2:
		sign = absNml.z < 0 ? 1 : 0;
		s = absNml.z / (sign ? -255 : 255);
		normal2 = (int16_t)std::roundf((absNml.x / s) + 256.f);
		normal1 = (int16_t)std::roundf((absNml.y / s) + 256.f);
		break;
	default:
		break;
	}

	char binormSign = tangent.w < 1 ? 1 : 0;

	return (binormSign << 31)
		| (droppedComponent << 29)
		| (sign << 28)
		| (normal2 << 19)
		| (normal1 << 10);
}


Vector64 PackPos_UINT64(Vector3 vec, bool& fieldOverflow)
{
	Vector64 pos{};

	// fix if bad, didn't want to calculate twice
	int values[3]{ ((vec.x + 1024.0) / 0.0009765625), ((vec.y + 1024.0) / 0.0009765625), ((vec.z + 2048.0) / 0.0009765625) };

	if (values[0] > 0x1FFFFF || values[1] > 0x1FFFFF || values[2] > 0x3FFFFF)
	{
		fieldOverflow = true;
		return pos;
	}

	pos.x = values[0];
	pos.y = values[1];
	pos.z = values[2];

	/*pos.x = ((vec.x + 1024.0) / 0.0009765625);
	pos.y = ((vec.y + 1024.0) / 0.0009765625);
	pos.z = ((vec.z + 2048.0) / 0.0009765625);*/

	return pos;
}

void CreateVGFile(const std::string& filePath, r5::v8::studiohdr_t* pHdr, char* vtxBuf, char* vvdBuf, char* vvcBuf, char* vvwBuf)
{
	FileHeader_t* vtx = reinterpret_cast<FileHeader_t*>(vtxBuf);
	vertexFileHeader_t* vvd = reinterpret_cast<vertexFileHeader_t*>(vvdBuf);
	vertexColorFileHeader_t* vvc = reinterpret_cast<vertexColorFileHeader_t*>(vvcBuf);
	vertexBoneWeightsExtraFileHeader_t* vvw = reinterpret_cast<vertexBoneWeightsExtraFileHeader_t*>(vvwBuf);

	if ((!vvcBuf && (pHdr->flags & STUDIOHDR_FLAGS_USES_VERTEX_COLOR)) || (!vvcBuf && (pHdr->flags & STUDIOHDR_FLAGS_USES_UV2)))
		Error("model requires 'vvc' file but could not be found \n");

	if (!vvwBuf && (pHdr->flags & STUDIOHDR_FLAGS_COMPLEX_WEIGHTS))
		Error("model requires 'vvw' file but could not be found \n");

	int numVertices = 0;
	int numMeshes = 0;
	int numLODs = vtx->numLODs;
	int vertCacheSize = 0;

	std::vector<ModelLODHeader_VG_t> lods;
	std::vector<uint16_t> indices;
	std::vector<StripHeader_t> strips;
	std::vector<Vertex_VG_t> vertices;
	std::vector<mstudioboneweight_t> legacyWeights; // we can't just use these so obviously a hold over from vvd
	std::vector<mstudioexternalweight_t> externalWeights;
	std::vector<MeshHeader_VG_t> meshes;
	std::vector<uint8_t> BoneStates;
	std::map<uint8_t, uint8_t> boneMap;

	int boneMapIdx = 0;
	short externalWeightIdx = 0;
	int vtxVertOffset = 0;

	for (int lodIdx = 0; lodIdx < vtx->numLODs; ++lodIdx)
	{
		vtxVertOffset = 0;

		for (int i = 0; i < vtx->numBodyParts; ++i)
		{
			BodyPartHeader_t* bodyPart = vtx->bodyPart(i);
			mstudiobodyparts_t* rmdlBodyPart = pHdr->bodypart(i);

			for (int j = 0; j < bodyPart->numModels; ++j)
			{
				ModelHeader_t* model = bodyPart->model(j);

				ModelLODHeader_t* lod = model->lod(lodIdx);

				lods.push_back(ModelLODHeader_VG_t{ (short)numMeshes, (short)lod->numMeshes, lod->switchPoint });

				numMeshes += lod->numMeshes;

				r5::v8::mstudiomodel_t* rmdlModel = reinterpret_cast<r5::v8::mstudiomodel_t*>((char*)rmdlBodyPart + rmdlBodyPart->modelindex) + j;

				for (int l = 0; l < lod->numMeshes; ++l)
				{
					externalWeightIdx = 0; // reset index for new mesh

					MeshHeader_t* mesh = lod->mesh(l);
					r5::v8::mstudiomesh_t* rmdlMesh = rmdlModel->mesh(l);

					MeshHeader_VG_t newMesh{};

					newMesh.stripOffset = strips.size();
					newMesh.indexOffset = indices.size();
					newMesh.externalWeightOffset = externalWeights.size() * sizeof(mstudioexternalweight_t);
					newMesh.legacyWeightOffset = numVertices;
					newMesh.flags = 0x2005A40; // hard coded packed weights and pos

					bool vertexOverflow = false; // set to true if a position in one of the vertexes is too large to pack.

					newMesh.vertOffset = numVertices;

					for (int m = 0; m < mesh->numStripGroups; ++m)
					{
						StripGroupHeader_t* stripGroup = mesh->stripGroup(m);

						numVertices += stripGroup->numVerts;

						newMesh.numVerts += stripGroup->numVerts;
						newMesh.numStrips += 1;//stripGroup->numStrips;
						newMesh.numIndices += stripGroup->numIndices;
						newMesh.numLegacyWeights += stripGroup->numVerts;

						StripHeader_t newStripHeader{};

						newStripHeader.stripFlags |= 0x1; // trilist

						for (int stripIdx = 0; stripIdx < stripGroup->numStrips; stripIdx++)
						{
							StripHeader_t* strip = stripGroup->strip(stripIdx);

							newStripHeader.numIndices += strip->numIndices;
							newStripHeader.numVerts += strip->numVerts;
							newStripHeader.numBones += strip->numBones;

							for (int vertIdx = 0; vertIdx < strip->numVerts; ++vertIdx)
							{
								Vertex_t* vertVtx = stripGroup->vert(strip->vertexOffset + vertIdx);

								mstudiovertex_t* vertVvd = vvd->vertex(vtxVertOffset + vertVtx->origMeshVertID);
								Vector4 tangentVvd = vvd->GetTangent(vtxVertOffset + vertVtx->origMeshVertID);

								//printf("vertex %i \n", vtxVertOffset + vertVtx->origMeshVertID);

								Vertex_VG_t newVert{};
								newVert.m_NormalTangentPacked = PackNormalTangent_UINT32(vertVvd->m_vecNormal, tangentVvd);
								newVert.m_vecPosition = vertVvd->m_vecPosition;
								newVert.m_vecTexCoord = vertVvd->m_vecTexCoord;

								if (!vertexOverflow)
									newVert.m_vecPositionPacked = PackPos_UINT64(vertVvd->m_vecPosition, vertexOverflow);

								newVert.parentMeshIndex = l; // set the mesh index for later usage

								for (int n = 0; n < vertVvd->m_BoneWeights.numbones; n++)
								{
									if (n < 3 && !boneMap.count(vertVvd->m_BoneWeights.bone[n]))
									{
										boneMap.insert(std::pair<uint8_t, uint8_t>(vertVvd->m_BoneWeights.bone[n], boneMapIdx));
										printf("added bone %i at idx %i\n", boneMap.find(vertVvd->m_BoneWeights.bone[n])->first, boneMap.find(vertVvd->m_BoneWeights.bone[n])->second);
										boneMapIdx++;

										BoneStates.push_back(boneMap.find(vertVvd->m_BoneWeights.bone[n])->first);
									}
									else if (n >= 3) // don't try to read extended weights if below this amount, will cause issues
									{
										mstudioexternalweight_t* externalWeight = vvw->weight(vertVvd->m_BoneWeights.weights.packedweight.externalweightindex + (n - 3));

										// make sure that our weight bone id is valid, else the cast will mess with the value
										assert(externalWeight->bone <= 0xff && "External weight bone id is invalid (above limit of 255)");
										uint8_t boneId = (uint8_t)externalWeight->bone;

										if (!boneMap.count(boneId))
										{
											boneMap.insert(std::pair<uint8_t, uint8_t>(boneId, boneMapIdx));
											printf("added bone %i at idx %i\n", boneMap.find(boneId)->first, boneMap.find(boneId)->second);
											boneMapIdx++;

											BoneStates.push_back(boneMap.find(boneId)->first);
										}
									}
								}

								// set the actual weights
								if ((pHdr->flags & STUDIOHDR_FLAGS_COMPLEX_WEIGHTS) && pHdr->version == MdlVersion::APEXLEGENDS) // add version check just in case as we are reading off a header flag
								{
									// "complex" weights
									newVert.m_BoneWeightsPacked.weight[1] = externalWeightIdx; // set this before so we can add for the next one

									for (int n = 0; n < vertVvd->m_BoneWeights.numbones; n++)
									{
										// this causes the weird conversion artifact that normal vg has where verts with one weight have the same bone in two slots
										// so obviously respawn is doing something similar
										if (n == (vertVvd->m_BoneWeights.numbones - 1))
										{
											if (n > 0 && n < 3)
											{
												newVert.m_BoneWeightsPacked.bone[1] = boneMap.find(vertVvd->m_BoneWeights.bone[n])->second;
											}
											else if (n > 2)
											{
												mstudioexternalweight_t* vvwWeight = vvw->weight(vertVvd->m_BoneWeights.weights.packedweight.externalweightindex + (n - 3)); // subtract three to get the real idx

												newVert.m_BoneWeightsPacked.bone[1] = boneMap.find(vvwWeight->bone)->second; // change this to the bone remap
											}
										}
										else
										{
											if (n > 0 && n < 3) // we have to "build" this external weight so we do funny thing
											{
												mstudioexternalweight_t newExternalWeight{};

												newExternalWeight.weight = vertVvd->m_BoneWeights.weights.packedweight.weight[n];
												newExternalWeight.bone = boneMap.find(vertVvd->m_BoneWeights.bone[n])->second;

												externalWeights.push_back(newExternalWeight);

												externalWeightIdx++;
											}
											else if (n > 2)
											{
												mstudioexternalweight_t* vvwWeight = vvw->weight(vertVvd->m_BoneWeights.weights.packedweight.externalweightindex + (n - 3));

												mstudioexternalweight_t newExternalWeight{};

												newExternalWeight.weight = vvwWeight->weight;
												newExternalWeight.bone = boneMap.find(vvwWeight->bone)->second; // change this to the bone remap

												externalWeights.push_back(newExternalWeight);

												externalWeightIdx++;
											}
										}
									}

									// first slot is fixed, 2nd bone id will always be the last weight, with the weight value dropped
									newVert.m_BoneWeightsPacked.weight[0] = (vertVvd->m_BoneWeights.weights.packedweight.weight[0]);
									newVert.m_BoneWeightsPacked.bone[0] = boneMap.find(vertVvd->m_BoneWeights.bone[0])->second;

									newVert.m_BoneWeightsPacked.numbones = (vertVvd->m_BoneWeights.numbones - 1);
								}
								else
								{
									// "simple" weights
									// use vvd bone count instead of previously set as it should never be funky
									for (int n = 0; n < vertVvd->m_BoneWeights.numbones; n++)
									{
										// don't set weight for third because it gets dropped
										// third weight is gotten by subtracting the weights that were not dropped from 1.0f
										if (n < 2)
										{
											newVert.m_BoneWeightsPacked.weight[n] = (vertVvd->m_BoneWeights.weights.weight[n] * 32767.0); // "pack" float into short, weight will always be <= 1.0f
										}

										newVert.m_BoneWeightsPacked.bone[n] = boneMap.find(vertVvd->m_BoneWeights.bone[n])->second;
									}

									newVert.m_BoneWeightsPacked.numbones = (vertVvd->m_BoneWeights.numbones - 1);
								}

								mstudioboneweight_t newLegacyWeight{};

								newLegacyWeight = vertVvd->m_BoneWeights;

								legacyWeights.push_back(newLegacyWeight);

								// check header flags so we don't pull color or uv2 when we don't want it
								if (pHdr->flags & STUDIOHDR_FLAGS_USES_UV2)
								{
									Vector2* uvlayer = vvc->uv(i);

									newVert.m_vecTexCoord2.x = uvlayer->x;
									newVert.m_vecTexCoord2.y = uvlayer->y;
								}

								if (pHdr->flags & STUDIOHDR_FLAGS_USES_VERTEX_COLOR)
								{
									VertexColor_t* color = vvc->color(i);

									newVert.m_color.r = color->r;
									newVert.m_color.g = color->g;
									newVert.m_color.b = color->b;
									newVert.m_color.a = color->a;
								}

								vertices.push_back(newVert);
							}
						}

						strips.push_back(newStripHeader);

						int indicesOffset = indices.size();
						indices.resize(indices.size() + stripGroup->numIndices);
						std::memcpy(indices.data() + indicesOffset, stripGroup->indices(), stripGroup->numIndices * sizeof(uint16_t));
					}

					// set after we've gone through everything
					if (vertexOverflow)
					{
						newMesh.flags |= VERTEX_HAS_POSITION;
						newMesh.vertCacheSize += sizeof(Vector3);
					}
					else
					{
						newMesh.flags |= VERTEX_HAS_POSITION_PACKED;
						newMesh.vertCacheSize += sizeof(Vector64);
					}

					if (pHdr->flags & STUDIOHDR_FLAGS_USES_UV2)
					{
						newMesh.flags |= VERTEX_HAS_UV2;
						newMesh.vertCacheSize += sizeof(Vector2);
					}

					if (pHdr->flags & STUDIOHDR_FLAGS_USES_VERTEX_COLOR)
					{
						newMesh.flags |= VERTEX_HAS_COLOR;
						newMesh.vertCacheSize += sizeof(VertexColor_t);
					}

					newMesh.vertCacheSize += sizeof(mstudiopackedboneweight_t) + sizeof(uint32_t) + sizeof(Vector2); // packed weight size, packed normal size, uv size

					newMesh.vertOffset *= newMesh.vertCacheSize;

					// current vertex offset into vvd
					vtxVertOffset += rmdlMesh->vertexloddata.numLODVertexes[lodIdx];

					newMesh.externalWeightSize = externalWeightIdx * sizeof(mstudioexternalweight_t);

					if (newMesh.numVerts == 0)
					{
						newMesh.vertCacheSize = 0;
						newMesh.flags = 0x0;
					}

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
			if (tempLod.meshCount > 0)
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

	// cycle through lods and set correct mesh index
	for (int i = 0; i < newLods.size(); i++)
	{
		newLods.at(i).meshIndex = (meshes.size() / newLods.size()) * i;
	}

	lods = newLods;

	BinaryIO io;
	io.open(filePath, BinaryIOMode::Write);

	VertexGroupHeader_t header{};

	header.numMeshes = meshes.size();
	header.numIndices = indices.size();
	header.numVerts = vertices.size() * vertCacheSize;
	header.numLODs = lods.size();
	header.numStrips = strips.size();
	header.externalWeightsSize = externalWeights.size() * sizeof(mstudioexternalweight_t);
	header.numLegacyWeights = legacyWeights.size();
	header.numUnknown = header.numMeshes / header.numLODs;
	header.numBoneStateChanges = BoneStates.size();

	io.write(header);

	header.boneStateChangeOffset = io.tell();
	io.getWriter()->write((char*)BoneStates.data(), BoneStates.size() * sizeof(uint8_t));

	header.meshOffset = io.tell();
	io.getWriter()->write((char*)meshes.data(), meshes.size() * sizeof(MeshHeader_VG_t));

	header.indexOffset = io.tell();
	io.getWriter()->write((char*)indices.data(), indices.size() * sizeof(uint16_t));

	header.vertOffset = io.tell();
	// write vertcies based on flags so we can have vvc stuff, and potentially other flag based stuff in the future
	for (int i = 0; i < vertices.size(); i++)
	{
		Vertex_VG_t vertex = vertices.at(i);

		__int64 localFlags = meshes[vertex.parentMeshIndex].flags;

		if (localFlags & VERTEX_HAS_POSITION_PACKED)
			io.getWriter()->write((char*)&vertex.m_vecPositionPacked, sizeof(Vector64));
		else
			io.getWriter()->write((char*)&vertex.m_vecPosition, sizeof(Vector3));

		io.getWriter()->write((char*)&vertex.m_BoneWeightsPacked, sizeof(mstudiopackedboneweight_t));
		io.getWriter()->write((char*)&vertex.m_NormalTangentPacked, sizeof(uint32_t));

		if (localFlags & VERTEX_HAS_COLOR)
			io.getWriter()->write((char*)&vertex.m_color, sizeof(VertexColor_t));

		io.getWriter()->write((char*)&vertex.m_vecTexCoord, sizeof(Vector2));

		if (localFlags & VERTEX_HAS_UV2)
			io.getWriter()->write((char*)&vertex.m_vecTexCoord2, sizeof(Vector2));
	}

	header.externalWeightOffset = io.tell();
	io.getWriter()->write((char*)externalWeights.data(), externalWeights.size() * sizeof(mstudioexternalweight_t));

	char* unknownBuf = new char[header.numUnknown * 0x30] {};
	header.unknownOffset = io.tell();
	io.getWriter()->write((char*)unknownBuf, header.numUnknown * 0x30);

	header.lodOffset = io.tell();
	io.getWriter()->write((char*)lods.data(), lods.size() * sizeof(ModelLODHeader_VG_t));

	header.legacyWeightOffset = io.tell();
	io.getWriter()->write((char*)legacyWeights.data(), legacyWeights.size() * sizeof(mstudioboneweight_t));

	header.stripOffset = io.tell();
	io.getWriter()->write((char*)strips.data(), strips.size() * sizeof(StripHeader_t));

	header.dataSize = io.tell();

	io.seek(0);
	io.write(header);

	io.close();
}