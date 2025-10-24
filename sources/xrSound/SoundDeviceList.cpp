#include "stdafx.h"
#include "SoundDeviceList.h"
#include <alc.h>
#include <al.h>

const char* AL_GENERIC_HARDWARE = "Generic Hardware";
const char* AL_GENERIC_SOFTWARE = "Generic Software";
const char* AL_SOFT = "OpenAL Soft";

SoundDeviceList::SoundDeviceList() :
    m_devices(std::vector<SoundDevice>{}),
    m_defaultDeviceName(""),
    m_isOalSoftEnabled(false)
{
    EnumerateDevices();
}

SoundDeviceList::~SoundDeviceList()
{
    for (auto i = 0; snd_devices_token[i].name; i++)
    {
        xr_free(snd_devices_token[i].name);
    }

    xr_free(snd_devices_token);
    snd_devices_token = NULL;

    m_devices.clear();
}

SoundDevice SoundDeviceList::operator[](uint32_t index)
{
    return m_devices[index];
}

uint32_t SoundDeviceList::Count() const
{
    return m_devices.size();
}

bool SoundDeviceList::IsOalSoftEnabled() const
{
    return m_isOalSoftEnabled;
}

void SoundDeviceList::SelectBestDevice()
{
    int32_t bestMajorVersion = -1;
    int32_t bestMinorVersion = -1;

    if (snd_device_id == uint32_t(-1))
    {
        auto newDeviceId = snd_device_id;

        for (size_t i = 0; i < Count(); ++i)
        {
            auto device = m_devices[i];

            if (device.Name != m_defaultDeviceName)
            {
                continue;
            }

            if ((device.VersionMajor > bestMajorVersion) || 
                (device.VersionMajor == bestMajorVersion && device.VersionMinor > bestMinorVersion))
            {
                bestMajorVersion = device.VersionMajor;
                bestMinorVersion = device.VersionMinor;
                newDeviceId = i;
            }
        }

        if (newDeviceId == uint32_t(-1))
        {
            R_ASSERT(Count() != 0);
            newDeviceId = 0;
        };
        snd_device_id = newDeviceId;
    }

    if (Count() == 0)
    {
        Msg("Can't select device: sound device list is empty");
        return;
    }

    Msg("Selected the best device: %s", m_devices[snd_device_id].Name.c_str());
    m_isOalSoftEnabled = m_devices[snd_device_id].Name == AL_SOFT;
}

void SoundDeviceList::EnumerateDevices()
{
    Msg("OpenAL: enumerate devices...");

    m_devices.clear();
    CoUninitialize();

    if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT"))
    {
        Msg("OpenAL: enumerationExtension is present");

        auto devices = (char*)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
        Msg("Devices %s", devices);

        m_defaultDeviceName = (char*)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
        Msg("OpenAL: default sound device name is %s", m_defaultDeviceName.c_str());

        while (*devices != NULL)
        {
            auto device = alcOpenDevice(devices);
            if (!device)
            {
                alcCloseDevice(device);
                continue;                
            }

            auto context = alcCreateContext(device, NULL);
            if (!context)
            {
                alcDestroyContext(context);
                continue;
            }

            alcMakeContextCurrent(context);

            auto deviceName = alcGetString(device, ALC_DEVICE_SPECIFIER);
            if (deviceName == nullptr || xr_strlen(deviceName) == 0)
            {
                alcDestroyContext(context);
                alcCloseDevice(device);
                continue;
            }

            int major, minor;
            alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &major);
            alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &minor);
            
            auto currentContext = alcGetCurrentContext();
            auto currentDevice = alcGetContextsDevice(currentContext);

            auto soundDevice = SoundDevice(deviceName, minor, major);

            soundDevice.Props.Efx = soundDevice.Name != AL_SOFT
                ? alcIsExtensionPresent(currentDevice, "ALC_EXT_EFX")
                : 0;

            soundDevice.Props.EaxRam = soundDevice.Name != AL_SOFT
                ? alcIsExtensionPresent(currentDevice, "EAX_RAM")
                : alIsExtensionPresent("EAX-RAM");

            soundDevice.Props.IsEaxUnwanted = 0;

            if (soundDevice.Name == AL_SOFT)
            {
                if (alIsExtensionPresent("EAX2.0"))
                    m_devices.back().Props.Eax = 2;
                if (alIsExtensionPresent("EAX3.0"))
                    m_devices.back().Props.Eax = 3;
                if (alIsExtensionPresent("EAX4.0"))
                    m_devices.back().Props.Eax = 4;
                if (alIsExtensionPresent("EAX5.0"))
                    m_devices.back().Props.Eax = 5;
            }

            m_devices.push_back(soundDevice);

            alcDestroyContext(context);
            alcCloseDevice(device);

            devices += xr_strlen(devices) + 1;
        }
    }
    else
    {
        Msg("OpenAL: enumerationExtension is not present");
    }

    // make token
    snd_devices_token = xr_alloc<xr_token>(Count() + 1);
    snd_devices_token[Count()].id = -1;
    snd_devices_token[Count()].name = nullptr;

    for (size_t i = 0; i < Count(); ++i)
    {
        snd_devices_token[i].id = i;
        snd_devices_token[i].name = xr_strdup(m_devices[i].Name.c_str());
    }

    if (0 != Count())
    {
        Msg("OpenAL: All available devices:");
    }

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
}
