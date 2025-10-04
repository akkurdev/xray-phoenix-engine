#pragma once
#include <stdint.h>
#include "Sound.h"
#include "SoundRenderCache.h"

__interface XRSOUND_API ISoundRenderSource
{
    void Load(const char* fileName);
    void Unload();
    void Decompress(uint32_t lineNumber, OggVorbis_File* oggFile);

    const char* FileName() const;
    float Length() const;    
    float BaseVolume() const;
    float MinDistance() const;
    float MaxDistance() const;
    float MaxAiDistance() const;
    uint32_t Type() const;
    uint32_t BytesCount() const;
    WAVEFORMATEX Format() const;
    CacheTable* Cache() const;
};
