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

