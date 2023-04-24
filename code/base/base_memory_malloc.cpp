//////////////////////////////////
// NOTE(adam): Malloc Base Implementation of M_BaseMemory

function void*
m_malloc_reserve(void *ctx, u64 size) {
  return(malloc(size));
}

function void
m_malloc_decommit(void *ctx, void *ptr, u64 size) { }

function void
m_malloc_commit(void *ctx, void *ptr, u64 size) {}

function void
m_malloc_release(void *ctx, void *ptr, u64 size) {
  free(ptr);
}

function M_BaseMemory*
m_malloc_base_memory(void) {
  local M_BaseMemory memory = {};
  if (memory.reserve == 0) {
    memory.reserve = m_malloc_reserve;
    memory.commit = m_change_memory_noop;
    memory.decommit = m_change_memory_noop;
    memory.release = m_malloc_release;
  }
  return (&memory);
}

///////////////////////////////////
// NOTE(adam): Arena Functions

function M_Arena
m_make_arena_reserve(M_BaseMemory *base, u64 reserve_size) {
  M_Arena result = {};
  result.base = base;
  result.memory = (u8*)base->reserve(base->ctx, reserve_size);
  result.cap = reserve_size;
  return (result);
}

function M_Arena
m_make_arena(M_BaseMemory *base) {
  M_Arena result = m_make_arena_reserve(base, M_DEFAULT_RESERVE_SIZE);
  return (result);
}

function void 
m_arena_release(M_Arena *arena) {
  M_BaseMemory *base = arena->base;
  base->release(base->ctx, arena->memory, arena->cap);
}

function void*
m_arena_push(M_Arena *arena, u64 size) {
  void *result = 0;
  if (arena->pos + size <= arena->cap) {
    result = arena->memory + arena->pos;
    arena->pos += size;

    u64 p = arena->pos;
    u64 commit_p = arena->commit_pos;
    if (p > commit_p) {
      u64 p_aligned = AlignUpPow2(p, M_COMMIT_BLOCK_SIZE);
      u64 next_commit_p= ClampTop(p_aligned, arena->cap);
      u64 commit_size = next_commit_p- commit_p;

      M_BaseMemory *base = arena->base;
      base->commit(base->ctx, arena->memory + commit_p, commit_size);

      arena->commit_pos = next_commit_p;
    }
  }
  Assert(result);
  return (result); 
}

function void
m_arena_pop_to(M_Arena *arena, u64 pos) {
  if (pos < arena->pos) {
    arena->pos = pos;

    u64 p = arena->pos;
    u64 p_aligned = AlignUpPow2(p, M_COMMIT_BLOCK_SIZE);
    u64 next_commit_p = ClampTop(p_aligned, arena->cap);

    u64 commit_p = arena->commit_pos;
    if (next_commit_p < commit_p) {
      u64 decommit_size = commit_p - next_commit_p;
      M_BaseMemory *base = arena->base;

      base->decommit(base->ctx, arena->memory + next_commit_p,
          decommit_size);
      arena->commit_pos = next_commit_p;
    }
  }
}

function void
m_arena_pop_amount(M_Arena *arena, u64 amount) {
   m_arena_pop_to(arena, arena->pos - amount);
}

function void*
m_arena_push_zero(M_Arena *arena, u64 size) {
  void *result = m_arena_push(arena, size);
  MemoryZero(result, size);
  return (result);
}

function void
m_arena_align(M_Arena *arena, u64 pow2_align) {
  u64 p = arena->pos;
  u64 p_aligned = AlignUpPow2(p, pow2_align);
  u64 z = p_aligned - p;
  if (z > 0) {
    m_arena_push(arena, z);
  }
}

function void
m_arena_align_zero(M_Arena *arena, u64 pow2_align) {
  u64 p = arena->pos;
  u64 p_aligned = AlignUpPow2(p, pow2_align);
  u64 z = p_aligned - p;
  if (z > 0) {
    m_arena_push_zero(arena, z);
  }
}

