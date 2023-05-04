////////////////////////////////////////
// NOTE(adam): Symbolic Constant Functions
#include "base_types.h"

function OperatingSystem
operating_system_from_context(void) {
  OperatingSystem result = OperatingSystem_Null;
#if OS_WINDOWS
  result = OperatingSystem_Windows;
#elif OS_LINUX
  result = OperatingSystem_Linux;
#elif OS_MAC
  result = OperatingSystem_Mac;
#endif
  return (result);
};

function Architecture
architecture_from_context(void) {
  Architecture result = Architecture_Null;
#if ARCH_X86
  result = Architecture_X86;
#elif ARCH_X64
  result = Architecture_X64;
#elif ARCH_ARM
  result = Architecture_ARM;
#elif ARCH_ARM64
  result = Architecture_ARM64;
#endif
  return (result);
}

function char*
string_from_operating_system(OperatingSystem os) {
  char *result = "(null)";
  switch (os) {
    case OperatingSystem_Windows: {
      result = "windows";
    } break;
    case OperatingSystem_Linux: {
      result = "linux";
    } break;
    case OperatingSystem_Mac: {
      result = "mac";
    } break;
  }

  return (result);
}

function char* string_from_architecture(Architecture arch) {
  char *result = "(null)";
  switch (arch) {
    case Architecture_X86: {
      result = "x86";
    } break;
    case Architecture_X64: {
      result = "x64";
    } break;
    case Architecture_ARM: {
      result = "arm";
    } break;
    case Architecture_ARM64: {
      result = "arm64";
    } break;
  }

  return (result);
}

function char* string_from_month(Month month) {
  char *result = "(null)";
  switch (month) {
    case Month_January: {
      result = "january";
    } break;
    case Month_February: {
      result = "february";
    } break;
    case Month_March: {
      result = "march";
    } break;
    case Month_April: {
      result = "april";
    } break;
    case Month_May: {
      result = "may";
    } break;
    case Month_June: {
      result = "june";
    } break;
    case Month_July: {
      result = "july";
    } break;
    case Month_August: {
      result = "august";
    } break;
    case Month_September: {
      result = "september";
    } break;
    case Month_October: {
      result = "october";
    } break;
    case Month_November: {
      result = "november";
    } break;
    case Month_December: {
      result = "december";
    } break;
  }

  return (result);
} 


function char* string_from_day_of_week(DayOfWeek day_of_week) {
  char *result = "(null)";
  switch (day_of_week) {
    case DayOfWeek_Sunday: {
      result = "sunday";
    } break;
    case DayOfWeek_Monday: {
      result = "monday";
    } break;
    case DayOfWeek_Tuesday: {
      result = "tuesday";
    } break;
    case DayOfWeek_Wednesday: {
      result = "wednesday";
    } break;
    case DayOfWeek_Thursday: {
      result = "thursday";
    } break;
    case DayOfWeek_Friday: {
      result = "friday";
    } break;
    case DayOfWeek_Saturday: {
      result = "saturday";
    } break;
  }

  return (result);
}


//////////////////////////////
// NOTE(adam): Float Constant Functions
//
function f32
inf_f32(void) {
  union{ f32 f; u32 u; } r;
  r.u = 0x7f800000;
  return (r.f);
}

function f32
neg_inf_f32(void) {
  union{ f32 f; u32 u; } r;
  r.u = 0xff800000;
  return (r.f);
}

function f64
inf_f64(void) {
  union{ f64 f; u64 u; } r;
  r.u = 0x7ff0000000000000llu;
  return (r.f);
}

function f64
neg_inf_f64(void) {
  union{ f64 f; u64 u; } r;
  r.u = 0xfff0000000000000llu;
  return (r.f);
}

//////////////////////////////////
// NOTE(adam): Math Functions
function f32
abs_f32(f32 x) {
  union{ f32 f; u32 u; } r;
  r.f = x;
  r.u &= 0x7fffffff;
  return (r.f);
}

function f64
abs_f64(f64 x) {
  union{ f64 f; u64 u; } r;
  r.f = x;
  r.u &= 0x7fffffffffffffffllu;
  return (r.f);
}

#include <math.h>

function f32 sqrt_f32(f32 x) {
  return sqrtf(x);
}

function f32 sin_f32(f32 x) {
  return sinf(x);
}

function f32 cos_f32(f32 x) {
  return cosf(x);
}

function f32 tan_f32(f32 x) {
  return tanf(x);
}

function f32 ln_f32(f32 x) {
  return logf(x);
}

function f64 sqrt_f64(f64 x) {
  return sqrt(x);
}

function f64 sin_f64(f64 x) {
  return sin(x);
}

function f64 cos_f64(f64 x) {
  return cos(x);
}

function f64 tan_f64(f64 x) {
  return tan(x);
}

function f64 ln_f64(f64 x) {
  return log(x);
}

function f32
lerp_f32(f32 a, f32 t, f32 b) {
  f32 x = a + (b - a) * t;
  return x;
}

function f32
unlerp_f32(f32 a, f32 x, f32 b) {
  f32 t = 0.f;
  if (a != b) {
    t = (x - a) / (b - a);
  }
  return t;
}

function i32
lerp_i32(i32 a, f64 t, i32 b) {
  i32 x = (i32)(a + (b - a) * t);
  return x;
}

function f64
unlerp_i32(i32 a, i32 x, i32 b) {
  f64 t = 0.f;
  if (a != b) {
    t = (f64)(x - a) / (f64)(b - a);
  }
  return t;
}

function f64
lerp(f64 a, f64 t, f64 b) {
  f64 x = a + (b - a) * t;
  return x;
}

function f64
unlerp(f64 a, f64 x, f64 b) {
  f64 t = 0.;
  if (a != b) {
    t = (x - a) / (b - a);
  }
  return t;
}

//////////////////////////////////////
///// NOTE(adam): Signed Encode/Decode

function u64
encode_u64_from_i64(i64 x) {
  // TODO(adam): rotleft(x, 1)
  u64 result = (((u64)x) << 1) | (((u64)x) >> 63);
  return result;
}

function i64
decode_i64_from_u64(u64 x) {
  i64 result = 0;
  if (x & 1) {
    result = -(i64)(x >> 1);
  } else {
    result =  (i64)(x >> 1);
  }

  return result;
}

//////////////////////////////////////
// NOTE(adam): Compound Type Functions

function Vec2_i32 vec2_i32(i32 x, i32 y) {
  Vec2_i32 r = { x, y };
  return r;
}

function Vec2_f32 vec2_f32(f32 x, f32 y) {
  Vec2_f32 r = { x, y };
  return r;
}

function Vec3_f32 vec3_f32(f32 x, f32 y, f32 z) {
  Vec3_f32 r = { x, y, z };
  return r;
}

function Vec4_f32 vec4_f32(f32 x, f32 y, f32 z, f32 w){
  Vec4_f32 r = { x, y, z, w };
  return r;
}

function Interval1_f32 interval1_f32(f32 min, f32 max) {
  Interval1_f32 r = { min, max };
  if (max < min) {
    r.min = max;
    r.max = min;
  }
  return r;
}

function Interval1_u64 interval1_u64(u64 min, u64 max) {
  Interval1_u64 r = { min, max };
  if (max < min) {
    r.min = max;
    r.max = min;
  }
  return r;
}

function Interval2_i32 interval2_i32(i32 x0, i32 y0, i32 x1, i32 y1) {
  Interval2_i32 r = { x0, y0,  x1, y1 };
  if (x1 < x0) {
    r.x0 = x1;
    r.x1 = x0;
  }
  if (y1 < y0) {
    r.y0 = y1;
    r.y1 = y0;
  }
  return r;
}

function Interval2_i32 interval2_i32_vec(Vec2_i32 min, Vec2_i32 max) {
  Interval2_i32 r = { min.x, min.y, max.x, max.y };
  return r;
}

function Interval2_f32 interval2_f32(f32 x0, f32 y0, f32 x1, f32 y1) {
  Interval2_f32 r = { x0, y0,  x1, y1 };
  if (x1 < x0) {
    r.x0 = x1;
    r.x1 = x0;
  }
  if (y1 < y0) {
    r.y0 = y1;
    r.y1 = y0;
  }
  return r;
}

function Interval2_f32 interval2_f32_vec(Vec2_f32 min, Vec2_f32 max) {
  Interval2_f32 r = { min.x, min.y, max.x, max.y };
  return r;
}

function Interval2_f32 interval2_f32_range(Interval1_f32 x, Interval1_f32 y) {
  Interval2_f32 r = { x.min, y.min, x.max, y.max };
  return r;
} 

function Vec2_i32 operator+(const Vec2_i32 &a, const Vec2_i32 &b) {
  Vec2_i32 r = { a.x + b.x, a.y + b.y };
  return r;
}

function Vec2_f32 operator+(const Vec2_f32 &a, const Vec2_f32 &b) {
  Vec2_f32 r = { a.x + b.x, a.y + b.y };
  return r;
}

function Vec3_f32 operator+(const Vec3_f32 &a, const Vec3_f32 &b) {
  Vec3_f32 r = { a.x + b.x, a.y + b.y, a.z + b.z };
  return r;
}

function Vec4_f32 operator+(const Vec4_f32 &a, const Vec4_f32 &b) {
  Vec4_f32 r = { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
  return r;
}

function Vec2_i32 operator-(const Vec2_i32 &a, const Vec2_i32 &b) {
  Vec2_i32 r = { a.x - b.x, a.y - b.y };
  return r;
}

function Vec2_f32 operator-(const Vec2_f32 &a, const Vec2_f32 &b) {
  Vec2_f32 r = { a.x - b.x, a.y - b.y };
  return r;
}

function Vec3_f32 operator-(const Vec3_f32 &a, const Vec3_f32 &b) {
  Vec3_f32 r = { a.x - b.x, a.y - b.y, a.z - b.z };
  return r;
}

function Vec4_f32 operator-(const Vec4_f32 &a, const Vec4_f32 &b) {
  Vec4_f32 r = { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
  return r;
}

function Vec2_i32 operator*(const Vec2_i32 &v, const i32 &s) {
  Vec2_i32 r = { v.x * s, v.y * s };
  return r;
}

function Vec2_f32 operator*(const Vec2_f32 &v, const f32 &s) {
  Vec2_f32 r = { v.x * s, v.y * s };
  return r;
}

function Vec3_f32 operator*(const Vec3_f32 &v, const f32 &s) {
  Vec3_f32 r = { v.x * s, v.y * s, v.z * s };
  return r;
}

function Vec4_f32 operator*(const Vec4_f32 &v, const f32 &s) {
  Vec4_f32 r = { v.x * s, v.y * s, v.z * s, v.w * s };
  return r;
}

function Vec2_i32 operator*(const i32 &s, const Vec2_i32 &v) {
  Vec2_i32 r = { v.x * s, v.y * s };
  return r;
}

function Vec2_f32 operator*(const f32 &s, const Vec2_f32 &v) {
  Vec2_f32 r = { v.x * s, v.y * s };
  return r;
}

function Vec3_f32 operator*(const f32 &s, const Vec3_f32 &v) {
  Vec3_f32 r = { v.x * s, v.y * s, v.z * s };
  return r;
}

function Vec4_f32 operator*(const f32 &s, const Vec4_f32 &v) {
  Vec4_f32 r = { v.x * s, v.y * s, v.z * s, v.w * s };
  return r;
}

function Vec2_f32 vec_hadamard(Vec2_f32 a, Vec2_f32 b) {
  Vec2_f32 r = { a.x * b.x, a.y * b.y };
  return r;
}

function Vec3_f32 vec_hadamard(Vec3_f32 a, Vec3_f32 b) {
  Vec3_f32 r = { a.x * b.x, a.y * b.y, a.z * b.z };
  return r;
}

function Vec4_f32 vec_hadamard(Vec4_f32 a, Vec4_f32 b) { 
  Vec4_f32 r = { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
  return r;
}

function f32 dot(Vec2_f32 a, Vec2_f32 b) {
  f32 r = a.x * b.x + a.y * b.y;
  return r;
}

function f32 dot(Vec3_f32 a, Vec3_f32 b) {
  f32 r = a.x * b.x + a.y * b.y + a.z * b.z;
  return r;
}

function f32 dot(Vec4_f32 a, Vec4_f32 b) {
  f32 r = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  return r;
}

function b32 interval_overlaps(Interval1_f32 a, Interval1_f32 b) {
  b32 r = (a.min < b.max) && (b.min < a.max);
  return r;
}

function b32 interval_contains(Interval1_f32 r, f32 x) {
  b32 result = (r.min <= x) && (x <= r.max);
  return result;
}

function b32 interval_overlaps(Interval2_f32 a, Interval2_f32 b) {
  b32 r = (b.x0 < a.x1 && a.x0 < b.x1 &&
           b.y0 < a.y1 && a.y0 < b.y1);
  return r;
}

function b32 interval_contains(Interval2_f32 r, Vec2_f32 p) {
  b32 result = (r.x0 <= p.x && p.x < r.x1 &&
           r.y0 <= p.y && p.y < r.y1);
  return result;
}

function b32 interval_overlaps(Interval2_i32 a, Interval2_i32 b) {
  b32 r = (b.x0 < a.x1 && a.x0 < b.x1 &&
           b.y0 < a.y1 && a.y0 < b.y1);
  return r;
}

function b32 interval_contains(Interval2_i32 r, Vec2_i32 p) {
  b32 result = (r.x0 <= p.x && p.x < r.x1 &&
           r.y0 <= p.y && p.y < r.y1);
  return result;

}

function f32 interval_dim(Interval1_f32 r) {
  f32 result = r.max - r.min;
  return result;
}

function u64 interval_dim(Interval1_u64 r) {
  u64 result = r.max - r.min;
  return result;
}

function Vec2_f32 interval_dim(Interval2_f32 r) {
  Vec2_f32 result = vec2_f32(r.x1 - r.x0, r.y1 - r.y0);
  return result;
}

function Vec2_i32 interval_dim(Interval2_i32 r) {
  Vec2_i32 result = vec2_i32(r.x1 - r.x0, r.y1 - r.y0);
  return result;
}

function f32 interval_center(Interval1_f32 r) {
  f32 result = (r.min + r.max) * 0.5f;
  return result;
}

function u64 interval_center(Interval1_u64 r) {
  u64 result = (r.min + r.max) / 2;
  return result;
}

function Vec2_f32 interval_center(Interval2_f32 r) {
  Vec2_f32 result = vec2_f32((r.x0 + r.x1) * 0.5f, (r.y0 + r.y1) * 0.5f);
  return result;
}

function Vec2_i32 interval_center(Interval2_i32 r) {
  Vec2_i32 result = vec2_i32((r.x0 + r.x1) / 2, (r.y0 + r.y1) / 2);
  return result;
}

function Interval1_f32 interval_axis(Interval2_f32 r, Axis axis) {
  Interval1_f32 result = {
    r.p[0].v[axis],
    r.p[1].v[axis],
  };
  return result;
}

//////////////////////////////////////
///// NOTE(adam): Time Functions

function DenseTime
dense_time_from_date_time(DateTime *in) {
  u32 year_encoded = (u32)((i32)in->year + 0x8000);
  DenseTime result = 0;

  result += year_encoded;
  result *= 12;
  result += (in->mon - 1);
  result *= 31;
  result += in->day;
  result *= 24;
  result += in->hour;
  result *= 60;
  result += in->min;
  result *= 61;
  result += in->sec;
  result *= 1000;
  result += in->msec;

  return result;
}

function DateTime
date_time_from_dense_time(DenseTime in) {
  DateTime result = {};

  result.msec = in % 1000;
  in /= 1000;
  result.sec = in % 61;
  in /= 61;
  result.min = in % 60;
  in /= 60;
  result.hour = in % 24;
  in /= 24;
  result.day = in % 31;
  in /= 31;
  result.mon = (in % 12) + 1;
  in /= 12;
  i32 year_encoded = ((i32)in - 0x8000);
  result.year = year_encoded;

  return result;
}


