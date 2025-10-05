#include "stdafx.h"
#include "SoundRender_Emitter.h"
#include "SoundRender_Core.h"
#include "SoundRenderSource.h"

XRSOUND_API extern float psSoundCull;
constexpr float TIME_TO_STOP_INFINITE = static_cast<float>(0xffffffff);

inline u32 calc_cursor(const float& fTimeStarted, float& fTime, const float& fTimeTotal, const float& fFreq, const WAVEFORMATEX& wfx)
{
    if (fTime < fTimeStarted)
        fTime = fTimeStarted; // Андрюха посоветовал, ассерт что ниже вылетел из за паузы как то хитро
    R_ASSERT((fTime - fTimeStarted) >= 0.0f);
    while ((fTime - fTimeStarted) > fTimeTotal / fFreq) // looped
    {
        fTime -= fTimeTotal / fFreq;
    }
    u32 curr_sample_num = iFloor((fTime - fTimeStarted) * fFreq * wfx.nSamplesPerSec);
    return curr_sample_num * (wfx.wBitsPerSample / 8) * wfx.nChannels;
}

void CSoundRender_Emitter::Update(float deltaTime)
{
    float fTime = SoundRender->fTimer_Value;
    float fDeltaTime = SoundRender->fTimer_Delta;

    VERIFY2(!!(owner_data) || (!(owner_data) && (m_current_state == stStopped)), "owner");
    VERIFY2(owner_data ? *(int*)(&owner_data->feedback) : 1, "owner");

    if (m_isRewind)
    {
        if (m_renderTarget)
            SoundRender->i_rewind(this);
        m_isRewind = false;
    }

    switch (m_state)
    {
    case EmitterState::Stopped: break;
    case EmitterState::StartingDelayed:
        if (m_paused)
            break;
        m_startDelay -= deltaTime;
        if (m_startDelay <= 0)
            m_state = EmitterState::Starting;
        break;
    case EmitterState::Starting:
        if (m_paused)
            break;
        m_startTime = fTime;
        m_stopTime = fTime + (get_length_sec() / m_params.freq); //--#SM+#--
        m_propagadeTime = fTime;
        m_fadeVolume = 1.f;
        m_occluderVolume = SoundRender->get_occlusion(m_params.position, .2f, m_occluder);
        m_smoothVolume = m_params.base_volume * m_params.volume * (m_soundData->s_type == st_Effect ? psSoundVEffects * psSoundVFactor : psSoundVMusic) * (m_is2D ? 1.f : m_occluderVolume);

        if (UpdateCulling(deltaTime))
        {
            m_state = EmitterState::Playing;
            set_cursor(0);
            SoundRender->i_start(this);
        }
        else
            m_state = EmitterState::Simulating;
        break;
    case EmitterState::StartingLoopedDelayed:
        if (m_paused)
            break;
        m_startDelay -= deltaTime;
        if (m_startDelay <= 0)
            m_state = EmitterState::StartingLooped;
        break;
    case EmitterState::StartingLooped:
        if (m_paused)
            break;
        m_startTime = fTime;
        m_stopTime = TIME_TO_STOP_INFINITE;
        m_propagadeTime = fTime;
        m_fadeVolume = 1.f;
        m_occluderVolume = SoundRender->get_occlusion(m_params.position, .2f, m_occluder);
        m_smoothVolume = m_params.base_volume * m_params.volume * (m_soundData->s_type == st_Effect ? psSoundVEffects * psSoundVFactor : psSoundVMusic) * (m_is2D ? 1.f : m_occluderVolume);
        
        if (UpdateCulling(deltaTime))
        {
            m_state = EmitterState::PlayingLooped;
            set_cursor(0);
            SoundRender->i_start(this);
        }
        else
            m_state = EmitterState::SimulatingLooped;
        break;
    case EmitterState::Playing:
        if (m_paused)
        {
            if (m_renderTarget)
            {
                SoundRender->i_stop(this);
                m_state = EmitterState::Simulating;
            }
            m_startTime += fDeltaTime;
            m_stopTime += fDeltaTime;
            m_propagadeTime += fDeltaTime;
            break;
        }
        if (fTime >= m_stopTime)
        {
            // STOP
            m_state = EmitterState::Stopped;
            SoundRender->i_stop(this);
        }
        else
        {
            if (!UpdateCulling(deltaTime))
            {
                // switch to: SIMULATE
                m_state = EmitterState::Simulating; // switch state
                SoundRender->i_stop(this);
            }
            else
            {
                // We are still playing
                update_environment(deltaTime);
            }
        }
        break;
    case EmitterState::Simulating:
        if (m_paused)
        {
            m_startTime += fDeltaTime;
            m_stopTime += fDeltaTime;
            m_propagadeTime += fDeltaTime;
            break;
        }
        if (fTime >= m_stopTime)
        {
            // STOP
            m_state = EmitterState::Stopped;
        }
        else
        {
            u32 ptr = calc_cursor(m_startTime, fTime, get_length_sec(), m_params.freq, RenderSource()->Format()); //--#SM+#--
            set_cursor(ptr);

            if (UpdateCulling(deltaTime))
            {
                // switch to: PLAY
                m_state = EmitterState::Playing;
                SoundRender->i_start(this);
            }
        }
        break;
    case EmitterState::PlayingLooped:
        if (m_paused)
        {
            if (m_renderTarget)
            {
                SoundRender->i_stop(this);
                m_state = EmitterState::SimulatingLooped;
            }
            m_startTime += fDeltaTime;
            m_propagadeTime += fDeltaTime;
            break;
        }
        if (!UpdateCulling(deltaTime))
        {
            // switch to: SIMULATE
            m_state = EmitterState::SimulatingLooped; // switch state
            SoundRender->i_stop(this);
        }
        else
        {
            // We are still playing
            update_environment(deltaTime);
        }
        break;
    case EmitterState::SimulatingLooped:
        if (m_paused)
        {
            m_startTime += fDeltaTime;
            m_propagadeTime += fDeltaTime;
            break;
        }
        if (UpdateCulling(deltaTime))
        {
            // switch to: PLAY
            m_state = EmitterState::PlayingLooped; // switch state
            u32 ptr = calc_cursor(m_startTime, fTime, get_length_sec(), m_params.freq, RenderSource()->Format()); //--#SM+#--
            set_cursor(ptr);

            SoundRender->i_start(this);
        }
        break;
    }

    // hard rewind
    switch (m_state)
    {
    case EmitterState::Starting:
    case EmitterState::StartingLooped:
    case EmitterState::Playing:
    case EmitterState::Simulating:
    case EmitterState::PlayingLooped:
    case EmitterState::SimulatingLooped:
        if (m_rewindTime > 0.0f)
        {
            float fLength = get_length_sec();
            bool bLooped = (m_stopTime == 0xffffffff);

            R_ASSERT2(fLength >= m_rewindTime, "set_time: target time is bigger than length of sound");

            float fRemainingTime = (fLength - m_rewindTime) / m_params.freq;
            float fPastTime = m_rewindTime / m_params.freq;

            m_startTime = SoundRender->fTimer_Value - fPastTime;
            m_propagadeTime = m_startTime; //--> For AI events

            if (m_startTime < 0.0f)
            {
                R_ASSERT2(m_startTime >= 0.0f, "Possible error in sound rewind logic! See log.");

                m_startTime = SoundRender->fTimer_Value;
                m_propagadeTime = m_startTime;
            }

            if (!bLooped)
            {
                //--> Пересчитываем время, когда звук должен остановиться [recalculate stop time]
                m_stopTime = SoundRender->fTimer_Value + fRemainingTime;
            }

            u32 ptr = calc_cursor(m_startTime, fTime, fLength, m_params.freq, RenderSource()->Format());
            set_cursor(ptr);

            m_rewindTime = 0.0f;
        }
    default: break;
    }

    // if deffered stop active and volume==0 -> physically stop sound
    if (m_isStopped && fis_zero(m_fadeVolume))
        i_stop();

    VERIFY2(!!(owner_data) || (!(owner_data) && (m_current_state == stStopped)), "owner");
    VERIFY2(owner_data ? *(int*)(owner_data->feedback) : 1, "owner");

    // footer
    m_isMoved = false;
    if (m_state != EmitterState::Stopped)
    {
        if (fTime >= m_propagadeTime)
            Event_Propagade();
    }
    else if (m_soundData)
    {
        VERIFY(this == owner_data->feedback);
        m_soundData->feedback = 0;
        m_soundData = nullptr;
    }
}

IC void volume_lerp(float& c, float t, float s, float dt)
{
    float diff = t - c;
    float diff_a = _abs(diff);
    if (diff_a < EPS_S)
        return;
    float mot = s * dt;
    if (mot > diff_a)
        mot = diff_a;
    c += (diff / diff_a) * mot;
}
#include "ai_sounds.h"

bool CSoundRender_Emitter::UpdateCulling(float deltaTime)
{
    if (m_is2D)
    {
        m_occluderVolume = 1.f;
        m_fadeVolume += deltaTime * 10.f * (m_isStopped ? -1.f : 1.f);
    }
    else
    {
        // Check range
        float dist = SoundRender->listener_position().distance_to(m_params.position);
        if (dist > m_params.max_distance)
        {
            m_smoothVolume = 0;
            return FALSE;
        }

        // Calc attenuated volume
        float fade_scale =
            m_isStopped || (Attitude() * m_params.base_volume * m_params.volume * (m_soundData->s_type == st_Effect ? psSoundVEffects * psSoundVFactor : psSoundVMusic) < psSoundCull) ?
            -1.f :
            1.f;
        m_fadeVolume += deltaTime * 10.f * fade_scale;

        // Update occlusion
        float occ = (m_soundData->g_type == SOUND_TYPE_WORLD_AMBIENT) ? 1.0f : SoundRender->get_occlusion(m_params.position, .2f, m_occluder);
        volume_lerp(m_occluderVolume, occ, 1.f, deltaTime);
        clamp(m_occluderVolume, 0.f, 1.f);
    }
    clamp(m_fadeVolume, 0.f, 1.f);
    // Update smoothing
    m_smoothVolume = .9f * m_smoothVolume +
        .1f * (m_params.base_volume * m_params.volume * (m_soundData->s_type == st_Effect ? psSoundVEffects * psSoundVFactor : psSoundVMusic) * m_occluderVolume * m_fadeVolume);
    if (m_smoothVolume < psSoundCull)
        return FALSE; // allow volume to go up
    // Here we has enought "PRIORITY" to be soundable
    // If we are playing already, return OK
    // --- else check availability of resources
    if (m_renderTarget)
        return TRUE;
    else
        return SoundRender->i_allow_play(this);
}

float CSoundRender_Emitter::Priority() { return m_smoothVolume * Attitude() * m_priorityScale; }

float CSoundRender_Emitter::Attitude()
{
    float dist = SoundRender->listener_position().distance_to(m_params.position);
    float rolloff_dist = psSoundRolloff * dist;

    // Calc linear fade --#SM+#--
    // https://www.desmos.com/calculator/lojovfugle
    const float fMinDistDiff = rolloff_dist - m_params.min_distance;
    float att;
    if (fMinDistDiff > 0.f)
    {
        const float fMaxDistDiff = m_params.max_distance - m_params.min_distance;
        att = pow(1.f - (fMinDistDiff / fMaxDistDiff), psSoundLinearFadeFactor);
    }
    else
        att = 1.f;
    clamp(att, 0.f, 1.f);

    return att;
}

void CSoundRender_Emitter::update_environment(float dt)
{
}
