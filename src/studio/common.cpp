#include <pch.h>
#include <studio/studio.h>
#include <studio/versions.h>

//-----------------------------------------------------------------------------
// Collision data
//-----------------------------------------------------------------------------

void ConvertSurfaceProperties(const char* const pOldBVHData, char* const pNewBVHData)
{
	const r5::v8::mstudiocollmodel_t* const pOldCollModel = reinterpret_cast<const r5::v8::mstudiocollmodel_t*>(pOldBVHData);
	const r5::v8::mstudiocollmodel_t* const pNewCollModel = reinterpret_cast<const r5::v8::mstudiocollmodel_t*>(pNewBVHData);

	const r5::v120::mstudiocollheader_t* const pOldCollHeaders = reinterpret_cast<const r5::v120::mstudiocollheader_t*>(pOldBVHData + sizeof(r5::v8::mstudiocollmodel_t));

	// Only the first header appears to index into the new surface prop data with
	// surfacePropDataIndex, subsequent ones have the same value as vertIndex.
	// The reason for this in still unknown.
	const r5::v120::mstudiocollheader_t& const oldHeader = pOldCollHeaders[0];
	const r5::v120::dsurfacepropertydata_t* const oldSurfPropDatas = reinterpret_cast<const r5::v120::dsurfacepropertydata_t*>(&pOldBVHData[oldHeader.surfacePropDataIndex]);

	const r5::v8::dsurfaceproperty_t* const pOldSurfProps = reinterpret_cast<const r5::v8::dsurfaceproperty_t*>(&pOldBVHData[pOldCollModel->surfacePropsIndex]);
	r5::v8::dsurfaceproperty_t* const pNewSurfProps = reinterpret_cast<r5::v8::dsurfaceproperty_t*>(&pNewBVHData[pNewCollModel->surfacePropsIndex]);

	// Newer models have an array of surface properties, so the size is actually
	// surfacePropArrayCount * surfacePropCount, however V10 models can only store
	// 1 array of them so we just take the first one. The new system also stores
	// 2 surface properties per entry, from here we just take the first one.
	for (int i = 0; i < oldHeader.surfacePropCount; i++)
	{
		const static bool useSecondProp = false;

		// On the V12 and higher models, the surfacePropId field gets used to index into the
		// new dsurfacepropertydata_t array. On V10, this is the actual surfacePropId. V11 is
		// unknown but that model version does exist in some S5 or S6 build.
		const r5::v8::dsurfaceproperty_t& oldSurfProp = pOldSurfProps[i];
		const r5::v120::dsurfacepropertydata_t& const oldSurfPropData = oldSurfPropDatas[useSecondProp + oldHeader.surfacePropArrayCount * oldSurfProp.surfacePropId];

		r5::v8::dsurfaceproperty_t& newSurfProp = pNewSurfProps[i];
		newSurfProp.surfacePropId = oldSurfPropData.surfacePropId1;
	}
}

static void CopyCollisionBuffers(r5::v8::mstudiocollmodel_t* const newCollModel, const r5::v8::mstudiocollmodel_t* const oldCollModel, 
	const r5::v120::mstudiocollheader_t* const oldCollHeaders)
{
	const char* const oldBase = (char*)oldCollModel;
	const char* const newBase = (char*)newCollModel;

	// copy content masks, surface props, and surface names from source file to destination file
	const int surfacePropsSize = oldCollModel->contentMasksIndex - oldCollModel->surfacePropsIndex;
	const int contentMasksSize = oldCollModel->surfaceNamesIndex - oldCollModel->contentMasksIndex;
	const int surfaceNamesSize = oldCollHeaders[0].surfacePropDataIndex - oldCollModel->surfaceNamesIndex;

	newCollModel->surfacePropsIndex = g_model.pData - newBase;
	memcpy(g_model.pData, oldBase + oldCollModel->surfacePropsIndex, surfacePropsSize);
	g_model.pData += surfacePropsSize;

	newCollModel->contentMasksIndex = g_model.pData - newBase;
	memcpy(g_model.pData, oldBase + oldCollModel->contentMasksIndex, contentMasksSize);
	g_model.pData += contentMasksSize;

	newCollModel->surfaceNamesIndex = g_model.pData - newBase;
	memcpy(g_model.pData, oldBase + oldCollModel->surfaceNamesIndex, surfaceNamesSize);
	g_model.pData += surfaceNamesSize;
}

void ConvertCollisionData_V120(const r5::v121::studiohdr_t* const oldStudioHdr, const char* const pOldBVHData)
{
	g_model.hdrV54()->bvhOffset = g_model.pData - g_model.pBase;

	const r5::v8::mstudiocollmodel_t* const pOldCollModel = reinterpret_cast<const r5::v8::mstudiocollmodel_t*>(pOldBVHData);
	r5::v8::mstudiocollmodel_t* const pNewCollModel = reinterpret_cast<r5::v8::mstudiocollmodel_t*>(g_model.pData);

	const int headerCount = pOldCollModel->headerCount;
	// there's only one non-index variable in this struct, so no point in using memcpy
	pNewCollModel->headerCount = headerCount;

	g_model.pData += sizeof(r5::v8::mstudiocollmodel_t);

	const r5::v120::mstudiocollheader_t* const pOldCollHeaders = reinterpret_cast<const r5::v120::mstudiocollheader_t*>(pOldBVHData + sizeof(r5::v8::mstudiocollmodel_t));
	r5::v8::mstudiocollheader_t* const pNewCollHeaders = reinterpret_cast<r5::v8::mstudiocollheader_t*>(g_model.pData);

	g_model.pData += headerCount * sizeof(r5::v8::mstudiocollheader_t);

	// this cant be used with the later hack of just copying over all data and shifting offsets
	// copy surface props, content masks, and surface names from one buffer to the other
	CopyCollisionBuffers(pNewCollModel, pOldCollModel, pOldCollHeaders);

	// Fixup the surface properties, these have changed as of V120. Must happen after we copied
	// the collision buffers.
	ConvertSurfaceProperties(pOldBVHData, reinterpret_cast<char*>(pNewCollModel));

	// convert each coll header and its data
	for (int i = 0; i < headerCount; ++i)
	{
		const r5::v120::mstudiocollheader_t* const oldHeader = &pOldCollHeaders[i];
		r5::v8::mstudiocollheader_t* const newHeader = &pNewCollHeaders[i];

		newHeader->unk = oldHeader->unk;
		memcpy_s(newHeader->origin, sizeof(newHeader->origin), oldHeader->origin, sizeof(oldHeader->origin));

		newHeader->scale = oldHeader->scale; // decode scale

		// --- copy coll verts ---
		const __int64 vertSize = oldHeader->bvhLeafIndex - oldHeader->vertIndex;
		const void* const vertData = reinterpret_cast<const char*>(pOldCollModel) + oldHeader->vertIndex;

		ALIGN64(g_model.pData);
		newHeader->vertIndex = g_model.pData - reinterpret_cast<char*>(pNewCollModel);
		memcpy_s(g_model.pData, vertSize, vertData, vertSize);
		g_model.pData += vertSize;

		// --- copy coll leaf data ---
		__int64 leafSize;

		// [amos]: small change here, previously we calculated the size of
		// the bvh leaf data by subtracting its index from the node index.
		// However, the way this is structured in the original file is that
		// leafs always come before the vertices of the next collision
		// model unless we are the last collision model, then the next hunk
		// of data will be the bvh node of the first collision model.
		if (i != headerCount - 1)
			leafSize = pOldCollHeaders[i + 1].vertIndex - oldHeader->bvhLeafIndex;
		else
			leafSize = pOldCollHeaders[0].bvhNodeIndex - oldHeader->bvhLeafIndex;

		const void* const leafData = reinterpret_cast<const char*>(pOldCollModel) + oldHeader->bvhLeafIndex;

		ALIGN64(g_model.pData);
		newHeader->bvhLeafIndex = g_model.pData - reinterpret_cast<char*>(pNewCollModel);
		memcpy_s(g_model.pData, leafSize, leafData, leafSize);
		g_model.pData += leafSize;
	}

	// We need to do a second pass here, as vertices and leafs are written
	// contiguously after another for all collision models. All nodes for
	// each collision model are written contiguously after another after 
	// all the vertices and leafs.
	for (int i = 0; i < headerCount; ++i)
	{
		const r5::v120::mstudiocollheader_t* const oldHeader = &pOldCollHeaders[i];
		r5::v8::mstudiocollheader_t* const newHeader = &pNewCollHeaders[i];

		// --- copy coll node data ---
		__int64 nodeSize;

		if (i != headerCount - 1) // if not the last header
			nodeSize = pOldCollHeaders[i + 1].bvhNodeIndex - oldHeader->bvhNodeIndex;
		else
			nodeSize = offsetof(r5::v140::studiohdr_t, vgLODOffset) + oldStudioHdr->vgLODOffset - (oldStudioHdr->bvhOffset + oldHeader->bvhNodeIndex);

		const void* nodeData = reinterpret_cast<const char*>(pOldCollModel) + oldHeader->bvhNodeIndex;
		ALIGN64(g_model.pData);
		newHeader->bvhNodeIndex = g_model.pData - reinterpret_cast<char*>(pNewCollModel);
		memcpy_s(g_model.pData, nodeSize, nodeData, nodeSize);
		g_model.pData += nodeSize;
	}
}

// Only removes the new fields from the new headers and shifts the moved fields back
// to their original offsets.
void ConvertCollisionData_V120_HeadersOnly(const char* const pOldBVHData, char* const newData)
{
	const r5::v8::mstudiocollmodel_t* const pOldCollModel = reinterpret_cast<const r5::v8::mstudiocollmodel_t*>(pOldBVHData);
	const int headerCount = pOldCollModel->headerCount;

	r5::v8::mstudiocollheader_t* const pNewCollHeaders = reinterpret_cast<r5::v8::mstudiocollheader_t*>(newData + sizeof(r5::v8::mstudiocollmodel_t));
	const r5::v120::mstudiocollheader_t* const pOldCollHeaders = reinterpret_cast<const r5::v120::mstudiocollheader_t*>(pOldBVHData + sizeof(r5::v8::mstudiocollmodel_t));

	// Empty new headers first
	memset(pNewCollHeaders, 0, headerCount * sizeof(r5::v120::mstudiocollheader_t));

	// convert each coll header
	for (int i = 0; i < headerCount; ++i)
	{
		const r5::v120::mstudiocollheader_t* const oldHeader = &pOldCollHeaders[i];
		r5::v8::mstudiocollheader_t* const newHeader = &pNewCollHeaders[i];

		newHeader->unk = oldHeader->unk;
		newHeader->bvhNodeIndex = oldHeader->bvhNodeIndex;
		newHeader->vertIndex = oldHeader->vertIndex;
		newHeader->bvhLeafIndex = oldHeader->bvhLeafIndex;

		newHeader->origin[0] = oldHeader->origin[0];
		newHeader->origin[1] = oldHeader->origin[1];
		newHeader->origin[2] = oldHeader->origin[2];

		newHeader->scale = oldHeader->scale;
	}
}

//-----------------------------------------------------------------------------
// Animation data
//-----------------------------------------------------------------------------

void CopyAnimRefData(const char* const pOldAnimRefData, char* const pNewAnimRefData, const int numlocalseq)
{
	for (int i = 0; i < numlocalseq; i++)
	{
		const r5::v8::mstudioseqdesc_t* oldSeqdesc = reinterpret_cast<const r5::v8::mstudioseqdesc_t*>(pOldAnimRefData);
		r5::v8::mstudioseqdesc_t* newSeqDesc = reinterpret_cast<r5::v8::mstudioseqdesc_t*>(pNewAnimRefData);

		// For now we just copy it raw, we update the indices during the conversion
		*newSeqDesc = *oldSeqdesc;

		AddToStringTable((char*)newSeqDesc, &newSeqDesc->szlabelindex, &pOldAnimRefData[oldSeqdesc->szlabelindex]);
		AddToStringTable((char*)newSeqDesc, &newSeqDesc->szactivitynameindex, &pOldAnimRefData[oldSeqdesc->szactivitynameindex]);

		oldSeqdesc++;
		newSeqDesc++;
	}
}
