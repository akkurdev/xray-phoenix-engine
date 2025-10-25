#pragma once
#include "dllexp.h"

class XRSOUND_API SoundParams
{
public:
    Fvector Position;
    float BaseVolume;
    float Volume;
    float Frequency;
    float DistanceMin;
    float DistanceMax;
    float DistanceAI;
};