#include "common.h"
#include "NeonMath.h"

void
CVector::Normalise(void)
{
#ifdef __ARM_NEON
	float sq = MagnitudeSqr();
	if (sq > 0.0f) {
		float invsqrt;
		NEON_RECIP_SQRT(sq, invsqrt);
		x *= invsqrt;
		y *= invsqrt;
		z *= invsqrt;
	} else
		x = 1.0f;
#else
	float sq = MagnitudeSqr();
	if (sq > 0.0f) {
		float invsqrt = RecipSqrt(sq);
		x *= invsqrt;
		y *= invsqrt;
		z *= invsqrt;
	} else
		x = 1.0f;
#endif
}

CVector
CrossProduct(const CVector &v1, const CVector &v2)
{
#ifdef __ARM_NEON
	CVector result;
	NEON_CROSS_PRODUCT(&result.x, &v1.x, &v2.x);
	return result;
#else
	return CVector(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
#endif
}

CVector
Multiply3x3(const CMatrix &mat, const CVector &vec)
{
#ifdef __ARM_NEON
	CVector result;
	NEON_MULTIPLY_3X3(&result.x, mat.f[0], &vec.x);
	return result;
#else
	// TODO: VU0 code
	return CVector(mat.rx * vec.x + mat.fx * vec.y + mat.ux * vec.z,
	               mat.ry * vec.x + mat.fy * vec.y + mat.uy * vec.z,
	               mat.rz * vec.x + mat.fz * vec.y + mat.uz * vec.z);
#endif
}

CVector
Multiply3x3(const CVector &vec, const CMatrix &mat)
{
	return CVector(mat.rx * vec.x + mat.ry * vec.y + mat.rz * vec.z,
	               mat.fx * vec.x + mat.fy * vec.y + mat.fz * vec.z,
	               mat.ux * vec.x + mat.uy * vec.y + mat.uz * vec.z);
}

CVector
operator*(const CMatrix &mat, const CVector &vec)
{
#ifdef __ARM_NEON
	CVector result;
	NEON_TRANSFORM_POINT(&result.x, mat.f[0], &vec.x);
	return result;
#else
	// TODO: VU0 code
	return CVector(mat.rx * vec.x + mat.fx * vec.y + mat.ux * vec.z + mat.px,
	               mat.ry * vec.x + mat.fy * vec.y + mat.uy * vec.z + mat.py,
	               mat.rz * vec.x + mat.fz * vec.y + mat.uz * vec.z + mat.pz);
#endif
}
