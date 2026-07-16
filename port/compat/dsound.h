// dsound.h — DirectSound type shim. The real audio engine is a custom SDL2
// mixer (port/sdl/sdlsound.cpp). sound.h keeps its DirectSound-typed members;
// here those types map onto the port's own buffer object.

#ifndef _PORT_DSOUND_H_
#define _PORT_DSOUND_H_

#include <windows.h>

// A loaded sound: PCM data decoded to the mixer's native format
// (mono/stereo float32 at the device rate is handled inside the mixer).
struct PortSoundBuffer
{
    short*   samples;       // interleaved 16-bit PCM
    int      sampleCount;   // per channel
    int      channels;      // 1 or 2
    int      sampleRate;    // source rate (Hz)
    // playback state (owned by the mixer)
    double   cursor;        // fractional read position in frames
    BOOL     bPlaying;
    BOOL     bLoop;
    float    volume;        // 0..1
    float    pan;           // -1..+1
    float    pitch;         // frequency multiplier
};

typedef PortSoundBuffer* LPDIRECTSOUNDBUFFER;
typedef PortSoundBuffer* LPDIRECTSOUND3DBUFFER;   // same object; 3D handled by mixer
typedef void*            LPDIRECTSOUND;
typedef void*            LPDIRECTSOUND3DLISTENER;

#define DSBSTATUS_PLAYING 0x00000001

#endif // _PORT_DSOUND_H_
