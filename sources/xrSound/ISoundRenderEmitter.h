#pragma once
#include <stdint.h>
#include "Sound.h"

class Fvector;

__interface XRSOUND_API ISoundRenderEmitter
{
    bool Is2D() const;
    CSound_params* Params() const;

    void SwitchTo2D();
    void SwitchTo3D();
    void SetPosition(const Fvector& position);
    void SetFrequency(float frequency);
    void SetRange(float min, float max);
    void SetVolume(float volume);
    void SetPriority(float priority);
    void SetTime(float time);
    void Stop(bool isDeffered);    
    uint32_t PlayTime();
};
