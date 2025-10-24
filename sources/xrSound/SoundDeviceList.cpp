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
    char* devices;
    int major, minor, index;
    LPCSTR actualDeviceName;

    Msg("OpenAL: enumerate devices...");

    m_devices.clear();
    CoUninitialize();

    // grab function pointers for 1.0-API functions, and if successful proceed to enumerate all devices
    if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT"))
    {
        Msg("OpenAL: enumerationExtension is present");

        devices = (char*)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
        Msg("Devices %s", devices);

        m_defaultDeviceName = (char*)alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
        Msg("OpenAL: default sound device name is %s", m_defaultDeviceName.c_str());

        index = 0;
        // go through device list (each device terminated with a single NULL, list terminated with double NULL)
        while (*devices != NULL)
        {
            ALCdevice* device = alcOpenDevice(devices);
            if (device)
            {
                ALCcontext* context = alcCreateContext(device, NULL);
                if (context)
                {
                    alcMakeContextCurrent(context);
                    // if new actual device name isn't already in the list, then add it...
                    actualDeviceName = alcGetString(device, ALC_DEVICE_SPECIFIER);

                    if ((actualDeviceName != NULL) && xr_strlen(actualDeviceName) > 0)
                    {
                        alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &major);
                        alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &minor);
                        m_devices.emplace_back(actualDeviceName, minor, major);

                        if (m_devices.back().Name != AL_SOFT)
                        {
                            m_devices.back().Props.Efx = alcIsExtensionPresent(alcGetContextsDevice(alcGetCurrentContext()), "ALC_EXT_EFX");
                            m_devices.back().Props.XRam = alcIsExtensionPresent(alcGetContextsDevice(alcGetCurrentContext()), "EAX_RAM");
                        }
                        else
                        {
                            if (alIsExtensionPresent("EAX2.0"))
                                m_devices.back().Props.Eax = 2;
                            if (alIsExtensionPresent("EAX3.0"))
                                m_devices.back().Props.Eax = 3;
                            if (alIsExtensionPresent("EAX4.0"))
                                m_devices.back().Props.Eax = 4;
                            if (alIsExtensionPresent("EAX5.0"))
                                m_devices.back().Props.Eax = 5;

                            m_devices.back().Props.XRam = alIsExtensionPresent("EAX-RAM");
                        }

                        // KD: disable unwanted eax flag to force eax on all devices
                        m_devices.back().Props.IsEaxUnwanted = 0;
                        ++index;
                    }
                    alcDestroyContext(context);
                }
                else
                {
                    Msg("OpenAL: cant create context for %s", device);
                }
                alcCloseDevice(device);
            }
            else
            {
                Msg("OpenAL: cant open device %s", devices);
            }
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

    //int majorVersion, minorVersion;

    /*for (u32 j = 0; j < Count(); j++)
    {
        GetDeviceVersion(j, &majorVersion, &minorVersion);
        Msg("%d. %s, Spec Version %d.%d %s eax[%d] efx[%s] xram[%s]", j + 1, GetDeviceName(j), majorVersion, minorVersion,
            (stricmp(GetDeviceName(j), m_defaultDeviceName) == 0) ? "(default)" : "", GetDeviceDesc(j).props.eax, GetDeviceDesc(j).props.efx ? "yes" : "no",
            GetDeviceDesc(j).props.xram ? "yes" : "no");
    }*/

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
}
