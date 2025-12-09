#pragma once

// ARM NEON SIMD optimizations for vector/matrix math on Android
// Only enabled when __ARM_NEON is defined by the compiler
// Uses macros instead of functions for maximum inlining

#ifdef __ARM_NEON
#include <arm_neon.h>

// =============================================================================
// NEON Vector Operations (as macros for maximum optimization)
// =============================================================================

// Fast reciprocal square root approximation (1/sqrt(x))
// Uses NEON's vrsqrte with Newton-Raphson refinement for accuracy
#define NEON_RECIP_SQRT(x, result) do { \
	float32x2_t _v = vdup_n_f32(x); \
	float32x2_t _est = vrsqrte_f32(_v); \
	_est = vmul_f32(_est, vrsqrts_f32(vmul_f32(_v, _est), _est)); \
	_est = vmul_f32(_est, vrsqrts_f32(vmul_f32(_v, _est), _est)); \
	(result) = vget_lane_f32(_est, 0); \
} while(0)

// Cross product of two 3D vectors: result = v1 x v2
#define NEON_CROSS_PRODUCT(result, v1, v2) do { \
	float _ax = (v1)[0], _ay = (v1)[1], _az = (v1)[2]; \
	float _bx = (v2)[0], _by = (v2)[1], _bz = (v2)[2]; \
	(result)[0] = _ay * _bz - _az * _by; \
	(result)[1] = _az * _bx - _ax * _bz; \
	(result)[2] = _ax * _by - _ay * _bx; \
} while(0)

// =============================================================================
// NEON Matrix-Vector Operations
//
// CMatrix layout (row-major, float f[4][4]):
//   f[0] = { rx, ry, rz, rw }  - right vector row
//   f[1] = { fx, fy, fz, fw }  - forward vector row
//   f[2] = { ux, uy, uz, uw }  - up vector row
//   f[3] = { px, py, pz, pw }  - position row
//
// mat * vec: result = row0*vx + row1*vy + row2*vz + row3
// =============================================================================

// Transform point by matrix (with translation): result = mat * vec
#define NEON_TRANSFORM_POINT(result, mat, v) do { \
	float32x4_t _row0 = vld1q_f32(mat); \
	float32x4_t _row1 = vld1q_f32((mat) + 4); \
	float32x4_t _row2 = vld1q_f32((mat) + 8); \
	float32x4_t _row3 = vld1q_f32((mat) + 12); \
	float _vx = (v)[0], _vy = (v)[1], _vz = (v)[2]; \
	float32x4_t _res = vmulq_n_f32(_row0, _vx); \
	_res = vmlaq_n_f32(_res, _row1, _vy); \
	_res = vmlaq_n_f32(_res, _row2, _vz); \
	_res = vaddq_f32(_res, _row3); \
	(result)[0] = vgetq_lane_f32(_res, 0); \
	(result)[1] = vgetq_lane_f32(_res, 1); \
	(result)[2] = vgetq_lane_f32(_res, 2); \
} while(0)

// Transform vector by 3x3 portion of matrix (no translation)
#define NEON_MULTIPLY_3X3(result, mat, v) do { \
	float32x4_t _row0 = vld1q_f32(mat); \
	float32x4_t _row1 = vld1q_f32((mat) + 4); \
	float32x4_t _row2 = vld1q_f32((mat) + 8); \
	float _vx = (v)[0], _vy = (v)[1], _vz = (v)[2]; \
	float32x4_t _res = vmulq_n_f32(_row0, _vx); \
	_res = vmlaq_n_f32(_res, _row1, _vy); \
	_res = vmlaq_n_f32(_res, _row2, _vz); \
	(result)[0] = vgetq_lane_f32(_res, 0); \
	(result)[1] = vgetq_lane_f32(_res, 1); \
	(result)[2] = vgetq_lane_f32(_res, 2); \
} while(0)

// =============================================================================
// NEON Matrix-Matrix Multiplication
//
// out = m1 * m2 (row-major)
// out.rx = m1.rx*m2.rx + m1.fx*m2.ry + m1.ux*m2.rz
// =============================================================================

#define NEON_MATRIX_MULTIPLY(out, m1, m2) do { \
	float32x4_t _m1_row0 = vld1q_f32(m1); \
	float32x4_t _m1_row1 = vld1q_f32((m1) + 4); \
	float32x4_t _m1_row2 = vld1q_f32((m1) + 8); \
	float32x4_t _m1_row3 = vld1q_f32((m1) + 12); \
	float32x4_t _m2_row0 = vld1q_f32(m2); \
	float32x4_t _m2_row1 = vld1q_f32((m2) + 4); \
	float32x4_t _m2_row2 = vld1q_f32((m2) + 8); \
	float32x4_t _m2_row3 = vld1q_f32((m2) + 12); \
	float32x4_t _out_row0 = vmulq_n_f32(_m1_row0, vgetq_lane_f32(_m2_row0, 0)); \
	_out_row0 = vmlaq_n_f32(_out_row0, _m1_row1, vgetq_lane_f32(_m2_row0, 1)); \
	_out_row0 = vmlaq_n_f32(_out_row0, _m1_row2, vgetq_lane_f32(_m2_row0, 2)); \
	float32x4_t _out_row1 = vmulq_n_f32(_m1_row0, vgetq_lane_f32(_m2_row1, 0)); \
	_out_row1 = vmlaq_n_f32(_out_row1, _m1_row1, vgetq_lane_f32(_m2_row1, 1)); \
	_out_row1 = vmlaq_n_f32(_out_row1, _m1_row2, vgetq_lane_f32(_m2_row1, 2)); \
	float32x4_t _out_row2 = vmulq_n_f32(_m1_row0, vgetq_lane_f32(_m2_row2, 0)); \
	_out_row2 = vmlaq_n_f32(_out_row2, _m1_row1, vgetq_lane_f32(_m2_row2, 1)); \
	_out_row2 = vmlaq_n_f32(_out_row2, _m1_row2, vgetq_lane_f32(_m2_row2, 2)); \
	float32x4_t _out_row3 = vmulq_n_f32(_m1_row0, vgetq_lane_f32(_m2_row3, 0)); \
	_out_row3 = vmlaq_n_f32(_out_row3, _m1_row1, vgetq_lane_f32(_m2_row3, 1)); \
	_out_row3 = vmlaq_n_f32(_out_row3, _m1_row2, vgetq_lane_f32(_m2_row3, 2)); \
	_out_row3 = vaddq_f32(_out_row3, _m1_row3); \
	vst1q_f32(out, _out_row0); \
	vst1q_f32((out) + 4, _out_row1); \
	vst1q_f32((out) + 8, _out_row2); \
	vst1q_f32((out) + 12, _out_row3); \
} while(0)

#endif // __ARM_NEON
