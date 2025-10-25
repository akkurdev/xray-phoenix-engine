#include "stdafx.h"
#include "soundrender_core.h"
#include "SoundRenderSource.h"

int ov_seek_func(void* dataSource, s64 offset, int whence)
{
    switch (whence)
    {
        case SEEK_SET: ((IReader*)dataSource)->seek((int)offset); break;
        case SEEK_CUR: ((IReader*)dataSource)->advance((int)offset); break;
        case SEEK_END: ((IReader*)dataSource)->seek((int)offset + ((IReader*)dataSource)->length()); break;
    }
    return 0;
}

size_t ov_read_func(void* ptr, size_t size, size_t nmemb, void* dataSource)
{
    IReader* reader = (IReader*)dataSource;
    size_t existBlock = _max(0ul, iFloor(reader->elapsed() / (float)size));
    size_t readBlock = _min(existBlock, nmemb);
    reader->r(ptr, (int)(readBlock * size));

    return readBlock;
}

int ov_close_func(void* dataSource) 
{ 
    return 0; 
}

long ov_tell_func(void* dataSource) 
{ 
    return ((IReader*)dataSource)->tell(); 
}

SoundRenderSource::SoundRenderSource() :
    m_fileName(nullptr),
    m_volume(1.f),
    m_minDistance(1.f),
    m_maxDistance(1.f),
    m_maxAiDistance(1.f),
    m_gameType(0),
    m_length(0.f),
    m_bytesCount(0)    
{
    m_cache = xr_new<CacheTable>();
    m_cache->Size = 0;
    m_cache->Lines = nullptr;

    m_format = WAVEFORMATEX{};
}

SoundRenderSource::~SoundRenderSource()
{
    Unload();
    xr_delete(m_cache);
}

void SoundRenderSource::Load(const char* fileName)
{
    PrepareFile(fileName);

    // Load file into memory and parse WAV-format
    OggVorbis_File oggFile;
    ov_callbacks oggCallbacks = 
    { 
        ov_read_func, 
        ov_seek_func, 
        ov_close_func, 
        ov_tell_func 
    };
    IReader* reader = FS.r_open(m_fileName.c_str());

    R_ASSERT3(reader && reader->length(), "Can't open wave file:", m_fileName.c_str());
    ov_open_callbacks(reader, &oggFile, NULL, 0, oggCallbacks);

    ReadFormat(&oggFile);
    ReadComments(&oggFile);

    ov_clear(&oggFile);
    FS.r_close(reader);

    SoundRender->Cache().CreateTable(*m_cache, m_bytesCount);
}

void SoundRenderSource::Unload()
{
    SoundRender->Cache().DestroyTable(*m_cache);
    m_length = 0.0f;
    m_bytesCount = 0;
}

void SoundRenderSource::Decompress(uint32_t lineNumber, OggVorbis_File* oggFile)
{
    VERIFY(oggFile);

    // decompression of one cache-line
    uint32_t lineSize = SoundRender->Cache().LineSize();
    char* cachePosition = (char*)SoundRender->Cache().GetDataById(*m_cache, lineNumber);
    uint32_t bufferOffset = (lineNumber * lineSize) / 2 / m_format.nChannels;

    uint32_t leftPosition = m_bytesCount - bufferOffset;
    uint32_t cursorPosition = (uint32_t)_min(leftPosition, lineSize);

    // seek
    uint32_t seekPos = uint32_t(ov_pcm_tell(oggFile));
    if (seekPos != bufferOffset)
    {
        ov_pcm_seek(oggFile, bufferOffset);
    }

    // decompress
    int32_t currentSection = 0;
    long position = 0;
    long result = 0;

    // Read loop
    while (position < (long)cursorPosition)
    {
        result = ov_read(oggFile, cachePosition + position, cursorPosition - position, 0, 2, 1, &currentSection);

        // if end of file or read limit exceeded
        if (result == 0)
        {
            break;
        }        
        position += result;
    }
}

const char* SoundRenderSource::FileName() const
{
    return m_fileName.c_str();
}

float SoundRenderSource::Length() const
{
    return m_length;
}

uint32_t SoundRenderSource::Type() const
{
    return m_gameType;
}

float SoundRenderSource::BaseVolume() const
{
    return m_volume;
}

float SoundRenderSource::MinDistance() const
{
    return m_minDistance;
}

float SoundRenderSource::MaxDistance() const
{
    return m_maxDistance;
}

float SoundRenderSource::MaxAiDistance() const
{
    return m_maxAiDistance;
}

uint32_t SoundRenderSource::BytesCount() const
{
    return m_bytesCount;
}

WAVEFORMATEX SoundRenderSource::Format() const
{
    return m_format;
}

CacheTable* SoundRenderSource::Cache() const
{
    return m_cache;
}

void SoundRenderSource::PrepareFile(const char* fileName)
{
    string_path filePath, temp;
    xr_strcpy(temp, fileName);

    strlwr(temp);
    if (strext(temp))
    {
        *strext(temp) = 0;
    }

    strconcat(sizeof(filePath), filePath, temp, ".ogg");
    if (!FS.exist("$level$", filePath))
        FS.update_path(filePath, "$game_sounds$", filePath);

    ASSERT_FMT_DBG(FS.exist(filePath), "Can't find sound [%s.ogg]", temp);
    if (!FS.exist(filePath))
    {
        FS.update_path(filePath, "$game_sounds$", "$no_sound.ogg");
    }

    m_fileName = filePath;
}

void SoundRenderSource::ReadFormat(OggVorbis_File* oggFile)
{
    vorbis_info* oggFileInfo = ov_info(oggFile, -1);

    R_ASSERT3(oggFileInfo, "Invalid ogg file info:", m_fileName.c_str());
    R_ASSERT3(oggFileInfo->rate == 44100, "Invalid ogg file rate:", m_fileName.c_str());
    ZeroMemory(&m_format, sizeof(WAVEFORMATEX));

    m_format.nSamplesPerSec = (oggFileInfo->rate);
    m_format.wFormatTag = WAVE_FORMAT_PCM;
    m_format.nChannels = u16(oggFileInfo->channels);
    m_format.wBitsPerSample = 16;
    m_format.nBlockAlign = m_format.wBitsPerSample / 8 * m_format.nChannels;
    m_format.nAvgBytesPerSec = m_format.nSamplesPerSec * m_format.nBlockAlign;

    s64 pcm = ov_pcm_total(oggFile, -1);
    m_bytesCount = u32(pcm * m_format.nBlockAlign);
    m_length = s_f_def_source_footer + m_bytesCount / float(m_format.nAvgBytesPerSec);
}

void SoundRenderSource::ReadComments(OggVorbis_File* oggFile)
{
    vorbis_comment* oggComment = ov_comment(oggFile, -1);

    if (!oggComment->comments)
    {
        Msg("Missing ogg-comments at file: [%s]", m_fileName.c_str());
        return;
    }

    IReader reader(oggComment->user_comments[0], oggComment->comment_lengths[0]);
    uint32_t commentVersion = 0;

    if (reader.elapsed() <= static_cast<int>(sizeof(uint32_t)))
    {
        Msg("Invalid ogg-comment section at file: [%s]", m_fileName.c_str());
        return;
    }
    commentVersion = reader.r_u32();

    if (commentVersion > OGG_COMMENT_VERSION)
    {
        Msg("! Invalid ogg-comment version, file: [%s]", m_fileName.c_str());
    }

    m_minDistance = reader.r_float();
    m_maxDistance = reader.r_float();
    m_volume = commentVersion == 0x0001 ? 1.f : reader.r_float();
    m_gameType = reader.r_u32();
    m_maxAiDistance = commentVersion == OGG_COMMENT_VERSION ? reader.r_float() : m_maxDistance;

    R_ASSERT3((m_maxAiDistance >= 0.1f) || (m_maxDistance >= 0.1f), "Invalid maximum distance", m_fileName.c_str());
}
