// io.h — _findfirst/_findnext shim for POSIX (Android); passes through to the
// real MinGW <io.h> on desktop Windows builds.

#ifndef _PORT_IO_H_
#define _PORT_IO_H_

#if defined(_WIN32) && !defined(__ANDROID__)

#include_next <io.h>

#else

#include <stddef.h>
#include <stdint.h>

#define _A_NORMAL 0x00
#define _A_SUBDIR 0x10

struct _finddata_t
{
    unsigned attrib;
    long     time_create;
    long     time_access;
    long     time_write;
    unsigned long size;
    char     name[260];
};

#ifdef __cplusplus
extern "C" {
#endif

intptr_t _findfirst(const char* pattern, struct _finddata_t* data);
int      _findnext(intptr_t handle, struct _finddata_t* data);
int      _findclose(intptr_t handle);

#ifdef __cplusplus
}
#endif

#endif // !_WIN32

#endif // _PORT_IO_H_
