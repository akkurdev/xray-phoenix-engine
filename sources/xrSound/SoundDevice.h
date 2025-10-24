#pragma once
#include <stdint.h>
#include <string>
#include "SoundDeviceProperties.h"

struct SoundDevice
{
    std::string Name;
    int32_t VersionMinor;
    int32_t VersionMajor;    
    SoundDeviceProperties Props;

    SoundDevice(const std::string& name, int32_t versionMinor, int32_t versionMajor)
    {
        Name = name;
        VersionMinor = versionMinor;
        VersionMajor = versionMajor;

        Props = SoundDeviceProperties{};
        Props.Storage = 0;
        Props.IsEaxUnwanted = true;
    }
};