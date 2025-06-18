#include <pch.h>
#include <studio/studio.h>
#include <studio/versions.h>
#include <studio/common.h>

//static void TestSurfaceProperties(const char* const assetPath, const char* const pOldBVHData/*, char* const newData*/)
//{
//	const r5::v8::mstudiocollmodel_t* const pOldCollModel = reinterpret_cast<const r5::v8::mstudiocollmodel_t*>(pOldBVHData);
//	const r5::v120::mstudiocollheader_t* pOldCollHeaders = reinterpret_cast<const r5::v120::mstudiocollheader_t*>(pOldBVHData + sizeof(r5::v8::mstudiocollmodel_t));
//
//	const r5::v8::dsurfaceproperty_t* const pSurfProps = reinterpret_cast<const r5::v8::dsurfaceproperty_t*>(&pOldBVHData[pOldCollModel->surfacePropsIndex]);
//	const int propCount = (pOldCollModel->contentMasksIndex - pOldCollModel->surfacePropsIndex) / sizeof(r5::v8::dsurfaceproperty_t);
//
//	int lastCount = INT32_MAX;
//
//	for (int i = 0; i < propCount; i++)
//	{
//		const r5::v8::dsurfaceproperty_t& const surfProp = pSurfProps[i];
//
//		if (surfProp.surfacePropId == lastCount)
//			printf("DEBUG: model %s with a higher surfacePropId than former property; %d > %d\n", assetPath, surfProp.surfacePropId, lastCount);
//
//		lastCount = surfProp.surfacePropId;
//	}
//}

//
// ConvertRMDL120To10
// Purpose: converts mdl data from rmdl v53 subversion 12.0 (Season 6) to rmdl v9 (Apex Legends Season 2/3)
//
void ConvertRMDL120To10(char* pMDL, const size_t fileSize, const std::string& pathIn, const std::string& pathOut)
{
	//std::string rawModelName = std::filesystem::path(pathIn).filename().u8string();

	//printf("Converting model '%s' from version 54 (subversion 12.1) to version 54 (subversion 10)...\n", rawModelName.c_str());

	TIME_SCOPE(__FUNCTION__);

	//std::string rmdlPath = ChangeExtension(pathOut, "rmdl_conv");
	std::ofstream out(pathOut, std::ios::out | std::ios::binary | std::ios::trunc);

	// allocate temp file buffer
	g_model.pBase = new char[fileSize] {};
	g_model.pData = g_model.pBase;

	memcpy(g_model.pBase, pMDL, fileSize);

	// Header is the same as v8
	r5::v8::studiohdr_t* const oldHeader = reinterpret_cast<r5::v8::studiohdr_t*>(pMDL);

	if (oldHeader->bvhOffset)
	{
		ConvertSurfaceProperties(&pMDL[oldHeader->bvhOffset], &g_model.pData[oldHeader->bvhOffset]);
		ConvertCollisionData_V120_HeadersOnly(&pMDL[oldHeader->bvhOffset], &g_model.pData[oldHeader->bvhOffset]);
	}

	out.write(g_model.pBase, oldHeader->length);
}
