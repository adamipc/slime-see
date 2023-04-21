#ifndef BASE_H
#define BASE_H

////////////////////////////////
// NOTE(adam): Context cracking

// NOTE(adam): Checking CLANG first because it also defines
// _MSC_VER
#if defined(__clang__)
# define COMPILER_CLANG 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error missing OS detection
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_I86)
#  define ARCH_X86 1
# elif defined_M_ARM)
#  define ARCH_ARM 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error missing ARCH detection
# endif

#elif defined(_MSC_VER)
# define COMPILER_CL 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# else
#  error missing OS detection
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_I86)
#  define ARCH_X86 1
# elif defined_M_ARM)
#  define ARCH_ARM 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error missing ARCH detection
# endif

#elif defined(__GNUC__)
# define COMPILER_GCC 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error missing OS detection
# endif

# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_I86)
#  define ARCH_X86 1
# elif defined_M_ARM)
#  define ARCH_ARM 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error missing ARCH detection
# endif
#else
# error no context cracking for this compiler
#endif

// NOTE(adam): Fill in defaults to zero
#if !defined(COMPILER_CL)
# define COMPILER_CL 0
#endif
#if !defined(COMPILER_GCC)
# define COMPILER_GCC 0
#endif
#if !defined(COMPILER_CLANG)
# define COMPILER_CLANG 0
#endif

#if !defined(OS_WINDOWS)
# define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
# define OS_LINUX 0
#endif
#if !defined(OS_MAC)
# define OS_MAC 0
#endif

#if !defined(ARCH_X64)
# define ARCH_X64 0
#endif
#if !defined(ARCH_X86)
# define ARCH_X86 0
#endif
#if !defined(ARCH_ARM)
# define ARCH_ARM 0
#endif
#if !defined(ARCH_ARM64)
# define ARCH_ARM64 0
#endif

////////////////////////////////////
// NOTE(adam): Helper Macros
#define Stmnt(S) do{ S }while(0)
#if COMPILER_CL
#define AssertBreak() (*(int*)0 = 0) 
#else
#define AssertBreak() __builtin_trap()
#endif

#if ENABLE_ASSERT
# define Assert(c) Stmnt( if (!(c)) { AssertBreak(); } )
#else
# define Assert(c)
#endif

#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)
#define Glue_(A,B) A##B
#define Glue(A,B) Glue_(A,B)

#define ArrayCount(a) (sizeof(a)/sizeof((a)[0]))

#define IntFromPtr(p) (u64)((u8*)p - (u8*)0)
#define PtrFromInt(n) (void*)((u8*)0 + (n))

#define Member(T,m) (((T*)0)->m)
#define OffsetOfMember(T,m) IntFromPtr(&Member(T,m))

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))
#define Clamp(a,x,b) (((x)<(a))?(a):\
                      ((x)>(b))?(b):(x))
#define ClampTop(a,b) Min(a,b)
#define ClampBottom(a,b) Max(a,b)

#define global static
#define local static
#define function static

#define c_linkage_begin extern "C" {
#define c_linkage_end }
#define c_linkage extern "C"

#include <string.h>
#define MemoryZero(p,z) memset((p), 0, (z))
#define MemoryZeroStruct(p) MemoryZero((p), sizeof(*(p)))
#define MemoryZeroArray(p) MemoryZero((p), sizeof(p))
#define MemoryZeroTyped(p,c) MemoryZero((p), sizeof(*(p))*(c))

#define MemoryMatch(a,b,z) (memcmp((a), (b), (z)) == 0)

#define MemoryCopy(d,s,z) memmove((d), (s), (z))
#define MemoryCopyStruct(d,s) MemoryCopy((d),(s),\
                                         Min(sizeof(*(d)),\
                                             sizeof(*(s))))
#define MemoryCopyArray(d,s) MemoryCopy((d),(s),\
                                        Min(sizeof(d),\
                                            sizeof(s)))
#define MemoryCopyTyped(d,s,c) MemoryCopy((d),(s),\
                                          Min(sizeof(*(d)),\
                                              sizeof(*(s)))*(c))

///////////////////////////////
// NOTE(adam): Basic Types

#include <stdint.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;
typedef float f32;
typedef double f64;

typedef void VoidFunc(void);

////////////////////////////////
// NOTE(adam): Helpful Constants

global i8   min_i8  = (i8) 0x80; 
global i16  min_i16 = (i16)0x8000; 
global i32  min_i32 = (i32)0x80000000; 
global i64  min_i64 = (i64)0x800000000000000llu; 

global i8   max_i8  = (i8) 0x7f; 
global i16  max_i16 = (i16)0x7fff; 
global i32  max_i32 = (i32)0x7fffffff; 
global i64  max_i64 = (i64)0x7fffffffffffffffllu; 

global u8   max_u8  = (u8) 0xff;
global u16  max_u16 = (u16)0xffff;
global u32  max_u32 = (u32)0xffffffff;
global u64  max_u64 = (u64)0xffffffffffffffffllu;

global f32 machine_epsilon_f32 = 1.19209290e-7f;
global f32 pi_f32 = 3.14159265359f;
global f32 tau_f32 = 6.28318530718f;
global f32 gold_big_f32 = 1.61803398875f;
global f32 gold_small_f32 = 0.61803398875f;
global f32 e_f32 = 2.71828182846f;

global f64 machine_epsilon_f64 = 2.2204460492503131e-16;
global f64 pi_f64 = 3.14159265359;
global f64 tau_f64 = 6.28318530718;
global f64 gold_big_f64 = 1.61803398875;
global f64 gold_small_f64 = 0.61803398875;
global f64 e_f64 = 2.71828182846;

////////////////////////////////////
// NOTE(adam): Symbolic Constants

enum Axis {
  Axis_X,
  Axis_Y,
  Axis_Z,
  Axis_W,
};

enum Side {
  Side_Min,
  Side_Max,
};

enum OperatingSystem {
  OperatingSystem_Null,
  OperatingSystem_Windows,
  OperatingSystem_Linux,
  OperatingSystem_Mac,
  OperatingSystem_COUNT,
};

enum Architecture {
  Architecture_Null,
  Architecture_X86,
  Architecture_X64,
  Architecture_ARM,
  Architecture_ARM64,
  Architecture_COUNT,
};

enum Month {
  Month_January,
  Month_February,
  Month_March,
  Month_April,
  Month_May,
  Month_June,
  Month_July,
  Month_August,
  Month_September,
  Month_October,
  Month_November,
  Month_December,
};

enum DayOfWeek {
  DayOfWeek_Sunday,
  DayOfWeek_Monday,
  DayOfWeek_Tuesday,
  DayOfWeek_Wednesday,
  DayOfWeek_Thursday,
  DayOfWeek_Friday,
  DayOfWeek_Saturday,
};

////////////////////////////////////
// NOTE(adam): Compound Types

union Vec2_i32 {
  struct {
    i32 x;
    i32 y;
  };
  i32 v[2];
};

union Vec2_f32 {
  struct {
    f32 x;
    f32 y;
  };
  f32 v[2];
};

union Vec3_f32 {
  struct {
    f32 x;
    f32 y;
    f32 z;
  };
  f32 v[3];
};

union Vec4_f32 {
  struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
  };
  f32 v[4];
};

union Interval1_f32 {
  struct {
    f32 min;
    f32 max;
  };
  f32 v[2];
};

union Interval1_u64 {
  struct {
    u64 min;
    u64 max;
  };
  struct {
    u64 first;
    u64 opl;
  };
  u64 v[2];
};

union Interval2_f32 {
  struct {
    Vec2_f32 min;
    Vec2_f32 max;
  };
  struct {
    Vec2_f32 p0;
    Vec2_f32 p1;
  };
  struct {
    f32 x0;
    f32 y0;
    f32 x1;
    f32 y1;
  };
  Vec2_f32 p[2];
  f32 v[4];
};

union Interval2_i32 {
  struct {
    Vec2_i32 min;
    Vec2_i32 max;
  };
  struct {
    Vec2_i32 p0;
    Vec2_i32 p1;
  };
  struct {
    i32 x0;
    i32 y0;
    i32 x1;
    i32 y1;
  };
  Vec2_i32 p[2];
  i32 v[4];
};

//////////////////////////////////
// NOTE(adam): Symbolic Constant Functions

function OperatingSystem operating_system_from_context(void);
function Architecture architecture_from_context(void);

function char* string_from_operating_system(OperatingSystem os);
function char* string_from_architecture(Architecture arch);
function char* string_from_month(Month month);
function char* string_from_day_of_week(DayOfWeek day_of_week);

//////////////////////////////////
// NOTE(adam): Float Constant Functions
function f32 inf_f32(void);
function f32 neg_inf_f32(void);
function f64 inf_f64(void);
function f64 neg_inf_f64(void);

////////////////////////////////////
// NOTE(adam): Math Functions
function f32 abs_f32(f32 x);
function f64 abs_f64(f64 x);

function f32 sqrt_f32(f32 x);
function f32 sin_f32(f32 x);
function f32 cos_f32(f32 x);
function f32 tan_f32(f32 x);
function f32 ln_f32(f32 x);

function f64 sqrt_f64(f64 x);
function f64 sin_f64(f64 x);
function f64 cos_f64(f64 x);
function f64 tan_f64(f64 x);
function f64 ln_f64(f64 x);

function f32 lerp_f32(f32 a, f32 t, f32 b);
function f32 unlerp_f32(f32 a, f32 x, f32 b);

function f64 lerp(f64 a, f64 t, f64 b);
function f64 unlerp(f64 a, f64 x, f64 b);

/////////////////////////////////////
// NOTE(adam): Compound Type Functions

function Vec2_i32 vec2_i32(i32 x, i32 y);

function Vec2_f32 vec2_f32(f32 x, f32 y);
function Vec3_f32 vec3_f32(f32 x, f32 y, f32 z);
function Vec4_f32 vec4_f32(f32 x, f32 y, f32 z, f32 w);

function Interval1_f32 interval1_f32(f32 min, f32 max);
function Interval1_u64 interval1_u64(u64 min, u64 max);

function Interval2_i32 interval2_i32(i32 x0, i32 y0, i32 x1, i32 y1);
function Interval2_i32 interval2_i32_vec(Vec2_i32 min, Vec2_i32 max);

function Interval2_f32 interval2_f32(f32 x0, f32 y0, f32 x1, f32 y1);
function Interval2_f32 interval2_f32_vec(Vec2_f32 x, Vec2_f32 y);
function Interval2_f32 interval2_f32_range(Interval1_f32 x, Interval1_f32 y);

function Vec2_i32 operator+(const Vec2_i32 &a, const Vec2_i32 &b);
function Vec2_f32 operator+(const Vec2_f32 &a, const Vec2_f32 &b);
function Vec3_f32 operator+(const Vec3_f32 &a, const Vec3_f32 &b);
function Vec4_f32 operator+(const Vec4_f32 &a, const Vec4_f32 &b);

function Vec2_i32 operator-(const Vec2_i32 &a, const Vec2_i32 &b);
function Vec2_f32 operator-(const Vec2_f32 &a, const Vec2_f32 &b);
function Vec3_f32 operator-(const Vec3_f32 &a, const Vec3_f32 &b);
function Vec4_f32 operator-(const Vec4_f32 &a, const Vec4_f32 &b);

function Vec2_i32 operator*(const Vec2_i32 &v, const i32 &s);
function Vec2_f32 operator*(const Vec2_f32 &v, const f32 &s);
function Vec3_f32 operator*(const Vec3_f32 &v, const f32 &s);
function Vec4_f32 operator*(const Vec4_f32 &v, const f32 &s);

function Vec2_i32 operator*(const i32 &s, const Vec2_i32 &v);
function Vec2_f32 operator*(const f32 &s, const Vec2_f32 &v);
function Vec3_f32 operator*(const f32 &s, const Vec3_f32 &v);
function Vec4_f32 operator*(const f32 &s, const Vec4_f32 &v);

function Vec2_f32 vec_hadamard(Vec2_f32 a, Vec2_f32 b);
function Vec3_f32 vec_hadamard(Vec3_f32 a, Vec3_f32 b);
function Vec4_f32 vec_hadamard(Vec4_f32 a, Vec4_f32 b);

function f32 dot(Vec2_f32 a, Vec2_f32 b);
function f32 dot(Vec3_f32 a, Vec3_f32 b);
function f32 dot(Vec4_f32 a, Vec4_f32 b);

function b32 interval_overlaps(Interval1_f32 a, Interval1_f32 b);
function b32 interval_contains(Interval1_f32 r, f32 x);

function b32 interval_overlaps(Interval2_f32 a, Interval2_f32 b);
function b32 interval_contains(Interval2_f32 r, Vec2_f32 p);

function b32 interval_overlaps(Interval2_i32 a, Interval2_i32 b);
function b32 interval_contains(Interval2_i32 r, Vec2_i32 p);

function f32 interval_dim(Interval1_f32 r);
function u64 interval_dim(Interval1_u64 r);
function Vec2_f32 interval_dim(Interval2_f32 r);
function Vec2_i32 interval_dim(Interval2_i32 r);

function f32 interval_center(Interval1_f32 r);
function u64 interval_center(Interval1_u64 r);
function Vec2_f32 interval_center(Interval2_f32 r);
function Vec2_i32 interval_center(Interval2_i32 r);

function Interval1_f32 interval_axis(Interval2_f32 r, Axis axis);

#endif // BASE_H

