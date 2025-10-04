#include "stdafx.h"
#include "SoundEnvironmentLibrary.h"

SoundEnvironmentLibrary::SoundEnvironmentLibrary()
{
    m_library = EnvironmentLibrary{};
}

void SoundEnvironmentLibrary::Load(const char* name)
{
    R_ASSERT(m_library.empty());

    IReader* reader = FS.r_open(name);
    IReader* envReader;

    m_library.reserve(256);

    for (uint32_t chunk = 0; 0 != (envReader = reader->open_chunk(chunk)); chunk++)
    {
        SoundEnvironment* env = xr_new<SoundEnvironment>();
        if (env->Load(envReader))
        {
            m_library.push_back(env);
        }
        envReader->close();
    }
    FS.r_close(reader);
}

bool SoundEnvironmentLibrary::Save(const char* name)
{
    IWriter* writer = FS.w_open(name);

    if (!writer)
    {
        return false;
    }    

    for (uint32_t chunk = 0; chunk < m_library.size(); chunk++)
    {
        writer->open_chunk(chunk);
        m_library[chunk]->Save(writer);
        writer->close_chunk();
    }

    FS.w_close(writer);
    return true;
}

void SoundEnvironmentLibrary::Unload()
{
    for (uint32_t chunk = 0; chunk < m_library.size(); chunk++)
    {
        xr_delete(m_library[chunk]);
    }
    m_library.clear();
}

int32_t SoundEnvironmentLibrary::GetEnvironmentId(const char* name)
{    
    return FindByName(name);
}

EnvironmentLibrary SoundEnvironmentLibrary::Library()
{
    return m_library;
}

SoundEnvironment* SoundEnvironmentLibrary::GetById(int32_t id)
{
    return (!m_library.empty() && id < m_library.size()) 
        ? m_library[id] 
        : nullptr;
}

SoundEnvironment* SoundEnvironmentLibrary::GetByName(const char* name)
{
    auto envIndex = FindByName(name);

    return envIndex != -1 
        ? m_library[envIndex] 
        : nullptr;
}

SoundEnvironment* SoundEnvironmentLibrary::Append(SoundEnvironment* parentEnv)
{
    auto* env = parentEnv
        ? xr_new<SoundEnvironment>(*parentEnv)
        : xr_new<SoundEnvironment>();
    
    m_library.push_back(env);
    return m_library.back();
}

void SoundEnvironmentLibrary::Remove(int32_t id)
{
    xr_delete(m_library[id]);
    m_library.erase(m_library.begin() + id);
}

void SoundEnvironmentLibrary::Remove(const char* name)
{    
    for (auto it = m_library.begin(); it != m_library.end(); it++)
    {
        if (0 == stricmp(name, *(*it)->Name()))
        {
            xr_delete(*it);
            m_library.erase(it);
            break;
        }
    }
}

int32_t SoundEnvironmentLibrary::FindByName(const char* name)
{
    auto index = std::find_if(m_library.begin(), m_library.end(), [=](SoundEnvironment* env)
        {
            return env->Name() == name;
        });
    
    return index != m_library.end() 
        ? (index - m_library.begin()) 
        : -1;
}
