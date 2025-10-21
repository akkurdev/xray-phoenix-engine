#pragma once
#include "dllexp.h"

class XRSOUND_API CSound_params
{
public:
    Fvector position;
    float base_volume;
    float volume;
    float freq;
    float min_distance;
    float max_distance;
    float max_ai_distance;
};