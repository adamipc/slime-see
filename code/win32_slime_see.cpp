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
  M_BaseMemory *os_base = os_base_memory();
  M_Arena arena = m_make_arena(os_base);

  int *foo = push_array(&arena, int, 1000);
  foo[999] = 0;

  printf("arena: %llu %llu %llu\n", arena.pos, arena.commit_pos, arena.cap);

  printf("Press any key to exit...");
  int ch = getchar();

  return 0;
}
