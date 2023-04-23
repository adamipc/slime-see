#ifndef OS_ESSENTIAL_H
#define OS_ESSNETIAL_H

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

#endif // OS_ESSENTIAL_H
