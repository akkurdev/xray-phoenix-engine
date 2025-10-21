#pragma once
#include "soundrender.h"
#include <ISoundRenderTarget.h>
#include <EmitterState.h>

class CSoundRender_Emitter : public CSound_emitter
{
public:
    CSoundRender_Emitter();
    ~CSoundRender_Emitter();

    void FillBlock(void* ptr, uint32_t size);
    float Priority();
    void Start(ref_sound* sound, bool isLooped, float delay);
    void Cancel();
    void Update(float deltaTime);
    void Rewind();
    virtual void Stop(bool isDeffered);
    void Pause(bool hasValue, int32_t pausedId);
    virtual uint32_t PlayTime();
    virtual bool Is2D();
    virtual void SetPriority(float priority);
    virtual void SwitchTo2D();
    virtual void SwitchTo3D();
    virtual void SetTime(float time);
    void SetMarker(uint32_t marker);
    void SetRenderTarget(ISoundRenderTarget* target);
    void SetStopTime(float stopTime);

    ISoundRenderSource* RenderSource();
    ISoundRenderTarget* RenderTarget();    
    virtual CSound_params* Params();    
    virtual void set_position(const Fvector& pos);
    virtual void set_frequency(float scale);
    virtual void set_range(float min, float max);
    virtual void set_volume(float vol);
    BOOL isPlaying(void);
    u32 Marker() const;    
    ref_sound_data_ptr OwnerData();
    float SmoothVolume() const;    
    float StopTime() const;
    

private:
    float Attitude();
    void FillData(uint8_t* ptr, uint32_t offset, uint32_t size);
    bool UpdateCulling(float deltaTime);
    u32 get_bytes_total() const;
    float get_length_sec() const;
    void i_stop();
    void set_cursor(u32 p);
    u32 get_cursor(bool b_absolute) const;
    void move_cursor(int offset);
    void Event_Propagade();
    void Event_ReleaseOwner();

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
