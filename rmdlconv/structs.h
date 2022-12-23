#pragma once

#include <cstdint>
#include <mathlib.h>

// normal source uses these data types
typedef char byte;
typedef uint8_t uint8;

struct Vector2
{
	float x, y;
};

struct Vector3
{
	float x, y, z;
};

struct Vector4
{
	float x, y, z, w;
};

struct Vector64
{
	uint64_t x : 21;
	uint64_t y : 21;
	uint64_t z : 22;
};

struct VertexColor_t
{
	uint8 r, g, b, a;
};
