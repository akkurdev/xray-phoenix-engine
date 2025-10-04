#pragma once
#include "SoundRender_Cache.h"
#include "ISoundRenderSource.h" 

class SoundRenderSource final : public ISoundRenderSource
{
public:
    SoundRenderSource();
    virtual ~SoundRenderSource();

    virtual void Load(const char* fileName);
    virtual void Unload();
    virtual void Decompress(uint32_t lineNumber, OggVorbis_File* oggFile);

    virtual const char* FileName() const;
    virtual float Length() const;
    virtual uint32_t Type() const;
    virtual float BaseVolume() const;
    virtual float MinDistance() const;
    virtual float MaxDistance() const;
    virtual float MaxAiDistance() const;
    virtual uint32_t BytesCount() const;

    virtual WAVEFORMATEX Format() const;
    virtual CacheTable* Cache() const;

private:
    void PrepareFile(const char* fileName);
    void ReadFormat(OggVorbis_File* oggFile);
    void ReadComments(OggVorbis_File* oggFile);

private:
    float m_volume;
    float m_minDistance;
    float m_maxDistance;
    float m_maxAiDistance;    
    float m_length;
    uint32_t m_gameType;
    uint32_t m_bytesCount;

    CacheTable* m_cache;
    shared_str m_fileName;
    WAVEFORMATEX m_format;
};
