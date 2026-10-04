#ifndef __CM_SIMD_H__
#define __CM_SIMD_H__

// four floats at a time for the collision code: SSE2 on x86, NEON on arm64, plain code elsewhere

// without optimisation the small wrappers below are real calls and cost more than they save
#if defined(CM_NO_SIMD) || (defined(_MSC_VER) && defined(_DEBUG)) || (!defined(_MSC_VER) && !defined(__OPTIMIZE__))
#define CM_SIMD 0
#elif defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
#include <emmintrin.h>
#define CM_SIMD 1

struct Float4
{
	__m128 v;

	static Float4 load( const float* p ) { return { _mm_loadu_ps( p ) }; }
	static Float4 splat( float f ) { return { _mm_set1_ps( f ) }; }
	void store( float* p ) const { _mm_storeu_ps( p, v ); }
	Float4 operator+( Float4 o ) const { return { _mm_add_ps( v, o.v ) }; }
	Float4 operator-( Float4 o ) const { return { _mm_sub_ps( v, o.v ) }; }
	Float4 operator*( Float4 o ) const { return { _mm_mul_ps( v, o.v ) }; }
	// per lane: negative ? whenNegative : otherwise
	static Float4 selectNegative( Float4 value, Float4 whenNegative, Float4 otherwise )
	{
		const __m128 mask = _mm_cmplt_ps( value.v, _mm_setzero_ps() );
		return { _mm_or_ps( _mm_and_ps( mask, whenNegative.v ), _mm_andnot_ps( mask, otherwise.v ) ) };
	}
	// bit per lane where a < b
	static int lessMask( Float4 a, Float4 b ) { return _mm_movemask_ps( _mm_cmplt_ps( a.v, b.v ) ); }
};

#elif defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>
#define CM_SIMD 1

struct Float4
{
	float32x4_t v;

	static Float4 load( const float* p ) { return { vld1q_f32( p ) }; }
	static Float4 splat( float f ) { return { vdupq_n_f32( f ) }; }
	void store( float* p ) const { vst1q_f32( p, v ); }
	Float4 operator+( Float4 o ) const { return { vaddq_f32( v, o.v ) }; }
	Float4 operator-( Float4 o ) const { return { vsubq_f32( v, o.v ) }; }
	Float4 operator*( Float4 o ) const { return { vmulq_f32( v, o.v ) }; }
	static Float4 selectNegative( Float4 value, Float4 whenNegative, Float4 otherwise )
	{
		return { vbslq_f32( vcltq_f32( value.v, vdupq_n_f32( 0.0f ) ), whenNegative.v, otherwise.v ) };
	}
	static int lessMask( Float4 a, Float4 b )
	{
		const uint32x4_t bits = { 1, 2, 4, 8 };
		return static_cast<int>( vaddvq_u32( vandq_u32( vcltq_f32( a.v, b.v ), bits ) ) );
	}
};

#else
#define CM_SIMD 0
#endif

// plane data of a brush packed four sides to a block: nx[4] ny[4] nz[4] dist[4]
constexpr int cmPlaneBlockFloats = 16;

#endif
