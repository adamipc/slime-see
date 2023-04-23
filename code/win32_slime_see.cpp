#include <windows.h>
#include <stdio.h>
#include "base/base_inc.h"

#include "base/base_inc.cpp"

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

  printf("Press any key to exit...");
  int ch = getchar();

  return 0;
}
