#include "common.h"
#ifdef VU_COLLISION
#include "VuVector.h"
#include "VuCollision.h"

#ifdef __ARM_NEON
#include "NeonMath.h"
#endif

#ifndef GTA_PS2
int16 vi01;
CVuVector vf01;
CVuVector vf02;
CVuVector vf03;

CVuVector
DistanceBetweenSphereAndLine(const CVuVector &center, const CVuVector &p0, const CVuVector &line)
{
    CVuVector ret;    // VF16
#ifdef __ARM_NEON
	float result[4];
	NEON_DISTANCE_SPHERE_LINE(result, &center.x, &p0.x, &line.x);
	ret.x = result[0];
	ret.y = result[1];
	ret.z = result[2];
	ret.w = result[3];
#else
	// center  VF12
	// p0      VF14
	// line    VF15
	CVuVector p1 = p0+line;
	CVuVector dist0 = center - p0;	// VF20
	CVuVector dist1 = center - p1;	// VF25
	float lenSq = line.MagnitudeSqr();	// VF21
	float distSq0 = dist0.MagnitudeSqr();	// VF22
	float distSq1 = dist1.MagnitudeSqr();
	float dot = DotProduct(dist0, line);	// VF23
	if(dot < 0.0f){
		// not above line, closest to p0
		ret = p0;
		ret.w = distSq0;
		return ret;
	}
	float t = dot/lenSq;	// param of nearest point on infinite line
	if(t > 1.0f){
		// not above line, closest to p1
		ret = p1;
		ret.w = distSq1;
		return ret;
	}
	// closest to line
	ret = p0 + line*t;
	ret.w = (ret - center).MagnitudeSqr();
#endif
	return ret;
}

inline int SignFlags(const CVector &v)
{
	int f = 0;
	if(v.x < 0.0f) f |= 1;
	if(v.y < 0.0f) f |= 2;
	if(v.z < 0.0f) f |= 4;
	return f;
}
#endif

extern "C" void
LineToTriangleCollision(const CVuVector &p0, const CVuVector &p1,
	const CVuVector &v0, const CVuVector &v1, const CVuVector &v2,
	const CVuVector &plane)
{
#ifdef GTA_PS2
	__asm__ volatile (
		".set noreorder\n"
		"lqc2\tvf12, 0x0(%0)\n"
		"lqc2\tvf13, 0x0(%1)\n"
		"lqc2\tvf14, 0x0(%2)\n"
		"lqc2\tvf15, 0x0(%3)\n"
		"lqc2\tvf16, 0x0(%4)\n"
		"lqc2\tvf17, 0x0(%5)\n"
		"vcallms\tVu0LineToTriangleCollisionStart\n"
		".set reorder\n"
		:
		: "r" (&p0), "r" (&p1), "r" (&v0), "r" (&v1), "r" (&v2), "r" (&plane)
	);
#elif defined(__ARM_NEON)
	// NEON-optimized path using macros
	float dot0, dot1;
	NEON_DOT_PRODUCT_3(&plane.x, &p0.x, dot0);
	NEON_DOT_PRODUCT_3(&plane.x, &p1.x, dot1);
	float dist0 = plane.w - dot0;
	float dist1 = plane.w - dot1;

	if(dist0 * dist1 > 0.0f){
		vi01 = 0;
		return;
	}

	CVuVector diff = p1 - p0;
	float t = dist0/(dot1 - dot0);
	CVuVector p = p0 + diff*t;
	p.w = 0.0f;
	vf01 = p;
	vf03.x = t;

	// Cross products using NEON macro
	CVector pv0 = p - v0, e01 = v1 - v0, cross1;
	NEON_CROSS_PRODUCT(&cross1.x, &pv0.x, &e01.x);
	
	CVector pv1 = p - v1, e12 = v2 - v1, cross2;
	NEON_CROSS_PRODUCT(&cross2.x, &pv1.x, &e12.x);
	
	CVector pv2 = p - v2, e20 = v0 - v2, cross3;
	NEON_CROSS_PRODUCT(&cross3.x, &pv2.x, &e20.x);
	
	int flagmask = 0;
	if(Abs(plane.x) > 0.5f) flagmask |= 1;
	if(Abs(plane.y) > 0.5f) flagmask |= 2;
	if(Abs(plane.z) > 0.5f) flagmask |= 4;
	
	int flags1, flags2, flags3;
	NEON_SIGN_FLAGS(&cross1.x, flags1); flags1 &= flagmask;
	NEON_SIGN_FLAGS(&cross2.x, flags2); flags2 &= flagmask;
	NEON_SIGN_FLAGS(&cross3.x, flags3); flags3 &= flagmask;
	
	if(flags1 != flags2 || flags1 != flags3){
		vi01 = 0;
		return;
	}
	vi01 = 1;
	vf02 = plane;
#else
	float dot0 = DotProduct(plane, p0);
	float dot1 = DotProduct(plane, p1);
	float dist0 = plane.w - dot0;
	float dist1 = plane.w - dot1;

	if(dist0 * dist1 > 0.0f){
		vi01 = 0;
		return;
	}

	CVuVector diff = p1 - p0;
	float t = dist0/(dot1 - dot0);
	CVuVector p = p0 + diff*t;
	p.w = 0.0f;
	vf01 = p;
	vf03.x = t;

	CVector cross1 = CrossProduct(p-v0, v1-v0);
	CVector cross2 = CrossProduct(p-v1, v2-v1);
	CVector cross3 = CrossProduct(p-v2, v0-v2);
	int flagmask = 0;
	if(Abs(plane.x) > 0.5f) flagmask |= 1;
	if(Abs(plane.y) > 0.5f) flagmask |= 2;
	if(Abs(plane.z) > 0.5f) flagmask |= 4;
	int flags1 = SignFlags(cross1) & flagmask;
	int flags2 = SignFlags(cross2) & flagmask;
	int flags3 = SignFlags(cross3) & flagmask;
	if(flags1 != flags2 || flags1 != flags3){
		vi01 = 0;
		return;
	}
	vi01 = 1;
	vf02 = plane;
#endif
}

extern "C" void
LineToTriangleCollisionCompressed(const CVuVector &p0, const CVuVector &p1, VuTriangle &tri)
{
#ifdef GTA_PS2
	__asm__ volatile (
		".set noreorder\n"
		"lqc2\tvf12, 0x0(%0)\n"
		"lqc2\tvf13, 0x0(%1)\n"
		"lqc2\tvf14, 0x0(%2)\n"
		"lqc2\tvf15, 0x10(%2)\n"
		"lqc2\tvf16, 0x20(%2)\n"
		"lqc2\tvf17, 0x30(%2)\n"
		"vcallms\tVu0LineToTriangleCollisionCompressedStart\n"
		".set reorder\n"
		:
		: "r" (&p0), "r" (&p1), "r" (&tri)
	);
#elif defined(__ARM_NEON)
	// NEON-optimized decompression using macro
	CVuVector v0, v1, v2, plane;
	NEON_DECOMPRESS_TRIANGLE(&v0.x, &v1.x, &v2.x, &plane.x, tri.v0, tri.v1, tri.v2, tri.plane);
	LineToTriangleCollision(p0, p1, v0, v1, v2, plane);
#else
	CVuVector v0, v1, v2, plane;
	v0.x = tri.v0[0]/128.0f;
	v0.y = tri.v0[1]/128.0f;
	v0.z = tri.v0[2]/128.0f;
	v0.w = tri.v0[3]/128.0f;
	v1.x = tri.v1[0]/128.0f;
	v1.y = tri.v1[1]/128.0f;
	v1.z = tri.v1[2]/128.0f;
	v1.w = tri.v1[3]/128.0f;
	v2.x = tri.v2[0]/128.0f;
	v2.y = tri.v2[1]/128.0f;
	v2.z = tri.v2[2]/128.0f;
	v2.w = tri.v2[3]/128.0f;
	plane.x = tri.plane[0]/4096.0f;
	plane.y = tri.plane[1]/4096.0f;
	plane.z = tri.plane[2]/4096.0f;
	plane.w = tri.plane[3]/128.0f;
	LineToTriangleCollision(p0, p1, v0, v1, v2, plane);
#endif
}

extern "C" void
SphereToTriangleCollision(const CVuVector &sph,
	const CVuVector &v0, const CVuVector &v1, const CVuVector &v2,
	const CVuVector &plane)
{
#ifdef GTA_PS2
	__asm__ volatile (
		".set noreorder\n"
		"lqc2\tvf12, 0x0(%0)\n"
		"lqc2\tvf14, 0x0(%1)\n"
		"lqc2\tvf15, 0x0(%2)\n"
		"lqc2\tvf16, 0x0(%3)\n"
		"lqc2\tvf17, 0x0(%4)\n"
		"vcallms\tVu0SphereToTriangleCollisionStart\n"
		".set reorder\n"
		:
		: "r" (&sph), "r" (&v0), "r" (&v1), "r" (&v2), "r" (&plane)
	);
#elif defined(__ARM_NEON)
	// NEON-optimized path using macros
	float planedist;
	NEON_DOT_PRODUCT_3(&plane.x, &sph.x, planedist);
	planedist = planedist - plane.w;
	
	if(Abs(planedist) > sph.w){
		vi01 = 0;
		return;
	}
	
	CVuVector p = sph - planedist*plane;
	p.w = 0.0f;
	vf01 = p;
	planedist = Abs(planedist);
	
	CVuVector v01 = v1 - v0;
	CVuVector v12 = v2 - v1;
	CVuVector v20 = v0 - v2;
	
	CVector pv0 = p - v0, pv1 = p - v1, pv2 = p - v2;
	CVector cross1, cross2, cross3;
	NEON_CROSS_PRODUCT(&cross1.x, &pv0.x, &v01.x);
	NEON_CROSS_PRODUCT(&cross2.x, &pv1.x, &v12.x);
	NEON_CROSS_PRODUCT(&cross3.x, &pv2.x, &v20.x);
	
	int flagmask = 0;
	if(Abs(plane.x) > 0.1f) flagmask |= 1;
	if(Abs(plane.y) > 0.1f) flagmask |= 2;
	if(Abs(plane.z) > 0.1f) flagmask |= 4;
	
	int nflags, flags1, flags2, flags3;
	NEON_SIGN_FLAGS(&plane.x, nflags); nflags &= flagmask;
	NEON_SIGN_FLAGS(&cross1.x, flags1); flags1 &= flagmask;
	NEON_SIGN_FLAGS(&cross2.x, flags2); flags2 &= flagmask;
	NEON_SIGN_FLAGS(&cross3.x, flags3); flags3 &= flagmask;
	
	int testcase = 0;
	CVuVector closest(0.0f, 0.0f, 0.0f);
	if(flags1 == nflags){ closest += v2; testcase++; }
	if(flags2 == nflags){ closest += v0; testcase++; }
	if(flags3 == nflags){ closest += v1; testcase++; }
	
	if(testcase == 3){
		vf02 = plane;
		vf02.w = vf03.x = planedist;
		vi01 = 1;
	}else if(testcase == 1){
		vf01 = closest;
		vf02 = sph - closest;
		float distSq;
		NEON_MAGNITUDE_SQR_3(&vf02.x, distSq);
		vi01 = sph.w*sph.w > distSq;
		NEON_SQRT(distSq, vf03.x);
		float invDist;
		NEON_RECIP_SQRT(distSq, invDist);
		vf02.x *= invDist;
		vf02.y *= invDist;
		vf02.z *= invDist;
	}else{
		if(flags1 != nflags)
			closest = DistanceBetweenSphereAndLine(sph, v0, v01);
		else if(flags2 != nflags)
			closest = DistanceBetweenSphereAndLine(sph, v1, v12);
		else
			closest = DistanceBetweenSphereAndLine(sph, v2, v20);
		vi01 = sph.w*sph.w > closest.w;
		vf01 = closest;
		vf02 = sph - closest;
		NEON_SQRT(closest.w, vf03.x);
		float invDist;
		NEON_RECIP_SQRT(closest.w, invDist);
		vf02.x *= invDist;
		vf02.y *= invDist;
		vf02.z *= invDist;
	}
#else
	float planedist = DotProduct(plane, sph) - plane.w;
	if(Abs(planedist) > sph.w){
		vi01 = 0;
		return;
	}
	CVuVector p = sph - planedist*plane;
	p.w = 0.0f;
	vf01 = p;
	planedist = Abs(planedist);
	CVuVector v01 = v1 - v0;
	CVuVector v12 = v2 - v1;
	CVuVector v20 = v0 - v2;
	CVector cross1 = CrossProduct(p-v0, v01);
	CVector cross2 = CrossProduct(p-v1, v12);
	CVector cross3 = CrossProduct(p-v2, v20);
	int flagmask = 0;
	if(Abs(plane.x) > 0.1f) flagmask |= 1;
	if(Abs(plane.y) > 0.1f) flagmask |= 2;
	if(Abs(plane.z) > 0.1f) flagmask |= 4;
	int nflags = SignFlags(plane) & flagmask;
	int flags1 = SignFlags(cross1) & flagmask;
	int flags2 = SignFlags(cross2) & flagmask;
	int flags3 = SignFlags(cross3) & flagmask;
	int testcase = 0;
	CVuVector closest(0.0f, 0.0f, 0.0f);
	if(flags1 == nflags){ closest += v2; testcase++; }
	if(flags2 == nflags){ closest += v0; testcase++; }
	if(flags3 == nflags){ closest += v1; testcase++; }
	if(testcase == 3){
		vf02 = plane;
		vf02.w = vf03.x = planedist;
		vi01 = 1;
	}else if(testcase == 1){
		vf01 = closest;
		vf02 = sph - closest;
		float distSq = vf02.MagnitudeSqr();
		vi01 = sph.w*sph.w > distSq;
		vf03.x = Sqrt(distSq);
		vf02 *= 1.0f/vf03.x;
	}else{
		if(flags1 != nflags)
			closest = DistanceBetweenSphereAndLine(sph, v0, v01);
		else if(flags2 != nflags)
			closest = DistanceBetweenSphereAndLine(sph, v1, v12);
		else
			closest = DistanceBetweenSphereAndLine(sph, v2, v20);
		vi01 = sph.w*sph.w > closest.w;
		vf01 = closest;
		vf02 = sph - closest;
		vf03.x = Sqrt(closest.w);
		vf02 *= 1.0f/vf03.x;
	}
#endif
}

extern "C" void
SphereToTriangleCollisionCompressed(const CVuVector &sph, VuTriangle &tri)
{
#ifdef GTA_PS2
	__asm__ volatile (
		".set noreorder\n"
		"lqc2\tvf12, 0x0(%0)\n"
		"lqc2\tvf14, 0x0(%1)\n"
		"lqc2\tvf15, 0x10(%1)\n"
		"lqc2\tvf16, 0x20(%1)\n"
		"lqc2\tvf17, 0x30(%1)\n"
		"vcallms\tVu0SphereToTriangleCollisionCompressedStart\n"
		".set reorder\n"
		:
		: "r" (&sph), "r" (&tri)
	);
#elif defined(__ARM_NEON)
	// NEON-optimized decompression using macro
	CVuVector v0, v1, v2, plane;
	NEON_DECOMPRESS_TRIANGLE(&v0.x, &v1.x, &v2.x, &plane.x, tri.v0, tri.v1, tri.v2, tri.plane);
	SphereToTriangleCollision(sph, v0, v1, v2, plane);
#else
	CVuVector v0, v1, v2, plane;
	v0.x = tri.v0[0]/128.0f;
	v0.y = tri.v0[1]/128.0f;
	v0.z = tri.v0[2]/128.0f;
	v0.w = tri.v0[3]/128.0f;
	v1.x = tri.v1[0]/128.0f;
	v1.y = tri.v1[1]/128.0f;
	v1.z = tri.v1[2]/128.0f;
	v1.w = tri.v1[3]/128.0f;
	v2.x = tri.v2[0]/128.0f;
	v2.y = tri.v2[1]/128.0f;
	v2.z = tri.v2[2]/128.0f;
	v2.w = tri.v2[3]/128.0f;
	plane.x = tri.plane[0]/4096.0f;
	plane.y = tri.plane[1]/4096.0f;
	plane.z = tri.plane[2]/4096.0f;
	plane.w = tri.plane[3]/128.0f;
	SphereToTriangleCollision(sph, v0, v1, v2, plane);
#endif
}
#endif