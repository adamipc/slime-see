#ifndef OS_ESSENTIAL_H
#define OS_ESSNETIAL_H

//////////////////////////////////////
///// NOTE(adam): File Iteration

struct OS_FileIter {
  u8 v[640];
};

//////////////////////////////////////
///// NOTE(adam): Setup

function void os_init(void);

////////////////////////////////
/// NOTE(adam): Memory Functions

function void* os_memory_reserve(u64 size);
function void os_memory_commit(void *ptr, u64 size);
function void os_memory_decommit(void *ptr, u64 size);
function void os_memory_release(void *ptr, u64 size);

//////////////////////////////////////
///// NOTE(adam): Thread Context

function void  os_thread_context_set(void *ptr);
function void* os_thread_context_get(void);

//////////////////////////////////////
///// NOTE(adam): File Handling

function String8 os_file_read(M_Arena *arena, String8 file_name);
function b32     os_file_write(String8 file_name, String8List data);

function FileProperties os_file_properties(String8 file_name);

function b32 os_file_delete(String8 file_name);
function b32 os_file_rename(String8 og_name, String8 new_name);
function b32 os_file_make_directory(String8 path);
function b32 os_file_delete_directory(String8 path);

function OS_FileIter os_file_iter_init(String8 path);
function b32  os_file_iter_next(M_Arena *arena, OS_FileIter *iter,
                                String8 *name_out, FileProperties *prop_out);
function void os_file_iter_end(OS_FileIter *iter);

//////////////////////////////////////
///// NOTE(adam): Time

function DateTime os_now_universal_time(void);
function DateTime os_local_time_from_universal(DateTime *date_time);
function DateTime os_universal_time_from_local(DateTime *date_time);

function u64  os_now_microseconds(void);
function void os_sleep_milliseconds(u64 t);

//////////////////////////////////////
///// NOTE(adam): Entropy

function void os_get_entropy(void *data, u64 size);

#endif // OS_ESSENTIAL_H
