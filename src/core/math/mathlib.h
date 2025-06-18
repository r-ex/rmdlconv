#pragma once

#define MATH_ASSERTS 1
#define MATH_SIMD 0

#define DEG2RAD(x) (static_cast<float>(x) * (static_cast<float>(M_PI) / 180.f))
#define RAD2DEG(x) (static_cast<float>(x) * (180.f / static_cast<float>(M_PI)))

#if MATH_SIMD
#define SubFloat(m, i) (m.m128_f32[i])
#define SubInt(m, i) (m.m128_i32[i])

#define BINOP(op) 														\
	__m128 retVal;                                          				\
	SubFloat( retVal, 0 ) = ( SubFloat( a, 0 ) op SubFloat( b, 0 ) );	\
	SubFloat( retVal, 1 ) = ( SubFloat( a, 1 ) op SubFloat( b, 1 ) );	\
	SubFloat( retVal, 2 ) = ( SubFloat( a, 2 ) op SubFloat( b, 2 ) );	\
	SubFloat( retVal, 3 ) = ( SubFloat( a, 3 ) op SubFloat( b, 3 ) );	\
    return retVal;

#define IBINOP(op) 														\
	__m128 retVal;														\
	SubInt( retVal, 0 ) = ( SubInt( a, 0 ) op SubInt ( b, 0 ) );		\
	SubInt( retVal, 1 ) = ( SubInt( a, 1 ) op SubInt ( b, 1 ) );		\
	SubInt( retVal, 2 ) = ( SubInt( a, 2 ) op SubInt ( b, 2 ) );		\
	SubInt( retVal, 3 ) = ( SubInt( a, 3 ) op SubInt ( b, 3 ) );		\
    return retVal;

__forceinline __m128 OrSIMD(const __m128& a, const __m128& b)				// a | b
{
	IBINOP(| );
}

__forceinline __m128 AndSIMD(const __m128& a, const __m128& b)				// a & b
{
	IBINOP(&);
}

__forceinline __m128 AndNotSIMD(const __m128& a, const __m128& b)			// ~a & b
{
	__m128 retVal;
	SubInt(retVal, 0) = ~SubInt(a, 0) & SubInt(b, 0);
	SubInt(retVal, 1) = ~SubInt(a, 1) & SubInt(b, 1);
	SubInt(retVal, 2) = ~SubInt(a, 2) & SubInt(b, 2);
	SubInt(retVal, 3) = ~SubInt(a, 3) & SubInt(b, 3);
	return retVal;
}

static constexpr __m128 s_SimdSplatOne = { 1.0f };

__forceinline __m128 MaskedAssign(const __m128& ReplacementMask, const __m128& NewValue, const __m128& OldValue)
{
	//return _mm_or_ps( _mm_add_ps(ReplacementMask, NewValue), _mm_andnot_ps(ReplacementMask, OldValue));
	return OrSIMD(AndSIMD(ReplacementMask, NewValue), AndNotSIMD(ReplacementMask, OldValue));
}

__forceinline __m128 ReplicateX4(float flValue)
{
	__m128 value = _mm_set_ss(flValue);
	return _mm_shuffle_ps(value, value, 0);
}

__forceinline __m128 MaddSIMD(const __m128& a, const __m128& b, const __m128& c)				// a*b + c
{
	return _mm_add_ps(_mm_mul_ps(a, b), c);
}

__forceinline __m128 Dot4SIMD(const __m128& a, const __m128& b)
{
	__m128 m = _mm_mul_ps(a, b);
	float flDot = SubFloat(m, 0) + SubFloat(m, 1) + SubFloat(m, 2) + SubFloat(m, 3);
	return ReplicateX4(flDot);
}
#endif // MATH_SIMD


// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/mathlib/mathlib.h#L537C1-L543C3
// MOVEMENT INFO
enum
{
	PITCH = 0,	// up / down
	YAW,		// left / right
	ROLL		// fall over
};

// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/mathlib/mathlib.h#L745C1-L751C2
template <class T>
__forceinline void V_swap(T& x, T& y)
{
	T temp = x;
	x = y;
	y = temp;
}

inline void SinCos(float x, float* fsin, float* fcos)
{
	*fsin = sin(x);
	*fcos = cos(x);
}

static __forceinline float __vectorcall FastSqrtFast(float x)
{
	// use intrinsics
	__m128 root = _mm_sqrt_ss(_mm_load_ss(&x));
	return _mm_cvtss_f32(root);
};

static __forceinline float __vectorcall FastSqrtFast(const __m128& x)
{
	// use intrinsics
	__m128 root = _mm_sqrt_ss(x);
	return _mm_cvtss_f32(root);
};

// reciprocal
static __forceinline float __vectorcall FastRSqrtFast(float x)
{
	// use intrinsics
	__m128 rroot = _mm_rsqrt_ss(_mm_load_ss(&x));
	return _mm_cvtss_f32(rroot);
};

static __forceinline float __vectorcall FastRSqrtFast(const __m128& x)
{
	// use intrinsics
	__m128 rroot = _mm_rsqrt_ss(x);
	return _mm_cvtss_f32(rroot);
};

float AngleDiff(float destAngle, float srcAngle);
class Vector;
class Quaternion;
class RadianEuler;
class QAngle;
struct matrix3x4_t;

// lovely chunk of quaternion functions from source
// https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/public/mathlib/mathlib.h#L594

void QuaternionBlend(const Quaternion& p, const Quaternion& q, float t, Quaternion& qt);
void QuaternionBlendNoAlign(const Quaternion& p, const Quaternion& q, float t, Quaternion& qt);



void QuaternionAlign(const Quaternion& p, const Quaternion& q, Quaternion& qt);



float QuaternionNormalize(Quaternion& q);


void QuaternionMatrix(const Quaternion& q, matrix3x4_t& matrix);
void QuaternionMatrix(const Quaternion& q, const Vector& pos, matrix3x4_t& matrix);
void QuaternionAngles(const Quaternion& q, QAngle& angles);
void AngleQuaternion(const QAngle& angles, Quaternion& qt);
void QuaternionAngles(const Quaternion& q, RadianEuler& angles);
void AngleQuaternion(RadianEuler const& angles, Quaternion& qt);