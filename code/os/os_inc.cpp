/////////////////////////
/// NOTE(adam): Includes

#include "os_helpers.cpp"

#if OS_WINDOWS
# include "win32/win32_essential.h"
# include "win32/win32_essential.cpp"
# include "win32/win32_media.cpp"
#else
# error no backend for os_inc.cpp on this operating system
#endif

