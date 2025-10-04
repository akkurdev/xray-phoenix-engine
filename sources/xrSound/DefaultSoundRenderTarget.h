#pragma once
#include "ISoundRenderTarget.h"
#include "soundrender_CoreA.h"

class DefaultSoundRenderTarget final : public ISoundRenderTarget
{
public:
    DefaultSoundRenderTarget();
    virtual ~DefaultSoundRenderTarget();

    virtual OggVorbis_File* OggFile();
    virtual CSoundRender_Emitter* Emitter();

    virtual bool IsRendering() const;
    virtual float CacheGain();
    virtual float CachePitch();
    virtual float Priority();
    virtual bool HasAlSoft();

    virtual void SetPriority(float priority);
    virtual void UseAlSoft(bool useAlSoft);

    virtual void Attach();
    virtual void Detach();

    virtual bool Initialize();
    virtual void Destroy();
    virtual void Restart();

    virtual void Start(CSoundRender_Emitter* emitter);
    virtual void Render();
    virtual void Rewind();
    virtual void Stop();
    virtual void Update();
    virtual void Fill();
    virtual void OpenALAuxInit(ALuint slot);
    virtual void OnSourceChanged();

private:
    void FillBlock(ALuint bufferId);
    void ProcessBuffers(ALint buffersCount);
    ALint GetBuffersCount();
    ALint GetQueuedBuffersCount();
    ALint GetState();

private:
    ALuint m_source;
    ALuint m_bufferBlock;
    ALuint m_buffers[sdef_target_count];
    float m_cacheGain;
    float m_cachePitch;
    float m_priority;
    bool m_isRendering;
    bool m_hasAlSoft;

    CSoundRender_Emitter* m_emitter;
    OggVorbis_File* m_oggFile;
    IReader* m_reader;    
};
