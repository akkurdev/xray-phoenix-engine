#pragma once
#include <stdint.h>

class IReader;
class IWriter;
class shared_str;

class SoundEnvironment final
{
public:
    SoundEnvironment();
    ~SoundEnvironment() = default;

    shared_str Name() const;
    uint32_t Version() const;
    uint32_t Id() const;

    float RoomEffectFactor() const;
    float RoomEffectHighFactor() const;
    float RoomRollOffFactor() const;
    float DecayTimeFactor() const;
    float DecayHighFrequencyFactor() const;
    float ReflectionFactor() const;
    float ReflectionDelayFactor() const;
    float ReverberationFactor() const;
    float ReverberationDelayFactor() const;
    float EnvironmentDiffusionFactor() const;
    float AirAbsorptionFactor() const;
    float EnvironmentSize() const;

    void SetIdentity();
    void SetDefault();
    void Clamp();
    void Lerp(SoundEnvironment& env1, SoundEnvironment& env2, float delta);
    bool Load(IReader* reader);
    void Save(IWriter* writer);

private:
    shared_str m_name;
    uint32_t m_version;
    uint32_t m_id;

    float m_roomEffectFactor;
    float m_roomEffectHighFactor;
    float m_roomRollOffFactor;
    float m_decayTimeFactor;
    float m_decayHighFrequencyFactor;
    float m_reflectionFactor;
    float m_reflectionDelayFactor;
    float m_reverberationFactor;
    float m_reverberationDelayFactor;
    float m_environmentDiffusionFactor;    
    float m_airAbsorptionFactor;
    float m_environmentSize;
};
