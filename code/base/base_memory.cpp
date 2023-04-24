///////////////////////////////
// NOTE(adam): Base Memory Helper Function

function void m_change_memory_noop(void *ctx, void *ptr, u64 size) { }

///////////////////////////////////////////
// NOTE(adam): Temp Helper Functions

function M_Temp
m_begin_temp(M_Arena *arena) {
  M_Temp temp = {arena, arena->pos};
  return (temp);
}

function void
m_end_temp(M_Temp temp) {
  m_arena_pop_to(temp.arena, temp.pos);
}

M_TempBlock::M_TempBlock(M_Arena *arena) {
  this->temp = m_begin_temp(arena);
}

M_TempBlock::~M_TempBlock() {
  m_end_temp(this->temp);
}

M_TempBlock::operator M_Arena*(void) {
  return (this->temp.arena);
}

void
M_TempBlock::reset() {
  m_end_temp(this->temp);
}

