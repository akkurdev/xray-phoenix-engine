#include "stdafx.h"
#include "SoundEnvironment.h"
#include <eax.h>
#include <SoundRender.h>

SoundEnvironment::SoundEnvironment() :
    m_version(0),
    m_id(0),
    m_roomEffectFactor(0.f),
    m_roomEffectHighFactor(0.f),
    m_roomRollOffFactor(0.f),
    m_decayTimeFactor(0.f),
    m_decayHighFrequencyFactor(0.f),
    m_reflectionFactor(0.f),
    m_reflectionDelayFactor(0.f),
    m_reverberationFactor(0.f),
    m_reverberationDelayFactor(0.f),
    m_airAbsorptionFactor(0.f),
    m_environmentDiffusionFactor(0.f),
    m_environmentSize(0.f)
{
    m_version = sdef_env_version;
    m_name = "_engine_default_";

    SetDefault();
}

shared_str SoundEnvironment::Name() const
{
    return m_name;
}

uint32_t SoundEnvironment::Version() const
{
    return m_version;
}

uint32_t SoundEnvironment::Id() const
{
    return m_id;
}

float SoundEnvironment::RoomEffectFactor() const
{
    return m_roomEffectFactor;
}

float SoundEnvironment::RoomEffectHighFactor() const
{
    return m_roomEffectHighFactor;
}

float SoundEnvironment::RoomRollOffFactor() const
{
    return m_roomRollOffFactor;
}

float SoundEnvironment::DecayTimeFactor() const
{
    return m_decayTimeFactor;
}

float SoundEnvironment::DecayHighFrequencyFactor() const
{
    return m_decayHighFrequencyFactor;
}

float SoundEnvironment::ReflectionFactor() const
{
    return m_reflectionFactor;
}

float SoundEnvironment::ReflectionDelayFactor() const
{
    return m_reflectionDelayFactor;
}

float SoundEnvironment::ReverberationFactor() const
{
    return m_reverberationFactor;
}

float SoundEnvironment::ReverberationDelayFactor() const
{
    return m_reverberationDelayFactor;
}

float SoundEnvironment::EnvironmentDiffusionFactor() const
{
    return m_environmentDiffusionFactor;
}

float SoundEnvironment::AirAbsorptionFactor() const
{
    return m_airAbsorptionFactor;
}

float SoundEnvironment::EnvironmentSize() const
{
    return m_environmentSize;
}

void SoundEnvironment::SetIdentity()
{
    SetDefault();
    m_roomEffectFactor = EAXLISTENER_MINROOM;
    Clamp();
}

void SoundEnvironment::SetDefault()
{
    m_id = EAX_ENVIRONMENT_GENERIC;

    m_roomEffectFactor = EAXLISTENER_DEFAULTROOM;
    m_roomEffectHighFactor = EAXLISTENER_DEFAULTROOMHF;
    m_roomRollOffFactor = EAXLISTENER_DEFAULTROOMROLLOFFFACTOR;
    m_decayTimeFactor = EAXLISTENER_DEFAULTDECAYTIME;
    m_decayHighFrequencyFactor = EAXLISTENER_DEFAULTDECAYHFRATIO;
    m_reflectionFactor = EAXLISTENER_DEFAULTREFLECTIONS;
    m_reflectionDelayFactor = EAXLISTENER_DEFAULTREFLECTIONSDELAY;
    m_reverberationFactor = EAXLISTENER_DEFAULTREVERB;
    m_reverberationDelayFactor = EAXLISTENER_DEFAULTREVERBDELAY;
    m_environmentSize = EAXLISTENER_DEFAULTENVIRONMENTSIZE;
    m_environmentDiffusionFactor = EAXLISTENER_DEFAULTENVIRONMENTDIFFUSION;
    m_airAbsorptionFactor = EAXLISTENER_DEFAULTAIRABSORPTIONHF;
}

void SoundEnvironment::Clamp()
{
    ::clamp(m_roomEffectFactor, (float)EAXLISTENER_MINROOM, (float)EAXLISTENER_MAXROOM);
    ::clamp(m_roomEffectHighFactor, (float)EAXLISTENER_MINROOMHF, (float)EAXLISTENER_MAXROOMHF);
    ::clamp(m_roomRollOffFactor, EAXLISTENER_MINROOMROLLOFFFACTOR, EAXLISTENER_MAXROOMROLLOFFFACTOR);
    ::clamp(m_decayTimeFactor, EAXLISTENER_MINDECAYTIME, EAXLISTENER_MAXDECAYTIME);
    ::clamp(m_decayHighFrequencyFactor, EAXLISTENER_MINDECAYHFRATIO, EAXLISTENER_MAXDECAYHFRATIO);
    ::clamp(m_reflectionFactor, (float)EAXLISTENER_MINREFLECTIONS, (float)EAXLISTENER_MAXREFLECTIONS);
    ::clamp(m_reflectionDelayFactor, EAXLISTENER_MINREFLECTIONSDELAY, EAXLISTENER_MAXREFLECTIONSDELAY);
    ::clamp(m_reverberationFactor, (float)EAXLISTENER_MINREVERB, (float)EAXLISTENER_MAXREVERB);
    ::clamp(m_reverberationDelayFactor, EAXLISTENER_MINREVERBDELAY, EAXLISTENER_MAXREVERBDELAY);
    ::clamp(m_environmentSize, EAXLISTENER_MINENVIRONMENTSIZE, EAXLISTENER_MAXENVIRONMENTSIZE);
    ::clamp(m_environmentDiffusionFactor, EAXLISTENER_MINENVIRONMENTDIFFUSION, EAXLISTENER_MAXENVIRONMENTDIFFUSION);
    ::clamp(m_airAbsorptionFactor, EAXLISTENER_MINAIRABSORPTIONHF, EAXLISTENER_MAXAIRABSORPTIONHF);
}

void SoundEnvironment::Lerp(SoundEnvironment& env1, SoundEnvironment& env2, float delta)
{
    float fi = 1.f - delta;

    m_roomEffectFactor = fi * env1.RoomEffectFactor() + delta * env2.RoomEffectFactor();
    m_roomEffectHighFactor = fi * env1.RoomEffectHighFactor() + delta * env2.RoomEffectHighFactor();
    m_roomRollOffFactor = fi * env1.RoomRollOffFactor() + delta * env2.RoomRollOffFactor();
    m_decayTimeFactor = fi * env1.DecayTimeFactor() + delta * env2.DecayTimeFactor();
    m_decayHighFrequencyFactor = fi * env1.DecayHighFrequencyFactor() + delta * env2.DecayHighFrequencyFactor();
    m_reflectionFactor = fi * env1.ReflectionFactor() + delta * env2.ReflectionFactor();
    m_reflectionDelayFactor = fi * env1.ReflectionDelayFactor() + delta * env2.ReflectionDelayFactor();
    m_reverberationFactor = fi * env1.ReverberationFactor() + delta * env2.ReverberationFactor();
    m_reverberationDelayFactor = fi * env1.ReverberationDelayFactor() + delta * env2.ReverberationDelayFactor();
    m_environmentSize = fi * env1.EnvironmentSize() + delta * env2.EnvironmentSize();
    m_environmentDiffusionFactor = fi * env1.EnvironmentDiffusionFactor() + delta * env2.EnvironmentDiffusionFactor();
    m_airAbsorptionFactor = fi * env1.AirAbsorptionFactor() + delta * env2.AirAbsorptionFactor();

    Clamp();
}

bool SoundEnvironment::Load(IReader* reader)
{
    m_version = reader->r_u32();
    if (m_version < 0x0003)
    {
        return false;
    }

    reader->r_stringZ(m_name);

    m_roomEffectFactor = reader->r_float();
    m_roomEffectHighFactor = reader->r_float();
    m_roomRollOffFactor = reader->r_float();
    m_decayTimeFactor = reader->r_float();
    m_decayHighFrequencyFactor = reader->r_float();
    m_reflectionFactor = reader->r_float();
    m_reflectionDelayFactor = reader->r_float();
    m_reverberationFactor = reader->r_float();
    m_reverberationDelayFactor = reader->r_float();
    m_environmentSize = reader->r_float();
    m_environmentDiffusionFactor = reader->r_float();
    m_airAbsorptionFactor = reader->r_float();

    if (m_version > 0x0003)
    {
        m_id = reader->r_u32();
    }
    return true;
}

void SoundEnvironment::Save(IWriter* writer)
{
    writer->w_u32(sdef_env_version);
    writer->w_stringZ(m_name);

    writer->w_float(m_roomEffectFactor);
    writer->w_float(m_roomEffectHighFactor);
    writer->w_float(m_roomRollOffFactor);
    writer->w_float(m_decayTimeFactor);
    writer->w_float(m_decayHighFrequencyFactor);
    writer->w_float(m_reflectionFactor);
    writer->w_float(m_reflectionDelayFactor);
    writer->w_float(m_reverberationFactor);
    writer->w_float(m_reverberationDelayFactor);
    writer->w_float(m_environmentSize);
    writer->w_float(m_environmentDiffusionFactor);
    writer->w_float(m_airAbsorptionFactor);

    writer->w_u32(m_id);
}
