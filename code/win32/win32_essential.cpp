///////////////////////////////
/// NOTE(adam): OS Includes

#include <windows.h>

//////////////////////////////////////
///// NOTE(adam): Variables

global DWORD win32_thread_context_index = 0;
global u64 w32_ticks_per_second = 1;

global M_Arena w32_perm_arena = {};
global String8 w32_binary_path = {};

//////////////////////////////////////
///// NOTE(adam): Setup

function void
os_init(void) {
  // setup thread context
  win32_thread_context_index = TlsAlloc();

  // setup precision time
  LARGE_INTEGER perf_freq = {};
  if (QueryPerformanceFrequency(&perf_freq)) {
    w32_ticks_per_second = perf_freq.QuadPart;
  } 
  timeBeginPeriod(1);

  // arena
  w32_perm_arena = m_make_arena(os_base_memory());

  // TODO(adam): setup main thread's context here
  // so we can init file paths and stuff.
}

//////////////////////////////////////
///// NOTE(adam): Memory Functions

function void*
os_memory_reserve(u64 size) {
  void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
  return result;
}

function void
os_memory_commit(void *ptr, u64 size) {
  VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

function void
os_memory_decommit(void *ptr, u64 size) {
  VirtualFree(ptr, size, MEM_DECOMMIT);
}

function void
os_memory_release(void *ptr, u64 size) {
  VirtualFree(ptr, size, MEM_RELEASE);
}

//////////////////////////////////////
///// NOTE(adam): Thread Context

function void
os_thread_context_set(void *ptr) {
 TlsSetValue(win32_thread_context_index, ptr); 
}

function void*
os_thread_context_get(void) {
  void *result = TlsGetValue(win32_thread_context_index);
  return result;
}

//////////////////////////////////////
///// NOTE(adam): Time

function DateTime
w32_date_time_from_system_time(SYSTEMTIME *in) {
  DateTime result = {};
  result.year = in->wYear;
  result.mon= (u8)in->wMonth;
  result.day = (u8)(in->wDay - 1);
  result.hour = (u8)in->wHour;
  result.min= (u8)in->wMinute;
  result.sec= (u8)in->wSecond;
  result.msec = in->wMilliseconds;
  return result;
}

function SYSTEMTIME
w32_system_time_from_date_time(DateTime *in) {
  SYSTEMTIME result = {};
  result.wYear = (WORD)in->year;
  result.wMonth = in->mon;
  result.wDay = in->day + 1;
  result.wHour = in->hour;
  result.wMinute = in->min;
  result.wSecond = in->sec;
  result.wMilliseconds = in->msec;
  return result;
}

function DenseTime
w32_dense_time_from_file_time(FILETIME *file_time) {
  SYSTEMTIME system_time = {};
  FileTimeToSystemTime(file_time, &system_time);
  DateTime date_time = w32_date_time_from_system_time(&system_time);
  DenseTime result = dense_time_from_date_time(&date_time);

  return result;
}

function DateTime
os_now_universal_time(void) {
  SYSTEMTIME system_time = {};
  GetSystemTime(&system_time);
  DateTime result = w32_date_time_from_system_time(&system_time);
  return result;
}

function DateTime
os_local_time_from_universal(DateTime *univ_date_time) {
  SYSTEMTIME univ_system_time = w32_system_time_from_date_time(univ_date_time);
  FILETIME univ_file_time = {};
  SystemTimeToFileTime(&univ_system_time, &univ_file_time);
  FILETIME local_file_time = {};
  FileTimeToLocalFileTime(&univ_file_time, &local_file_time);
  SYSTEMTIME local_system_time = {};
  FileTimeToSystemTime(&local_file_time, &local_system_time);
  DateTime result = w32_date_time_from_system_time(&local_system_time);
  return result;
}

function DateTime
os_universal_time_from_local(DateTime *local_date_time) {
  SYSTEMTIME local_system_time = w32_system_time_from_date_time(local_date_time);
  FILETIME local_file_time = {};
  SystemTimeToFileTime(&local_system_time, &local_file_time);
  FILETIME univ_file_time = {};
  LocalFileTimeToFileTime(&local_file_time, &univ_file_time);
  SYSTEMTIME univ_system_time = {};
  FileTimeToSystemTime(&univ_file_time, &univ_system_time);
  DateTime result = w32_date_time_from_system_time(&univ_system_time);
  return result;
}

function u64
os_now_microseconds(void) {
  u64 result = 0;
  LARGE_INTEGER perf_counter = {};
  if (QueryPerformanceCounter(&perf_counter)) {
    u64 ticks = perf_counter.QuadPart;
    result = ticks*Million(1)/w32_ticks_per_second;
  }

  return result;
}

function void
os_sleep_milliseconds(u64 t) {
  Sleep((DWORD)t);
}

//////////////////////////////////////
///// NOTE(adam): File Handling

function String8
os_file_read(M_Arena *arena, String8 file_name) {
  // get handle
  M_Scratch scratch(arena);
  String16 file_name16 = str16_from_str8(scratch, file_name);
  HANDLE file = CreateFileW((WCHAR*)file_name16.str,
                            GENERIC_READ, 0, 0,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                            0);

  String8 result = {};
  if (file != INVALID_HANDLE_VALUE) {
    // get size
    DWORD hi_size = 0;
    DWORD lo_size = GetFileSize(file, &hi_size);
    u64 total_size = (((u64)hi_size) << 32) | (u64)lo_size;

    // allocate buffer
    M_Temp restore_point = m_begin_temp(arena);
    u8 *buffer = push_array(arena, u8, total_size);

    // read
    u8 *ptr = buffer;
    u8 *opl = buffer + total_size;
    b32 success = true;
    for (;ptr < opl;) {
      u64 total_to_read = (u64)(opl - ptr);
      DWORD to_read = (DWORD)total_to_read;
      if (total_to_read > max_u32) {
        to_read = max_u32;
      }
      DWORD actual_read = 0;
      if (!ReadFile(file, ptr, to_read, &actual_read, 0)) {
        success = false;
        break;
      }

      ptr += actual_read;
    }

    // set result or reset memory
    if (success) {
      result.str = buffer;
      result.size = total_size;
    } else {
      m_end_temp(restore_point);
    } 

    CloseHandle(file);
  }

  return result;
}

function u8*
os_file_read_binary(M_Arena *arena, String8 file_name, u64 *bytes_read) {
  // get handle
  M_Scratch scratch(arena);
  String16 file_name16 = str16_from_str8(scratch, file_name);
  HANDLE file = CreateFileW((WCHAR*)file_name16.str,
                            GENERIC_READ, 0, 0,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                            0);

  u8 *result = 0;
  if (file != INVALID_HANDLE_VALUE) {
    // get size
    DWORD hi_size = 0;
    DWORD lo_size = GetFileSize(file, &hi_size);
    u64 total_size = (((u64)hi_size) << 32) | (u64)lo_size;

    // allocate buffer
    M_Temp restore_point = m_begin_temp(arena);
    u8 *buffer = push_array(arena, u8, total_size);

    // read
    u8 *ptr = buffer;
    u8 *opl = buffer + total_size;
    b32 success = true;
    for (;ptr < opl;) {
      u64 total_to_read = (u64)(opl - ptr);
      DWORD to_read = (DWORD)total_to_read;
      if (total_to_read > max_u32) {
        to_read = max_u32;
      }
      DWORD actual_read = 0;
      if (!ReadFile(file, ptr, to_read, &actual_read, 0)) {
        success = false;
        break;
      }

      ptr += actual_read;
    }

    // set result or reset memory
    if (success) {
      result = buffer;
      *bytes_read = total_size;
    } else {
      m_end_temp(restore_point);
    } 

    CloseHandle(file);
  }

  return result;
}

function b32
os_file_write(String8 file_name, String8List data) {
  // get handle
  M_Scratch scratch;
  String16 file_name16 = str16_from_str8(scratch, file_name);
  HANDLE file = CreateFileW((WCHAR*)file_name16.str,
                            GENERIC_WRITE, 0, 0,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                            0);

  b32 result = false;
  if (file != INVALID_HANDLE_VALUE) {
    result = true;

    for (String8Node *node = data.first;
         node != 0;
         node = node->next) {
      u8 *ptr = node->string.str;
      u8 *opl = ptr + node->string.size;
      for (;ptr < opl;) {
        u64 total_to_write = (u64)(opl - ptr);
        DWORD to_write = (DWORD)total_to_write;
        if (total_to_write > max_u32) {
          to_write = max_u32;
        }
        DWORD actual_write = 0;
        if (!WriteFile(file, ptr, to_write, &actual_write, 0)) {
          result = false;
          goto dbl_break;
        }
        ptr += actual_write;
      }
    }
    dbl_break:;

    CloseHandle(file);
  }

  return result;
}

function b32
os_file_write_binary(String8 file_name, u8 *data, u64 size) {
  // get handle
  M_Scratch scratch;
  String16 file_name16 = str16_from_str8(scratch, file_name);
  HANDLE file = CreateFileW((WCHAR*)file_name16.str,
                            GENERIC_WRITE, 0, 0,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                            0);

  b32 result = false;
  if (file != INVALID_HANDLE_VALUE) {
    result = true;

    u8 *ptr = data;
    u8 *opl = ptr + size;

    for (;ptr < opl;) {
      u64 total_to_write = (u64)(opl - ptr);
      DWORD to_write = (DWORD)total_to_write;
      if (total_to_write > max_u32) {
        to_write = max_u32;
      }
      DWORD actual_write = 0;
      if (!WriteFile(file, ptr, to_write, &actual_write, 0)) {
        result = false;
        break;
      }
      ptr += actual_write;
    }

    CloseHandle(file);
  }

  return result;
}

function FilePropertyFlags
w32_file_property_flags_from_attributes(DWORD attribs) {
  FilePropertyFlags result = 0;
  if (attribs & FILE_ATTRIBUTE_DIRECTORY) {
    result |= FilePropertyFlag_Directory;
  }

  return result;
}

function DataAccessFlags
w32_access_from_attributes(DWORD attribs) {
  FilePropertyFlags result = 0;
  if (!(attribs & FILE_ATTRIBUTE_READONLY)) {
    result |= DataAccessFlag_Write;
  }

  return result;
}

function FileProperties
os_file_properties(String8 file_name) {
  // convert name
  M_Scratch stratch;
  String16 file_name16 = str16_from_str8(stratch, file_name);

  // get attributes and convert to properties
  FileProperties result = {};
  WIN32_FILE_ATTRIBUTE_DATA attribs = {};
  if (GetFileAttributesExW((WCHAR*)file_name16.str, GetFileExInfoStandard,
                           &attribs)) {
    result.size = ((u64)attribs.nFileSizeHigh << 32) | (u64)attribs.nFileSizeLow;
    result.flags = w32_file_property_flags_from_attributes(attribs.dwFileAttributes);
    result.create_time = w32_dense_time_from_file_time(&attribs.ftCreationTime);
    result.modify_time = w32_dense_time_from_file_time(&attribs.ftLastWriteTime);
    result.access = w32_access_from_attributes(attribs.dwFileAttributes);
  }

  return result;
}

function b32
os_file_delete(String8 file_name) {
  // convert name
  M_Scratch stratch;
  String16 file_name16 = str16_from_str8(stratch, file_name);

  // delete file
  b32 result = DeleteFileW((WCHAR*)file_name16.str);

  return result;
}

function b32 
os_file_rename(String8 og_name, String8 new_name) {
  // convert names
  M_Scratch stratch;
  String16 og_name16 = str16_from_str8(stratch, og_name);
  String16 new_name16 = str16_from_str8(stratch, new_name);

  // rename file
  b32 result = MoveFileW((WCHAR*)og_name16.str, (WCHAR*)new_name16.str);

  return result;
}

function b32 
os_file_make_directory(String8 path) {
  // convert name
  M_Scratch stratch;
  String16 path16 = str16_from_str8(stratch, path);

  // make directory
  b32 result = CreateDirectoryW((WCHAR*)path16.str, 0);

  return result;
}

function b32 
os_file_delete_directory(String8 path) {
  // convert name
  M_Scratch stratch;
  String16 path16 = str16_from_str8(stratch, path);

  // delete directory
  b32 result = RemoveDirectoryW((WCHAR*)path16.str);

  return result;
}

function OS_FileIter 
os_file_iter_init(String8 path) {
  OS_FileIter result = {};
  // convert name
  String8Node nodes[2];
  String8List list = {};
  str8_list_push_explicit(&list, path, nodes + 0);
  str8_list_push_explicit(&list, str8_lit("\\*"), nodes + 1);

  M_Scratch scratch;
  String8 path_star = str8_join(scratch, &list, 0);
  // TODO(adam): make unicode conversion take string lists.
  String16 path16 = str16_from_str8(scratch, path_star);

  // init iterator
  W32_FileIter *w32_iter = (W32_FileIter*)&result;
  WIN32_FIND_DATAW find_data = {};
  w32_iter->handle = FindFirstFileW((WCHAR*)path16.str, &w32_iter->find_data);
  if (w32_iter->handle != INVALID_HANDLE_VALUE) {
    w32_iter->done = false;
  }

  return result;
}

function b32
os_file_iter_next(M_Arena *arena, OS_FileIter *iter,
                                String8 *name_out, FileProperties *prop_out) {
  b32 result = false;

  W32_FileIter *w32_iter = (W32_FileIter*)iter;
  if (w32_iter->handle != 0 &&
      w32_iter->handle != INVALID_HANDLE_VALUE) {
    for (;!w32_iter->done;) {
      // skip . and ..
      WCHAR *file_name = w32_iter->find_data.cFileName;
      b32 is_dot = file_name[0] == '.' &&
                   file_name[1] == 0;
      b32 is_dotdot = file_name[0] == '.' &&
                      file_name[1] == '.' &&
                      file_name[2] == 0;

      // setup to emit
      b32 emit = (!is_dot && !is_dotdot);
      WIN32_FIND_DATAW data = {};
      if (emit) {
        MemoryCopyStruct(&data, &w32_iter->find_data);
      }

      // increment the iterator
      if (!FindNextFileW(w32_iter->handle, &w32_iter->find_data)) {
        w32_iter->done = true;
      }

      // do the emit if we saved one earlier
      if (emit) {
        *name_out = str8_from_str16(arena, str16_cstring((u16*)data.cFileName));

        // get properties
        prop_out->size = ((u64)data.nFileSizeHigh << 32) | (u64)data.nFileSizeLow;
        prop_out->flags = w32_file_property_flags_from_attributes(data.dwFileAttributes);
        prop_out->create_time = w32_dense_time_from_file_time(&data.ftCreationTime);
        prop_out->modify_time = w32_dense_time_from_file_time(&data.ftLastWriteTime);
        prop_out->access = w32_access_from_attributes(data.dwFileAttributes);

        result = true;
        break;
      }
    }
  }

  return result;
}

function void
os_file_iter_end(OS_FileIter *iter) {
  W32_FileIter *w32_iter = (W32_FileIter*)iter;
  if (w32_iter->handle != 0 &&
      w32_iter->handle != INVALID_HANDLE_VALUE) {
    FindClose(w32_iter->handle);
  }
}

function String8
os_file_path(M_Arena *arena, OS_SystemPath path) {
  String8 result = {};

  switch (path) {
    case OS_SystemPath_CurrentDirectory: {
      u32 size = GetCurrentDirectoryW(0, 0);
      LPWSTR wstr = (LPWSTR)push_array(arena, u16, size * sizeof(u16));
      GetCurrentDirectoryW(size, wstr);
      result = str8_from_str16(arena, str16_cstring((u16*)wstr));
    } break;
    case OS_SystemPath_Binary: {
      if (w32_binary_path.size == 0) {
        M_Scratch scratch(arena);
        DWORD cap = 2048;
        u16 *buffer = push_array(arena, u16, cap);
        DWORD size = GetModuleFileNameW(0, (WCHAR*)buffer, cap);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        result = str8_from_str16(arena, str16(buffer, size));
      }
    } break;
    case OS_SystemPath_HomeDirectory: {
      u32 size = GetEnvironmentVariableW(L"USERPROFILE", 0, 0);
      LPWSTR wstr = (LPWSTR)push_array(arena, u16, size * sizeof(u16));
      GetEnvironmentVariableW(L"USERPROFILE", wstr, size);
      result = str8_from_str16(arena, str16_cstring((u16*)wstr));
    } break;
    case OS_SystemPath_TemporaryDirectory: {
      u32 size = GetTempPathW(0, 0);
      LPWSTR wstr = (LPWSTR)push_array(arena, u16, size * sizeof(u16));
      GetTempPathW(size, wstr);
      result = str8_from_str16(arena, str16_cstring((u16*)wstr));
    } break;
    default: {
   } break;
  }

  return result;
}

//////////////////////////////////////
///// NOTE(adam): Entropy

function void
os_get_entropy(void *data, u64 size) {
  HCRYPTPROV prov = 0;
  CryptAcquireContext(&prov, 0, 0, PROV_DSS, CRYPT_VERIFYCONTEXT);
  CryptGenRandom(prov, (DWORD)size, (BYTE*)data);
  CryptReleaseContext(prov, 0);
}
