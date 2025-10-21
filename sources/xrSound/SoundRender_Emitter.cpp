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

float CSoundRender_Emitter::Volume() const
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

float CSoundRender_Emitter::Priority() const
{
    return m_smoothVolume * Attitude() * m_priorityScale;
}

float CSoundRender_Emitter::Attitude() const
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


void CSoundRender_Emitter::Start(ref_sound* sound, bool isLooped, float delay)
{
    m_startDelay = delay;

    VERIFY(_owner);
    m_soundData = sound->_p;
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
        m_state = isLooped ? EmitterState::StartingLooped : EmitterState::Starting;
    }
    else
    {
        m_state = isLooped ? EmitterState::StartingLoopedDelayed : EmitterState::StartingDelayed;
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
    if (m_soundData)
    {
        Event_ReleaseOwner();
        VERIFY(this == m_soundData->feedback);
        m_soundData->feedback = NULL;
        m_soundData = NULL;
    }
    m_state = EmitterState::Stopped;
}

void CSoundRender_Emitter::Stop(bool isDeffered)
{
    if (isDeffered)
        m_isStopped = true;
    else
        i_stop();
}

void CSoundRender_Emitter::Rewind()
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

void CSoundRender_Emitter::Pause(bool hasValue, int32_t pausedId)
{
    if (hasValue)
    {
        if (0 == m_paused)
            m_paused = pausedId;
    }
    else
    {
        if (pausedId == m_paused)
            m_paused = 0;
    }
}

uint32_t CSoundRender_Emitter::PlayTime() const
{
    if (m_state == EmitterState::Playing ||
        m_state == EmitterState::PlayingLooped ||
        m_state == EmitterState::Simulating ||
        m_state == EmitterState::SimulatingLooped)
        return iFloor((SoundRender->fTimer_Value - m_startTime) * 1000.0f);
    else
        return 0;
}

void CSoundRender_Emitter::Cancel()
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


void CSoundRender_Emitter::FillData(uint8_t* ptr, uint32_t offset, uint32_t size)
{
    u32 line_size = SoundRender->cache.LineSize();
    u32 line = offset / line_size;

    // prepare for first line (it can be unaligned)
    u32 line_offs = offset - line * line_size;
    u32 line_amount = line_size - line_offs;

    while (size)
    {
        // cache access
        if (SoundRender->cache.Request(*RenderSource()->Cache(), line))
        {
            RenderSource()->Decompress(line, m_renderTarget->OggFile());
        }

        // fill block
        u32 blk_size = _min(size, line_amount);
        u8* cachePtr = (u8*)SoundRender->cache.GetDataById(*RenderSource()->Cache(), line);
        CopyMemory(ptr, cachePtr + line_offs, blk_size);

        // advance
        line++;
        size -= blk_size;
        ptr += blk_size;
        offset += blk_size;
        line_offs = 0;
        line_amount = line_size;
    }
}

void CSoundRender_Emitter::FillBlock(void* ptr, uint32_t size)
{
    // Msg			("stream: %10s - [%X]:%d, p=%d, t=%d",*source->fname,ptr,size,position,source->dwBytesTotal);
    LPBYTE dest = LPBYTE(ptr);
    u32 dwBytesTotal = get_bytes_total();

    if ((get_cursor(true) + size) > dwBytesTotal)
    {
        // We are reaching the end of data, what to do?
        switch (m_state)
        {
        case EmitterState::Playing: { // Fill as much data as we can, zeroing remainder
            if (get_cursor(true) >= dwBytesTotal)
            {
                // ??? We requested the block after remainder - just zero
                Memory.mem_fill(dest, 0, size);
            }
            else
            {
                // Calculate remainder
                u32 sz_data = dwBytesTotal - get_cursor(true);
                u32 sz_zero = (get_cursor(true) + size) - dwBytesTotal;
                VERIFY(size == (sz_data + sz_zero));
                FillData(dest, get_cursor(false), sz_data);
                Memory.mem_fill(dest + sz_data, 0, sz_zero);
            }
            move_cursor(size);
        }
                                  break;
        case EmitterState::PlayingLooped: {
            u32 hw_position = 0;
            do
            {
                u32 sz_data = dwBytesTotal - get_cursor(true);
                u32 sz_write = _min(size - hw_position, sz_data);
                FillData(dest + hw_position, get_cursor(true), sz_write);
                hw_position += sz_write;
                move_cursor(sz_write);
                set_cursor(get_cursor(true) % dwBytesTotal);
            } while (0 != (size - hw_position));
        }
                                        break;
        default: FATAL("SOUND: Invalid emitter state"); break;
        }
    }
    else
    {
        u32 bt_handle = m_soundData->handle->BytesCount();
        if (get_cursor(true) + size > m_handleCursor + bt_handle)
        {
            R_ASSERT(m_soundData->fn_attached[0].size());

            u32 rem = 0;
            if ((m_handleCursor + bt_handle) > get_cursor(true))
            {
                rem = (m_handleCursor + bt_handle) - get_cursor(true);
                FillData(dest, get_cursor(false), rem);
                move_cursor(rem);
            }
            FillBlock(dest + rem, size - rem);
        }
        else
        {
            // Everything OK, just stream
            FillData(dest, get_cursor(false), size);
            move_cursor(size);
        }
    }
}

ISoundRenderSource* CSoundRender_Emitter::RenderSource()
{
    return m_soundData->handle;
}

ISoundRenderTarget* CSoundRender_Emitter::RenderTarget()
{
    return m_renderTarget;
}

u32 CSoundRender_Emitter::get_bytes_total() const
{
    return m_soundData->dwBytesTotal;
}

float CSoundRender_Emitter::get_length_sec() const
{
    return m_soundData->get_length_sec();
}
