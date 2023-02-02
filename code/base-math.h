// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_MATH_H)
#define BASE_MATH_H

// TODO(Ryan): sse_mathfun.h

#include <math.h>

INTERNAL u64
round_to_nearest(u64 val, u64 near)
{
  u64 result = val;

  result += near - 1;
  result -= result % near;
  
  return result;
}

INTERNAL u32
math_floor_f32_to_u32(f32 float32)
{
  u32 result = 0;

  result = (u32)floorf(float32);

  return result;
}

#if 0
// IMPORTANT(Ryan): Due to ASLR, could use pointer address where appropriate 
INTERNAL void
linux_get_entropy(void *buffer, size_t length)
{
  ERRNO_ASSERT(getentropy(buffer, length) != -1);
}

INTERNAL u32 
rand_u32(random_series *r)
{
    uint32_t x = r->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    r->state = x;
    return r->state;
}

// returns random [0, 1) float
static inline float random_unilateral (random_series_t *r)
{
    // NOTE: Stolen from rnd.h, courtesy of Jonatan Hedborg
    uint32_t exponent = 127;
    uint32_t mantissa = random_uint32(r) >> 9;
    uint32_t bits = (exponent << 23) | mantissa;
    float result = *(float *)&bits - 1.0f;
    return result;
}

// returns random [-1, 1) float
static inline float random_bilateral(random_series_t *r)
{
    return -1.0f + 2.0f*random_unilateral(r);
}

// returns random number in range [0, range)
static inline uint32_t random_choice(random_series_t *r, uint32_t range)
{
    uint32_t result = random_uint32(r) % range;
    return result;
}

// returns random number in range [1, sides]
static inline uint32_t dice_roll(random_series_t *r, uint32_t sides)
{
    uint32_t result = 1 + random_choice(r, sides);
    return result;
}

// returns random number in range [min, max]
static inline int32_t random_range_i32(random_series_t *r, int32_t min, int32_t max)
{
    if (max < min)
        max = min;

    int32_t result = min + (int32_t)random_uint32(r) % (max - min + 1);
    return result;
}

// returns random float in range [min, max)
static inline float random_range_f32(random_series_t *r, float min, float max)
{
    float range = random_unilateral(r);
    float result = min + range*(max - min);
    return result;
}


// put in math.h trig, sqrt, log INTERNALs for now
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
28:#define STBTT_pow(x, y)  __builtin_powf(x, y)
29:#define STBTT_fmod(x, y) __builtin_fmodf(x, y)
31:#define STBTT_acos(x)    __builtin_acosf(x)

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

TODO(Ryan): Investigate retro-fps math sse routines
INTERNAL f32 sqrt_ss(f32 x)
{
  return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set1_ps(x)));
}

static inline float rsqrt_ss(float x)
{
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set1_ps(x)));
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
lerp(f32 a, f32 b, f32 t)
{
  f32 result = 0.f;
  result = ((b - a) * t) + a; 
  return result;
}

// unlerp(10.f, lerp(10.f, .5f, 100.f), 100.f);
INTERNAL f32
unlerp(f32 a, f32 b, f32 t)
{
  f32 result = 0.f;
  if (a != b)
  {
    result = (t - a)/(b - a);
  }
  return result;
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

INTERNAL u32
round_r32_to_u32(r32 real32)
{
  u32 result = 0;

  result = (u32)roundf(real32);

  return result;
}

INTERNAL s32
round_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = (s32)roundf(real32);

  return result;
}


INTERNAL s32
floor_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = (s32)floorf(real32);

  return result;
}

INTERNAL u32
ceil_r32_to_u32(r32 real32)
{
  u32 result = 0;

  result = ceilf(real32);

  return result;
}

INTERNAL s32
ceil_r32_to_s32(r32 real32)
{
  s32 result = 0;

  result = ceilf(real32);

  return result;
}

INTERNAL r32
square(r32 x)
{
  r32 result = 0.0f;

  result = x * x;

  return result;
}

INTERNAL r32
square_root(r32 val)
{
    r32 result = 0.0f;

    result = sqrt(val);

    return result;
}

INTERNAL r32
lerp(r32 start, r32 end, r32 p)
{
  r32 result = 0.0f;

  result = ((end - start) * p) + start;

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
