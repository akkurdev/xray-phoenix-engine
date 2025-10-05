#include "stdafx.h"
#include "SoundRender_Core.h"
#include "SoundRender_Emitter.h"
#include "SoundRenderSource.h"

void CSoundRender_Emitter::start(ref_sound* _owner, BOOL _loop, float delay)
{
    starting_delay = delay;

    VERIFY(_owner);
    owner_data = _owner->_p;
    VERIFY(owner_data);
    //	source					= (CSoundRender_Source*)owner_data->handle;
    p_source.position.set(0, 0, 0);
    p_source.min_distance = RenderSource()->MinDistance(); // DS3D_DEFAULTMINDISTANCE;
    p_source.max_distance = RenderSource()->MaxDistance(); // 300.f;
    p_source.base_volume = RenderSource()->BaseVolume(); // 1.f
    p_source.volume = 1.f; // 1.f
    p_source.freq = 1.f;
    p_source.max_ai_distance = RenderSource()->MaxAiDistance(); // 300.f;

    if (fis_zero(delay, EPS_L))
    {
        m_current_state = _loop ? EmitterState::StartingLooped : EmitterState::Starting;
    }
    else
    {
        m_current_state = _loop ? EmitterState::StartingLoopedDelayed : EmitterState::StartingDelayed;
        m_propagadeTime = SoundRender->Timer.GetElapsed_sec();
    }
    bStopping = FALSE;
    bRewind = FALSE;
}

void CSoundRender_Emitter::i_stop()
{
    bRewind = FALSE;
    if (m_target)
        SoundRender->i_stop(this);
    if (owner_data)
    {
        Event_ReleaseOwner();
        VERIFY(this == owner_data->feedback);
        owner_data->feedback = NULL;
        owner_data = NULL;
    }
    m_current_state = EmitterState::Stopped;
}

void CSoundRender_Emitter::stop(BOOL bDeffered)
{
    if (bDeffered)
        bStopping = TRUE;
    else
        i_stop();
}

void CSoundRender_Emitter::rewind()
{
    bStopping = FALSE;

    float fTime = SoundRender->Timer.GetElapsed_sec();
    float fDiff = fTime - fTimeStarted;
    fTimeStarted += fDiff;
    fTimeToStop += fDiff;
    m_propagadeTime = fTime;

    set_cursor(0);
    bRewind = TRUE;
}

void CSoundRender_Emitter::pause(BOOL bVal, int id)
{
    if (bVal)
    {
        if (0 == iPaused)
            iPaused = id;
    }
    else
    {
        if (id == iPaused)
            iPaused = 0;
    }
}

void CSoundRender_Emitter::cancel()
{
    // Msg		("- %10s : %3d[%1.4f] : %s","cancel",dbg_ID,priority(),source->fname);
    switch (m_current_state)
    {
    case EmitterState::Playing:
        // switch to: SIMULATE
        m_current_state = EmitterState::Simulating; // switch state
        SoundRender->i_stop(this);
        break;
    case EmitterState::PlayingLooped:
        // switch to: SIMULATE
        m_current_state = EmitterState::SimulatingLooped; // switch state
        SoundRender->i_stop(this);
        break;
    default: FATAL("Non playing ref_sound forced out of render queue"); break;
    }
}
