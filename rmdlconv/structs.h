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


	Vector3 Abs() const
	{
		return { fabs(x), fabs(y), fabs(z) };
	}

	void operator-=(Vector3 a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
	}

	Vector3 operator*(Vector3& a)
	{
		return { x * a.x, y * a.y, z * a.z };
	}

	Vector3 operator*(float a)
	{
		return { x * a, y * a, z * a };
	}

	inline float& operator[](int i)
	{
		return ((float*)this)[i];
	}

	inline float operator[](int i) const
	{
		return ((float*)this)[i];
	}
};

inline float Dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct Vector4
{
	float x, y, z, w;

	const Vector3& AsVector3() const
	{
		return *(Vector3*)this;
	}

	Vector4& operator=(const Vector4& vecIn)
	{
		Vector4 vecOut;

		vecOut.x = vecIn.x;
		vecOut.y = vecIn.y;
		vecOut.z = vecIn.z;
		vecOut.w = vecIn.w;

		return vecOut;
	}
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
