#include "stdafx.h"
#include "soundrender_coreA.h"
#include "OalSoundRenderTarget.h"

CSoundRender_CoreA* SoundRenderA = 0;

CSoundRender_CoreA::CSoundRender_CoreA() : CSoundRender_Core()
{
    pDevice = 0;
    //pDeviceList = 0;
    pContext = 0;
    eaxSet = 0;
    eaxGet = 0;
}

CSoundRender_CoreA::~CSoundRender_CoreA() {}

BOOL CSoundRender_CoreA::EAXQuerySupport(BOOL bDeferred, const GUID* guid, u32 prop, void* val, u32 sz)
{
    if (AL_NO_ERROR != eaxGet(guid, prop, 0, val, sz))
        return FALSE;
    if (AL_NO_ERROR != eaxSet(guid, (bDeferred ? DSPROPERTY_EAXLISTENER_DEFERRED : 0) | prop, 0, val, sz))
        return FALSE;
    return TRUE;
}

BOOL CSoundRender_CoreA::EAXTestSupport(BOOL bDeferred)
{
    EAXLISTENERPROPERTIES ep;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOM, &ep.lRoom, sizeof(LONG)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOMHF, &ep.lRoomHF, sizeof(LONG)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR, &ep.flRoomRolloffFactor, sizeof(float)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_DECAYTIME, &ep.flDecayTime, sizeof(float)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_DECAYHFRATIO, &ep.flDecayHFRatio, sizeof(float)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REFLECTIONS, &ep.lReflections, sizeof(LONG)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY, &ep.flReflectionsDelay, sizeof(float)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REVERB, &ep.lReverb, sizeof(LONG)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_REVERBDELAY, &ep.flReverbDelay, sizeof(float)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION, &ep.flEnvironmentDiffusion, sizeof(float)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF, &ep.flAirAbsorptionHF, sizeof(float)))
        return FALSE;
    if (!EAXQuerySupport(bDeferred, &DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_FLAGS, &ep.dwFlags, sizeof(DWORD)))
        return FALSE;
    return TRUE;
}

void CSoundRender_CoreA::_restart()
{
    inherited::_restart();
}

void CSoundRender_CoreA::_initialize(int stage)
{
    if (stage == 0)
    {
        if (0 == pDeviceList.Count())
        {
            CHECK_OR_EXIT(0, "OpenAL: Can't create sound device.");
        }
        return;
    }

    pDeviceList.SelectBestDevice();
    R_ASSERT(snd_device_id >= 0 && snd_device_id < pDeviceList.Count());
    auto deviceDesc = pDeviceList[snd_device_id];

    // OpenAL device
    pDevice = alcOpenDevice(deviceDesc.Name.c_str());
    if (pDevice == NULL)
    {
        CHECK_OR_EXIT(0, "SOUND: OpenAL: Failed to create device.");
        bPresent = FALSE;
        return;
    }

    // Get the device specifier.
    const ALCchar* deviceSpecifier;
    deviceSpecifier = alcGetString(pDevice, ALC_DEVICE_SPECIFIER);

    // Create context
    pContext = alcCreateContext(pDevice, NULL);

    if (0 == pContext)
    {
        CHECK_OR_EXIT(0, "OpenAL: Failed to create context.");
        bPresent = FALSE;
        alcCloseDevice(pDevice);
        pDevice = 0;
        return;
    }

    // clear errors
    alGetError();
    alcGetError(pDevice);

    // Set active context
    alcMakeContextCurrent(pContext);

    // initialize listener
    alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
    alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
    Fvector orient[2] = {{0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}};

    alListenerfv(AL_ORIENTATION, &orient[0].x);
    alListenerf(AL_GAIN, 1.f);

    if (!pDeviceList.IsOalSoftEnabled())
    {
        // Check for EAX extension
        m_hasEax = deviceDesc.Props.Eax && !deviceDesc.Props.IsEaxUnwanted;

        eaxSet = (EAXSet)alGetProcAddress((const ALchar*)"EAXSet");
        if (eaxSet == NULL)
            m_hasEax = false;
        eaxGet = (EAXGet)alGetProcAddress((const ALchar*)"EAXGet");
        if (eaxGet == NULL)
            m_hasEax = false;

        if (m_hasEax)
        {
            m_hasDeferredEax = EAXTestSupport(true);
            m_hasEax = EAXTestSupport(false);
        }
        Msg("[OpenAL] EAX 2.0 extension: %s", m_hasEax ? "present" : "absent");
        Msg("[OpenAL] EAX 2.0 deferred: %s", m_hasDeferredEax ? "present" : "absent");
    }
    else if (deviceDesc.Props.Efx)
    {
        InitAlEFXAPI();
        m_hasEfx = EFXTestSupport();
        Msg("[OpenAL] EFX: %s", m_hasEfx ? "present" : "absent");
    }

    inherited::_initialize(stage);

    if (stage == 1) // first initialize
    {
        // Pre-create targets
        ISoundRenderTarget* T = 0;
        for (u32 tit = 0; tit < u32(psSoundTargets); tit++)
        {
            T = xr_new<DefaultSoundRenderTarget>();
            if (T->Initialize())
            {
                if (m_hasEfx)
                    T->OpenALAuxInit(m_slot);

                T->UseAlSoft(pDeviceList.IsOalSoftEnabled());
                m_renderTargets.push_back(T);
            }
            else
            {
                Msg("OpenAL: Max targets - [%u]", tit);
                T->Destroy();
                xr_delete(T);
                break;
            }
        }
    }
}

void CSoundRender_CoreA::set_master_volume(float f)
{
    if (bPresent)
    {
        alListenerf(AL_GAIN, f);
    }
}

void CSoundRender_CoreA::_clear()
{
    inherited::_clear();
    // remove targets
    ISoundRenderTarget* T = 0;
    for (u32 tit = 0; tit < m_renderTargets.size(); tit++)
    {
        T = m_renderTargets[tit];
        T->Destroy();
        xr_delete(T);
    }
    // Reset the current context to NULL.
    alcMakeContextCurrent(NULL);
    // Release the context and the device.
    alcDestroyContext(pContext);
    pContext = 0;
    alcCloseDevice(pDevice);
    pDevice = 0;
}

void CSoundRender_CoreA::i_eax_set(const GUID* guid, u32 prop, void* val, u32 sz) { eaxSet(guid, prop, 0, val, sz); }
void CSoundRender_CoreA::i_eax_get(const GUID* guid, u32 prop, void* val, u32 sz) { eaxGet(guid, prop, 0, val, sz); }

void CSoundRender_CoreA::update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt)
{
    inherited::update_listener(P, D, N, dt);

    if (!Listener.position.similar(P))
    {
        Listener.position.set(P);
        m_isListenerMoved = true;
    }
    Listener.orientation[0].set(D.x, D.y, -D.z);
    Listener.orientation[1].set(N.x, N.y, -N.z);

    alListener3f(AL_POSITION, Listener.position.x, Listener.position.y, -Listener.position.z);
    alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
    alListenerfv(AL_ORIENTATION, &Listener.orientation[0].x);
}
