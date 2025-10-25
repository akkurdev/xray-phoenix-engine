#pragma once
#include <stdint.h>
#include "SoundParams.h"
#include "ref_sound_data.h"

__interface ISoundRenderSource;
__interface ISoundRenderTarget;

__interface ISoundEmitter
{
    bool Is2D() const;
    float Priority() const;    
    float Volume() const;
    float StopTime() const;
    bool IsPlaying() const;
    uint32_t PlayTime() const;
    uint32_t Marker() const;
    SoundParams* Params();
    ref_sound_data_ptr SoundData();
    ISoundRenderSource* RenderSource();
    ISoundRenderTarget* RenderTarget();

    void Start(ref_sound* sound, bool isLooped, float delay);
    void Cancel();
    void Update(float deltaTime);
    void Rewind();
    void Stop(bool isDeffered);
    void Pause(bool hasValue, int32_t pausedId);
    void SwitchTo2D();
    void SwitchTo3D();
    void SetPriority(float priority);
    void SetTime(float time);
    void SetMarker(uint32_t marker);
    void SetRenderTarget(ISoundRenderTarget* target);
    void SetStopTime(float stopTime);
    void SetPosition(const Fvector& position);
    void SetFrequency(float frequency);
    void SetRange(float minDistance, float maxDistance);
    void SetVolume(float volume);
    void FillBlock(void* ptr, uint32_t size);
};
