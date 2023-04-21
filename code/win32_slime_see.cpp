#include <windows.h>
#include <stdio.h>
#include "base.h"

#include "base.cpp"

#define EvalPrint(x) printf("%s = %d\n", #x, (i32)(x))
#define EvalPrintU(x) printf("%s = %u\n", #x, (u32)(x))
#define EvalPrintLL(x) printf("%s = %lld\n", #x, (i64)(x))
#define EvalPrintULL(x) printf("%s = %llu\n", #x, (u64)(x))
#define EvalPrintF(x) printf("%s = %e [%f]\n", #x, (f64)(x), (f64)(x))
#define EvalPrintB(x) printf("%s = %s\n", #x, (char*)((x)?"true":"false"))
#define EvalPrintS(x) printf("%s = %s\n", #x, (char*)(x))

struct Node {
  Node *next;
  Node *prev;

  int x;
};

struct TestStruct {
  i32 a;
  i32 b;
  i32 c;
  i32 d;
};

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

  /*
  printf("COMPILER_CL: %d\n", COMPILER_CL);
  printf("COMPILER_GCC: %d\n", COMPILER_GCC);
  printf("COMPILER_CLANG: %d\n", COMPILER_CLANG);
  printf("OS_WINDOWS: %d\n", OS_WINDOWS);
  printf("OS_LINUX: %d\n", OS_LINUX);
  printf("OS_MAC: %d\n", OS_MAC);
  printf("ARCH_X86: %d\n", ARCH_X86);
  printf("ARCH_X64: %d\n", ARCH_X64);
  printf("ARCH_ARM: %d\n", ARCH_ARM);
  printf("ARCH_ARM64: %d\n", ARCH_ARM64);
  */

  Node nodes[10];
  for (int i = 0; i < ArrayCount(nodes); i += 1) {
    nodes[i].x = i;
  }

  {
    Node *first =0;

    for (int i = 0; i < 10; i += 1) {
      SLLStackPush(first, &nodes[i]);
    }
    SLLStackPop(first);
    SLLStackPop(first);
    SLLStackPop(first);
    for (Node *node = first;
         node != 0;
         node = node->next) {
      EvalPrint(node->x);
    }
  }
  /*
  {
    Node *first = 0;
    Node *last = 0;
    for (int i = 0; i < 5; i += 1) {
      DLLPushBack(first, last, &nodes[i]);
    }
    for (int i = 5; i < 10; i += 1) {
      DLLPushFront(first, last, &nodes[i]);
    }
    for (Node *node = first;
         node != 0;
         node = node->next) {
      EvalPrint(node->x);
    }

  } 
  {
    Node *first = 0;
    Node *last = 0;
    for (int i = 0; i < 9; i += 1) {
      SLLQueuePush(first, last, &nodes[i]);
    }
    SLLQueuePushFront(first, last, &nodes[9]);
    for (Node *node = first;
         node != 0;
         node = node->next) {
      EvalPrint(node->x);
    }

  } // */

  int foo[100];
  for (int i = 0; i < ArrayCount(foo); i += 1) {
    foo[i] = i;
  }

  EvalPrint(ArrayCount(foo));

  int bar[100];
  MemoryCopyArray(bar, foo);
  EvalPrint(bar[50]);
  EvalPrint(MemoryMatch(foo, bar, sizeof(foo)));
  MemoryZeroArray(bar);
  EvalPrint(bar[50]);
  EvalPrint(MemoryMatch(foo, bar, sizeof(foo)));

  EvalPrint(OffsetOfMember(TestStruct, a));
  EvalPrint(OffsetOfMember(TestStruct, b));
  EvalPrint(OffsetOfMember(TestStruct, c));
  EvalPrint(OffsetOfMember(TestStruct, d));

  TestStruct t = {1, 2, 3, 4};
  EvalPrint(t.a);
  EvalPrint(t.d);
  MemoryZeroStruct(&t);
  EvalPrint(t.a);
  EvalPrint(t.d);

  EvalPrint(Min(1, 10));
  EvalPrint(Min(100, 10));
  EvalPrint(Max(1, 10));
  EvalPrint(Max(100, 10));
  EvalPrint(Clamp(1, 10, 100));
  EvalPrint(Clamp(1, 0, 100));
  EvalPrint(Clamp(1, 500, 100));

  EvalPrint(min_i8);
  EvalPrint(min_i16);
  EvalPrint(min_i32);
  EvalPrintLL(min_i64);

  EvalPrint(max_i8);;
  EvalPrint(max_i16);
  EvalPrint(max_i32);
  EvalPrintLL(max_i64);

  EvalPrintU(max_u8);;
  EvalPrintU(max_u16);
  EvalPrintU(max_u32);
  EvalPrintULL(max_u64);

  EvalPrintF(machine_epsilon_f32);
  EvalPrintF(machine_epsilon_f64);

  EvalPrintF(inf_f32());
  EvalPrintF(neg_inf_f32());

  EvalPrintF(inf_f64());
  EvalPrintF(neg_inf_f64());

  EvalPrintF(abs_f32(-100.f));
  EvalPrintF(abs_f32(100.f));
  EvalPrintF(abs_f32(0));
  EvalPrintF(abs_f32(neg_inf_f32()));

  EvalPrintF(abs_f64(-100.));
  EvalPrintF(abs_f64(100.));
  EvalPrintF(abs_f64(0));
  EvalPrintF(abs_f64(neg_inf_f64()));

  EvalPrintF(sqrt_f32(100.f));
  EvalPrintF(sin_f32(tau_f32*.3f));
  EvalPrintF(cos_f32(tau_f32*.3f));
  EvalPrintF(tan_f32(0.5f));
  EvalPrintF(ln_f32(1.f));
  EvalPrintF(ln_f32(e_f32));

  EvalPrintF(sqrt_f64(100.f));
  EvalPrintF(sin_f64(tau_f64*.3f));
  EvalPrintF(cos_f64(tau_f64*.3f));
  EvalPrintF(tan_f64(0.5f));
  EvalPrintF(ln_f64(1.));
  EvalPrintF(ln_f64(e_f64));

  EvalPrintF(lerp(0, 0.3f, 1.f));
  EvalPrintF(lerp(10.f, 0.5f, 100.f));
  EvalPrintF(unlerp(0, 0.3f, 1.f));
  EvalPrintF(unlerp(10.f, lerp(10.f, 0.5f, 100.f), 100.f));

  EvalPrintB(interval_contains(interval2_f32(20, 20, 200, 200), vec2_f32(100, 199)));
  EvalPrintB(interval_contains(interval2_f32(20, 20, 200, 200), vec2_f32(100, 201)));
  EvalPrintB(interval_contains(interval2_f32(20, 20, 200, 200), vec2_f32(19, 199)));

  EvalPrintS(string_from_operating_system(operating_system_from_context()));
  EvalPrintS(string_from_day_of_week(DayOfWeek_Wednesday));


  printf("Press any key to exit...");
  int ch = getchar();

  return 0;
}
