#include "stdafx.h"
#include "SoundRender_Core.h"
#include "SoundRender_Emitter.h"
#include "SoundRenderSource.h"

void CSoundRender_Emitter::start(ref_sound* _owner, BOOL _loop, float delay)
{
    m_startDelay = delay;

    VERIFY(_owner);
    m_ownerData = _owner->_p;
    VERIFY(owner_data);
    m_params.position.set(0, 0, 0);
    m_params.min_distance = RenderSource()->MinDistance(); // DS3D_DEFAULTMINDISTANCE;
    m_params.max_distance = RenderSource()->MaxDistance(); // 300.f;
    m_params.base_volume = RenderSource()->BaseVolume(); // 1.f
    m_params.volume = 1.f; // 1.f
    m_params.freq = 1.f;
    m_params.max_ai_distance = RenderSource()->MaxAiDistance(); // 300.f;

    if (fis_zero(delay, EPS_L))
    {
        m_state = _loop ? EmitterState::StartingLooped : EmitterState::Starting;
    }
    else
    {
        m_state = _loop ? EmitterState::StartingLoopedDelayed : EmitterState::StartingDelayed;
        m_propagadeTime = SoundRender->Timer.GetElapsed_sec();
    }
    m_isStopped = false;
    m_isRewind = false;
}

void CSoundRender_Emitter::i_stop()
{
    m_isRewind = false;
    if (m_renderTarget)
        SoundRender->i_stop(this);
    if (m_ownerData)
    {
        Event_ReleaseOwner();
        VERIFY(this == m_ownerData->feedback);
        m_ownerData->feedback = NULL;
        m_ownerData = NULL;
    }
    m_state = EmitterState::Stopped;
}

void CSoundRender_Emitter::stop(BOOL bDeffered)
{
    if (bDeffered)
        m_isStopped = true;
    else
        i_stop();
}

void CSoundRender_Emitter::rewind()
{
    m_isStopped = false;

    float fTime = SoundRender->Timer.GetElapsed_sec();
    float fDiff = fTime - m_startTime;
    m_startTime += fDiff;
    m_stopTime += fDiff;
    m_propagadeTime = fTime;

    set_cursor(0);
    m_isRewind = true;
}

void CSoundRender_Emitter::pause(BOOL bVal, int id)
{
    if (bVal)
    {
        if (0 == m_paused)
            m_paused = id;
    }
    else
    {
        if (id == m_paused)
            m_paused = 0;
    }
}

void CSoundRender_Emitter::cancel()
{
    // Msg		("- %10s : %3d[%1.4f] : %s","cancel",dbg_ID,priority(),source->fname);
    switch (m_state)
    {
    case EmitterState::Playing:
        // switch to: SIMULATE
        m_state = EmitterState::Simulating; // switch state
        SoundRender->i_stop(this);
        break;
    case EmitterState::PlayingLooped:
        // switch to: SIMULATE
        m_state = EmitterState::SimulatingLooped; // switch state
        SoundRender->i_stop(this);
        break;
    default: FATAL("Non playing ref_sound forced out of render queue"); break;
    }
}
