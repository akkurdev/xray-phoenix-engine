#pragma once
#include <al.h>
#include "SoundRender.h"

__interface ISoundEmitter;

__interface ISoundRenderTarget
{    
    OggVorbis_File* OggFile();
    ISoundEmitter* Emitter();

    bool IsRendering() const;
    float CacheGain();
    float CachePitch();
    float Priority();
    bool HasAlSoft();

    void SetPriority(float priority);
    void UseAlSoft(bool useAlSoft);

    void Attach();
    void Detach();

    bool Initialize();
    void Destroy();
    void Restart();

    void Start(ISoundEmitter* emitter);
    void Render();
    void Rewind();
    void Stop();
    void Update();
    void Fill();

    void OpenALAuxInit(ALuint slot);
    void OnSourceChanged();
};
