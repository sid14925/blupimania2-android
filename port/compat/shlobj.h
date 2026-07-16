// shlobj.h — shell folder shim. CSIDL_PERSONAL ("My Documents") maps to the
// port's writable data directory (SDL pref path on Android, cwd on desktop).

#ifndef _PORT_SHLOBJ_H_
#define _PORT_SHLOBJ_H_

#include <windows.h>

typedef void* LPITEMIDLIST;
#define CSIDL_PERSONAL 0x0005

#ifdef __cplusplus
extern "C" {
#endif

HRESULT SHGetSpecialFolderLocation(HWND hwnd, int csidl, LPITEMIDLIST* ppidl);
BOOL    SHGetPathFromIDList(LPITEMIDLIST pidl, char* pszPath);

#ifdef __cplusplus
}
#endif

#endif // _PORT_SHLOBJ_H_
