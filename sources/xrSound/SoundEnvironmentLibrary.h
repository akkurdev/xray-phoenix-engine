#pragma once
#include "SoundEnvironment.h"

using EnvironmentLibrary = std::vector<SoundEnvironment*>;

class SoundEnvironmentLibrary final
{
public:
    SoundEnvironmentLibrary();
    ~SoundEnvironmentLibrary() = default;

    void Load(const char* name);
    bool Save(const char* name);
    void Unload();

    int32_t GetEnvironmentId(const char* name);
    EnvironmentLibrary Library();

    SoundEnvironment* GetById(int32_t id);
    SoundEnvironment* GetByName(const char* name);

    SoundEnvironment* Append(SoundEnvironment* parentEnv = nullptr);
    void Remove(int32_t id);
    void Remove(const char* name);

private:
    int32_t FindByName(const char* name);

private:
    EnvironmentLibrary m_library;
};
