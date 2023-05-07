#ifndef OS_HELPERS_H
#define OS_HELPERS_H

//////////////////////////////////////
///// NOTE(adam): Thread Context

#if !defined(TCTX_SCRATCH_POOL_CAP)
# define TCTX_SCRATCH_POOL_CAP 4
#endif

struct OS_ThreadContext {
    M_BaseMemory *memory;
    M_Arena scratch_pool[TCTX_SCRATCH_POOL_CAP];
};

//////////////////////////////////////
///// NOTE(adam): Base Memory

function M_BaseMemory* os_base_memory(void);

//////////////////////////////////////
///// NOTE(adam): Thread Setup

function void os_thread_init(OS_ThreadContext *tctx_memory);

//////////////////////////////////////
///// NOTE(adam): Thread Context

function void os_tctx_init(OS_ThreadContext *tctx, M_BaseMemory *memory);
function M_Arena*  os_tctx_get_scratch(M_Arena **conflict_array, u32 count);

//////////////////////////////////////
///// NOTE(adam): Files

function b32 os_file_write(String8 file_name, String8 data);
function u8* os_file_read_binary(M_Arena *arena, String8 file_name, u64 *bytes_read);

//////////////////////////////////////
///// NOTE(adam): Scratch Arena Wrapper

struct M_Scratch {
  M_Temp temp;

  M_Scratch(void);
  M_Scratch(M_Arena *a1);
  M_Scratch(M_Arena *a1, M_Arena *a2);
  ~M_Scratch(void);

  operator M_Arena*();

  void reset(void);
};

#endif // OS_HELPERS_H
