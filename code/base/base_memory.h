#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

//////////////////////////////////
// NOTE(adam): Base Memory V-Table

typedef void* M_ReserveFunc(void *ctx, u64 size);
typedef void M_ChangeMemoryFunc(void *ctx, void *ptr, u64 size);

struct M_BaseMemory {
  M_ReserveFunc *reserve;
  M_ChangeMemoryFunc *commit;
  M_ChangeMemoryFunc *decommit;
  M_ChangeMemoryFunc *release;  
  void *ctx;
};

//////////////////////////////////
// NOTE(adam): Arena Types

struct M_Arena {
  M_BaseMemory *base;
  u8 *memory;
  u64 cap;
  u64 pos;
  u64 commit_pos;
};

struct M_Temp {
  M_Arena *arena;
  u64 pos;
};

struct M_TempBlock {
  M_Temp temp;

  M_TempBlock(M_Arena *arena);
  ~M_TempBlock(void);
  void reset(void);
};

//////////////////////////////////
// NOTE(adam): Base Memory Helper Functions

function void m_change_memory_noop(void *ctx, void *ptr, u64 size);

//////////////////////////////////
// NOTE(adam): Arena Functions

#define M_DEFAULT_RESERVE_SIZE GB(1)
#define M_COMMIT_BLOCK_SIZE MB(64)

function M_Arena m_make_arena_reserve(M_BaseMemory *base, u64 reserve_size);
function M_Arena m_make_arena(M_BaseMemory *base);

function void  m_arena_release(M_Arena *arena);

function void* m_arena_push(M_Arena *arena, u64 size);
function void m_arena_pop_to(M_Arena *arena, u64 pos);
function void m_arena_pop_amount(M_Arena *arena, u64 amount);

function void* m_arena_push_zero(M_Arena *arena, u64 size);
function void  m_arena_align(M_Arena *arena, u64 pow2_align);
function void  m_arena_align_zero(M_Arena *arena, u64 pow2_align);

#define push_array(a,T,c) (T*)m_arena_push((a), sizeof(T)*(c))
#define push_array_zero(a,T,c) (T*)m_arena_push_zero((a), sizeof(T)*(c))

function M_Temp  m_begin_temp(M_Arena *arena);
function void    m_end_tem(M_Temp temp);

#endif // BASE_MEMORY_H
