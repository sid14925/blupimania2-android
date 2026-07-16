// compat.cpp — implementations for the Win32 shim (port/compat/windows.h).

#define PORT_NO_STDIO_REDIRECT      // this file calls the real fopen/remove
#include <windows.h>
#include <SDL.h>

#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

// ------------------------------------------------------------------ strings

int PortStriCmp(const char* a, const char* b)
{
    while (*a && *b)
    {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int PortStrniCmp(const char* a, const char* b, size_t n)
{
    while (n-- > 0)
    {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        if (ca == 0) return 0;
        a++; b++;
    }
    return 0;
}

char* PortStrCpyN(char* dst, const char* src, int n)
{
    if (n <= 0) return dst;
    int i;
    for (i = 0; i < n-1 && src[i]; i++) dst[i] = src[i];
    dst[i] = 0;
    return dst;
}

// ------------------------------------------------------- path-aware stdio

// converts Windows-style relative paths to the host filesystem:
// backslashes -> slashes; lowercased on Android (case-sensitive FS)
static void PortFixPath(const char* in, char* out, int outSize)
{
    int i;
    for (i = 0; i < outSize-1 && in[i]; i++)
    {
        char c = in[i];
        if (c == '\\') c = '/';
#ifdef __ANDROID__
        c = (char)tolower((unsigned char)c);
#endif
        out[i] = c;
    }
    out[i] = 0;
}

char* Port_fullpath(char* absPath, const char* relPath, size_t maxLength)
{
    char fixed[512];
    PortFixPath(relPath, fixed, sizeof(fixed));
    if (fixed[0] == '/')
    {
        PortStrCpyN(absPath, fixed, (int)maxLength);
        return absPath;
    }
    char cwd[512];
#ifdef _WIN32
    if (_getcwd(cwd, sizeof(cwd)) == NULL) cwd[0] = 0;
#else
    if (getcwd(cwd, sizeof(cwd)) == NULL) cwd[0] = 0;
#endif
    snprintf(absPath, maxLength, "%s/%s", cwd, fixed);
    return absPath;
}

FILE* Port_fopen(const char* path, const char* mode)
{
    char fixed[512];
    PortFixPath(path, fixed, sizeof(fixed));
    return fopen(fixed, mode);
}

int Port_remove(const char* path)
{
    char fixed[512];
    PortFixPath(path, fixed, sizeof(fixed));
    return remove(fixed);
}

int Port_rename(const char* oldp, const char* newp)
{
    char f1[512], f2[512];
    PortFixPath(oldp, f1, sizeof(f1));
    PortFixPath(newp, f2, sizeof(f2));
    return rename(f1, f2);
}

int Port_mkdir(const char* path)
{
    char fixed[512];
    PortFixPath(path, fixed, sizeof(fixed));
    // strip a trailing slash: _mkdir("files\\") is common in the game
    size_t n = strlen(fixed);
    if (n > 0 && fixed[n-1] == '/') fixed[n-1] = 0;
#ifdef _WIN32
    return _mkdir(fixed);
#else
    return mkdir(fixed, 0755);
#endif
}

// ------------------------------------------------- _findfirst on POSIX

#if !defined(_WIN32) || defined(__ANDROID__)

#include <io.h>
#include <dirent.h>

struct PortFindState
{
    DIR* dir;
    char dirPath[512];
    char pattern[260];
};

static BOOL PortWildMatch(const char* pat, const char* str)
{
    while (*pat)
    {
        if (*pat == '*')
        {
            pat++;
            if (*pat == 0) return TRUE;
            for (const char* s = str; *s; s++)
                if (PortWildMatch(pat, s)) return TRUE;
            return FALSE;
        }
        if (*pat == '?')
        {
            if (*str == 0) return FALSE;
        }
        else if (tolower((unsigned char)*pat) != tolower((unsigned char)*str))
        {
            return FALSE;
        }
        pat++;
        str++;
    }
    return *str == 0;
}

static int PortFindFill(PortFindState* st, struct _finddata_t* data)
{
    struct dirent* e;
    while ((e = readdir(st->dir)) != NULL)
    {
        if (!PortWildMatch(st->pattern, e->d_name)) continue;

        char full[800];
        snprintf(full, sizeof(full), "%s/%s", st->dirPath, e->d_name);
        struct stat sb;
        data->attrib = _A_NORMAL;
        data->size = 0;
        if (stat(full, &sb) == 0)
        {
            if (S_ISDIR(sb.st_mode)) data->attrib |= _A_SUBDIR;
            data->size = (unsigned long)sb.st_size;
        }
        PortStrCpyN(data->name, e->d_name, sizeof(data->name));
        return 0;
    }
    return -1;
}

extern "C" intptr_t _findfirst(const char* pattern, struct _finddata_t* data)
{
    char fixed[512];
    PortFixPath(pattern, fixed, sizeof(fixed));

    PortFindState* st = (PortFindState*)calloc(1, sizeof(PortFindState));
    const char* slash = strrchr(fixed, '/');
    if (slash != NULL)
    {
        size_t n = (size_t)(slash - fixed);
        memcpy(st->dirPath, fixed, n);
        st->dirPath[n] = 0;
        PortStrCpyN(st->pattern, slash+1, sizeof(st->pattern));
    }
    else
    {
        strcpy(st->dirPath, ".");
        PortStrCpyN(st->pattern, fixed, sizeof(st->pattern));
    }

    st->dir = opendir(st->dirPath[0] ? st->dirPath : ".");
    if (st->dir == NULL) { free(st); return -1; }

    if (PortFindFill(st, data) != 0)
    {
        closedir(st->dir);
        free(st);
        return -1;
    }
    return (intptr_t)st;
}

extern "C" int _findnext(intptr_t handle, struct _finddata_t* data)
{
    if (handle == -1 || handle == 0) return -1;
    return PortFindFill((PortFindState*)handle, data);
}

extern "C" int _findclose(intptr_t handle)
{
    if (handle == -1 || handle == 0) return -1;
    PortFindState* st = (PortFindState*)handle;
    closedir(st->dir);
    free(st);
    return 0;
}

#endif // POSIX findfirst

// ---------------------------------------------------------------- debug/time

void OutputDebugString(const char* str)
{
    SDL_Log("%s", str);
}

DWORD timeGetTime(void)
{
    return (DWORD)SDL_GetTicks();
}

void Sleep(DWORD ms)
{
    SDL_Delay(ms);
}

// ------------------------------------------------------------------ INI file
// GetPrivateProfileString-compatible reader for blupimania.ini-style files:
//   [Section]
//   Key=value

static BOOL IniFindKey(FILE* f, const char* section, const char* key,
                       char* value, int valueSize)
{
    char line[512];
    BOOL bInSection = FALSE;
    size_t keyLen = strlen(key);

    while (fgets(line, sizeof(line), f) != NULL)
    {
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;

        if (*p == '[')
        {
            char* e = strchr(p, ']');
            if (e != NULL)
            {
                *e = 0;
                bInSection = (PortStriCmp(p+1, section) == 0);
            }
            continue;
        }
        if (!bInSection) continue;

        if (PortStrniCmp(p, key, keyLen) == 0)
        {
            char* q = p + keyLen;
            while (*q == ' ' || *q == '\t') q++;
            if (*q != '=') continue;
            q++;
            while (*q == ' ' || *q == '\t') q++;
            char* end = q + strlen(q);
            while (end > q && (end[-1] == '\n' || end[-1] == '\r' ||
                               end[-1] == ' '  || end[-1] == '\t')) end--;
            int len = (int)(end - q);
            if (len > valueSize-1) len = valueSize-1;
            memcpy(value, q, len);
            value[len] = 0;
            return TRUE;
        }
    }
    return FALSE;
}

DWORD GetPrivateProfileString(const char* section, const char* key,
                              const char* def, char* out, DWORD outSize,
                              const char* file)
{
    FILE* f = fopen(file, "r");
    if (f != NULL)
    {
        if (IniFindKey(f, section, key, out, (int)outSize))
        {
            fclose(f);
            return (DWORD)strlen(out);
        }
        fclose(f);
    }
    PortStrCpyN(out, def != NULL ? def : "", (int)outSize);
    return (DWORD)strlen(out);
}

UINT GetPrivateProfileInt(const char* section, const char* key,
                          int def, const char* file)
{
    char buffer[64];
    char defstr[32];
    sprintf(defstr, "%d", def);
    GetPrivateProfileString(section, key, defstr, buffer, sizeof(buffer), file);
    return (UINT)atoi(buffer);
}

BOOL WritePrivateProfileString(const char* section, const char* key,
                               const char* value, const char* file)
{
    // read whole file, patch or append the key, rewrite
    char* content = NULL;
    long size = 0;
    FILE* f = fopen(file, "rb");
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        content = (char*)malloc(size+1);
        fread(content, 1, size, f);
        content[size] = 0;
        fclose(f);
    }

    FILE* out = fopen(file, "wb");
    if (out == NULL) { free(content); return FALSE; }

    BOOL bWritten = FALSE;
    BOOL bInSection = FALSE;
    BOOL bSectionSeen = FALSE;
    size_t keyLen = strlen(key);

    if (content != NULL)
    {
        char* line = content;
        while (line != NULL && *line != 0)
        {
            char* next = strchr(line, '\n');
            char saved = 0;
            if (next != NULL) { saved = next[1]; next[1] = 0; }

            char* p = line;
            while (*p == ' ' || *p == '\t') p++;

            if (*p == '[')
            {
                if (bInSection && !bWritten)
                {   // leaving the section without finding the key: insert
                    fprintf(out, "%s=%s\n", key, value);
                    bWritten = TRUE;
                }
                char tmp[256];
                PortStrCpyN(tmp, p+1, sizeof(tmp));
                char* e = strchr(tmp, ']');
                if (e != NULL) *e = 0;
                bInSection = (PortStriCmp(tmp, section) == 0);
                if (bInSection) bSectionSeen = TRUE;
                fputs(line, out);
            }
            else if (bInSection && !bWritten &&
                     PortStrniCmp(p, key, keyLen) == 0 &&
                     (p[keyLen] == '=' || p[keyLen] == ' ' || p[keyLen] == '\t'))
            {
                fprintf(out, "%s=%s\n", key, value);
                bWritten = TRUE;
            }
            else
            {
                fputs(line, out);
            }

            if (next != NULL) { next[1] = saved; line = next+1; }
            else line = NULL;
        }
    }

    if (!bWritten)
    {
        if (!bSectionSeen) fprintf(out, "[%s]\n", section);
        fprintf(out, "%s=%s\n", key, value);
    }

    fclose(out);
    free(content);
    return TRUE;
}

// -------------------------------------------------------------- file helpers

BOOL CopyFileA_shim(const char* src, const char* dst, BOOL failIfExists)
{
    if (failIfExists)
    {
        FILE* t = fopen(dst, "rb");
        if (t != NULL) { fclose(t); return FALSE; }
    }
    FILE* in = fopen(src, "rb");
    if (in == NULL) return FALSE;
    FILE* out = fopen(dst, "wb");
    if (out == NULL) { fclose(in); return FALSE; }
    char buffer[65536];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0)
        fwrite(buffer, 1, n, out);
    fclose(in);
    fclose(out);
    return TRUE;
}

BOOL DeleteFileA_shim(const char* path)
{
    return remove(path) == 0;
}

BOOL CreateDirectoryA_shim(const char* path, void* sa)
{
    (void)sa;
#ifdef _WIN32
    return _mkdir(path) == 0;
#else
    return mkdir(path, 0755) == 0;
#endif
}

BOOL RemoveDirectoryA_shim(const char* path)
{
#ifdef _WIN32
    return _rmdir(path) == 0;
#else
    return rmdir(path) == 0;
#endif
}

int MessageBox(HWND hWnd, const char* text, const char* title, UINT type)
{
    (void)hWnd; (void)type;
    SDL_Log("[MessageBox] %s: %s", title != NULL ? title : "", text != NULL ? text : "");
    return IDOK;
}

// --------------------------------------------------------------- input stubs

BOOL GetKeyboardState(BYTE* state)
{
    if (state != NULL) memset(state, 0, 256);
    return TRUE;
}

HANDLE GetCurrentProcess(void) { return (HANDLE)(intptr_t)-1; }
HANDLE GetCurrentThread(void)  { return (HANDLE)(intptr_t)-2; }
BOOL SetPriorityClass(HANDLE process, DWORD priorityClass)
{
    (void)process; (void)priorityClass;
    return TRUE;
}
BOOL SetThreadPriority(HANDLE thread, int priority)
{
    (void)thread; (void)priority;
    return TRUE;
}

// shlobj shim: CSIDL_PERSONAL resolves to the port's writable directory
extern "C" HRESULT SHGetSpecialFolderLocation(HWND hwnd, int csidl, void** ppidl)
{
    (void)hwnd; (void)csidl;
    if (ppidl != NULL) *ppidl = (void*)1;
    return S_OK;
}

extern "C" BOOL SHGetPathFromIDList(void* pidl, char* pszPath)
{
    (void)pidl;
    if (pszPath == NULL) return FALSE;
#ifdef __ANDROID__
    char* pref = SDL_GetPrefPath("epsitec", "blupimania2");
    if (pref != NULL)
    {
        strcpy(pszPath, pref);
        // strip the trailing separator: callers append "\\..." themselves
        size_t n = strlen(pszPath);
        if (n > 0 && (pszPath[n-1] == '/' || pszPath[n-1] == '\\')) pszPath[n-1] = 0;
        SDL_free(pref);
        return TRUE;
    }
#endif
    strcpy(pszPath, ".");
    return TRUE;
}

int  ShowCursor(BOOL bShow) { (void)bShow; return 0; }
BOOL SetCursorPos(int x, int y) { (void)x; (void)y; return TRUE; }
BOOL GetCursorPos(POINT* p) { if (p) { p->x = 0; p->y = 0; } return TRUE; }
short GetKeyState(int vk) { (void)vk; return 0; }

int LoadString(HINSTANCE inst, UINT id, char* buffer, int bufferMax)
{
    (void)inst; (void)id;
    if (buffer != NULL && bufferMax > 0) buffer[0] = 0;
    return 0;
}

UINT timeBeginPeriod(UINT period) { (void)period; return 0; }
UINT timeEndPeriod(UINT period)   { (void)period; return 0; }

HCURSOR LoadCursor(HINSTANCE inst, const char* name) { (void)inst; (void)name; return NULL; }
HCURSOR SetCursor(HCURSOR cursor) { (void)cursor; return NULL; }

// ---------------------------------------------------------------- clipboard

HGLOBAL GlobalAlloc(UINT flags, size_t size)
{
    (void)flags;
    return (HGLOBAL)calloc(1, size > 0 ? size : 1);
}

void* GlobalLock(HGLOBAL h)   { return (void*)h; }
BOOL  GlobalUnlock(HGLOBAL h) { (void)h; return TRUE; }
HGLOBAL GlobalFree(HGLOBAL h) { free(h); return NULL; }

BOOL OpenClipboard(HWND hWnd)  { (void)hWnd; return TRUE; }
BOOL EmptyClipboard(void)      { return TRUE; }

HANDLE SetClipboardData(UINT format, HANDLE mem)
{
    if (format == CF_TEXT && mem != NULL)
    {
        SDL_SetClipboardText((const char*)mem);
        free(mem);      // clipboard takes ownership in Win32 semantics
    }
    return mem;
}

// returns a fresh copy each call; Win32 apps do not free clipboard handles,
// so the copy is intentionally left to leak (a few bytes per paste)
HANDLE GetClipboardData(UINT format)
{
    if (format != CF_TEXT) return NULL;
    if (!SDL_HasClipboardText()) return NULL;
    char* text = SDL_GetClipboardText();
    if (text == NULL) return NULL;
    char* copy = (char*)malloc(strlen(text)+1);
    strcpy(copy, text);
    SDL_free(text);
    return (HANDLE)copy;
}

BOOL CloseClipboard(void) { return TRUE; }

// ------------------------------------------------------------ registry stubs

LONG RegOpenKeyEx(HKEY root, const char* sub, DWORD opt, DWORD sam, HKEY* out)
{
    (void)root; (void)sub; (void)opt; (void)sam;
    if (out != NULL) *out = NULL;
    return 1;   // != ERROR_SUCCESS
}

LONG RegQueryValueEx(HKEY key, const char* name, DWORD* rsv, DWORD* type,
                     BYTE* data, DWORD* size)
{
    (void)key; (void)name; (void)rsv; (void)type; (void)data; (void)size;
    return 1;
}

LONG RegCloseKey(HKEY key) { (void)key; return 0; }

// ---------------------------------------------------------- Win32 file API

HANDLE CreateFile(const char* name, DWORD access, DWORD share, void* sa,
                  DWORD disposition, DWORD flags, HANDLE tmpl)
{
    (void)share; (void)sa; (void)flags; (void)tmpl;
    const char* mode;
    if (access & GENERIC_WRITE)
        mode = (disposition == CREATE_ALWAYS) ? "wb" : "r+b";
    else
        mode = "rb";
    FILE* f = fopen(name, mode);
    if (f == NULL) return INVALID_HANDLE_VALUE;
    return (HANDLE)f;
}

BOOL ReadFile(HANDLE h, void* buf, DWORD toRead, DWORD* read, void* ov)
{
    (void)ov;
    if (h == INVALID_HANDLE_VALUE || h == NULL) return FALSE;
    size_t n = fread(buf, 1, toRead, (FILE*)h);
    if (read != NULL) *read = (DWORD)n;
    return TRUE;
}

BOOL WriteFile(HANDLE h, const void* buf, DWORD toWrite, DWORD* written, void* ov)
{
    (void)ov;
    if (h == INVALID_HANDLE_VALUE || h == NULL) return FALSE;
    size_t n = fwrite(buf, 1, toWrite, (FILE*)h);
    if (written != NULL) *written = (DWORD)n;
    return n == toWrite;
}

BOOL CloseHandle(HANDLE h)
{
    if (h == INVALID_HANDLE_VALUE || h == NULL) return FALSE;
    fclose((FILE*)h);
    return TRUE;
}
