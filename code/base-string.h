// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_STRING_H)
#define BASE_STRING_H

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
// Use like: \"%.*s\"", s8_varg(string)
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

INTERNAL MD_String8
MD_S8Fmt(MD_Arena *arena, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MD_String8 result = MD_S8FmtV(arena, fmt, args);
    va_end(args);
    return result;
}

INTERNAL MD_String8
MD_S8FmtV(MD_Arena *arena, char *fmt, va_list args)
{
    MD_String8 result = MD_ZERO_STRUCT;
    va_list args2;
    va_copy(args2, args);
    MD_u64 needed_bytes = MD_IMPL_Vsnprintf(0, 0, fmt, args)+1;
    result.str = MD_PushArray(arena, MD_u8, needed_bytes);
    result.size = needed_bytes - 1;
    result.str[needed_bytes-1] = 0;
    MD_IMPL_Vsnprintf((char*)result.str, (int)needed_bytes, fmt, args2);
    return result;
}


INTERNAL void
MD_S8ListPush(MD_Arena *arena, MD_String8List *list, MD_String8 string)
{
    MD_String8Node *node = MD_PushArrayZero(arena, MD_String8Node, 1);
    node->string = string;
    
    MD_QueuePush(list->first, list->last, node);
    list->node_count += 1;
    list->total_size += string.size;
}

INTERNAL void
MD_S8ListPushFmt(MD_Arena *arena, MD_String8List *list, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MD_String8 string = MD_S8FmtV(arena, fmt, args);
    va_end(args);
    MD_S8ListPush(arena, list, string);
}

INTERNAL void
MD_S8ListConcat(MD_String8List *list, MD_String8List *to_push)
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

INTERNAL MD_String8List
MD_S8Split(MD_Arena *arena, MD_String8 string, int splitter_count,
           MD_String8 *splitters)
{
    MD_String8List list = MD_ZERO_STRUCT;
    
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
                MD_String8 split_string = MD_S8(string.str + split_start, i - split_start);
                MD_S8ListPush(arena, &list, split_string);
                split_start = i + splitters[split_idx].size;
                i += splitters[split_idx].size - 1;
                was_split = 1;
                break;
            }
        }
        
        if(was_split == 0 && i == string.size - 1)
        {
            MD_String8 split_string = MD_S8(string.str + split_start, i+1 - split_start);
            MD_S8ListPush(arena, &list, split_string);
            break;
        }
    }
    
    return list;
}

INTERNAL MD_String8
MD_S8ListJoin(MD_Arena *arena, MD_String8List list, MD_StringJoin *join_ptr)
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
    MD_String8 result = MD_ZERO_STRUCT;
    result.size = (list.total_size + join.pre.size +
                   sep_count*join.mid.size + join.post.size);
    result.str = MD_PushArrayZero(arena, MD_u8, result.size);
    
    // fill
    MD_u8 *ptr = result.str;
    MD_MemoryCopy(ptr, join.pre.str, join.pre.size);
    ptr += join.pre.size;
    for(MD_String8Node *node = list.first; node; node = node->next)
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

INTERNAL MD_String8
MD_S8ListJoinMid(MD_Arena *arena, MD_String8List list,
                 MD_String8 mid_separator)
{
    MD_StringJoin join = MD_ZERO_STRUCT;
    join.pre = MD_S8Lit("");
    join.post = MD_S8Lit("");
    join.mid = mid_separator;
    MD_String8 result = MD_S8ListJoin(arena, list, &join);
    return result;
}


INTERNAL String8 
s8_read_entire_file(MemArena *arena, String8 file_name)
{
  String8 result = ZERO_STRUCT;

  FILE *file = fopen((char *)file_name.str, "rb");

  if (file != NULL)
  {
    fseek(file, 0, SEEK_END);
    u64 file_size = (u64)ftell(file);
    fseek(file, 0, SEEK_SET);
    result.str = MEM_ARENA_PUSH_ARRAY(arena, u8, file_size + 1);
    if (result.str != NULL)
    {
      result.size = file_size;
      fread(result.str, 1, file_size, file);
      result.str[result.size] = '\0';
    }
    fclose(file);
  }

  return result;
}

INTERNAL void
s8_write_entire_file(String8 file_name, String8 data)
{
	FILE *file = fopen((char *)file_name.str, "w+");

  if (file != NULL)
  {
	  fputs((char *)data.str, file);
	  fclose(file);
  }
}

INTERNAL void
s8_append_to_file(String8 file_name, String8 data)
{
	FILE *file = fopen((char *)file_name.str, "a");

  if (file != NULL)
  {
	  fputs((char *)data.str, file);
	  fclose(file);
  }
}

#if 0
INTERNAL b32
os_file_rename(String8 og_name, String8 new_name){
    // convert name
    M_ArenaTemp scratch = m_get_scratch(0, 0);
    String16 og_name16 = str16_from_str8(scratch.arena, og_name);
    String16 new_name16 = str16_from_str8(scratch.arena, new_name);
    // rename file
    B32 result = MoveFileW((WCHAR*)og_name16.str, (WCHAR*)new_name16.str);
    m_release_scratch(scratch);
    return(result);
}

typedef MD_u32 MD_FileFlags;
enum
{
    MD_FileFlag_Directory = (1<<0),
};

typedef U32 DataAccessFlags;
enum{
  DataAccessFlag_Read    = (1 << 0),
  DataAccessFlag_Write   = (1 << 1),
  DataAccessFlag_Execute = (1 << 2),
};

typedef struct MD_FileInfo MD_FileInfo;
struct MD_FileInfo
{
    MD_FileFlags flags;
    MD_String8 filename;
    MD_u64 file_size;
  u64 create_time;
  u64 modify_time;
  DataAccessFlags access;
};

typedef struct MD_FileIter MD_FileIter;
struct MD_FileIter
{
    // This is opaque state to store OS-specific file-system iteration data.
    MD_u8 opaque[640];
};

// b32 file_start = file_iter_begin(&file_iter, path);
// FileInfo file_info = file_iter_next(&file_iter);
typedef struct MD_LINUX_FileIter MD_LINUX_FileIter;
struct MD_LINUX_FileIter
{
    int dir_fd;
    DIR *dir;
};
MD_StaticAssert(sizeof(MD_LINUX_FileIter) <= sizeof(MD_FileIter), file_iter_size_check);

static MD_b32
MD_LINUX_FileIterIncrement(MD_Arena *arena, MD_FileIter *opaque_it, MD_String8 path,
                           MD_FileInfo *out_info)
{
    MD_b32 result = 0;
    
    MD_LINUX_FileIter *it = (MD_LINUX_FileIter *)opaque_it;
    if(it->dir == 0)
    {
        it->dir = opendir((char*)path.str);
        it->dir_fd = open((char *)path.str, O_PATH|O_CLOEXEC);
    }
    
    if(it->dir != 0 && it->dir_fd != -1)
    {
        struct dirent *dir_entry = readdir(it->dir);
        if(dir_entry)
        {
            out_info->filename = MD_S8Fmt(arena, "%s", dir_entry->d_name);
            out_info->flags = 0;
            
            struct stat st; 
            if(fstatat(it->dir_fd, dir_entry->d_name, &st, AT_NO_AUTOMOUNT|AT_SYMLINK_NOFOLLOW) == 0)
            {
                if((st.st_mode & S_IFMT) == S_IFDIR)
                {
                    out_info->flags |= MD_FileFlag_Directory;
                }
                out_info->file_size = st.st_size;
            }
            result = 1;
        }
    }
    
    if(result == 0)
    {
        if(it->dir != 0)
        {
            closedir(it->dir);
            it->dir = 0;
        }
        if(it->dir_fd != -1)
        {
            close(it->dir_fd);
            it->dir_fd = -1;
        }
    }
    
    return result;
}

#if 0
internal b32
LinuxCopyFile(char *dest, char *source)
{
	FILE *source_fp;
	fseek(source_fp, 0, SEEK_END);
	u32 fsize = ftell(source_fp);
	rewind(source_fp);

	char *source_data = malloc(fsize + 1);
	fread(source_data, 1, fsize, source_fp);
	fclose(source_fp);

	FILE *dest_fp;
	dest_fp = fopen(dest, "w+");
	fputs(dest_fp, source_data);
	fclose(dest_fp);

	// Return 0 on error
	return 1;
}

function FileProperties
os_file_properties(String8 file_name){

linux_delete_file();

INTERNAL b32
linux_create_directory(String8 directory_name)
{
	if (mkdir(path) == 0)
	{
		return 1;
	}
	return 0;
}

INTERNAL b32
LinuxDoesFileExist(char *path)
{
	// This probably isn't the best way
	if (access(path, F_OK) != -1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

internal b32
LinuxDoesDirectoryExist(char *path)
{
	return LinuxDoesFileExist(path);
}

#endif
#endif

typedef struct String8List String8List;
struct String8List
{
  String8 *first;
  String8 *last;
  u64 node_count;
  u64 total_size;
};

#endif
