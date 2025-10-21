#pragma once
#include <stdint.h>

enum esound_type
{
    st_Effect = 0,
    st_Music = 1,
    st_forcedword = uint32_t(-1),
};