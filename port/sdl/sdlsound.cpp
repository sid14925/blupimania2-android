// sdlsound.cpp — CSound implemented on a custom SDL2 audio mixer
// (replaces src/sound.cpp). Supports per-channel volume, stereo pan, pitch
// (frequency) changes and the game's amplitude/frequency envelope system,
// which SDL_mixer cannot do — hence the hand-rolled mixer.

#define STRICT
#define D3D_OVERLOADS

#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <math.h>

#include <SDL.h>

#include "struct.h"
#include "D3DEngine.h"
#include "language.h"
#include "misc.h"
#include "iman.h"
#include "math3d.h"
#include "metafile.h"
#include "sound.h"

extern CMetaFile g_metafile;

#define AMPFAC      0.2f
#define FACTOR_AW   0.5f    // underwater amplitude factor
#define FACTOR_FW   0.75f   // underwater frequency factor

#define DEVICE_RATE 44100

// DirectSound-style attenuation curve -> linear gain
static float VolumeToGain(float volume)
{
    if (volume <= 0.0f) return 0.0f;
    if (volume >= 1.0f) volume = 1.0f;
    float dB100 = (powf(volume, AMPFAC) - 1.0f) * 10000.0f;   // -10000..0
    return powf(10.0f, dB100 / 2000.0f);
}

// ---------------------------------------------------------------------------
// mixer
// ---------------------------------------------------------------------------

static SDL_AudioDeviceID g_audioDevice = 0;
static CSound*           g_soundInstance = NULL;
static SoundChannel*     g_mixChannels = NULL;   // -> CSound::m_channel

static void AudioCallback(void* userdata, Uint8* stream, int len);

// PortSoundBuffer (typedef'd as LPDIRECTSOUNDBUFFER in the dsound.h shim)
// carries both the decoded PCM and the playback state; defined in dsound.h.

static PortSoundBuffer* LoadWavFromMetafile(char* metaname, char* filename)
{
    if (g_metafile.Open(metaname, filename, (char*)"b") != 0) return NULL;

    int len = g_metafile.RetLength();
    if (len <= 44) { g_metafile.Close(); return NULL; }

    unsigned char* data = (unsigned char*)malloc((size_t)len);
    g_metafile.Read(data, len);
    g_metafile.Close();

    // parse RIFF/WAVE
    if (memcmp(data, "RIFF", 4) != 0 || memcmp(data+8, "WAVE", 4) != 0)
    {
        free(data);
        return NULL;
    }

    int      pos = 12;
    int      fmtChannels = 0, fmtRate = 0, fmtBits = 0;
    unsigned char* pcm = NULL;
    int      pcmBytes = 0;

    while (pos + 8 <= len)
    {
        const unsigned char* chunk = data + pos;
        int chunkSize = *(const int*)(chunk+4);
        if (memcmp(chunk, "fmt ", 4) == 0 && chunkSize >= 16)
        {
            int audioFormat = *(const short*)(chunk+8);
            fmtChannels     = *(const short*)(chunk+10);
            fmtRate         = *(const int*)(chunk+12);
            fmtBits         = *(const short*)(chunk+22);
            if (audioFormat != 1) { free(data); return NULL; }   // PCM only
        }
        else if (memcmp(chunk, "data", 4) == 0)
        {
            pcm = (unsigned char*)(chunk+8);
            pcmBytes = chunkSize;
            if (pos + 8 + pcmBytes > len) pcmBytes = len - pos - 8;
        }
        pos += 8 + chunkSize + (chunkSize & 1);
    }

    if (pcm == NULL || fmtChannels < 1 || fmtChannels > 2 || fmtRate <= 0 ||
        (fmtBits != 8 && fmtBits != 16))
    {
        free(data);
        return NULL;
    }

    PortSoundBuffer* buf = (PortSoundBuffer*)calloc(1, sizeof(PortSoundBuffer));
    buf->channels    = fmtChannels;
    buf->sampleRate  = fmtRate;
    buf->sampleCount = pcmBytes / (fmtBits/8) / fmtChannels;
    buf->samples     = (short*)malloc((size_t)buf->sampleCount * fmtChannels * sizeof(short));
    buf->pitch       = 1.0f;
    buf->volume      = 1.0f;

    if (fmtBits == 16)
    {
        memcpy(buf->samples, pcm, (size_t)buf->sampleCount * fmtChannels * 2);
    }
    else
    {
        int n = buf->sampleCount * fmtChannels;
        for (int i = 0; i < n; i++)
            buf->samples[i] = (short)(((int)pcm[i] - 128) << 8);
    }

    free(data);
    return buf;
}

static void FreeSoundBuffer(PortSoundBuffer* buf)
{
    if (buf == NULL) return;
    free(buf->samples);
    free(buf);
}

// clones PCM by reference for playing the same sound on several channels:
// each channel gets its own PortSoundBuffer header but shares samples via
// the cache below, so headers own no sample memory
struct WavCacheEntry
{
    short* samples;
    int    sampleCount;
    int    channels;
    int    sampleRate;
    BOOL   bLoaded;
};
static WavCacheEntry g_wavCache[MAXFILES];

// ---------------------------------------------------------------------------
// CSound
// ---------------------------------------------------------------------------

CSound::CSound(CInstanceManager* iMan)
{
    m_iMan = iMan;
    if (m_iMan != NULL) m_iMan->AddInstance(CLASS_SOUND, this);

    m_bEnable      = FALSE;
    m_bState       = FALSE;
    m_bAudioTrack  = FALSE;
    m_ctrl3D       = FALSE;
    m_bComments    = TRUE;
    m_bWater       = FALSE;
    m_bDebugMode   = FALSE;
    m_lpDS         = NULL;
    m_listener     = NULL;
    m_MidiDeviceID = 0;
    m_MIDIMusic    = 0;
    m_bRepeatMusic = FALSE;
    m_audioVolume  = MAXVOLUME;
    m_midiVolume   = MAXVOLUME;
    m_lastMidiVolume = 0;
    m_lastTime     = 0.0f;
    m_playTime     = 0.0f;
    m_uniqueStamp  = 0;
    m_maxSound     = MAXSOUND;
    m_eye          = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_lookat       = D3DVECTOR(0.0f, 0.0f, 0.0f);
    m_hWnd         = NULL;
    m_engine       = NULL;
    m_CDpath[0]    = 0;

    memset(m_channel, 0, sizeof(m_channel));
    for (int i = 0; i < MAXFILES; i++) m_files[i] = NULL;
    memset(g_wavCache, 0, sizeof(g_wavCache));

    g_soundInstance = this;
}

CSound::~CSound()
{
    g_soundInstance = NULL;
    if (g_audioDevice != 0)
    {
        SDL_CloseAudioDevice(g_audioDevice);
        g_audioDevice = 0;
    }
    for (int i = 0; i < MAXSOUND; i++)
    {
        FreeSoundBuffer(m_channel[i].soundBuffer);
        m_channel[i].soundBuffer = NULL;
    }
    for (int i = 0; i < MAXFILES; i++)
    {
        free(g_wavCache[i].samples);
        g_wavCache[i].samples = NULL;
    }
}

void CSound::SetDebugMode(BOOL bMode)
{
    m_bDebugMode = bMode;
}

BOOL CSound::Create(HWND hWnd, BOOL b3D)
{
    (void)b3D;
    m_hWnd = hWnd;
    m_iMan->AddInstance(CLASS_SOUND, this);
    m_engine = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq     = DEVICE_RATE;
    want.format   = AUDIO_S16SYS;
    want.channels = 2;
    want.samples  = 1024;
    want.callback = AudioCallback;
    want.userdata = this;

    g_audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (g_audioDevice == 0)
    {
        SDL_Log("SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return FALSE;
    }
    g_mixChannels = m_channel;
    SDL_PauseAudioDevice(g_audioDevice, 0);

    m_bEnable = TRUE;
    m_bState  = TRUE;
    m_ctrl3D  = FALSE;      // 3D positioning is emulated with volume/pan
    return TRUE;
}

// preload every WAV from blupimania3.dat into the cache
void CSound::CacheAll()
{
    int  i;
    char meta[50];
    char name[50];

    if (!m_bEnable) return;

    if (m_bDebugMode) strcpy(meta, "");
    else              strcpy(meta, "blupimania3.dat");

    for (i = 0; i < MAXFILES; i++)
    {
        if (m_bDebugMode) sprintf(name, "sound\\sound%.3d.wav", i);
        else              sprintf(name, "sound%.3d.wav", i);

        if (!ReadFile((Sound)i, meta, name)) break;
    }
}

BOOL CSound::ReadFile(Sound sound, char *metaname, char *filename)
{
    PortSoundBuffer* buf = LoadWavFromMetafile(metaname, filename);
    if (buf == NULL) return FALSE;

    free(g_wavCache[sound].samples);
    g_wavCache[sound].samples     = buf->samples;
    g_wavCache[sound].sampleCount = buf->sampleCount;
    g_wavCache[sound].channels    = buf->channels;
    g_wavCache[sound].sampleRate  = buf->sampleRate;
    g_wavCache[sound].bLoaded     = TRUE;
    m_files[sound] = (char*)&g_wavCache[sound];

    buf->samples = NULL;    // ownership moved to the cache
    free(buf);
    return TRUE;
}

void CSound::SetState(BOOL bState)      { m_bState = bState; }
BOOL CSound::RetEnable()                { return m_bEnable; }
void CSound::SetCDpath(char *path)      { strcpy(m_CDpath, path != NULL ? path : ""); }
void CSound::SetAudioTrack(BOOL bAudio) { m_bAudioTrack = bAudio; }
void CSound::SetSound3D(BOOL bMode)     { (void)bMode; }
BOOL CSound::RetSound3D()               { return FALSE; }
BOOL CSound::RetSound3DCap()            { return FALSE; }
void CSound::SetComments(BOOL bMode)    { m_bComments = bMode; }
BOOL CSound::RetComments()              { return m_bComments; }
void CSound::SetWater(BOOL bWater)      { m_bWater = bWater; }
BOOL CSound::RetWater()                 { return m_bWater; }

void CSound::SetAudioVolume(int volume)
{
    if (volume < 0) volume = 0;
    if (volume > MAXVOLUME) volume = MAXVOLUME;
    m_audioVolume = volume;
}
int CSound::RetAudioVolume() { return m_audioVolume; }

void CSound::SetMidiVolume(int volume)
{
    if (volume < 0) volume = 0;
    if (volume > MAXVOLUME) volume = MAXVOLUME;
    m_midiVolume = volume;
}
int CSound::RetMidiVolume() { return m_midiVolume; }

void CSound::SetListener(D3DVECTOR eye, D3DVECTOR lookat)
{
    m_eye = eye;
    m_lookat = lookat;
}

// ---------------------------------------------------------------------------
// channels
// ---------------------------------------------------------------------------

BOOL CSound::CheckChannel(int &channel)
{
    int uniqueStamp = (channel >> 16) & 0xffff;
    channel &= 0xffff;

    if (!m_bEnable) return FALSE;
    if (!m_bState || m_audioVolume == 0) return FALSE;
    if (channel < 0 || channel >= m_maxSound) return FALSE;
    if (!m_channel[channel].bUsed) return FALSE;
    if (m_channel[channel].uniqueStamp != uniqueStamp) return FALSE;
    return TRUE;
}

int CSound::RetPriority(Sound sound)
{
    if (sound == SOUND_MESSAGE || sound == SOUND_ERROR) return 30;
    return 10;
}

BOOL CSound::SearchFreeBuffer(Sound sound, int &channel, BOOL &bAlreadyLoaded)
{
    int  i;
    int  priority = RetPriority(sound);

    bAlreadyLoaded = FALSE;

    SDL_LockAudioDevice(g_audioDevice);

    // an unused channel that already carries this sound?
    for (i = 0; i < m_maxSound; i++)
    {
        if (m_channel[i].bUsed &&
            m_channel[i].type == sound &&
            m_channel[i].soundBuffer != NULL &&
            !m_channel[i].soundBuffer->bPlaying)
        {
            bAlreadyLoaded = TRUE;
            channel = i;
            SDL_UnlockAudioDevice(g_audioDevice);
            return TRUE;
        }
    }

    // a completely free channel?
    for (i = 0; i < m_maxSound; i++)
    {
        if (!m_channel[i].bUsed)
        {
            channel = i;
            SDL_UnlockAudioDevice(g_audioDevice);
            return TRUE;
        }
    }

    // a finished channel?
    for (i = 0; i < m_maxSound; i++)
    {
        if (m_channel[i].soundBuffer != NULL &&
            !m_channel[i].soundBuffer->bPlaying)
        {
            FreeSoundBuffer(m_channel[i].soundBuffer);
            m_channel[i].soundBuffer = NULL;
            m_channel[i].bUsed = FALSE;
            channel = i;
            SDL_UnlockAudioDevice(g_audioDevice);
            return TRUE;
        }
    }

    // steal the lowest-priority channel
    int best = -1, bestPrio = priority;
    for (i = 0; i < m_maxSound; i++)
    {
        if (m_channel[i].priority < bestPrio)
        {
            bestPrio = m_channel[i].priority;
            best = i;
        }
    }
    if (best >= 0)
    {
        FreeSoundBuffer(m_channel[best].soundBuffer);
        m_channel[best].soundBuffer = NULL;
        m_channel[best].bUsed = FALSE;
        channel = best;
        SDL_UnlockAudioDevice(g_audioDevice);
        return TRUE;
    }

    SDL_UnlockAudioDevice(g_audioDevice);
    return FALSE;
}

BOOL CSound::CreateBuffer(int channel, Sound sound)
{
    if (sound < 0 || sound >= MAXFILES) return FALSE;
    if (!g_wavCache[sound].bLoaded) return FALSE;

    PortSoundBuffer* buf = (PortSoundBuffer*)calloc(1, sizeof(PortSoundBuffer));
    buf->samples     = g_wavCache[sound].samples;   // shared, not owned
    buf->sampleCount = g_wavCache[sound].sampleCount;
    buf->channels    = g_wavCache[sound].channels;
    buf->sampleRate  = g_wavCache[sound].sampleRate;
    buf->pitch       = 1.0f;
    buf->volume      = 1.0f;

    m_channel[channel].soundBuffer   = buf;
    m_channel[channel].soundBuffer3D = buf;
    m_channel[channel].type          = sound;
    m_channel[channel].bUsed         = TRUE;
    m_channel[channel].initFrequency = buf->sampleRate;
    return TRUE;
}

BOOL CSound::CreateSoundBuffer(int channel, DWORD size, DWORD freq,
                               DWORD bitsPerSample, DWORD blkAlign, BOOL bStereo)
{
    (void)channel; (void)size; (void)freq;
    (void)bitsPerSample; (void)blkAlign; (void)bStereo;
    return FALSE;
}

BOOL CSound::ReadData(LPDIRECTSOUNDBUFFER lpDSB, Sound sound, DWORD size)
{
    (void)lpDSB; (void)sound; (void)size;
    return FALSE;
}

void CSound::ComputeVolumePan2D(int channel, const D3DVECTOR &pos)
{
    float dist, a, g;

    if (pos.x == m_eye.x && pos.y == m_eye.y && pos.z == m_eye.z)
    {
        m_channel[channel].volume = 1.0f;
        m_channel[channel].pan    = 0.0f;
        return;
    }

    dist = Length(pos, m_eye);
    if (dist >= 210.0f)
    {
        m_channel[channel].volume = 0.0f;
        m_channel[channel].pan    = 0.0f;
        return;
    }
    if (dist <= 10.0f)
    {
        m_channel[channel].volume = 1.0f;
        m_channel[channel].pan    = 0.0f;
        return;
    }
    m_channel[channel].volume = 1.0f - ((dist-10.0f)/200.0f);

    a = RotateAngle(m_lookat.x-m_eye.x, m_eye.z-m_lookat.z);
    g = RotateAngle(pos.x-m_eye.x, m_eye.z-pos.z);
    m_channel[channel].pan = sinf(Direction(a, g));
}

static void ApplyChannelState(SoundChannel& ch, int audioVolume)
{
    PortSoundBuffer* buf = ch.soundBuffer;
    if (buf == NULL) return;

    float volume = ch.currentAmplitude;
    if (ch.bMute) volume = 0.0f;
    buf->volume = VolumeToGain(volume);
    buf->pan    = ch.pan;
    buf->pitch  = (ch.currentFrequency > 0.0f) ? ch.currentFrequency : 1.0f;
    (void)audioVolume;
}

int CSound::Play(Sound sound, float amplitude, float frequency, BOOL bLoop)
{
    return Play(sound, m_eye, amplitude, frequency, bLoop);
}

int CSound::Play(Sound sound, D3DVECTOR pos,
                 float amplitude, float frequency, BOOL bLoop)
{
    int   channel;
    BOOL  bAlreadyLoaded;
    float baseAmplitude;

    if (!m_bEnable) return -1;
    if (!m_bState || m_audioVolume == 0) return -1;
    if (sound < 0 || sound >= MAXFILES) return -1;
    if (m_engine == NULL)
    {
        m_engine = (CD3DEngine*)m_iMan->SearchInstance(CLASS_ENGINE);
        if (m_engine == NULL) return -1;
    }

    if (sound == SOUND_BLUPIshibi  || sound == SOUND_BLUPIouaaa  ||
        sound == SOUND_BLUPIhic    || sound == SOUND_BLUPIoups   ||
        sound == SOUND_BLUPIpfiou  || sound == SOUND_BLUPInon    ||
        sound == SOUND_BLUPIpousse || sound == SOUND_BLUPIeffort ||
        sound == SOUND_BLUPIaie    || sound == SOUND_BLUPIhhuu   ||
        sound == SOUND_BLUPIohhh   || sound == SOUND_BLUPIgrrr   ||
        sound == SOUND_BLUPIpeur   || sound == SOUND_BLUPIslurp  ||
        sound == SOUND_BLUPIouaou  || sound == SOUND_BLUPIblibli)
    {
        baseAmplitude = m_engine->RetSetup(ST_VOLBLUPI);
    }
    else if (sound == SOUND_BLUP  || sound == SOUND_PSHHH ||
             sound == SOUND_FLIC1 || sound == SOUND_FLIC2 ||
             sound == SOUND_FLIC3)
    {
        baseAmplitude = m_engine->RetSetup(ST_VOLAMBIANCE);
    }
    else
    {
        baseAmplitude = m_engine->RetSetup(ST_VOLSOUND);
    }
    if (baseAmplitude == 0.0f) return -1;

    amplitude *= baseAmplitude;

    if (m_bWater)
    {
        amplitude *= FACTOR_AW;
        frequency *= FACTOR_FW;
    }

    if (!SearchFreeBuffer(sound, channel, bAlreadyLoaded)) return -1;

    SDL_LockAudioDevice(g_audioDevice);

    if (!bAlreadyLoaded)
    {
        if (!CreateBuffer(channel, sound))
        {
            m_channel[channel].bUsed = FALSE;
            SDL_UnlockAudioDevice(g_audioDevice);
            return -1;
        }
    }

    SoundChannel& ch = m_channel[channel];
    ch.pos = pos;
    ComputeVolumePan2D(channel, pos);

    ch.priority         = RetPriority(sound);
    ch.baseAmplitude    = amplitude;
    ch.startAmplitude   = 1.0f;
    ch.changeAmplitude  = 1.0f;
    ch.currentAmplitude = amplitude * ch.volume * ((float)m_audioVolume/MAXVOLUME);
    ch.startFrequency   = frequency;
    ch.changeFrequency  = 1.0f;
    ch.currentFrequency = frequency;
    ch.bMute            = FALSE;
    ch.uniqueStamp      = (unsigned short)(m_uniqueStamp++ & 0xffff);
    memset(ch.oper, 0, sizeof(ch.oper));

    PortSoundBuffer* buf = ch.soundBuffer;
    buf->cursor   = 0.0;
    buf->bLoop    = bLoop;
    buf->bPlaying = TRUE;
    ApplyChannelState(ch, m_audioVolume);

    SDL_UnlockAudioDevice(g_audioDevice);

    return channel | (((int)ch.uniqueStamp & 0xffff) << 16);
}

BOOL CSound::FlushEnvelope(int channel)
{
    if (!CheckChannel(channel)) return FALSE;
    memset(m_channel[channel].oper, 0, sizeof(m_channel[channel].oper));
    return TRUE;
}

BOOL CSound::AddEnvelope(int channel, float amplitude, float frequency,
                         float time, SoundNext oper)
{
    if (!CheckChannel(channel)) return FALSE;

    SoundChannel& ch = m_channel[channel];
    for (int i = 0; i < MAXOPER; i++)
    {
        if (!ch.oper[i].bUsed)
        {
            ch.oper[i].bUsed          = TRUE;
            ch.oper[i].finalAmplitude = amplitude;
            ch.oper[i].finalFrequency = frequency;
            ch.oper[i].totalTime      = time;
            ch.oper[i].currentTime    = 0.0f;
            ch.oper[i].nextOper       = oper;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CSound::Position(int channel, D3DVECTOR pos)
{
    if (!CheckChannel(channel)) return FALSE;
    m_channel[channel].pos = pos;
    ComputeVolumePan2D(channel, pos);
    SDL_LockAudioDevice(g_audioDevice);
    ApplyChannelState(m_channel[channel], m_audioVolume);
    SDL_UnlockAudioDevice(g_audioDevice);
    return TRUE;
}

BOOL CSound::Amplitude(int channel, float amplitude)
{
    if (!CheckChannel(channel)) return FALSE;
    SoundChannel& ch = m_channel[channel];
    ch.changeAmplitude = amplitude;
    ch.currentAmplitude = ch.baseAmplitude * amplitude * ch.volume *
                          ((float)m_audioVolume/MAXVOLUME);
    SDL_LockAudioDevice(g_audioDevice);
    ApplyChannelState(ch, m_audioVolume);
    SDL_UnlockAudioDevice(g_audioDevice);
    return TRUE;
}

BOOL CSound::Frequency(int channel, float frequency)
{
    if (!CheckChannel(channel)) return FALSE;
    SoundChannel& ch = m_channel[channel];
    ch.changeFrequency = frequency;
    ch.currentFrequency = ch.startFrequency * frequency;
    SDL_LockAudioDevice(g_audioDevice);
    ApplyChannelState(ch, m_audioVolume);
    SDL_UnlockAudioDevice(g_audioDevice);
    return TRUE;
}

float CSound::RetAmplitude(int channel)
{
    if (!CheckChannel(channel)) return 0.0f;
    return m_channel[channel].currentAmplitude;
}

float CSound::RetFrequency(int channel)
{
    if (!CheckChannel(channel)) return 1.0f;
    return m_channel[channel].currentFrequency;
}

BOOL CSound::Stop(int channel)
{
    if (!CheckChannel(channel)) return FALSE;
    SDL_LockAudioDevice(g_audioDevice);
    if (m_channel[channel].soundBuffer != NULL)
        m_channel[channel].soundBuffer->bPlaying = FALSE;
    SDL_UnlockAudioDevice(g_audioDevice);
    return TRUE;
}

BOOL CSound::StopAll()
{
    SDL_LockAudioDevice(g_audioDevice);
    for (int i = 0; i < m_maxSound; i++)
    {
        if (m_channel[i].soundBuffer != NULL)
            m_channel[i].soundBuffer->bPlaying = FALSE;
        m_channel[i].bUsed = FALSE;
    }
    SDL_UnlockAudioDevice(g_audioDevice);
    return TRUE;
}

BOOL CSound::MuteAll(BOOL bMute)
{
    SDL_LockAudioDevice(g_audioDevice);
    for (int i = 0; i < m_maxSound; i++)
    {
        m_channel[i].bMute = bMute;
        ApplyChannelState(m_channel[i], m_audioVolume);
    }
    SDL_UnlockAudioDevice(g_audioDevice);
    return TRUE;
}

// ---------------------------------------------------------------------------
// envelopes
// ---------------------------------------------------------------------------

void CSound::OperNext(int channel)
{
    SoundChannel& ch = m_channel[channel];
    for (int i = 0; i < MAXOPER-1; i++)
    {
        ch.oper[i] = ch.oper[i+1];
    }
    ch.oper[MAXOPER-1].bUsed = FALSE;

    ch.startAmplitude = ch.currentAmplitude /
        ((ch.baseAmplitude*ch.changeAmplitude*ch.volume) > 0.0f ?
         (ch.baseAmplitude*ch.changeAmplitude*ch.volume) : 1.0f);
    ch.startFrequency = ch.currentFrequency /
        (ch.changeFrequency > 0.0f ? ch.changeFrequency : 1.0f);
}

void CSound::FrameMove(float rTime)
{
    SoundNext next;
    float     progress, volume, freq;
    int       i;

    m_playTime += rTime;

    SDL_LockAudioDevice(g_audioDevice);

    for (i = 0; i < m_maxSound; i++)
    {
        SoundChannel& ch = m_channel[i];
        if (!ch.bUsed) continue;
        if (ch.soundBuffer == NULL) continue;

        if (ch.bMute)
        {
            ch.soundBuffer->volume = 0.0f;
            continue;
        }

        if (!ch.oper[0].bUsed) continue;

        ch.oper[0].currentTime += rTime;

        progress = ch.oper[0].currentTime / ch.oper[0].totalTime;
        if (progress > 1.0f) progress = 1.0f;

        volume = progress;
        volume *= ch.oper[0].finalAmplitude - ch.startAmplitude;
        volume += ch.startAmplitude;
        volume *= ch.baseAmplitude;
        volume *= ch.changeAmplitude;
        volume *= ch.volume;
        volume *= (float)m_audioVolume/MAXVOLUME;
        ch.currentAmplitude = volume;

        freq = progress;
        freq *= ch.oper[0].finalFrequency - ch.startFrequency;
        freq += ch.startFrequency;
        freq *= ch.changeFrequency;
        ch.currentFrequency = freq;

        ApplyChannelState(ch, m_audioVolume);

        if (ch.oper[0].currentTime >= ch.oper[0].totalTime)
        {
            next = ch.oper[0].nextOper;
            if (next == SOPER_LOOP)
            {
                ch.oper[0].currentTime = 0.0f;
            }
            else
            {
                OperNext(i);
                if (next == SOPER_STOP)
                {
                    ch.soundBuffer->bPlaying = FALSE;
                }
            }
        }
    }

    SDL_UnlockAudioDevice(g_audioDevice);
}

// ---------------------------------------------------------------------------
// music — optional OGG/WAV tracks are not part of the RIP data; stubbed
// ---------------------------------------------------------------------------

BOOL CSound::PlayMusic(int rank, BOOL bRepeat)
{
    (void)rank; (void)bRepeat;
    return FALSE;
}
BOOL CSound::PlayAudioTrack(int rank) { (void)rank; return FALSE; }
BOOL CSound::RestartMusic()           { return FALSE; }
void CSound::SuspendMusic()           { }
void CSound::StopMusic()              { }
BOOL CSound::IsPlayingMusic()         { return FALSE; }
void CSound::AdaptVolumeMusic()       { }

// ---------------------------------------------------------------------------
// the mixer callback
// ---------------------------------------------------------------------------

static void AudioCallback(void* userdata, Uint8* stream, int len)
{
    CSound* snd = (CSound*)userdata;
    short* out = (short*)stream;
    int frames = len / 4;   // stereo S16

    memset(stream, 0, (size_t)len);
    if (snd == NULL) return;

    static float mixL[4096], mixR[4096];
    if (frames > 4096) frames = 4096;
    memset(mixL, 0, sizeof(float)*frames);
    memset(mixR, 0, sizeof(float)*frames);

    SoundChannel* channels = g_mixChannels;
    if (channels == NULL) return;

    for (int c = 0; c < MAXSOUND; c++)
    {
        PortSoundBuffer* buf = channels[c].soundBuffer;
        if (buf == NULL || !buf->bPlaying || buf->samples == NULL) continue;

        float pan = buf->pan;
        if (pan < -1.0f) pan = -1.0f;
        if (pan > 1.0f)  pan = 1.0f;
        float gainL = buf->volume * cosf((pan+1.0f)*0.7853982f);
        float gainR = buf->volume * sinf((pan+1.0f)*0.7853982f);

        double step = (double)buf->sampleRate * (double)buf->pitch / (double)DEVICE_RATE;
        if (step < 0.01) step = 0.01;
        if (step > 16.0) step = 16.0;

        double cursor = buf->cursor;
        int nch = buf->channels;
        int total = buf->sampleCount;

        for (int i = 0; i < frames; i++)
        {
            int idx = (int)cursor;
            if (idx >= total-1)
            {
                if (buf->bLoop)
                {
                    cursor -= total-1;
                    idx = (int)cursor;
                    if (idx >= total-1) { idx = 0; cursor = 0.0; }
                }
                else
                {
                    buf->bPlaying = FALSE;
                    break;
                }
            }
            float frac = (float)(cursor - idx);
            float sL, sR;
            if (nch == 1)
            {
                float s = buf->samples[idx]*(1.0f-frac) + buf->samples[idx+1]*frac;
                sL = sR = s;
            }
            else
            {
                sL = buf->samples[idx*2]*(1.0f-frac)   + buf->samples[(idx+1)*2]*frac;
                sR = buf->samples[idx*2+1]*(1.0f-frac) + buf->samples[(idx+1)*2+1]*frac;
            }
            mixL[i] += sL * gainL;
            mixR[i] += sR * gainR;
            cursor += step;
        }
        buf->cursor = cursor;
    }

    for (int i = 0; i < frames; i++)
    {
        int L = (int)mixL[i];
        int R = (int)mixR[i];
        if (L > 32767) L = 32767; if (L < -32768) L = -32768;
        if (R > 32767) R = 32767; if (R < -32768) R = -32768;
        out[i*2]   = (short)L;
        out[i*2+1] = (short)R;
    }
}
