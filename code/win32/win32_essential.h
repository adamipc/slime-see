#ifndef WIN32_ESSENTIAL_H
#define WIN32_ESSENTIAL_H

//////////////////////////////////////
///// NOTE(adam): Win32 File Iter

struct W32_FileIter {
  HANDLE handle;
  WIN32_FIND_DATAW find_data;
  b32 done;
};

StaticAssert(sizeof(W32_FileIter) <= sizeof(OS_FileIter), w32_filtiter);

#endif // WIN32_ESSENTIAL_H
