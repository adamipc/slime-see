#ifndef BASE_STRING_H
#define BASE_STRING_H

///////////////////////////////
// NOTE(adam): String Types

struct String8 {
  u8 *str;
  u64 size;
};

struct String8Node {
  String8Node *next;
  String8 string;
};

struct String8List {
  String8Node *first;
  String8Node *last;
  u64 node_count;
  u64 total_size;
};

struct StringJoin {
  String8 pre;
  String8 post;
  String8 mid;
};

typedef u32 StringMatchFlags;
enum {
  StringMatchFlag_NoCase = 1 << 0,
};

struct String16 {
  u16 *str;
  u64 size;
};

struct String32 {
  u32 *str;
  u64 size;
};

///////////////////////////////
// NOTE(adam): Unicode Helper Types

struct StringDecode {
  u32 codepoint;
  u32 size;
};

///////////////////////////////
// NOTE(adam): Character Functions

function u8 str8_char_uppercase(u8 c);
function u8 str8_char_lowercase(u8 c);

///////////////////////////////
// NOTE(adam): String Functions

function String8 str8(u8 *str, u64 size);
function String8 str8_range(u8 *first, u8 *opl);
function String8 str8_cstring(u8 *cstr);

#define str8_lit(s) str8((u8*)(s), sizeof(s) - 1)

function String8 str8_prefix(String8 str, u64 size);
function String8 str8_chop(String8 str, u64 amount);
function String8 str8_postfix(String8 str, u64 size);
function String8 str8_skip(String8 str, u64 amount);
function String8 str8_subst(String8 str, u64 first, u64 opl);

#define str8_expand(s) (int)((s).size), ((s).str)

function void str8_list_push_explicit(String8List *list, String8 string,
                                      String8Node *node_memory);

function void str8_list_push(M_Arena *arena, String8List *list, String8 string);

function String8 str8_join(M_Arena *arena, String8List *list,
                           StringJoin *join_optional);

function String8List str8_split(M_Arena *arena, String8 string,
                                u8 *split_characters, u32 count);

function String8 str8_pushfv(M_Arena *arena, char *fmt, va_list args);
function String8 str8_pushf(M_Arena *arena, char *fmt, ...);
function void    str8_list_pushf(M_Arena *arena, String8List *list, char *fmt, ...);

////////////////////////////////
// NOTE(adam): String Comparison Functions

function b32 str8_match(String8 a, String8 b, StringMatchFlags flags);

////////////////////////////////
// NOTE(adam): Unicode Functions

function StringDecode str_decode_utf8(u8 *str, u32 cap);
function u32          str_encode_utf8(u8 *dst, u32 codepoint);
function StringDecode str_decode_utf16(u16 *str, u32 cap);
function u32          str_encode_utf16(u16 *dst, u32 codepoint);

function String32     str32_from_str8(M_Arena *arena, String8 string);
function String8      str8_from_str32(M_Arena *arena, String32 string);
function String16     str16_from_str8(M_Arena *arena, String8 string);
function String8      str8_from_str16(M_Arena *arena, String16 string);


#endif // BASE_STRING_H
