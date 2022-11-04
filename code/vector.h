// SPDX-License-Identifier: zlib-acknowledgement
#pragma once

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

typedef u32 v2u __attribute__((VECTOR_SIZE(2, u32)));
typedef union V2u
{
  v2u vec;
  struct
  {
    u32 x, y;
  };
  u32 e[2];
} V2u;

typedef s32 v2 __attribute__((VECTOR_SIZE(2, s32)));
typedef union V2s
{
  v2s vec;
  struct
  {
    s32 x, y;
  };
  s32 e[2];
} V2s;

typedef r32 v3 __attribute__((VECTOR_SIZE(3, r32)));
typedef union V3
{
  v3 vec;
  struct
  {
    r32 x, y, z;
  };
  struct
  {
    r32 r, g, b;
  };
  r32 e[3];
} V3;

typedef u32 v3 __attribute__((VECTOR_SIZE(3, u32)));
typedef union V3u
{
  v3u vec;
  struct
  {
    u32 x, y, z;
  };
  struct
  {
    u32 r, g, b;
  };
  u32 e[3];
} V3u;

typedef s32 v3 __attribute__((VECTOR_SIZE(3, s32)));
typedef union V3s
{
  v3s vec;
  struct
  {
    s32 x, y, z;
  };
  struct
  {
    s32 r, g, b;
  };
  s32 e[3];
} V3s;

typedef r32 v4 __attribute__((VECTOR_SIZE(4, r32)));
typedef union V4
{
  v4 vec;
  struct
  {
    r32 x, y, z, w;
  };
  struct
  {
    r32 r, g, b, a;
  };
  r32 e[4];
} V4;

typedef u32 v4 __attribute__((VECTOR_SIZE(4, u32)));
typedef union V4u
{
  v4u vec;
  struct
  {
    u32 x, y, z, w;
  };
  struct
  {
    u32 r, g, b, a;
  };
  u32 e[4];
} V4u;

typedef s32 v4 __attribute__((VECTOR_SIZE(4, s32)));
typedef union V4s
{
  v4s vec;
  struct
  {
    s32 x, y, z, w;
  };
  struct
  {
    s32 r, g, b, a;
  };
  s32 e[4];
} V4s;
