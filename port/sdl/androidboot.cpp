// androidboot.cpp — first-launch bootstrap on Android:
// extracts the game data from the APK assets into internal storage and
// chdir()s there, so the game's relative fopen() paths work unchanged.
//
// Assets layout in the APK:
//   data/assets.lst   — newline-separated list of all data files
//   data/data.ver     — data version stamp (re-extract when it changes)
//   data/<files...>   — the game data itself

#ifdef __ANDROID__

#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

static void MakeDirsFor(const char* path)
{
    char tmp[600];
    strncpy(tmp, path, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = 0;
    for (char* p = tmp+1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
}

static char* ReadAssetAll(const char* name, size_t* outSize)
{
    SDL_RWops* rw = SDL_RWFromFile(name, "rb");
    if (rw == NULL) return NULL;
    Sint64 size = SDL_RWsize(rw);
    if (size < 0) { SDL_RWclose(rw); return NULL; }
    char* buf = (char*)malloc((size_t)size + 1);
    SDL_RWread(rw, buf, 1, (size_t)size);
    SDL_RWclose(rw);
    buf[size] = 0;
    if (outSize != NULL) *outSize = (size_t)size;
    return buf;
}

static int CopyAssetToFile(const char* assetName, const char* destPath)
{
    SDL_RWops* in = SDL_RWFromFile(assetName, "rb");
    if (in == NULL)
    {
        SDL_Log("bootstrap: missing asset %s", assetName);
        return 0;
    }
    MakeDirsFor(destPath);
    FILE* out = fopen(destPath, "wb");
    if (out == NULL)
    {
        SDL_Log("bootstrap: cannot write %s (%s)", destPath, strerror(errno));
        SDL_RWclose(in);
        return 0;
    }
    char buffer[65536];
    size_t n;
    while ((n = SDL_RWread(in, buffer, 1, sizeof(buffer))) > 0)
        fwrite(buffer, 1, n, out);
    fclose(out);
    SDL_RWclose(in);
    return 1;
}

extern "C" void PortAndroidBootstrap(void)
{
    const char* internal = SDL_AndroidGetInternalStoragePath();
    if (internal == NULL)
    {
        SDL_Log("bootstrap: no internal storage path!");
        return;
    }
    mkdir(internal, 0755);
    chdir(internal);
    SDL_Log("bootstrap: internal storage = %s", internal);

    // compare data versions
    char* assetVer = ReadAssetAll("data/data.ver", NULL);
    char  localVer[64] = "";
    FILE* f = fopen("data.ver", "rb");
    if (f != NULL)
    {
        size_t n = fread(localVer, 1, sizeof(localVer)-1, f);
        localVer[n] = 0;
        fclose(f);
    }

    if (assetVer != NULL && strcmp(assetVer, localVer) == 0)
    {
        SDL_Log("bootstrap: data up to date (%s)", localVer);
        free(assetVer);
        return;
    }

    SDL_Log("bootstrap: extracting game data...");
    size_t listSize = 0;
    char* list = ReadAssetAll("data/assets.lst", &listSize);
    if (list == NULL)
    {
        SDL_Log("bootstrap: data/assets.lst missing from APK!");
        free(assetVer);
        return;
    }

    int total = 0, ok = 0;
    char* line = strtok(list, "\r\n");
    while (line != NULL)
    {
        if (line[0] != 0)
        {
            char assetName[600], destPath[600];
            snprintf(assetName, sizeof(assetName), "data/%s", line);
            snprintf(destPath, sizeof(destPath), "%s", line);
            total++;
            if (CopyAssetToFile(assetName, destPath)) ok++;
        }
        line = strtok(NULL, "\r\n");
    }
    free(list);

    if (assetVer != NULL)
    {
        FILE* v = fopen("data.ver", "wb");
        if (v != NULL)
        {
            fwrite(assetVer, 1, strlen(assetVer), v);
            fclose(v);
        }
        free(assetVer);
    }
    SDL_Log("bootstrap: extracted %d/%d files", ok, total);
}

#endif // __ANDROID__
