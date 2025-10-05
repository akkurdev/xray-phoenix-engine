#include "stdafx.h"
#include "soundrender_emitter.h"
#include "soundrender_core.h"
#include "SoundRenderSource.h"
#include "OalSoundRenderTarget.h"

extern u32 psSoundModel;
extern float psSoundVEffects;

void CSoundRender_Emitter::set_position(const Fvector& pos)
{
    if (RenderSource()->Format().nChannels == 1 && _valid(pos))
        p_source.position = pos;
    else
        p_source.position.set(0, 0, 0);

    m_isMoved = true;
}
// Перемотка звука на заданную секунду [rewind snd to target time] --#SM+#--
void CSoundRender_Emitter::set_time(float t)
{
    if (t < 0.0f)
        t = 0.0f;
    R_ASSERT2(get_length_sec() >= t, "set_time: time is bigger than length of sound");
    m_rewindTime = t;
}

CSoundRender_Emitter::CSoundRender_Emitter(void)
{
    m_target = NULL;
    owner_data = NULL;
    smooth_volume = 1.f;
    occluder_volume = 1.f;
    fade_volume = 1.f;
    occluder[0].set(0, 0, 0);
    occluder[1].set(0, 0, 0);
    occluder[2].set(0, 0, 0);
    m_current_state = EmitterState::Stopped;
    set_cursor(0);
    m_isMoved = true;
    m_is2D = false;
    m_isStopped = false;
    m_isRewind = false;
    iPaused = 0;
    m_startTime = 0.0f;
    m_stopTime = 0.0f;
    m_propagadeTime = 0.0f;
    m_rewindTime = 0.0f;
    m_marker = 0xabababab;
    starting_delay = 0.f;
    priority_scale = 1.f;
    m_cur_handle_cursor = 0;
}

CSoundRender_Emitter::~CSoundRender_Emitter(void)
{
    // try to release dependencies, events, for example
    Event_ReleaseOwner();
}

//////////////////////////////////////////////////////////////////////
void CSoundRender_Emitter::Event_ReleaseOwner()
{
    if (!(owner_data))
        return;

    for (u32 it = 0; it < SoundRender->s_events.size(); it++)
    {
        if (owner_data == SoundRender->s_events[it].first)
        {
            SoundRender->s_events.erase(SoundRender->s_events.begin() + it);
            it--;
        }
    }
}

BOOL CSoundRender_Emitter::isPlaying(void)
{
    return m_current_state != EmitterState::Stopped;
}

u32 CSoundRender_Emitter::Marker() const
{
    return m_marker;
}

void CSoundRender_Emitter::SetMarker(u32 marker)
{
    m_marker = marker;
}

ref_sound_data_ptr CSoundRender_Emitter::OwnerData()
{
    return owner_data;
}

float CSoundRender_Emitter::SmoothVolume() const
{
    return smooth_volume;
}

void CSoundRender_Emitter::SetRenderTarget(ISoundRenderTarget* target)
{
    m_target = target;
}

float CSoundRender_Emitter::StopTime() const
{
    return m_stopTime;
}

void CSoundRender_Emitter::SetStopTime(float stopTime)
{
    m_stopTime = stopTime;
}

void CSoundRender_Emitter::set_volume(float vol)
{
    if (!_valid(vol))
        vol = 0.0f;
    p_source.volume = vol;
}

bool CSoundRender_Emitter::Is2D() { return m_is2D; }

CSound_params* CSoundRender_Emitter::get_params() { return &p_source; }

void CSoundRender_Emitter::set_range(float min, float max)
{
    VERIFY(_valid(min) && _valid(max));
    p_source.min_distance = min;
    p_source.max_distance = max;
}

void CSoundRender_Emitter::set_priority(float p) { priority_scale = p; }

void CSoundRender_Emitter::set_frequency(float scale)
{
    VERIFY(_valid(scale));
    p_source.freq = scale;
}

void CSoundRender_Emitter::Event_Propagade()
{
    m_propagadeTime += ::Random.randF(s_f_def_event_pulse - 0.030f, s_f_def_event_pulse + 0.030f);
    if (!(owner_data))
        return;
    if (0 == owner_data->g_type)
        return;
    if (0 == owner_data->g_object)
        return;
    if (0 == SoundRender->Handler)
        return;

    VERIFY(_valid(p_source.volume));
    // Calculate range
    float clip = p_source.max_ai_distance * p_source.volume;
    float range = _min(p_source.max_ai_distance, clip);
    if (range < 0.1f)
        return;

    // Inform objects
    SoundRender->s_events.push_back(mk_pair(owner_data, range));
}

void CSoundRender_Emitter::switch_to_2D()
{
    m_is2D = true;
    set_priority(100.f);
}

void CSoundRender_Emitter::switch_to_3D() { m_is2D = false; }

u32 CSoundRender_Emitter::play_time()
{
    if (m_current_state == EmitterState::Playing || 
        m_current_state == EmitterState::PlayingLooped || 
        m_current_state == EmitterState::Simulating || 
        m_current_state == EmitterState::SimulatingLooped)
        return iFloor((SoundRender->fTimer_Value - m_startTime) * 1000.0f);
    else
        return 0;
}


void CSoundRender_Emitter::set_cursor(u32 p)
{
    m_stream_cursor = p;

    if (owner_data._get() && owner_data->fn_attached[0].size())
    {
        u32 bt = (owner_data->handle)->BytesCount();
        if (m_stream_cursor >= m_cur_handle_cursor + bt)
        {
            SoundRender->i_destroy_source(owner_data->handle);
            owner_data->handle = SoundRender->i_create_source(owner_data->fn_attached[0].c_str());
            owner_data->fn_attached[0] = owner_data->fn_attached[1];
            owner_data->fn_attached[1] = "";
            m_cur_handle_cursor = get_cursor(true);

            if (m_target)
                m_target->OnSourceChanged();
        }
    }
}

u32 CSoundRender_Emitter::get_cursor(bool b_absolute) const
{
    if (b_absolute)
        return m_stream_cursor;
    else
    {
        VERIFY(m_stream_cursor - m_cur_handle_cursor >= 0);
        return m_stream_cursor - m_cur_handle_cursor;
    }
}

void CSoundRender_Emitter::move_cursor(int offset) { set_cursor(get_cursor(true) + offset); }
