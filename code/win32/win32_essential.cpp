///////////////////////////////
/// NOTE(adam): OS Includes

#include <windows.h>

//////////////////////////////////////
///// NOTE(adam): Variables

global DWORD win32_thread_context_index = 0;

//////////////////////////////////////
///// NOTE(adam): Setup

function void
os_init(void) {
  win32_thread_context_index = TlsAlloc();
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

function DenseTime
w32_dense_time_from_file(FILETIME file_time) {
  DenseTime result = {};
  result = (f64)file_time.dwLowDateTime / 10000000.0;
  result += (f64)file_time.dwHighDateTime * 4294967296.0 / 10000000.0;
  return result;
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
    result.create_time = w32_dense_time_from_file(attribs.ftCreationTime);
    result.modify_time = w32_dense_time_from_file(attribs.ftLastWriteTime);
    result.access = w32_access_from_attributes(attribs.dwFileAttributes);
  }

  return result;
}
