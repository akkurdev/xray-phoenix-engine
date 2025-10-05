#include "stdafx.h"
#include "OalSoundRenderTarget.h"
#include "soundrender_emitter.h"
#include "SoundRenderSource.h"
#include "soundrender_core.h"
#include <efx.h>

xr_vector<u8> g_target_temp_data;

extern int ov_seek_func(void* dataSource, s64 offset, int whence);
extern size_t ov_read_func(void* ptr, size_t size, size_t nmemb, void* dataSource);
extern int ov_close_func(void* dataSource);
extern long ov_tell_func(void* dataSource);

DefaultSoundRenderTarget::DefaultSoundRenderTarget() :
    m_source(0),
    m_bufferBlock(0),
    m_cacheGain(0.f),
    m_cachePitch(0.f),
    m_priority(0.f),
    m_isRendering(false),
    m_hasAlSoft(false),
    m_emitter(nullptr),
    m_reader(nullptr),
    m_oggFile(nullptr)
{
    m_oggFile = xr_new<OggVorbis_File>();
}

DefaultSoundRenderTarget::~DefaultSoundRenderTarget()
{
    xr_delete(m_oggFile);
}

OggVorbis_File* DefaultSoundRenderTarget::OggFile()
{
    if (!m_reader)
    {
        Attach();
    }
    return m_oggFile;
}

CSoundRender_Emitter* DefaultSoundRenderTarget::Emitter()
{
    return m_emitter;
}

bool DefaultSoundRenderTarget::IsRendering() const
{
    return m_isRendering;
}

float DefaultSoundRenderTarget::CacheGain()
{
    return m_cacheGain;
}

float DefaultSoundRenderTarget::CachePitch()
{
    return m_cachePitch;
}

float DefaultSoundRenderTarget::Priority()
{
    return m_priority;
}

bool DefaultSoundRenderTarget::HasAlSoft()
{
    return m_hasAlSoft;
}

void DefaultSoundRenderTarget::SetPriority(float priority)
{
    m_priority = priority;
}

void DefaultSoundRenderTarget::UseAlSoft(bool useAlSoft)
{
    m_hasAlSoft = useAlSoft;
}

void DefaultSoundRenderTarget::Attach()
{
    VERIFY(m_reader == nullptr);
    VERIFY(m_emitter);
    ov_callbacks ovc = { ov_read_func, ov_seek_func, ov_close_func, ov_tell_func };
    m_reader = FS.r_open(m_emitter->RenderSource()->FileName());
    R_ASSERT3(m_reader && m_reader->length(), "Can't open wave file:", m_emitter->RenderSource()->FileName());
    ov_open_callbacks(m_reader, m_oggFile, NULL, 0, ovc);
    VERIFY(m_reader != nullptr);
}

void DefaultSoundRenderTarget::Detach()
{
    if (m_reader)
    {
        ov_clear(m_oggFile);
        FS.r_close(m_reader);
    }
}

bool DefaultSoundRenderTarget::Initialize()
{
    // initialize buffer
    alGenBuffers(sdef_target_count, m_buffers);
    alGenSources(1, &m_source);
    ALenum error = alGetError();

    if (AL_NO_ERROR == error)
    {
        alSourcei(m_source, AL_LOOPING, AL_FALSE);
        alSourcef(m_source, AL_MIN_GAIN, 0.f);
        alSourcef(m_source, AL_MAX_GAIN, 1.f);
        alSourcef(m_source, AL_GAIN, m_cacheGain);
        alSourcef(m_source, AL_PITCH, m_cachePitch);
        return true;
    }
    else
    {
        Msg("Can't create OpenAL source. Error: %s.", (LPCSTR)alGetString(error));
        return false;
    }
}

void DefaultSoundRenderTarget::Destroy()
{
    // clean up target
    if (alIsSource(m_source))
    {
        alDeleteSources(1, &m_source);
    }
    alDeleteBuffers(sdef_target_count, m_buffers);
}

void DefaultSoundRenderTarget::Restart()
{
    Destroy();
    Initialize();
}

void DefaultSoundRenderTarget::Start(CSoundRender_Emitter* emitter)
{
    R_ASSERT(emitter);

    // *** Initial buffer startup ***
    // 1. Fill parameters
    // 4. Load 2 blocks of data (as much as possible)
    // 5. Deferred-play-signal (emitter-exist, rendering-false)
    m_emitter = emitter;
    m_isRendering = false;

    // Calc storage
    m_bufferBlock = sdef_target_block * emitter->RenderSource()->Format().nAvgBytesPerSec / 1000;
    g_target_temp_data.resize(m_bufferBlock);
}

void DefaultSoundRenderTarget::Render()
{
    for (u32 index = 0; index < sdef_target_count; index++)
    {
        FillBlock(m_buffers[index]);
    }
    alSourceQueueBuffers(m_source, sdef_target_count, m_buffers);
    alSourcePlay(m_source);

    m_isRendering = true;
}

void DefaultSoundRenderTarget::Rewind()
{
    R_ASSERT(m_isRendering);

    alSourceStop(m_source);
    alSourcei(m_source, AL_BUFFER, NULL);

    for (u32 index = 0; index < sdef_target_count; index++)
    {
        FillBlock(m_buffers[index]);
    }
    alSourceQueueBuffers(m_source, sdef_target_count, m_buffers);
    alSourcePlay(m_source);
}

void DefaultSoundRenderTarget::Stop()
{
    if (m_isRendering)
    {
        alSourceStop(m_source);
        alSourcei(m_source, AL_BUFFER, NULL);
        alSourcei(m_source, AL_SOURCE_RELATIVE, TRUE);
    }
    Detach();

    m_emitter = nullptr;
    m_isRendering = false;
}

void DefaultSoundRenderTarget::Update()
{
    R_ASSERT(m_emitter != nullptr);

    ALint buffers = GetBuffersCount();
    ALint state = GetState();

    if (alGetError() != AL_NO_ERROR)
    {
        Msg("Error checking source state: [%s]", __FUNCTION__);
        return;
    }

    if (m_hasAlSoft)
    {
        ProcessBuffers(buffers);

        /* Make sure the source hasn't underrun */
        /* If no buffers are queued, playback is finished */
        if (state != AL_PLAYING && state != AL_PAUSED && GetQueuedBuffersCount())
        {
            alSourcePlay(m_source);

            if (alGetError() != AL_NO_ERROR)
            {
                Msg("Error restarting playback: [%s]", __FUNCTION__);
                return;
            }
        }
    }
    else
    {
        ProcessBuffers(buffers);

        if (state != AL_PLAYING)
        {
            alSourcePlay(m_source);
        }
    }
}

void DefaultSoundRenderTarget::Fill()
{
    VERIFY(m_emitter);
    VERIFY(m_emitter->source()->file_name());

    // 3D params
    alSourcef(m_source, AL_REFERENCE_DISTANCE, m_emitter->get_params()->min_distance);
    alSourcef(m_source, AL_MAX_DISTANCE, m_emitter->get_params()->max_distance);
    alSource3f(m_source, AL_POSITION, m_emitter->get_params()->position.x, m_emitter->get_params()->position.y, -m_emitter->get_params()->position.z);
    alSourcei(m_source, AL_SOURCE_RELATIVE, m_emitter->Is2D());
    alSourcef(m_source, AL_ROLLOFF_FACTOR, psSoundRolloff);

    float _gain = m_emitter->SmoothVolume();
    clamp(_gain, EPS_S, 1.f);

    if (!fsimilar(_gain, m_cacheGain, 0.01f))
    {
        m_cacheGain = _gain;
        alSourcef(m_source, AL_GAIN, _gain);
    }

    // Correct sound "speed" by time factor
    float _pitch = m_emitter->get_params()->freq * psSoundTimeFactor;

    // Increase sound frequancy (speed) limit
    clamp(_pitch, EPS_L, 100.f); 

    if (!fsimilar(_pitch, m_cachePitch))
    {
        m_cachePitch = _pitch;
        alSourcef(m_source, AL_PITCH, _pitch);
    }
}

void DefaultSoundRenderTarget::OpenALAuxInit(ALuint slot)
{
    alSource3i(m_source, AL_AUXILIARY_SEND_FILTER, slot, 0, AL_FILTER_NULL);
}

void DefaultSoundRenderTarget::OnSourceChanged()
{
    Detach();
    Attach();
}

void DefaultSoundRenderTarget::FillBlock(ALuint bufferId)
{
    R_ASSERT(m_emitter);
    m_emitter->fill_block(&g_target_temp_data.front(), m_bufferBlock);

    ALuint format = (m_emitter->RenderSource()->Format().nChannels == 1)
        ? AL_FORMAT_MONO16 
        : AL_FORMAT_STEREO16;

    alBufferData(bufferId, format, &g_target_temp_data.front(), m_bufferBlock, m_emitter->RenderSource()->Format().nSamplesPerSec);
}

void DefaultSoundRenderTarget::ProcessBuffers(ALint buffersCount)
{
    while (buffersCount > 0)
    {
        ALuint bufferId;

        alSourceUnqueueBuffers(m_source, 1, &bufferId);
        FillBlock(bufferId);
        alSourceQueueBuffers(m_source, 1, &bufferId);

        buffersCount--;

        if (alGetError() != AL_NO_ERROR)
        {
            Msg("Error buffering data: [%s]", __FUNCTION__);
            return;
        }
    }
}

ALint DefaultSoundRenderTarget::GetBuffersCount()
{
    ALint buffersCount;
    alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &buffersCount);
    
    return buffersCount;
}

ALint DefaultSoundRenderTarget::GetQueuedBuffersCount()
{
    ALint buffersCount;
    alGetSourcei(m_source, AL_BUFFERS_QUEUED, &buffersCount);

    return buffersCount;
}

ALint DefaultSoundRenderTarget::GetState()
{
    ALint state;
    alGetSourcei(m_source, AL_SOURCE_STATE, &state);
    
    return state;
}
