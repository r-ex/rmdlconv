// Copyright (c) 2022, rexx
// See LICENSE.txt for licensing information (GPL v3)

#include <pch.h>
#include <studio/studio.h>
#include <studio/versions.h>


// my beloved
void GetVertexesFromVVD(vvd::vertexFileHeader_t* pVVD, vvc::vertexColorFileHeader_t* pVVC, const int lodLevel, std::vector<const vvd::mstudiovertex_t*>& pVvdVertices, std::vector<const Vector4D*>& pVvdTangents, std::vector<const Color32*>& pVvcColors, std::vector<const Vector2D*>& pVvcUv2s)
{
	pVvdVertices.clear();
	pVvdTangents.clear();
	pVvcColors.clear();
	pVvcUv2s.clear();

	// rebuild vertex vector per lod just incase it has fixups
	if (pVVD->numFixups)
	{
		for (int j = 0; j < pVVD->numFixups; j++)
		{
			const vvd::vertexFileFixup_t* pVertexFixup = pVVD->GetFixupData(j);

			if (pVertexFixup->lod >= lodLevel)
			{
				for (int k = 0; k < pVertexFixup->numVertexes; k++)
				{
					//const vvd::mstudiovertex_t* const vvdVert = pVVD->GetVertexData(vertexFixup->sourceVertexID + k);
					//const Vector4D* const vvdTangent = pVVD->GetTangentData(vertexFixup->sourceVertexID + k);

					pVvdVertices.push_back(pVVD->GetVertexData(pVertexFixup->sourceVertexID + k));
					pVvdTangents.push_back(pVVD->GetTangentData(pVertexFixup->sourceVertexID + k));

					// vvc
					if (pVVC)
					{
						// doesn't matter which we pack as long as it has vvc, we will only used what's needed later
						//const Color32* const vvcColor = pVVC->GetColorData(vertexFixup->sourceVertexID + k);
						//const Vector2D* const vvcUV2 = pVVC->GetUVData(vertexFixup->sourceVertexID + k);

						pVvcColors.push_back(pVVC->GetColorData(pVertexFixup->sourceVertexID + k));
						pVvcUv2s.push_back(pVVC->GetUVData(pVertexFixup->sourceVertexID + k));
					}
				}
			}
		}
	}
	else
	{
		// using per lod vertex count may have issues (tbd)
		for (int j = 0; j < pVVD->numLODVertexes[lodLevel]; j++)
		{
			//const vvd::mstudiovertex_t* vvdVert = pVVD->GetVertexData(j);
			//const Vector4D* const vvdTangent = pVVD->GetTangentData(j);

			pVvdVertices.push_back(pVVD->GetVertexData(j));
			pVvdTangents.push_back(pVVD->GetTangentData(j));

			// vvc
			if (pVVC)
			{
				// doesn't matter which we pack as long as it has vvc, we will only used what's needed later
				//const Color32* vvcColor = pVVC->GetColorData(j);
				//const Vector2D* vvcUV2 = pVVC->GetUVData(j);

				pVvcColors.push_back(pVVC->GetColorData(j));
				pVvcUv2s.push_back(pVVC->GetUVData(j));
			}
		}
	}
}

// calculates the size of a single vertex from mesh flags
// must mask off 0x40 (flags & ~0x40)
__int64 __fastcall CalculateVertexSizeFromFlags(unsigned __int64 flags)
{
	unsigned int i = 0;
	char v3 = 0; // cl

	unsigned __int64 v1 = flags >> 24;
	for (i = ((-4 * flags) & 0xC)
		+ ((flags >> 2) & 4)
		+ ((flags >> 3) & 8)
		+ ((0x960174006301300ui64 >> (((flags >> 6) & 0x3Cu) + 2)) & 0x3C)
		+ ((0x2132100 >> (((flags >> 10) & 0x1C) + 2)) & 0xC); v1; i += (0x48A31A20 >> (3 * (v3 & 0xF))) & 0x1C)
	{
		v3 = v1;
		v1 >>= 4;
	}
	return i;
}

uint32_t PackNormalTangent_UINT32(const Vector& normal, const Vector4D& tangent)
{
	Vector absNml(normal);
	absNml.ABS();

	uint8_t idx1 = 0;

	// dropped component. seems to be the most significant axis?
	if (absNml.x >= absNml.y && absNml.x >= absNml.z)
		idx1 = 0;
	else if (absNml.y >= absNml.x && absNml.y >= absNml.z)
		idx1 = 1;
	else
		idx1 = 2;

	// index of the second and third components of the normal
	// see 140455D12 and LegionPlus rmdlstructs.h UnpackNormal
	int idx2 = (0x124u >> (2 * idx1 + 2)) & 3; // (2 * idx1 + 2) % 3
	int idx3 = (0x124u >> (2 * idx1 + 4)) & 3; // (2 * idx1 + 4) % 3

	// changed from (sign ? -255 : 255) because when -255 is used, the result in game appears very wrong
	float s = 255 / absNml[idx1];
	float val2 = (normal[idx2] * s) + 256.f;
	float val3 = (normal[idx3] * s) + 256.f;


	/*  --- cleaned up tangent unpacking from hlsl ---
		r2.y = -1 / (1 + nml.z);
	
		if (nml.z < -0.999899983)
		{
			r2.yzw = float3(0, -1, 0);
			r3.xyz = float3(-1, 0, 0);
		}
		else
		{
			r3.x = r2.y * (nml.x * nml.y);
	
			r3.z = r2.y * (nml.x * nml.x) + 1;
			r4.x = r2.y * (nml.y * nml.y) + 1;
	
			r2.yzw = float3(r3.z, r3.x, -nml.x);
			r3.xyz = float3(r3.x, r4.x, -nml.y);
		}

		r1.w = 0.00614192151 * (uint)r2.x; // 0.00614192151 * (_Value & 0x3FF)
		sincos(r1.w, r2.x, r4.x);
	
		// tangent
		r2.xyz = (r2.yzw * r4.xxx) + (r3.xyz * r2.xxx);
		// binorm sign
		r0.w = r0.w ? -1 : 1; 
	*/

	// seems to calculate a modified orthonormal basis?
	// might be based on this: https://github.com/NVIDIA/Q2RTX/blob/9d987e755063f76ea86e426043313c2ba564c3b7/src/refresh/vkpt/shader/utils.glsl#L240-L255
	// we need to calculate this here so we can get the components of the tangent
	Vector a, b;
	if (normal.z < -0.999899983)
	{
		a = Vector{ 0, -1, 0 };
		b = Vector{ -1, 0, 0 };
	}
	else
	{
		float v1 = -1 / (1 + normal.z);
		float v2 = v1 * (normal.x * normal.y);
		float v3 = v1 * (normal.x * normal.x) + 1;
		float v4 = v1 * (normal.y * normal.y) + 1;

		a = Vector{ v3, v2, -normal.x };
		b = Vector{ v2, v4, -normal.y };
	}

	// get angle that gives the x and y components of our tangent
	float angle = atan2f(Vector::Dot(tangent.AsVector(), b), Vector::Dot(tangent.AsVector(), a));
	
	// add 360deg to the angle so it is always positive (can't store negative angles in 10 bits)
	if (angle < 0)
		angle += DEG2RAD(360);

	angle /= 0.00614192151;

	bool sign = normal[idx1] < 0; // sign on the dropped normal component
	char binormSign = tangent.w < 1 ? 1 : 0; // sign on tangent w component

	return (binormSign << 31)
		| (idx1 << 29)
		| (sign << 28)
		| (static_cast<unsigned short>(val2) << 19)
		| (static_cast<unsigned short>(val3) << 10)
		| (static_cast<unsigned short>(angle) & 0x3FF);
}

void CVertexHardwareDataFile_V1::SetupBoneStateFromDiskFile(vvd::vertexFileHeader_t* pVVD, vvw::vertexBoneWeightsExtraFileHeader_t* pVVW)
{
	boneMap.clear();
	boneStates.clear();

	int hitCount = 0;
	for (int vertIdx = 0; vertIdx < pVVD->numLODVertexes[0]; vertIdx++)
	{
		const vvd::mstudiovertex_t* const pVvdVertex = pVVD->GetVertexData(vertIdx);
		const vvd::mstudioboneweight_t* const pVvdWeight = reinterpret_cast<const vvd::mstudioboneweight_t* const>(pVvdVertex); // we can do this because weights are at the front end

		for (int boneIdx = 0; boneIdx < pVvdWeight->numbones; boneIdx++)
		{
			if (boneIdx < 3 && !boneMap.count(pVvdWeight->bone[boneIdx]))
			{
				boneMap.insert(std::pair<unsigned char, unsigned char>(pVvdWeight->bone[boneIdx], boneStates.size()));
				boneStates.push_back(pVvdWeight->bone[boneIdx]);
			}
			else if (boneIdx >= 3) // don't try to read extended weights if below this amount, will cause issues
			{
				if (!pVVW)
					Error("Conversion somehow got to a place that requires VVW and none was given!!!\n");

				const vvw::mstudioboneweightextra_t* const pExtraWeight = pVVW->GetWeightData(pVvdWeight->weightextra.extraweightindex + (boneIdx - 3));

				// make sure that our weight bone id is valid, else the cast will mess with the value
				assert(pExtraWeight->bone < 0x100 && "Extra bone weight id is invalid (above limit of 255)\n");
				unsigned char boneId = static_cast<unsigned char>(pExtraWeight->bone);

				if (!boneMap.count(boneId))
				{
					boneMap.insert(std::pair<uint8_t, uint8_t>(boneId, boneStates.size()));
					boneStates.push_back(boneId);
				}
			}
		}
	}
}

void CVertexHardwareDataFile_V1::FillFromDiskFiles(r5::v8::studiohdr_t* pHdr, OptimizedModel::FileHeader_t* pVtx, vvd::vertexFileHeader_t* pVVD, vvc::vertexColorFileHeader_t* pVVC, vvw::vertexBoneWeightsExtraFileHeader_t* pVVW)
{
	bool isLargeModel = false;

	// cheat because I think s3 is not a fan of mixed matched vertexes? we will see. remove if true
	if (!isLargeModel && (pHdr->hull_min.x < -1023.f || pHdr->hull_max.x > 1023.f))
		isLargeModel = true;

	if (!isLargeModel && (pHdr->hull_min.y < -1023.f || pHdr->hull_max.y > 1023.f))
		isLargeModel = true;

	if (!isLargeModel && (pHdr->hull_min.z < -2047.f || pHdr->hull_max.z > 2047.f))
		isLargeModel = true;

	// add some default flags
	defaultMeshFlags |= VERTEX_HAS_UNK;
	defaultMeshFlags |= VERTEX_HAS_NORMAL_PACKED;
	defaultMeshFlags |= VERTEX_HAS_UNK2;
	defaultMeshFlags |= VERTEX_HAS_UV1;

	if (pHdr->flags & STUDIOHDR_FLAGS_USES_VERTEX_COLOR)
		defaultMeshFlags |= VERTEX_HAS_COLOR;

	if (pHdr->flags & STUDIOHDR_FLAGS_USES_UV2)
		defaultMeshFlags |= VERTEX_HAS_UV2;

	// no need to remap with so few, or so I have observed
	if (pHdr->numbones > 2)
		SetupBoneStateFromDiskFile(pVVD, pVVW);

	hdr.boneStateChangeCount = boneStates.size(); // set bonestate count
	hdr.lodCount = pVtx->numLODs;

	hdr.id = MODEL_VERTEX_HWDATA_FILE_ID;
	hdr.version = MODEL_VERTEX_HWDATA_FILE_VERSION;

	std::vector<const vvd::mstudiovertex_t*> pVvdVertices;
	std::vector<const Vector4D*> pVvdTangents;
	std::vector<const Color32*> pVvcColors;
	std::vector<const Vector2D*> pVvcUv2s;

	for (int lodIdx = 0; lodIdx < pVtx->numLODs; lodIdx++)
	{
		int localVertOffset = 0; // gotta be a better way to offset into lods?

		vg::rev1::ModelLODHeader_t newHwLOD {};
		newHwLOD.meshOffset = meshes.size(); // there should be the same amount of meshes per lod so no reason this can't work

		// takes slightly longer but required for proper lods
		GetVertexesFromVVD(pVVD, pVVC, lodIdx, pVvdVertices, pVvdTangents, pVvcColors, pVvcUv2s);

		for (int bodyPartIdx = 0; bodyPartIdx < pVtx->numBodyParts; bodyPartIdx++)
		{
			OptimizedModel::BodyPartHeader_t* pVtxBodyPart = pVtx->pBodyPart(bodyPartIdx);
			r5::v8::mstudiobodyparts_t* pMdlBodyPart = pHdr->pBodypart(bodyPartIdx);

			for (int modelIdx = 0; modelIdx < pVtxBodyPart->numModels; modelIdx++)
			{
				OptimizedModel::ModelHeader_t* pVtxModel = pVtxBodyPart->pModel(modelIdx);
				r5::v8::mstudiomodel_t* pMdlModel = pMdlBodyPart->pModel(modelIdx);

				OptimizedModel::ModelLODHeader_t* pVtxLod = pVtxModel->pLOD(lodIdx);

				//newHwLOD.meshCount += pVtxLod->numMeshes; // add meshes
				newHwLOD.switchPoint = pVtxLod->switchPoint; // we can get away with this due to how LODs are actually created through qc

				for (int meshIdx = 0; meshIdx < pVtxLod->numMeshes; meshIdx++)
				{
					OptimizedModel::MeshHeader_t* pVtxMesh = pVtxLod->pMesh(meshIdx);
					r5::v8::mstudiomesh_t* pMdlMesh = pMdlModel->pMesh(meshIdx); // remove if unneeded

					vg::rev1::MeshHeader_t newHwMesh {};

					newHwMesh.stripOffset = strips.size();
					newHwMesh.indexOffset = indices.size();
					newHwMesh.vertOffset = currentVertexBufferSize;
					newHwMesh.flags |= defaultMeshFlags;

					// are we gonna have weights?
					if (pHdr->numbones > 1)
					{
						newHwMesh.extraBoneWeightOffset = extraBoneWeights.size() * sizeof(vvw::mstudioboneweightextra_t);
						newHwMesh.legacyWeightOffset = legacyBoneWeights.size();

						newHwMesh.flags |= VERTEX_HAS_WEIGHT_BONES;
						newHwMesh.flags |= VERTEX_HAS_WEIGHT_VALUES_2;
					}

					newHwMesh.flags |= isLargeModel ? VERTEX_HAS_POSITION : VERTEX_HAS_POSITION_PACKED;

					// strip parsing
					OptimizedModel::StripHeader_t newHwStrip {};

					newHwStrip.flags |= 0x1; // trilist

					newHwMesh.stripCount = 1; // hardcoded because apex doesn't like reading more than one

					for (int stripGrpIdx = 0; stripGrpIdx < pVtxMesh->numStripGroups; stripGrpIdx++)
					{
						OptimizedModel::StripGroupHeader_t* pVtxStripGrp = pVtxMesh->pStripGroup(stripGrpIdx);

						newHwMesh.vertCount += pVtxStripGrp->numVerts;
						//newHwMesh.stripCount += 1;//pVtxStripGrp->numStrips;
						newHwMesh.indexCount += pVtxStripGrp->numIndices;
						//newHwMesh.legacyWeightCount += pVtxStripGrp->numVerts;

						// skip strip processing, if issues occur reimplement
						newHwStrip.numIndices += pVtxStripGrp->numIndices;
						newHwStrip.numVerts += pVtxStripGrp->numVerts;
						//newHwStrip.numBones += strip->numBones;
						newHwStrip.numBones += pVtxStripGrp->pStrip(0)->numBones; // bad but whatever, not correct for models with multiple strips

						for (int vertIdx = 0; vertIdx < pVtxStripGrp->numVerts; ++vertIdx)
						{
							OptimizedModel::Vertex_t* pVtxVert = pVtxStripGrp->pVertex(vertIdx);

							const vvd::mstudiovertex_t* const pVvdVert = pVvdVertices[localVertOffset + pVtxVert->origMeshVertID];
							//const Vector4* const pVvdTangent = pVVD->GetTangentData(localVertOffset + pVtxVert->origMeshVertID);
							vg::Vertex_t newHwVert {};

							newHwVert.parentMeshIndex = meshIdx; // set the mesh index for later usage
							newHwVert.m_NormalTangentPacked = PackNormalTangent_UINT32(pVvdVert->m_vecNormal, *pVvdTangents[localVertOffset + pVtxVert->origMeshVertID]);
							newHwVert.m_vecPosition = pVvdVert->m_vecPosition;
							newHwVert.m_vecPositionPacked = pVvdVert->m_vecPosition;
							newHwVert.m_vecTexCoord = pVvdVert->m_vecTexCoord;

							// check if valid
							if (pVVC)
							{
								newHwVert.m_color = *pVvcColors[localVertOffset + pVtxVert->origMeshVertID];
								newHwVert.m_vecTexCoord2 = *pVvcUv2s[localVertOffset + pVtxVert->origMeshVertID];
							}

							// skip our weights if we don't have flags for it
							if (!(newHwMesh.flags & VERTEX_HAS_WEIGHT_BONES))
							{
								vertices.push_back(newHwVert);
								continue;
							}

							// add bone state func here if needed

							// set the actual weights
							// redo at somepoint but for now I cba to do it
							if ((pHdr->flags & STUDIOHDR_FLAGS_USES_EXTRA_BONE_WEIGHTS) && pHdr->version == MdlVersion::APEXLEGENDS) // add version check just in case as we are reading off a header flag
							{
								// "complex" weights
								newHwVert.m_WeightsPacked.weight[1] = newHwMesh.extraBoneWeightSize; // set this before so we can add for the next one

								for (int n = 0; n < pVvdVert->m_BoneWeights.numbones; n++)
								{
									// this causes the weird conversion artifact that normal vg has where verts with one weight have the same bone in two slots
									// so obviously respawn is doing something similar
									if (n == (pVvdVert->m_BoneWeights.numbones - 1))
									{
										if (n > 0 && n < 3)
										{
											newHwVert.m_BonesPacked.bones[1] = boneMap.find(pVvdVert->m_BoneWeights.bone[n])->second;
										}
										else if (n > 2)
										{
											const vvw::mstudioboneweightextra_t* const pVvwWeight = pVVW->GetWeightData(pVvdVert->m_BoneWeights.weightextra.extraweightindex + (n - 3)); // subtract three to get the real idx

											newHwVert.m_BonesPacked.bones[1] = boneMap.find(pVvwWeight->bone)->second; // change this to the bone remap
										}
									}
									else
									{
										if (n > 0 && n < 3) // we have to "build" this external weight so we do funny thing
										{
											vvw::mstudioboneweightextra_t newHwExtraWeight{};

											newHwExtraWeight.weight = pVvdVert->m_BoneWeights.weightextra.weight[n];
											newHwExtraWeight.bone = boneMap.find(pVvdVert->m_BoneWeights.bone[n])->second;

											extraBoneWeights.push_back(newHwExtraWeight);

											newHwMesh.extraBoneWeightSize++;
										}
										else if (n > 2)
										{
											const vvw::mstudioboneweightextra_t* const pVvwWeight = pVVW->GetWeightData(pVvdVert->m_BoneWeights.weightextra.extraweightindex + (n - 3));

											vvw::mstudioboneweightextra_t newHwExtraWeight{};

											newHwExtraWeight.weight = pVvwWeight->weight;
											newHwExtraWeight.bone = boneMap.find(pVvwWeight->bone)->second; // change this to the bone remap

											extraBoneWeights.push_back(newHwExtraWeight);

											newHwMesh.extraBoneWeightSize++;
										}
									}
								}

								// first slot is fixed, 2nd bone id will always be the last weight, with the weight value dropped
								newHwVert.m_WeightsPacked.weight[0] = (pVvdVert->m_BoneWeights.weightextra.weight[0]);
								newHwVert.m_BonesPacked.bones[0] = boneMap.find(pVvdVert->m_BoneWeights.bone[0])->second;

								newHwVert.m_BonesPacked.numbones = (pVvdVert->m_BoneWeights.numbones - 1);
							}
							else
							{
								// "simple" weights
								// use vvd bone count instead of previously set as it should never be funky
								for (int n = 0; n < pVvdVert->m_BoneWeights.numbones; n++)
								{
									// don't set weight for third because it gets dropped
									// third weight is gotten by subtracting the weights that were not dropped from 1.0f
									if (n < 2)
									{
										newHwVert.m_WeightsPacked.weight[n] = (pVvdVert->m_BoneWeights.weight[n] * 32767.0); // "pack" float into short, weight will always be <= 1.0f
									}

									newHwVert.m_BonesPacked.bones[n] = boneMap.find(pVvdVert->m_BoneWeights.bone[n])->second;
								}

								newHwVert.m_BonesPacked.numbones = (pVvdVert->m_BoneWeights.numbones - 1);
							}

							legacyBoneWeights.push_back(pVvdVert->m_BoneWeights);
							newHwMesh.legacyWeightCount++;

							vertices.push_back(newHwVert);
						}

						// copy indices
						int indicesOffset = indices.size();
						indices.resize(indices.size() + pVtxStripGrp->numIndices);
						std::memcpy(indices.data() + indicesOffset, pVtxStripGrp->indices(), pVtxStripGrp->numIndices * sizeof(unsigned short));
					}

					strips.push_back(newHwStrip);

					// fix our extra weights if used
					if (newHwMesh.extraBoneWeightSize > 0)
					{
						newHwMesh.extraBoneWeightSize *= sizeof(vvw::mstudioboneweightextra_t);
						hdr.extraBoneWeightSize += newHwMesh.extraBoneWeightSize;
					}

					if (newHwMesh.vertCount == 0)
						newHwMesh.flags = 0x0;

					newHwMesh.vertCacheSize = CalculateVertexSizeFromFlags(newHwMesh.flags & ~VERTEX_HAS_UNK);
					currentVertexBufferSize += (newHwMesh.vertCacheSize * newHwMesh.vertCount);

					meshes.push_back(newHwMesh);

					// current vertex offset into vvd
					localVertOffset += pMdlMesh->vertexloddata.numLODVertexes[lodIdx];

					//hdr.vertCount += newHwMesh.vertCount;
					hdr.indexCount += newHwMesh.indexCount;
					hdr.stripCount += newHwMesh.stripCount;
					hdr.legacyWeightCount += newHwMesh.legacyWeightCount;
					hdr.extraBoneWeightSize += newHwMesh.extraBoneWeightSize;

					hdr.meshCount++;
					newHwLOD.meshCount++;
				}
			}
		}

		lods.push_back(newHwLOD);
	}
}

void CVertexHardwareDataFile_V1::Write(const std::string& filePath)
{
	BinaryIO io;
	io.open(filePath, BinaryIOMode::Write);

	hdr.unknownCount = hdr.meshCount / hdr.lodCount;


	io.write(hdr);

	hdr.boneStateChangeOffset = io.tell();
	io.getWriter()->write((char*)boneStates.data(), boneStates.size() * sizeof(unsigned char));

	hdr.meshOffset = io.tell();
	io.getWriter()->write((char*)meshes.data(), meshes.size() * sizeof(vg::rev1::MeshHeader_t));

	hdr.indexOffset = io.tell();
	io.getWriter()->write((char*)indices.data(), indices.size() * sizeof(unsigned short));

	hdr.vertOffset = io.tell();
	// write vertices based on flags so we can have vvc stuff, and potentially other flag based stuff in the future
	for (int i = 0; i < vertices.size(); i++)
	{
		vg::Vertex_t vertex = vertices.at(i);

		__int64 localFlags = meshes[vertex.parentMeshIndex].flags;

		if (localFlags & VERTEX_HAS_POSITION_PACKED)
			io.getWriter()->write((char*)&vertex.m_vecPositionPacked, sizeof(Vector64));
		else
			io.getWriter()->write((char*)&vertex.m_vecPosition, sizeof(Vector));

		if (localFlags & VERTEX_HAS_WEIGHT_VALUES_2)
			io.getWriter()->write((char*)&vertex.m_WeightsPacked, sizeof(vg::mstudiopackedweights_t));

		if (localFlags & VERTEX_HAS_WEIGHT_BONES)
			io.getWriter()->write((char*)&vertex.m_BonesPacked, sizeof(vg::mstudiopackedbones_t));

		io.getWriter()->write((char*)&vertex.m_NormalTangentPacked, sizeof(uint32_t));

		if (localFlags & VERTEX_HAS_COLOR)
			io.getWriter()->write((char*)&vertex.m_color, sizeof(Color32));

		io.getWriter()->write((char*)&vertex.m_vecTexCoord, sizeof(Vector2D));

		if (localFlags & VERTEX_HAS_UV2)
			io.getWriter()->write((char*)&vertex.m_vecTexCoord2, sizeof(Vector2D));
	}

	hdr.vertBufferSize = io.tell() - hdr.vertOffset;

	hdr.extraBoneWeightOffset = io.tell();
	io.getWriter()->write((char*)extraBoneWeights.data(), extraBoneWeights.size() * sizeof(vvw::mstudioboneweightextra_t));

	char* unknownBuf = new char[hdr.unknownCount * 0x30] {};
	hdr.unknownOffset = io.tell();
	io.getWriter()->write((char*)unknownBuf, hdr.unknownCount * 0x30);

	hdr.lodOffset = io.tell();
	io.getWriter()->write((char*)lods.data(), lods.size() * sizeof(vg::rev1::ModelLODHeader_t));

	hdr.legacyWeightOffset = io.tell();
	io.getWriter()->write((char*)legacyBoneWeights.data(), legacyBoneWeights.size() * sizeof(vvd::mstudioboneweight_t));

	hdr.stripOffset = io.tell();
	io.getWriter()->write((char*)strips.data(), strips.size() * sizeof(OptimizedModel::StripHeader_t));

	hdr.dataSize = io.tell();

	io.seek(0);
	io.write(hdr);

	io.close();
}

void CreateVGFile(const std::string& filePath, r5::v8::studiohdr_t* pHdr, char* vtxBuf, char* vvdBuf, char* vvcBuf, char* vvwBuf)
{
	OptimizedModel::FileHeader_t* pVTX = reinterpret_cast<OptimizedModel::FileHeader_t*>(vtxBuf);
	vvd::vertexFileHeader_t* pVVD = reinterpret_cast<vvd::vertexFileHeader_t*>(vvdBuf);
	vvc::vertexColorFileHeader_t* pVVC = reinterpret_cast<vvc::vertexColorFileHeader_t*>(vvcBuf);
	vvw::vertexBoneWeightsExtraFileHeader_t* pVVW = reinterpret_cast<vvw::vertexBoneWeightsExtraFileHeader_t*>(vvwBuf);

	if ((!vvcBuf && (pHdr->flags & STUDIOHDR_FLAGS_USES_VERTEX_COLOR)) || (!vvcBuf && (pHdr->flags & STUDIOHDR_FLAGS_USES_UV2)))
		Error("model requires 'vvc' file but could not be found \n");

	if (!vvwBuf && (pHdr->flags & STUDIOHDR_FLAGS_USES_EXTRA_BONE_WEIGHTS))
		Error("model requires 'vvw' file but could not be found \n");

	CVertexHardwareDataFile_V1* newVertexHardwareFile = new CVertexHardwareDataFile_V1();

	newVertexHardwareFile->FillFromDiskFiles(pHdr, pVTX, pVVD, pVVC, pVVW);
	newVertexHardwareFile->Write(filePath);
}