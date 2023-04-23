#ifndef OS_ESSENTIAL_H
#define OS_ESSNETIAL_H

////////////////////////////////
/// NOTE(adam): Memory Functions

function void* os_memory_reserve(u64 size);
function void os_memory_commit(void *ptr, u64 size);
function void os_memory_decommit(void *ptr, u64 size);
function void os_memory_release(void *ptr, u64 size);

#endif // OS_ESSENTIAL_H
