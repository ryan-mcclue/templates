// SPDX-License-Identifier: zlib-acknowledgement

#if !defined(BASE_MAP_H)
#define BASE_MAP_H

typedef struct MapKey MapKey;
struct MapKey
{
  u64 hash;
  // size is used to determine if original key was a string?
  u64 size;
  // the ptr is the original key value?
  void *ptr;
};

// IMPORTANT(Ryan): Using chaining
typedef struct MapSlot MapSlot;
struct MapSlot
{
  MapSlot *next;
  MapKey key;
  void *val;
};

typedef struct MapBucket MapBucket;
struct MapBucket
{
  MapSlot *first;
  MapSlot *last;
};

typedef struct Map Map;
struct Map
{
  MapBucket *buckets;
  u64 bucket_count;
};

INTERNAL u64
map_hash_str(String8 string)
{
  u64 result = 5381;

  for (u64 i = 0; i < string.size; i += 1)
  {
    result = ((result << 5) + result) + string.str[i];
  }

  return result;
}

INTERNAL MapKey
map_key_str(String8 string)
{
  MapKey result = ZERO_STRUCT;

  if (string.size != 0)
  {
    result.hash = map_hash_str(string);
    result.size = string.size;
    if (string.size > 0)
    {
      result.ptr = string.str;
    }
  }

  return result;
}

INTERNAL u64 
map_hash_ptr(void *p)
{
  u64 h = (u64)p;

  h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9;
  h = (h ^ (h >> 27)) * 0x94d049bb133111eb;
  h = h ^ (h >> 31);

  return h;
}

INTERNAL MapKey
map_key_ptr(void *ptr)
{
  MapKey result = ZERO_STRUCT;

  if (ptr != NULL)
  {
    result.hash = map_hash_ptr(ptr);
    result.size = 0;
    result.ptr = ptr;
  }

  return result;
}

INTERNAL Map
map_create(MemArena *arena, u64 bucket_count)
{
  Map result = ZERO_STRUCT;

  result.bucket_count = bucket_count;
  result.buckets = MEM_ARENA_PUSH_ARRAY_ZERO(arena, MapBucket, bucket_count);

  return result;
}

INTERNAL MapSlot *
map_scan(MapSlot *first_slot, MapKey key)
{
  MapSlot *result = NULL;

  if (first_slot != NULL)
  {
    b32 ptr_kind = (key.size == 0);
    String8 key_string = s8((u8 *)key.ptr, key.size);
    for (MapSlot *slot = first_slot; slot != NULL; slot = slot->next)
    {
      if (slot->key.hash == key.hash)
      {
        if (ptr_kind)
        {
          if (slot->key.size == 0 && slot->key.ptr == key.ptr)
          {
            result = slot;
            break;
          }
        }
        else
        {
          String8 slot_string = s8((u8 *)slot->key.ptr, slot->key.size);
          if (s8_match(slot_string, key_string, 0))
          {
            result = slot;
            break;
          }
        }
      }
    }
  }

  return result;
}

INTERNAL MapSlot *
map_lookup(Map *map, MapKey key)
{
  MapSlot *result = NULL;

  if (map->bucket_count > 0)
  {
    u64 index = key.hash % map->bucket_count;
    result = map_scan(map->buckets[index].first, key);
  }

  return result;
}


INTERNAL MapSlot *
map_insert(MemArena *arena, Map *map, MapKey key, void *val)
{
  MapSlot *result = NULL;

  if (map->bucket_count > 0)
  {
    u64 index = key.hash % map->bucket_count;
    MapSlot *slot = MEM_ARENA_PUSH_ARRAY(arena, MapSlot, 1);
    MapBucket *bucket = &map->buckets[index];
    SLL_QUEUE_PUSH(bucket->first, bucket->last, slot);
    slot->key = key;
    slot->val = val;
    result = slot;
  }

  return result;
}

INTERNAL MapSlot *
map_overwrite(MemArena *arena, Map *map, MapKey key, void *val)
{
  MapSlot *result = map_lookup(map, key);

  if (result != NULL)
  {
    result->val = val;
  }
  else
  {
    result = map_insert(arena, map, key, val);
  }

  return result;
}

// have to know to cast to particular type
//map_insert(arena, &map, map_key_from_str(node->string), (void*)(u64)eval_result);

#endif
