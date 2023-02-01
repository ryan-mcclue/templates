// SPDX-License-Identifier: zlib-acknowledgement

#pragma once

#include <string.h>

#define MEMORY_ZERO(p, n) memset((p), 0, (n))
#define MEMORY_ZERO_STRUCT(p) MEMORY_ZERO((p), sizeof(*(p)))
#define MEMORY_ZERO_ARRAY(a) MEMORY_ZERO((a), sizeof(a[0]))

#define MEMORY_COPY(d, s, n) memmove((d), (s), (n))
#define MEMORY_COPY_STRUCT(d, s, n) MEMORY_COPY((d), (s), sizeof(*(s)))
#define MEMORY_COPY_ARRAY(d, s, n) MEMORY_COPY((d), (s), sizeof((s)))

#define MEMORY_MATCH(a, b, n) (memcmp((a), (b), (n)) == 0)

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) (((u64)x) << 30)
#define TB(x) (((u64)x) << 40)
