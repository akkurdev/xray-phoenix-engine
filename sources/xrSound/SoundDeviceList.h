#pragma once
#include <vector>
#include "SoundDevice.h"

class SoundDeviceList final
{
public:
    SoundDeviceList();
    ~SoundDeviceList();

    SoundDevice operator[](uint32_t index);    

    uint32_t Count() const;    
    bool IsOalSoftEnabled() const;

    void SelectBestDevice();

private:
    void EnumerateDevices();

private:
    std::vector<SoundDevice> m_devices;
    std::string m_defaultDeviceName;
    bool m_isOalSoftEnabled;
};
