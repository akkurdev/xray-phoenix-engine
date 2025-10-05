#pragma once
#include "soundrender.h"
#include <ISoundRenderTarget.h>
#include <EmitterState.h>

class CSoundRender_Emitter : public CSound_emitter
{
public:
    CSoundRender_Emitter();
    ~CSoundRender_Emitter();

    void fill_block(void* ptr, u32 size);
    void fill_data(u8* ptr, u32 offset, u32 size);
    float Priority();
    
    void start(ref_sound* _owner, BOOL _loop, float delay);
    void cancel(); 
    void update(float dt);
    BOOL update_culling(float dt);
    void update_environment(float dt);
    void rewind();
    virtual void stop(BOOL bDeffered);
    void pause(BOOL bVal, int id);
    virtual u32 play_time();
    virtual void set_priority(float p);
    virtual void set_time(float t); 
    virtual CSound_params* get_params();    
    ISoundRenderSource* RenderSource();
    ISoundRenderTarget* RenderTarget();
    virtual void switch_to_2D();
    virtual void switch_to_3D();
    virtual void set_position(const Fvector& pos);
    virtual void set_frequency(float scale);
    u32 get_bytes_total() const;
    float get_length_sec() const;
    virtual bool Is2D();
    virtual void set_range(float min, float max);
    virtual void set_volume(float vol);     
    void i_stop();
    void set_cursor(u32 p);
    u32 get_cursor(bool b_absolute) const;
    void move_cursor(int offset);
    void Event_Propagade();
    void Event_ReleaseOwner();
    BOOL isPlaying(void);
    u32 Marker() const;
    void SetMarker(u32 marker);
    ref_sound_data_ptr OwnerData();
    float SmoothVolume() const;
    void SetRenderTarget(ISoundRenderTarget* target);
    float StopTime() const;
    void SetStopTime(float stopTime);

private:
    float Attitude();

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
