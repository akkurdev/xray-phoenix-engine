#include "stdafx.h"
#include "ai_sounds.h"
#include "SoundEmitter.h"
#include "soundrender_core.h"
#include "SoundRenderSource.h"
#include "OalSoundRenderTarget.h"

extern u32 psSoundModel;
extern float psSoundVEffects;

XRSOUND_API extern float psSoundCull;
constexpr float TIME_TO_STOP_INFINITE = static_cast<float>(0xffffffff);

inline u32 calc_cursor(const float& fTimeStarted, const float& fTime, const float& fTimeTotal, const float& fFreq, const WAVEFORMATEX& wfx)
{
    auto time = fTime < fTimeStarted
        ? fTimeStarted
        : fTime;
    
    R_ASSERT((time - fTimeStarted) >= 0.0f);
    while ((time - fTimeStarted) > fTimeTotal / fFreq) // looped
    {
        time -= fTimeTotal / fFreq;
    }
    u32 curr_sample_num = iFloor((time - fTimeStarted) * fFreq * wfx.nSamplesPerSec);
    SoundRender->SetTime(time);

    return curr_sample_num * (wfx.wBitsPerSample / 8) * wfx.nChannels;
}

void volume_lerp(float& c, float t, float s, float dt)
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

SoundEmitter::SoundEmitter() :
    m_startDelay(0.f),
    m_renderTarget(nullptr),
    m_soundData(nullptr),
    m_priorityScale(1.f),
    m_smoothVolume(1.f),
    m_occluderVolume(1.f),
    m_fadeVolume(1.f),
    m_state(EmitterState::Stopped),
    m_streamCursor(0u),
    m_handleCursor(0u),
    m_paused(0),
    m_isMoved(true),
    m_is2D(false),
    m_isStopped(false),
    m_isRewind(false),
    m_startTime(0.f),
    m_stopTime(0.f),
    m_propagadeTime(0.f),
    m_rewindTime(0.f),
    m_marker(0xabababab)
{
    m_occluder[0].set(0, 0, 0);
    m_occluder[1].set(0, 0, 0);
    m_occluder[2].set(0, 0, 0);

    SetCursor(0);
}

SoundEmitter::~SoundEmitter()
{
    OnRelease();
}

bool SoundEmitter::Is2D() const
{
    return m_is2D;
}

float SoundEmitter::Priority() const
{
    return m_smoothVolume * Attitude() * m_priorityScale;
}

uint32_t SoundEmitter::PlayTime() const
{
    auto isPlayed = 
        m_state == EmitterState::Playing ||
        m_state == EmitterState::PlayingLooped ||
        m_state == EmitterState::Simulating ||
        m_state == EmitterState::SimulatingLooped;
    
    return isPlayed
        ? iFloor((SoundRender->Time() - m_startTime) * 1000.0f)
        : 0;
}

float SoundEmitter::Volume() const
{
    return m_smoothVolume;
}

float SoundEmitter::StopTime() const
{
    return m_stopTime;
}

bool SoundEmitter::IsPlaying() const
{
    return m_state != EmitterState::Stopped;
}

uint32_t SoundEmitter::Marker() const
{
    return m_marker;
}

CSound_params* SoundEmitter::Params()
{
    return &m_params;
}

ref_sound_data_ptr SoundEmitter::SoundData()
{
    return m_soundData;
}

ISoundRenderSource* SoundEmitter::RenderSource()
{
    return m_soundData->handle;
}

ISoundRenderTarget* SoundEmitter::RenderTarget()
{
    return m_renderTarget;
}

void SoundEmitter::Start(ref_sound* sound, bool isLooped, float delay)
{
    VERIFY(_owner);    
    VERIFY(owner_data);

    if (fis_zero(delay, EPS_L))
    {
        m_state = isLooped
            ? EmitterState::StartingLooped
            : EmitterState::Starting;
    }
    else
    {
        m_state = isLooped
            ? EmitterState::StartingLoopedDelayed
            : EmitterState::StartingDelayed;

        m_propagadeTime = SoundRender->ElapsedTime();
    }

    m_startDelay = delay;
    m_soundData = sound->_p;
    m_isStopped = false;
    m_isRewind = false;

    m_params.position.set(0, 0, 0);
    m_params.min_distance = RenderSource()->MinDistance(); // DS3D_DEFAULTMINDISTANCE;
    m_params.max_distance = RenderSource()->MaxDistance(); // 300.f;
    m_params.base_volume = RenderSource()->BaseVolume(); // 1.f
    m_params.volume = 1.f;
    m_params.freq = 1.f;
    m_params.max_ai_distance = RenderSource()->MaxAiDistance(); // 300.f;     
}

void SoundEmitter::Cancel()
{
    if (m_state == EmitterState::Playing)
    {
        m_state = EmitterState::Simulating;
    }
    else if (m_state == EmitterState::PlayingLooped)
    {
        m_state = EmitterState::SimulatingLooped;
    }
    else
    {
        FATAL("Non playing ref_sound forced out of render queue");
    }

    SoundRender->i_stop(this);
}

void SoundEmitter::Update(float deltaTime)
{
    if (m_isRewind)
    {
        if (m_renderTarget)
        {
            SoundRender->i_rewind(this);
        }
        m_isRewind = false;
    }

    switch (m_state)
    {
    case EmitterState::StartingDelayed:
        OnDelayedStart(deltaTime, false);
        break;
    case EmitterState::Starting:
        OnStart(deltaTime, false);
        break;
    case EmitterState::StartingLoopedDelayed:
        OnDelayedStart(deltaTime, true);
        break;
    case EmitterState::StartingLooped:
        OnStart(deltaTime, true);
        break;
    case EmitterState::Playing:
        OnPlay(deltaTime, false);
        break;
    case EmitterState::Simulating:
        OnSimulate(deltaTime);
        break;
    case EmitterState::PlayingLooped:
        OnPlay(deltaTime, true);
        break;
    case EmitterState::SimulatingLooped:
        OnLoopedSimulate(deltaTime);
        break;
    }

    auto canRewind =
        m_state == EmitterState::Starting ||
        m_state == EmitterState::StartingLooped ||
        m_state == EmitterState::Playing || 
        m_state == EmitterState::Simulating || 
        m_state == EmitterState::PlayingLooped || 
        m_state == EmitterState::SimulatingLooped;

    if (canRewind)
    {
        OnRewind();
    }

    // if deffered stop active and volume==0 -> physically stop sound
    if (m_isStopped && fis_zero(m_fadeVolume))
    {
        i_stop();
    }

    m_isMoved = false;

    if (m_state != EmitterState::Stopped)
    {
        if (SoundRender->Time() >= m_propagadeTime)
        {
            OnPropagade();
        }
    }
    else if (m_soundData)
    {
        VERIFY(this == owner_data->feedback);
        m_soundData->feedback = 0;
        m_soundData = nullptr;
    }
}

void SoundEmitter::Rewind()
{
    float fTime = SoundRender->ElapsedTime();
    float fDiff = fTime - m_startTime;

    m_isStopped = false;
    m_startTime += fDiff;
    m_stopTime += fDiff;
    m_propagadeTime = fTime;

    SetCursor(0);
    m_isRewind = true;
}

void SoundEmitter::Stop(bool isDeffered)
{
    if (isDeffered)
        m_isStopped = true;
    else
        i_stop();
}

void SoundEmitter::Pause(bool hasValue, int32_t pausedId)
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

void SoundEmitter::SwitchTo2D()
{
    m_is2D = true;
    SetPriority(100.f);
}

void SoundEmitter::SwitchTo3D()
{
    m_is2D = false;
}

void SoundEmitter::SetPriority(float priority)
{
    m_priorityScale = priority;
}

// Перемотка звука на заданную секунду [rewind snd to target time] --#SM+#--
void SoundEmitter::SetTime(float time)
{
    m_rewindTime = time >= 0.f
        ? time
        : 0.f;

    R_ASSERT2(m_soundData->get_length_sec() >= m_rewindTime, "set_time: time is bigger than length of sound");
}

void SoundEmitter::SetMarker(uint32_t marker)
{
    m_marker = marker;
}

void SoundEmitter::SetRenderTarget(ISoundRenderTarget* target)
{
    m_renderTarget = target;
}

void SoundEmitter::SetStopTime(float stopTime)
{
    m_stopTime = stopTime;
}

void SoundEmitter::SetPosition(const Fvector& position)
{
    m_params.position = RenderSource()->Format().nChannels == 1 && _valid(position)
        ? position
        : Fvector{ 0, 0, 0 };
    
    m_isMoved = true;
}

void SoundEmitter::SetFrequency(float frequency)
{
    VERIFY(_valid(scale));

    m_params.freq = frequency;
}

void SoundEmitter::SetRange(float minDistance, float maxDistance)
{
    VERIFY(_valid(min) && _valid(max));

    m_params.min_distance = minDistance;
    m_params.max_distance = maxDistance;
}

void SoundEmitter::SetVolume(float volume)
{
    m_params.volume = _valid(volume)
        ? volume
        : 0.f;
}

void SoundEmitter::FillBlock(void* ptr, uint32_t size)
{
    auto destination = LPBYTE(ptr);
    auto dwBytesTotal = m_soundData->dwBytesTotal;

    if ((m_streamCursor + size) > dwBytesTotal)
    {
        // We are reaching the end of data, what to do?
        if (m_state == EmitterState::Playing)
        {
            if (m_streamCursor < dwBytesTotal)
            {
                // Calculate remainder
                auto dataSize = dwBytesTotal - m_streamCursor;
                auto zeroSize = m_streamCursor + size - dwBytesTotal;
                VERIFY(size == (sz_data + sz_zero));

                FillData(destination, m_streamCursor - m_handleCursor, dataSize);
                Memory.mem_fill(destination + dataSize, 0, zeroSize);
            }
            else
            {
                Memory.mem_fill(destination, 0, size);
            }
            SetCursor(m_streamCursor + size);
        }
        else if (m_state == EmitterState::PlayingLooped)
        {
            uint32_t position = 0;
            do
            {
                auto dataSize = dwBytesTotal - m_streamCursor;
                auto writeSize = _min(size - position, dataSize);
                FillData(destination + position, m_streamCursor, writeSize);
                position += writeSize;

                SetCursor(m_streamCursor + writeSize);
                SetCursor(m_streamCursor % dwBytesTotal);
            } 
            while (0 != (size - position));
        }
        else
        {
            FATAL("Invalid emitter state");
        }
    }
    else
    {
        auto handleBytesCount = m_soundData->handle->BytesCount();

        if (m_streamCursor + size > m_handleCursor + handleBytesCount)
        {
            R_ASSERT(m_soundData->fn_attached[0].size());

            uint32_t reminder = 0;

            if ((m_handleCursor + handleBytesCount) > m_streamCursor)
            {
                reminder = (m_handleCursor + handleBytesCount) - m_streamCursor;
                FillData(destination, m_streamCursor - m_handleCursor, reminder);
                SetCursor(m_streamCursor + reminder);
            }
            FillBlock(destination + reminder, size - reminder);
        }
        else
        {
            // Everything OK, just stream
            FillData(destination, m_streamCursor - m_handleCursor, size);
            SetCursor(m_streamCursor + size);
        }
    }
}

void SoundEmitter::FillData(uint8_t* ptr, uint32_t offset, uint32_t size)
{
    auto lineSize = SoundRender->Cache().LineSize();
    auto line = offset / lineSize;

    // prepare for first line (it can be unaligned)
    auto lineOffset = offset - line * lineSize;
    auto lineAmount = lineSize - lineOffset;

    while (size)
    {
        // cache access
        if (SoundRender->Cache().Request(*RenderSource()->Cache(), line))
        {
            RenderSource()->Decompress(line, m_renderTarget->OggFile());
        }

        // fill block
        auto blk_size = _min(size, lineAmount);
        auto cachePtr = (uint8_t*)SoundRender->Cache().GetDataById(*RenderSource()->Cache(), line);

        CopyMemory(ptr, cachePtr + lineOffset, blk_size);

        line++;
        size -= blk_size;
        ptr += blk_size;
        offset += blk_size;

        lineOffset = 0;
        lineAmount = lineSize;
    }
}

bool SoundEmitter::UpdateCulling(float deltaTime)
{    
    m_fadeVolume = UpdateFadeVolume(deltaTime, m_fadeVolume);
    m_occluderVolume = UpdateOccludeVolume(deltaTime, m_occluderVolume);
    m_smoothVolume = UpdateSmoothVolume(deltaTime, m_smoothVolume, m_fadeVolume, m_occluderVolume);

    if (m_smoothVolume < psSoundCull)
    {
        return false;
    }

    return m_renderTarget
        ? true
        : SoundRender->i_allow_play(this);
}

void SoundEmitter::i_stop()
{
    m_isRewind = false;

    if (m_renderTarget)
    {
        SoundRender->i_stop(this);
    }

    if (m_soundData)
    {
        OnRelease();
        VERIFY(this == m_soundData->feedback);

        m_soundData->feedback = nullptr;
        m_soundData = nullptr;
    }
    m_state = EmitterState::Stopped;
}

void SoundEmitter::SetCursor(uint32_t position)
{
    m_streamCursor = position;

    if (m_soundData._get() && 
        m_soundData->fn_attached[0].size() && 
        m_streamCursor >= m_handleCursor + (m_soundData->handle)->BytesCount())
    {
        xr_delete(m_soundData->handle);

        m_soundData->handle = SoundRender->i_create_source(m_soundData->fn_attached[0].c_str());
        m_soundData->fn_attached[0] = m_soundData->fn_attached[1];
        m_soundData->fn_attached[1] = "";
        m_handleCursor = m_streamCursor;

        if (m_renderTarget)
        {
            m_renderTarget->OnSourceChanged();
        }
    }
}

void SoundEmitter::OnPropagade()
{
    m_propagadeTime += ::Random.randF(
        s_f_def_event_pulse - 0.030f, 
        s_f_def_event_pulse + 0.030f);

    if (!m_soundData || 
        !m_soundData->g_object || 
        !SoundRender->Handler || 
        m_soundData->g_type == 0)
    {
        return;
    }

    VERIFY(_valid(m_params.volume));

    float clip = m_params.max_ai_distance * m_params.volume;
    float range = _min(m_params.max_ai_distance, clip);

    if (range < 0.1f)
    {
        return;
    }
    SoundRender->s_events.push_back(mk_pair(m_soundData, range));
}

void SoundEmitter::OnRelease()
{
    if (!m_soundData)
    {
        return;
    }

    for (auto it = 0; it < SoundRender->s_events.size(); it++)
    {
        if (m_soundData == SoundRender->s_events[it].first)
        {
            SoundRender->s_events.erase(SoundRender->s_events.begin() + it);
            it--;
        }
    }
}

void SoundEmitter::OnStart(float deltaTime, bool isLooped)
{
    if (m_paused)
    {
        return;
    }

    auto effectScale = m_soundData->s_type == st_Effect 
        ? psSoundVEffects * psSoundVFactor 
        : psSoundVMusic;

    auto occludeVolumeScale = m_is2D
        ? 1.f
        : m_occluderVolume;

    m_startTime = SoundRender->Time();
    m_propagadeTime = SoundRender->Time();
    m_stopTime = isLooped 
        ? TIME_TO_STOP_INFINITE 
        : SoundRender->Time() + (m_soundData->get_length_sec() / m_params.freq);

    m_fadeVolume = 1.f;
    m_occluderVolume = SoundRender->get_occlusion(m_params.position, .2f, m_occluder);
    m_smoothVolume = m_params.base_volume * m_params.volume * effectScale * occludeVolumeScale;

    if (UpdateCulling(deltaTime))
    {
        m_state = isLooped 
            ? EmitterState::PlayingLooped 
            : EmitterState::Playing;

        SetCursor(0);
        SoundRender->i_start(this);
    }
    else
    {
        m_state = isLooped 
            ? EmitterState::SimulatingLooped 
            : EmitterState::Simulating;
    }
}

void SoundEmitter::OnDelayedStart(float deltaTime, bool isLooped)
{
    if (m_paused)
    {
        return;
    }

    m_startDelay -= deltaTime;

    if (m_startDelay <= 0)
    {
        m_state = isLooped 
            ? EmitterState::StartingLooped 
            : EmitterState::Starting;
    }
}

void SoundEmitter::OnPlay(float deltaTime, bool isLooped)
{    
    if (m_paused)
    {
        if (m_renderTarget)
        {
            SoundRender->i_stop(this);
            m_state = isLooped 
                ? EmitterState::SimulatingLooped 
                : EmitterState::Simulating;
        }

        m_startTime += SoundRender->DeltaTime();
        m_propagadeTime += SoundRender->DeltaTime();

        if (!isLooped)
        {
            m_stopTime += SoundRender->DeltaTime();
        }
        return;
    }
    if (SoundRender->Time() >= m_stopTime)
    {
        if (!isLooped)
        {
            m_state = EmitterState::Stopped;
            SoundRender->i_stop(this);
        }
    }
    else
    {
        if (!UpdateCulling(deltaTime))
        {
            m_state = isLooped 
                ? EmitterState::SimulatingLooped 
                : EmitterState::Simulating;
            SoundRender->i_stop(this);
        }
    }
}

void SoundEmitter::OnSimulate(float deltaTime)
{
    if (m_paused)
    {
        m_startTime += SoundRender->DeltaTime();
        m_stopTime += SoundRender->DeltaTime();
        m_propagadeTime += SoundRender->DeltaTime();

        return;
    }

    if (SoundRender->Time() >= m_stopTime)
    {
        m_state = EmitterState::Stopped;
    }
    else
    {
        auto cursor = calc_cursor(
            m_startTime, 
            SoundRender->Time(),
            m_soundData->get_length_sec(), 
            m_params.freq, 
            RenderSource()->Format());

        SetCursor(cursor);

        if (UpdateCulling(deltaTime))
        {
            m_state = EmitterState::Playing;
            SoundRender->i_start(this);
        }
    }
}

void SoundEmitter::OnLoopedSimulate(float deltaTime)
{
    if (m_paused)
    {
        m_startTime += SoundRender->DeltaTime();
        m_propagadeTime += SoundRender->DeltaTime();
        return;
    }

    if (UpdateCulling(deltaTime))
    {
        m_state = EmitterState::PlayingLooped;
        auto cursor = calc_cursor(
            m_startTime, 
            SoundRender->Time(),
            m_soundData->get_length_sec(), 
            m_params.freq, 
            RenderSource()->Format());

        SetCursor(cursor);
        SoundRender->i_start(this);
    }
}

void SoundEmitter::OnRewind()
{
    if (m_rewindTime <= 0.f)
    {
        return;
    }

    auto length = m_soundData->get_length_sec();
    auto isLooped = (m_stopTime == 0xffffffff);

    R_ASSERT2(length >= m_rewindTime, "set_time: target time is bigger than length of sound");

    float remainTime = (length - m_rewindTime) / m_params.freq;
    float pastTime = m_rewindTime / m_params.freq;

    m_startTime = SoundRender->Time() - pastTime;
    m_propagadeTime = m_startTime; //--> For AI events

    if (m_startTime < 0.0f)
    {
        R_ASSERT2(m_startTime >= 0.0f, "Possible error in sound rewind logic! See log.");

        m_startTime = SoundRender->Time();
        m_propagadeTime = m_startTime;
    }

    if (!isLooped)
    {
        //--> Пересчитываем время, когда звук должен остановиться [recalculate stop time]
        m_stopTime = SoundRender->Time() + remainTime;
    }

    auto cursor = calc_cursor(
        m_startTime, 
        SoundRender->Time(),
        length, 
        m_params.freq, 
        RenderSource()->Format());

    SetCursor(cursor);
    m_rewindTime = 0.0f;
}

float SoundEmitter::Attitude() const
{
    float distance = SoundRender->listener_position().distance_to(m_params.position);
    float minDistanceDiff = psSoundRolloff * distance - m_params.min_distance;
    float maxDistanceDiff = m_params.max_distance - m_params.min_distance;

    float attitude = minDistanceDiff > 0.f
        ? pow(1.f - (minDistanceDiff / maxDistanceDiff), psSoundLinearFadeFactor)
        : 1.f;

    clamp(attitude, 0.f, 1.f);
    return attitude;
}

float SoundEmitter::UpdateSmoothVolume(float deltaTime, float currentVolume, float fadeVolume, float occludeVolume)
{    
    if (m_is2D)
    {
        return currentVolume;
    }

    float distance = SoundRender->listener_position().distance_to(m_params.position);
    if (distance > m_params.max_distance)
    {
        return 0.f;
    }

    auto effectScale = m_soundData->s_type == st_Effect
        ? psSoundVEffects * psSoundVFactor
        : psSoundVMusic;

    return 0.9f * m_smoothVolume + 0.1f * (m_params.base_volume * m_params.volume * effectScale * occludeVolume * fadeVolume);
}

float SoundEmitter::UpdateFadeVolume(float deltaTime, float currentVolume)
{
    auto effectScale = m_soundData->s_type == st_Effect
        ? psSoundVEffects * psSoundVFactor
        : psSoundVMusic;

    auto attitude = Attitude();

    auto canCullByVolume = !m_is2D
        ? attitude * m_params.base_volume * m_params.volume * effectScale < psSoundCull
        : false;

    float fadeScale = m_isStopped || canCullByVolume
        ? -1.f
        : 1.f;

    auto volume = currentVolume + (deltaTime * 10.f * fadeScale);
    return std::clamp(volume, 0.f, 1.f);
}

float SoundEmitter::UpdateOccludeVolume(float deltaTime, float currentVolume)
{
    if (m_is2D)
    {
        return 1.f;
    }

    float occlusion = (m_soundData->g_type != SOUND_TYPE_WORLD_AMBIENT)
        ? SoundRender->get_occlusion(m_params.position, 0.2f, m_occluder)
        : 1.f;

    float volume = currentVolume;
    volume_lerp(volume, occlusion, 1.f, deltaTime);

    return std::clamp(volume, 0.f, 1.f);
}
