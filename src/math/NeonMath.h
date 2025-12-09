#pragma once

// =============================================================================
// ARM NEON SIMD optimizations for vector/matrix math on Android
// Equivalent to PS2 VU0 microcode optimizations
// All implemented as MACROS for maximum compile-time optimization
// =============================================================================

#ifdef __ARM_NEON
#include <arm_neon.h>

// =============================================================================
// Constants for decompression (same as PS2 VU: 1/128, 1/4096)
// =============================================================================
#define NEON_INV_128   0.0078125f
#define NEON_INV_4096  0.000244140625f

// =============================================================================
// NEON Vector Operations (MACROS)
// =============================================================================

// Fast reciprocal square root approximation (1/sqrt(x))
// Uses NEON's vrsqrte with Newton-Raphson refinement
#define NEON_RECIP_SQRT(x, result) do { \
	float32x2_t _nrs_v = vdup_n_f32(x); \
	float32x2_t _nrs_est = vrsqrte_f32(_nrs_v); \
	_nrs_est = vmul_f32(_nrs_est, vrsqrts_f32(vmul_f32(_nrs_v, _nrs_est), _nrs_est)); \
	_nrs_est = vmul_f32(_nrs_est, vrsqrts_f32(vmul_f32(_nrs_v, _nrs_est), _nrs_est)); \
	(result) = vget_lane_f32(_nrs_est, 0); \
} while(0)

// Fast square root: sqrt(x) = x * rsqrt(x)
#define NEON_SQRT(x, result) do { \
	if ((x) <= 0.0f) { (result) = 0.0f; } \
	else { float _ns_inv; NEON_RECIP_SQRT(x, _ns_inv); (result) = (x) * _ns_inv; } \
} while(0)

// Dot product of two 3D vectors
#define NEON_DOT_PRODUCT_3(a, b, result) do { \
	float32x4_t _ndp_va = vld1q_f32(a); \
	float32x4_t _ndp_vb = vld1q_f32(b); \
	float32x4_t _ndp_prod = vmulq_f32(_ndp_va, _ndp_vb); \
	(result) = vgetq_lane_f32(_ndp_prod, 0) + vgetq_lane_f32(_ndp_prod, 1) + vgetq_lane_f32(_ndp_prod, 2); \
} while(0)

// Magnitude squared of 3D vector
#define NEON_MAGNITUDE_SQR_3(v, result) do { \
	float32x4_t _nms_vec = vld1q_f32(v); \
	float32x4_t _nms_sq = vmulq_f32(_nms_vec, _nms_vec); \
	(result) = vgetq_lane_f32(_nms_sq, 0) + vgetq_lane_f32(_nms_sq, 1) + vgetq_lane_f32(_nms_sq, 2); \
} while(0)

// Cross product of two 3D vectors: result = v1 x v2
#define NEON_CROSS_PRODUCT(result, v1, v2) do { \
	float _ncp_ax = (v1)[0], _ncp_ay = (v1)[1], _ncp_az = (v1)[2]; \
	float _ncp_bx = (v2)[0], _ncp_by = (v2)[1], _ncp_bz = (v2)[2]; \
	(result)[0] = _ncp_ay * _ncp_bz - _ncp_az * _ncp_by; \
	(result)[1] = _ncp_az * _ncp_bx - _ncp_ax * _ncp_bz; \
	(result)[2] = _ncp_ax * _ncp_by - _ncp_ay * _ncp_bx; \
} while(0)

// Sign flags for a vector (bit 0=x<0, bit 1=y<0, bit 2=z<0)
#define NEON_SIGN_FLAGS(v, result) do { \
	int _nsf = 0; \
	if ((v)[0] < 0.0f) _nsf |= 1; \
	if ((v)[1] < 0.0f) _nsf |= 2; \
	if ((v)[2] < 0.0f) _nsf |= 4; \
	(result) = _nsf; \
} while(0)

// =============================================================================
// NEON Matrix-Vector Operations (MACROS)
// CMatrix layout (row-major): f[0]=right, f[1]=forward, f[2]=up, f[3]=pos
// =============================================================================

// Transform point by matrix (with translation): result = mat * vec
#define NEON_TRANSFORM_POINT(result, mat, v) do { \
	float32x4_t _ntp_row0 = vld1q_f32(mat); \
	float32x4_t _ntp_row1 = vld1q_f32((mat) + 4); \
	float32x4_t _ntp_row2 = vld1q_f32((mat) + 8); \
	float32x4_t _ntp_row3 = vld1q_f32((mat) + 12); \
	float _ntp_vx = (v)[0], _ntp_vy = (v)[1], _ntp_vz = (v)[2]; \
	float32x4_t _ntp_res = vmulq_n_f32(_ntp_row0, _ntp_vx); \
	_ntp_res = vmlaq_n_f32(_ntp_res, _ntp_row1, _ntp_vy); \
	_ntp_res = vmlaq_n_f32(_ntp_res, _ntp_row2, _ntp_vz); \
	_ntp_res = vaddq_f32(_ntp_res, _ntp_row3); \
	(result)[0] = vgetq_lane_f32(_ntp_res, 0); \
	(result)[1] = vgetq_lane_f32(_ntp_res, 1); \
	(result)[2] = vgetq_lane_f32(_ntp_res, 2); \
} while(0)

// Transform vector by 3x3 portion of matrix (no translation)
#define NEON_MULTIPLY_3X3(result, mat, v) do { \
	float32x4_t _nm3_row0 = vld1q_f32(mat); \
	float32x4_t _nm3_row1 = vld1q_f32((mat) + 4); \
	float32x4_t _nm3_row2 = vld1q_f32((mat) + 8); \
	float _nm3_vx = (v)[0], _nm3_vy = (v)[1], _nm3_vz = (v)[2]; \
	float32x4_t _nm3_res = vmulq_n_f32(_nm3_row0, _nm3_vx); \
	_nm3_res = vmlaq_n_f32(_nm3_res, _nm3_row1, _nm3_vy); \
	_nm3_res = vmlaq_n_f32(_nm3_res, _nm3_row2, _nm3_vz); \
	(result)[0] = vgetq_lane_f32(_nm3_res, 0); \
	(result)[1] = vgetq_lane_f32(_nm3_res, 1); \
	(result)[2] = vgetq_lane_f32(_nm3_res, 2); \
} while(0)

// Matrix-Matrix Multiplication: out = m1 * m2
#define NEON_MATRIX_MULTIPLY(out, m1, m2) do { \
	float32x4_t _nmm_m1r0 = vld1q_f32(m1); \
	float32x4_t _nmm_m1r1 = vld1q_f32((m1) + 4); \
	float32x4_t _nmm_m1r2 = vld1q_f32((m1) + 8); \
	float32x4_t _nmm_m1r3 = vld1q_f32((m1) + 12); \
	float32x4_t _nmm_m2r0 = vld1q_f32(m2); \
	float32x4_t _nmm_m2r1 = vld1q_f32((m2) + 4); \
	float32x4_t _nmm_m2r2 = vld1q_f32((m2) + 8); \
	float32x4_t _nmm_m2r3 = vld1q_f32((m2) + 12); \
	float32x4_t _nmm_or0 = vmulq_n_f32(_nmm_m1r0, vgetq_lane_f32(_nmm_m2r0, 0)); \
	_nmm_or0 = vmlaq_n_f32(_nmm_or0, _nmm_m1r1, vgetq_lane_f32(_nmm_m2r0, 1)); \
	_nmm_or0 = vmlaq_n_f32(_nmm_or0, _nmm_m1r2, vgetq_lane_f32(_nmm_m2r0, 2)); \
	float32x4_t _nmm_or1 = vmulq_n_f32(_nmm_m1r0, vgetq_lane_f32(_nmm_m2r1, 0)); \
	_nmm_or1 = vmlaq_n_f32(_nmm_or1, _nmm_m1r1, vgetq_lane_f32(_nmm_m2r1, 1)); \
	_nmm_or1 = vmlaq_n_f32(_nmm_or1, _nmm_m1r2, vgetq_lane_f32(_nmm_m2r1, 2)); \
	float32x4_t _nmm_or2 = vmulq_n_f32(_nmm_m1r0, vgetq_lane_f32(_nmm_m2r2, 0)); \
	_nmm_or2 = vmlaq_n_f32(_nmm_or2, _nmm_m1r1, vgetq_lane_f32(_nmm_m2r2, 1)); \
	_nmm_or2 = vmlaq_n_f32(_nmm_or2, _nmm_m1r2, vgetq_lane_f32(_nmm_m2r2, 2)); \
	float32x4_t _nmm_or3 = vmulq_n_f32(_nmm_m1r0, vgetq_lane_f32(_nmm_m2r3, 0)); \
	_nmm_or3 = vmlaq_n_f32(_nmm_or3, _nmm_m1r1, vgetq_lane_f32(_nmm_m2r3, 1)); \
	_nmm_or3 = vmlaq_n_f32(_nmm_or3, _nmm_m1r2, vgetq_lane_f32(_nmm_m2r3, 2)); \
	_nmm_or3 = vaddq_f32(_nmm_or3, _nmm_m1r3); \
	vst1q_f32(out, _nmm_or0); \
	vst1q_f32((out) + 4, _nmm_or1); \
	vst1q_f32((out) + 8, _nmm_or2); \
	vst1q_f32((out) + 12, _nmm_or3); \
} while(0)

// =============================================================================
// NEON Collision Functions (MACROS - equivalent to PS2 VU0 vu0Collision)
// =============================================================================

// Decompress triangle vertices and plane from VuTriangle using NEON
// Equivalent to PS2 VU: ITOF0 + MULi with 1/128 and 1/4096
#define NEON_DECOMPRESS_TRIANGLE(v0, v1, v2, plane, tri_v0, tri_v1, tri_v2, tri_plane) do { \
	int32x4_t _ndt_iv0 = vld1q_s32(tri_v0); \
	int32x4_t _ndt_iv1 = vld1q_s32(tri_v1); \
	int32x4_t _ndt_iv2 = vld1q_s32(tri_v2); \
	int32x4_t _ndt_ipl = vld1q_s32(tri_plane); \
	float32x4_t _ndt_fv0 = vcvtq_f32_s32(_ndt_iv0); \
	float32x4_t _ndt_fv1 = vcvtq_f32_s32(_ndt_iv1); \
	float32x4_t _ndt_fv2 = vcvtq_f32_s32(_ndt_iv2); \
	float32x4_t _ndt_fpl = vcvtq_f32_s32(_ndt_ipl); \
	float32x4_t _ndt_inv128 = vdupq_n_f32(NEON_INV_128); \
	float32x4_t _ndt_inv4096 = vdupq_n_f32(NEON_INV_4096); \
	_ndt_fv0 = vmulq_f32(_ndt_fv0, _ndt_inv128); \
	_ndt_fv1 = vmulq_f32(_ndt_fv1, _ndt_inv128); \
	_ndt_fv2 = vmulq_f32(_ndt_fv2, _ndt_inv128); \
	float32x4_t _ndt_pl_xyz = vmulq_f32(_ndt_fpl, _ndt_inv4096); \
	float _ndt_pw = vgetq_lane_f32(_ndt_fpl, 3) * NEON_INV_128; \
	vst1q_f32(v0, _ndt_fv0); \
	vst1q_f32(v1, _ndt_fv1); \
	vst1q_f32(v2, _ndt_fv2); \
	(plane)[0] = vgetq_lane_f32(_ndt_pl_xyz, 0); \
	(plane)[1] = vgetq_lane_f32(_ndt_pl_xyz, 1); \
	(plane)[2] = vgetq_lane_f32(_ndt_pl_xyz, 2); \
	(plane)[3] = _ndt_pw; \
} while(0)

// Distance between sphere center and line segment
// Returns: closest point in result[0-2], distance squared in result[3]
#define NEON_DISTANCE_SPHERE_LINE(result, center, lineOrigin, lineVec) do { \
	float _ndsl_p1x = (lineOrigin)[0] + (lineVec)[0]; \
	float _ndsl_p1y = (lineOrigin)[1] + (lineVec)[1]; \
	float _ndsl_p1z = (lineOrigin)[2] + (lineVec)[2]; \
	float _ndsl_d0x = (center)[0] - (lineOrigin)[0]; \
	float _ndsl_d0y = (center)[1] - (lineOrigin)[1]; \
	float _ndsl_d0z = (center)[2] - (lineOrigin)[2]; \
	float _ndsl_lenSq = (lineVec)[0]*(lineVec)[0] + (lineVec)[1]*(lineVec)[1] + (lineVec)[2]*(lineVec)[2]; \
	float _ndsl_distSq0 = _ndsl_d0x*_ndsl_d0x + _ndsl_d0y*_ndsl_d0y + _ndsl_d0z*_ndsl_d0z; \
	float _ndsl_dot = _ndsl_d0x*(lineVec)[0] + _ndsl_d0y*(lineVec)[1] + _ndsl_d0z*(lineVec)[2]; \
	if (_ndsl_dot < 0.0f) { \
		(result)[0] = (lineOrigin)[0]; (result)[1] = (lineOrigin)[1]; (result)[2] = (lineOrigin)[2]; \
		(result)[3] = _ndsl_distSq0; \
	} else { \
		float _ndsl_t = _ndsl_dot / _ndsl_lenSq; \
		if (_ndsl_t > 1.0f) { \
			float _ndsl_d1x = (center)[0] - _ndsl_p1x; \
			float _ndsl_d1y = (center)[1] - _ndsl_p1y; \
			float _ndsl_d1z = (center)[2] - _ndsl_p1z; \
			(result)[0] = _ndsl_p1x; (result)[1] = _ndsl_p1y; (result)[2] = _ndsl_p1z; \
			(result)[3] = _ndsl_d1x*_ndsl_d1x + _ndsl_d1y*_ndsl_d1y + _ndsl_d1z*_ndsl_d1z; \
		} else { \
			(result)[0] = (lineOrigin)[0] + (lineVec)[0] * _ndsl_t; \
			(result)[1] = (lineOrigin)[1] + (lineVec)[1] * _ndsl_t; \
			(result)[2] = (lineOrigin)[2] + (lineVec)[2] * _ndsl_t; \
			float _ndsl_dx = (result)[0] - (center)[0]; \
			float _ndsl_dy = (result)[1] - (center)[1]; \
			float _ndsl_dz = (result)[2] - (center)[2]; \
			(result)[3] = _ndsl_dx*_ndsl_dx + _ndsl_dy*_ndsl_dy + _ndsl_dz*_ndsl_dz; \
		} \
	} \
} while(0)

#endif // __ARM_NEON
