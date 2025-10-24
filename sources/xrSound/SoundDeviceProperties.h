#pragma once
#include <stdint.h>

union SoundDeviceProperties
{
    struct
    {
        uint16_t IsSelected : 1;
        uint16_t Eax : 3;
        uint16_t Efx : 1;
        uint16_t XRam : 1;
        uint16_t IsEaxUnwanted : 1;
        uint16_t Unused : 9;
    };
    uint16_t Storage;
};
