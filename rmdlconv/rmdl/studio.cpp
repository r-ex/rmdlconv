#include "stdafx.h"
#include "rmdl/studio_rmdl.h"

uint32_t PackNormalTangent_UINT32(Vector3 normal, Vector4 tangent)
{
	return PackNormalTangent_UINT32(normal.x, normal.y, normal.z, tangent.w);
}
uint32_t PackNormalTangent_UINT32(float v1, float v2, float v3, float v4)
{
	// normal 1 and normal 2
	int16_t normal1, normal2;
	uint8_t sign = 0, droppedComponent = 0;
	float s;

	float f1 = std::abs(v1);
	float f2 = std::abs(v2);
	float f3 = std::abs(v3);

	if (f1 >= f2 && f1 >= f3)
		droppedComponent = 0;
	else if (f2 >= f1 && f2 >= f3)
		droppedComponent = 1;
	else
		droppedComponent = 2;

	// changed from 256 -> 255.5 because it made results neat identical to vanilla vg and values read closer to vvd when using for subtraction
	switch (droppedComponent)
	{
	case 0:
		sign = v1 < 0 ? 1 : 0;
		s = v1 / (sign ? -255 : 255);
		normal2 = (int16_t)std::roundf((v2 / s) + 255.5);
		normal1 = (int16_t)std::roundf((v3 / s) + 255.5);
		break;
	case 1:
		sign = v2 < 0 ? 1 : 0;
		s = v2 / (sign ? -255 : 255);
		normal1 = (int16_t)std::roundf((v1 / s) + 255.5);
		normal2 = (int16_t)std::roundf((v3 / s) + 255.5);
		break;
	case 2:
		sign = v3 < 0 ? 1 : 0;
		s = v3 / (sign ? -255 : 255);
		normal2 = (int16_t)std::roundf((v1 / s) + 255.5);
		normal1 = (int16_t)std::roundf((v2 / s) + 255.5);
		break;
	default:
		break;
	}

	char binormSign = v4 < 1 ? 1 : 0;

	return (binormSign << 31) + (droppedComponent << 29) + (sign << 28) + (normal2 << 19) + (normal1 << (19 - 9));
}

Vector64 PackPos_UINT64(Vector3 vec)
{
	Vector64 pos;

	pos.x = ((vec.x + 1024.0) / 0.0009765625);
	pos.y = ((vec.y + 1024.0) / 0.0009765625);
	pos.z = ((vec.z + 2048.0) / 0.0009765625);

	return pos;
}
