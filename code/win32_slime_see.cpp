#include <windows.h>
#include <stdio.h>
#include "base/base_inc.h"
#include "os/os_inc.h"

#include "base/base_inc.cpp"
#include "os/os_inc.cpp"

#include <stdlib.h>

#include "base/base_memory_malloc.cpp"

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode) {
  BOOL bResult = AllocConsole();
  if (!bResult) {
    MessageBoxA(0, "AllocConsole failed", "Error", MB_OK);
  }
  FILE *fDummy;
  freopen_s(&fDummy, "CONOUT$", "w", stdout);
  freopen_s(&fDummy, "CONOUT$", "w", stderr);
  freopen_s(&fDummy, "CONIN$", "r", stdin);

  // Main code
  os_init();

  OS_ThreadContext tctx_memory = {};
  os_thread_init(&tctx_memory);

  M_Scratch scratch;

  int *foo = push_array(scratch, int, 1000);
  foo[999] = 0;

  M_Arena *arena = (M_Arena *)scratch;
  printf("scratch: %llu %llu %llu\n",
      arena->pos, arena->commit_pos, arena->cap);

  M_Scratch other_scratch((M_Arena*)scratch);
  printf("scratch      : %p\nother_scratch: %p\n",
         scratch.temp.arena, other_scratch.temp.arena);

  printf("Press any key to exit...");
  int ch = getchar();

  return 0;
}
