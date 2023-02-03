// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_MATH_H)
#define BASE_MATH_H

// TODO(Ryan): sse_mathfun.h

#include <math.h>

INTERNAL f32
sqrt_f32(f32 x)
{
  return sqrtf(x);
}

INTERNAL f32
sin_f32(f32 x)
{
  return sinf(x);
}

INTERNAL f32
cos_f32(f32 x)
{
  return cosf(x);
}

INTERNAL f32
tan_f32(f32 x)
{
  return tanf(x);
}

INTERNAL f32
ln_f32(f32 x)
{
  return logf(x);
}

INTERNAL f64
sqrt_f64(f64 x)
{
  return sqrt(x);
}

INTERNAL f64
sin_f64(f64 x)
{
  return sin(x);
}

INTERNAL f64
cos_f64(f64 x)
{
  return cos(x);
}

INTERNAL f64
tan_f64(f64 x)
{
  return tan(x);
}

INTERNAL f64
ln_f64(f64 x)
{
  return log(x);
}

INTERNAL f32
lerp_f32(f32 a, f32 b, f32 t)
{
  f32 result = 0.0f;
  result = ((b - a) * t) + a; 

  return result;
}

INTERNAL u32
round_f32_to_u32(f32 real32)
{
  u32 result = 0;

  result = (u32)roundf(real32);

  return result;
}

INTERNAL i32
round_f32_to_i32(f32 real32)
{
  i32 result = 0;

  result = (i32)roundf(real32);

  return result;
}

INTERNAL i32
floor_f32_to_i32(f32 real32)
{
  i32 result = 0;

  result = (i32)floorf(real32);

  return result;
}

INTERNAL u32
ceil_f32_to_u32(f32 real32)
{
  u32 result = 0;

  result = (u32)ceilf(real32);

  return result;
}

INTERNAL i32
ceil_f32_to_i32(f32 real32)
{
  i32 result = 0;

  result = (i32)ceilf(real32);

  return result;
}

INTERNAL f32
square(f32 x)
{
  f32 result = 0.0f;

  result = x * x;

  return result;
}

INTERNAL u64
round_to_nearest(u64 val, u64 near)
{
  u64 result = val;

  result += near - 1;
  result -= result % near;
  
  return result;
}

INTERNAL u32 
rand_u32(u32 *seed)
{
  u32 x = *seed;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  *seed = x;

  return *seed;
}

INTERNAL f32 
rand_unilateral_f32(u32 *seed)
{
  u32 exponent = 127;
  u32 mantissa = rand_u32(seed) >> 9;
  u32 bits = (exponent << 23) | mantissa;
  f32 result = *(f32 *)&bits - 1.0f;

  return result;
}

INTERNAL f32 
rand_bilateral_f32(u32 *seed)
{
  return -1.0f + (2.0f * rand_unilateral_f32(seed));
}

INTERNAL u32 
rand_range_u32(u32 *seed, u32 range)
{
  u32 result = rand_u32(seed) % range;

  return result;
}

INTERNAL i32
rand_range_i32(u32 *seed, i32 min, i32 max)
{
  if (max < min)
  {
    max = min;
  }

  i32 result = min + (i32)rand_u32(seed) % (max - min + 1);

  return result;
}

INTERNAL f32 
rand_range_f32(u32 *seed, f32 min, f32 max)
{
  f32 range = rand_unilateral_f32(seed);
  f32 result = min + range * (max - min);

  return result;
}

#if 0
// IMPORTANT(Ryan): Due to ASLR, could use pointer address where appropriate 
INTERNAL void
linux_get_entropy(void *buffer, size_t length)
{
  ERRNO_ASSERT(getentropy(buffer, length) != -1);
}

random_series is u32_entropy

28:#define STBTT_pow(x, y)  __builtin_powf(x, y)
29:#define STBTT_fmod(x, y) __builtin_fmodf(x, y)
31:#define STBTT_acos(x)    __builtin_acosf(x)

// put in math.h trig, sqrt, log INTERNALs for now

TODO(Ryan): Investigate retro-fps math sse routines
INTERNAL f32 sqrt_ss(f32 x)
{
  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set1_ps(x)));
}

static inline float rsqrt_ss(float x)
{
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set1_ps(x)));
}


// TODO(Ryan): Perhaps include round_to, ceil functions etc.

#define M(mat, row, col) (mat)->e[(mat)->n*(col) + (row)]

// IMPORTANT(Ryan): although typing out tedious, better than codegen complexity
// i.e. prefer tedious over complexity
union V2S32
{
  struct
  {
    s32 x, y;
  };
  s32 v[2];
};

union V2F32
{
  struct
  {
    f32 x, y;
  };
  f32 v[2];
};

union V3F32
{
  struct
  {
    f32 x, y, z;
  };
  f32 v[3];
};

union V4F32
{
  struct
  {
    f32 x, y, z, w;
  };
  f32 v[4];
};

// IMPORTANT: an axis-aligned rect
union I1F32
{
  struct
  {
    f32 min, max;
  };
  f32 v[2];
};

// TODO(Ryan): useful for memory ranges?
union I1U64
{
  struct
  {
    u64 min, max;
  };
  struct
  {
    u64 first, op1;
  };
  u64 v[2];
};

union I1U64
{
  struct
  {
    u64 min, max;
  };
  u64 v[2];
};

union I2F32
{
  struct
  {
    V2F32 min, max;
  };
  struct
  {
    V2F32 p0, p1;
  };
  struct
  {
    f32 x0, y0, x1, y1;
  };
  V2F32 p[2];
  F32 v[4];
};

INTERNAL V2S32
v2s32(s32 x, s32 y)
{
  V2S32 result = {x, y};
  return result;
}

INTERNAL V2S32
v2s32_vec(V2F32 x, V2F32 y);

INTERNAL V2S32
v2s32_range(I1F32 x, I1F32 y);

INTERNAL I2S32
i2s32(s32 x0, s32 y0, s32 x1, s32 y1)
{
  I2S32 result = {};
  if (x1 < x0)
  {
    result.x0 = x1;
    result.x1 = x0;
  }
}


// IMPORTANT(Ryan): Only works on multiples of 2
#define VECTOR_SIZE(amount, type) \
  vector_size(amount * sizeof(type))

typedef r32 v2 __attribute__((VECTOR_SIZE(2, r32)));
typedef union V2
{
  v2 vec;
  struct
  {
    r32 x, y;
  };
  r32 e[2];
} V2;

V2S32 operator+(const V2S32 &a, const V2S32 &b);
V2S32 operator-(const V2S32 &a, const V2S32 &b);
V2S32 operator*(const V2S32 &a, s32 b);

INTERNAL f32
vec_hadamard(V2F32 a, V2F32 b);
INTERNAL f32
vec_dot(V2F32 a, V2F32 b);
INTERNAL b32
interval_overlaps(I1F32 a, I1F32 b)
{
  b32 result = false;

  result = (b.min < b.max && a.min < b.max);

  return result;
}
INTERNAL b32
interval_contains(I1F32 r, F32 x)
{
  b32 result = false;

  result = (r.min <= x && x < r.max);

  return result;
}
INTERNAL f32
interval_dim(I1F32 r)
{
  f32 result = 0.f;

  result = (r.max - r.min);

  return result;
}
INTERNAL f32
interval_centre(I1F32 r)
{
  (r.min + r.max)*0.5f;
}
INTERNAL I1F32
interval_axis(I2F32 r, AXIS axis)
{
  I1F32 result = {};
  result.p[0] = r.p[0].v[axis];
  result.p[1] = r.p[1].v[axis];
  return result;
}

#if defined(COMPILER_GCC) && defined(ARCH_X86_64)
  #include <x86intrin.h>

  INTERNAL ALWAYS_INLINE u32 
  count_bits_set_u32(u32 val)
  {
    return __builtin_popcount(val);
  }

  // NOTE(Ryan): This is from most significant bits
  INTERNAL ALWAYS_INLINE u32 
  count_leading_zeroes_u32(u32 val)
  {
    return __builtin_clz(val);
  }

  INTERNAL ALWAYS_INLINE u32 
  count_trailing_zeroes_u32(u32 val)
  {
    return __builtin_ctz(val);
  }

  INTERNAL ALWAYS_INLINE u32 
  get_parity_u32(u32 val)
  {
    return __builtin_parity(val);
  }

  INTERNAL ALWAYS_INLINE u16 
  endianness_swap_u16(u16 val)
  {
    return __builtin_bswap16(val);
  }

  INTERNAL ALWAYS_INLINE u32 
  endianness_swap_u32(u32 val)
  {
    return __builtin_bswap32(val);
  }

  INTERNAL ALWAYS_INLINE u64 
  endianness_swap_u64(u64 val)
  {
    return __builtin_bswap64(val);
  }

  // TODO(Ryan): Enable optimisation flags for particular math routines we have no need to step through
  // math.h functions
  r64 __builtin_powi(r64, u32)
  r32 __builtin_powif(r32, u32)

#endif
#endif
#endif
