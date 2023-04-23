///////////////////////////////
/// NOTE(adam): OS Includes

#include <windows.h>

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

