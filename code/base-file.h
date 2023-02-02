// SPDX-License-Identifier: zlib-acknowledgement
#if !defined(BASE_FILE_H)
#define BASE_FILE_H

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
	  fwrite(data.str, 1, data.size, file);
	  fclose(file);
  }
}

INTERNAL void
s8_append_to_file(String8 file_name, String8 data)
{
	FILE *file = fopen((char *)file_name.str, "a");

  if (file != NULL)
  {
	  fwrite(data.str, 1, data.size, file);
	  fclose(file);
  }
}

INTERNAL void
s8_copy_file(MemArena *arena, String8 source_file, String8 dest_file)
{
  String8 source_file_data = s8_read_entire_file(arena, source_file);
  s8_write_entire_file(dest_file, source_file_data);
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
#endif
