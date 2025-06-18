#pragma once

extern void ConvertSurfaceProperties(const char* const pOldBVHData, char* const pNewBVHData);

extern void ConvertCollisionData_V120(const r5::v140::studiohdr_t* const oldStudioHdr, const char* const pOldBVHData);
extern void ConvertCollisionData_V120_HeadersOnly(const char* const pOldBVHData, char* const newData);

extern void CopyAnimRefData(const char* const pOldAnimRefData, char* const pNewAnimRefData, const int numlocalseq);
