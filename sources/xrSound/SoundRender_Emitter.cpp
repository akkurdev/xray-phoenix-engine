#include "stdafx.h"
#include "soundrender_emitter.h"
#include "soundrender_core.h"
#include "SoundRenderSource.h"
#include "OalSoundRenderTarget.h"

extern u32 psSoundModel;
extern float psSoundVEffects;

void CSoundRender_Emitter::SetPosition(const Fvector& position)
{
    if (RenderSource()->Format().nChannels == 1 && _valid(position))
        m_params.position = position;
    else
        m_params.position.set(0, 0, 0);

    m_isMoved = true;
}

// Перемотка звука на заданную секунду [rewind snd to target time] --#SM+#--
void CSoundRender_Emitter::SetTime(float time)
{
    m_rewindTime = time >= 0.f 
        ? time 
        : 0.f;

    R_ASSERT2(get_length_sec() >= m_rewindTime, "set_time: time is bigger than length of sound");
}

CSoundRender_Emitter::CSoundRender_Emitter(void)
{
    m_renderTarget = nullptr;
    m_soundData = nullptr;
    m_smoothVolume = 1.f;
    m_occluderVolume = 1.f;
    m_fadeVolume = 1.f;
    m_occluder[0].set(0, 0, 0);
    m_occluder[1].set(0, 0, 0);
    m_occluder[2].set(0, 0, 0);
    m_state = EmitterState::Stopped;
    set_cursor(0);
    m_isMoved = true;
    m_is2D = false;
    m_isStopped = false;
    m_isRewind = false;
    m_paused = 0;
    m_startTime = 0.0f;
    m_stopTime = 0.0f;
    m_propagadeTime = 0.0f;
    m_rewindTime = 0.0f;
    m_marker = 0xabababab;
    m_startDelay = 0.f;
    m_priorityScale = 1.f;
    m_handleCursor = 0;
}

CSoundRender_Emitter::~CSoundRender_Emitter(void)
{
    // try to release dependencies, events, for example
    Event_ReleaseOwner();
}

//////////////////////////////////////////////////////////////////////
void CSoundRender_Emitter::Event_ReleaseOwner()
{
    if (!(m_soundData))
        return;

    for (u32 it = 0; it < SoundRender->s_events.size(); it++)
    {
        if (m_soundData == SoundRender->s_events[it].first)
        {
            SoundRender->s_events.erase(SoundRender->s_events.begin() + it);
            it--;
        }
    }
}

bool CSoundRender_Emitter::IsPlaying() const
{
    return m_state != EmitterState::Stopped;
}

uint32_t CSoundRender_Emitter::Marker() const
{
    return m_marker;
}

void CSoundRender_Emitter::SetMarker(uint32_t marker)
{
    m_marker = marker;
}

ref_sound_data_ptr CSoundRender_Emitter::OwnerData()
{
    return m_soundData;
}

float CSoundRender_Emitter::SmoothVolume() const
{
    return m_smoothVolume;
}

void CSoundRender_Emitter::SetRenderTarget(ISoundRenderTarget* target)
{
    m_renderTarget = target;
}

float CSoundRender_Emitter::StopTime() const
{
    return m_stopTime;
}

void CSoundRender_Emitter::SetStopTime(float stopTime)
{
    m_stopTime = stopTime;
}

void CSoundRender_Emitter::SetVolume(float volume)
{
    if (!_valid(volume))
        volume = 0.0f;
    m_params.volume = volume;
}

bool CSoundRender_Emitter::Is2D() const
{ 
    return m_is2D; 
}

CSound_params* CSoundRender_Emitter::Params() 
{ 
    return &m_params; 
}

void CSoundRender_Emitter::SetRange(float minDistance, float maxDistance)
{
    VERIFY(_valid(min) && _valid(max));
    m_params.min_distance = minDistance;
    m_params.max_distance = maxDistance;
}

void CSoundRender_Emitter::SetPriority(float priority) { m_priorityScale = priority; }

void CSoundRender_Emitter::SetFrequency(float frequency)
{
    VERIFY(_valid(scale));
    m_params.freq = frequency;
}

void CSoundRender_Emitter::Event_Propagade()
{
    m_propagadeTime += ::Random.randF(s_f_def_event_pulse - 0.030f, s_f_def_event_pulse + 0.030f);
    if (!(m_soundData))
        return;
    if (0 == m_soundData->g_type)
        return;
    if (0 == m_soundData->g_object)
        return;
    if (0 == SoundRender->Handler)
        return;

    VERIFY(_valid(m_params.volume));
    // Calculate range
    float clip = m_params.max_ai_distance * m_params.volume;
    float range = _min(m_params.max_ai_distance, clip);
    if (range < 0.1f)
        return;

    // Inform objects
    SoundRender->s_events.push_back(mk_pair(m_soundData, range));
}

void CSoundRender_Emitter::SwitchTo2D()
{
    m_is2D = true;
    SetPriority(100.f);
}

void CSoundRender_Emitter::SwitchTo3D() { m_is2D = false; }

void CSoundRender_Emitter::set_cursor(u32 p)
{
    m_streamCursor = p;

    if (m_soundData._get() && m_soundData->fn_attached[0].size())
    {
        u32 bt = (m_soundData->handle)->BytesCount();
        if (m_streamCursor >= m_handleCursor + bt)
        {
            SoundRender->i_destroy_source(m_soundData->handle);
            m_soundData->handle = SoundRender->i_create_source(m_soundData->fn_attached[0].c_str());
            m_soundData->fn_attached[0] = m_soundData->fn_attached[1];
            m_soundData->fn_attached[1] = "";
            m_handleCursor = get_cursor(true);

            if (m_renderTarget)
                m_renderTarget->OnSourceChanged();
        }
    }
}

u32 CSoundRender_Emitter::get_cursor(bool b_absolute) const
{
    if (b_absolute)
        return m_streamCursor;
    else
    {
        VERIFY(m_streamCursor - m_handleCursor >= 0);
        return m_streamCursor - m_handleCursor;
    }
}

void CSoundRender_Emitter::move_cursor(int offset) 
{ 
    set_cursor(get_cursor(true) + offset); 
}
