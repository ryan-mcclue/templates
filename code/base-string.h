// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_STRING_H)
#define BASE_STRING_H

#define STB_SPRINTF_IMPLEMENTATION 1
#include "stb/stb_sprintf.h"

typedef struct String8 String8;
struct String8
{
  u8 *str;
  u64 size;
};

typedef u32 MATCH_FLAGS;
typedef u32 S8_MATCH_FLAGS;
enum
{
  MATCH_FLAG_FIND_LAST = (1 << 0),
};
enum
{
  S8_MATCH_FLAG_RIGHT_SIDE_LAZY = (1 << 4),
  S8_MATCH_FLAG_CASE_INSENSITIVE = (1 << 5),
};

typedef struct String8Node String8Node;
struct String8Node
{
  String8Node *next;
  String8 string;
};

typedef struct String8List String8List;
struct String8List
{
  String8 *first;
  String8 *last;
  u64 node_count;
  u64 total_size;
};

INTERNAL String8
s8(u8 *str, u64 size)
{
  String8 result = ZERO_STRUCT;

  result.str = str;
  result.size = size;

  return result;
}

#define s8_lit(s) s8((u8 *)(s), sizeof(s) - 1)
#define s8_cstring(s) s8((u8 *)(s), strlen(s))
// IMPORTANT(Ryan): When substringing will run into situations where not null terminated.
// So, use like: "%.*s", s8_varg(string)
#define s8_varg(s) (int)(s).size, (s).str

INTERNAL String8
s8_up_to(u8 *start, u8 *up_to)
{
  String8 string = ZERO_STRUCT;

  string.str = first;
  string.size = (u64)(up_to - first);

  return string;
}

INTERNAL String8
s8_substring(String8 str, u64 start, u64 end)
{
  if (end > str.size)
  {
    end = str.size;
  }

  if (start > str.size)
  {
    start = str.size;
  }

  if (start > end)
  {
    SWAP(u64, start, end);
  }

  str.size = end - start;
  str.str += start;

  return str;
}

INTERNAL String8
s8_advance(String8 str, u64 advance)
{
  return s8_substring(str, advance, str.size);
}

INTERNAL String8
s8_chop(MD_String8 str, u64 chop)
{
  return s8_substring(str, 0, str.size - chop);
}

INTERNAL String8
s8_prefix(String8 str, u64 prefix)
{
  return s8_substring(str, 0, prefix);
}

INTERNAL String8
s8_suffix(String8 str, u64 suffix)
{
  return s8_substring(str, str.size - suffix, str.size);
}

INTERNAL b32
s8_match(String8 a, String8 b, S8_MATCH_FLAGS flags)
{
  b32 result = false;

  if (a.size == b.size || flags & S8_MATCH_FLAG_RIGHT_SIDE_LAZY)
  {
    result = true;

    for (u64 i = 0; i < a.size && i < b.size; i += 1)
    {
      b32 match = (a.str[i] == b.str[i]);

      if (flags & S8_MATCH_FLAG_CASE_INSENSITIVE)
      {
        match |= (tolower(a.str[i]) == tolower(b.str[i]));
      }

      if (!match)
      {
        result = false;
        break;
      }
    }
  }

  return result;
}

INTERNAL u64
s8_find_substring(String8 str, String8 substring, u64 start_pos, MATCH_FLAGS flags)
{
  b32 found = false;
  u64 found_idx = str.size;

  for (u64 i = start_pos; i < str.size; i += 1)
  {
    if (i + substring.size <= str.size)
    {
      String8 substr_from_str = s8_substring(str, i, i + substring.size);

      if (s8_match(substr_from_str, substring, flags))
      {
        found_idx = i;
        found = true;

        if (!(flags & MATCH_FLAG_FIND_LAST))
        {
          break;
        }
      }
    }
  }

  return found_idx;
}

INTERNAL String8
s8_copy(MemArena *arena, String8 string)
{
  String8 result = ZERO_STRUCT;

  result.size = string.size;
  result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, string.size + 1);
  MEMORY_COPY(result.str, string.str, string.size);
  result.str[string.size] = '\0';

  return result;
}

INTERNAL String8
s8_fmt(MemArena *arena, char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  String8 result = ZERO_STRUCT;
  u64 needed_bytes = (u64)stbsp_vsnprintf(NULL, 0, fmt, args) + 1;
  result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, needed_bytes);
  result.size = needed_bytes - 1;
  result.str[needed_bytes - 1] = '\0';
  stbsp_vsnprintf((char *)result.str, (int)needed_bytes, fmt, args);

  va_end(args);

  return result;
}

INTERNAL void
s8_list_push(MemArena *arena, String8List *list, String8 string)
{
  String8Node *node = MD_PushArrayZero(arena, String8Node, 1);
  node->string = string;

  MD_QueuePush(list->first, list->last, node);
  list->node_count += 1;
  list->total_size += string.size;
}

INTERNAL void
MD_S8ListPushFmt(MemArena *arena, String8List *list, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String8 string = MD_S8FmtV(arena, fmt, args);
    va_end(args);
    MD_S8ListPush(arena, list, string);
}

INTERNAL void
MD_S8ListConcat(String8List *list, String8List *to_push)
{
    if(to_push->first)
    {
        list->node_count += to_push->node_count;
        list->total_size += to_push->total_size;
        
        if(list->last == 0)
        {
            *list = *to_push;
        }
        else
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
        }
    }
    MD_MemoryZeroStruct(to_push);
}

INTERNAL String8List
MD_S8Split(MemArena *arena, String8 string, int splitter_count,
           String8 *splitters)
{
    String8List list = MD_ZERO_STRUCT;
    
    MD_u64 split_start = 0;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        MD_b32 was_split = 0;
        for(int split_idx = 0; split_idx < splitter_count; split_idx += 1)
        {
            MD_b32 match = 0;
            if(i + splitters[split_idx].size <= string.size)
            {
                match = 1;
                for(MD_u64 split_i = 0; split_i < splitters[split_idx].size && i + split_i < string.size; split_i += 1)
                {
                    if(splitters[split_idx].str[split_i] != string.str[i + split_i])
                    {
                        match = 0;
                        break;
                    }
                }
            }
            if(match)
            {
                String8 split_string = MD_S8(string.str + split_start, i - split_start);
                MD_S8ListPush(arena, &list, split_string);
                split_start = i + splitters[split_idx].size;
                i += splitters[split_idx].size - 1;
                was_split = 1;
                break;
            }
        }
        
        if(was_split == 0 && i == string.size - 1)
        {
            String8 split_string = MD_S8(string.str + split_start, i+1 - split_start);
            MD_S8ListPush(arena, &list, split_string);
            break;
        }
    }
    
    return list;
}

INTERNAL String8
MD_S8ListJoin(MemArena *arena, String8List list, MD_StringJoin *join_ptr)
{
    // setup join parameters
    MD_StringJoin join = MD_ZERO_STRUCT;
    if (join_ptr != 0)
    {
        MD_MemoryCopy(&join, join_ptr, sizeof(join));
    }
    
    // calculate size & allocate
    MD_u64 sep_count = 0;
    if (list.node_count > 1)
    {
        sep_count = list.node_count - 1;
    }
    String8 result = MD_ZERO_STRUCT;
    result.size = (list.total_size + join.pre.size +
                   sep_count*join.mid.size + join.post.size);
    result.str = MD_PushArrayZero(arena, MD_u8, result.size);
    
    // fill
    MD_u8 *ptr = result.str;
    MD_MemoryCopy(ptr, join.pre.str, join.pre.size);
    ptr += join.pre.size;
    for(String8Node *node = list.first; node; node = node->next)
    {
        MD_MemoryCopy(ptr, node->string.str, node->string.size);
        ptr += node->string.size;
        if (node != list.last)
        {
            MD_MemoryCopy(ptr, join.mid.str, join.mid.size);
            ptr += join.mid.size;
        }
    }
    MD_MemoryCopy(ptr, join.post.str, join.post.size);
    ptr += join.post.size;
    
    return(result);
}

INTERNAL String8
MD_S8ListJoinMid(MemArena *arena, String8List list,
                 String8 mid_separator)
{
    MD_StringJoin join = MD_ZERO_STRUCT;
    join.pre = MD_S8Lit("");
    join.post = MD_S8Lit("");
    join.mid = mid_separator;
    String8 result = MD_S8ListJoin(arena, list, &join);
    return result;
}


#endif
