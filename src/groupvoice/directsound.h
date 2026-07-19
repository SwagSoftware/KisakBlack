#pragma once

#include <Windows.h>
#ifdef _WIN32
#include <mmreg.h>
#endif
#include <dsound.h>

struct __declspec(align(4)) dsound_sample_t // sizeof=0x48
{                                       // XREF: .data:s_recordingSamples/r
    IDirectSoundCaptureBuffer *DSCB;
    IDirectSoundBuffer *DSB;
    unsigned int dwBufferSize;
    unsigned int dwCaptureOffset;
    unsigned int currentOffset;
    unsigned int lastOffset;
    unsigned int currentBufferLength;
    int stopPosition;
    unsigned int lastPlayPos;
    int bytesBuffered;
    int mode;
    int frequency;
    int volume;
    int pan;
    int channels;
    bool playing;
    // padding byte
    // padding byte
    // padding byte
    int channel;
    unsigned __int8 playMode;
    // padding byte
    // padding byte
    // padding byte
};
