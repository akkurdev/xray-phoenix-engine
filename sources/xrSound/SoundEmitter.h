#pragma once
#include "SoundRender.h"
#include "ISoundEmitter.h"
#include "ISoundRenderTarget.h"
#include "EmitterState.h"

class SoundEmitter final : public ISoundEmitter
{
public:
    SoundEmitter();
    virtual ~SoundEmitter();

    virtual bool Is2D() const;
    virtual float Priority() const;
    virtual uint32_t PlayTime() const;
    virtual float Volume() const;
    virtual float StopTime() const;
    virtual bool IsPlaying() const;
    virtual uint32_t Marker() const;
    virtual CSound_params* Params();
    virtual ref_sound_data_ptr SoundData();
    virtual ISoundRenderSource* RenderSource();
    virtual ISoundRenderTarget* RenderTarget();
    
    virtual void Start(ref_sound* sound, bool isLooped, float delay);
    virtual void Cancel();
    virtual void Update(float deltaTime);
    virtual void Rewind();
    virtual void Stop(bool isDeffered);
    virtual void Pause(bool hasValue, int32_t pausedId);
    virtual void SwitchTo2D();
    virtual void SwitchTo3D();
    virtual void SetPriority(float priority);    
    virtual void SetTime(float time);
    virtual void SetMarker(uint32_t marker);
    virtual void SetRenderTarget(ISoundRenderTarget* target);
    virtual void SetStopTime(float stopTime);
    virtual void SetPosition(const Fvector& position);
    virtual void SetFrequency(float frequency);
    virtual void SetRange(float minDistance, float maxDistance);
    virtual void SetVolume(float volume);
    virtual void FillBlock(void* ptr, uint32_t size);

private:    
    void FillData(uint8_t* ptr, uint32_t offset, uint32_t size);
    bool UpdateCulling(float deltaTime);
    void i_stop();
    void SetCursor(uint32_t position);

    void OnPropagade();
    void OnRelease();
    void OnStart(float deltaTime, bool isLooped);
    void OnDelayedStart(float deltaTime, bool isLooped);
    void OnPlay(float deltaTime, bool isLooped);
    void OnSimulate(float deltaTime);
    void OnLoopedSimulate(float deltaTime);

    float Attitude() const;
    float UpdateSmoothVolume(float deltaTime, float currentVolume, float fadeVolume, float occludeVolume);
    float UpdateFadeVolume(float deltaTime, float currentVolume);
    float UpdateOccludeVolume(float deltaTime, float currentVolume);

private:
    float m_startDelay;
    ISoundRenderTarget* m_renderTarget;
    ref_sound_data_ptr m_soundData;
    float m_priorityScale;
    float m_smoothVolume;
    float m_occluderVolume;
    float m_fadeVolume;
    Fvector m_occluder[3]{};
    EmitterState m_state;
    uint32_t m_streamCursor;
    uint32_t m_handleCursor;
    CSound_params m_params;
    int32_t m_paused;
    bool m_isMoved;
    bool m_is2D;
    bool m_isStopped;
    bool m_isRewind;
    float m_startTime;
    float m_stopTime;
    float m_propagadeTime;
    float m_rewindTime;
    uint32_t m_marker;
};
