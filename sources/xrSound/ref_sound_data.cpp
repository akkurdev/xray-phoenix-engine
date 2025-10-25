#include "stdafx.h"
#include "ISoundEmitter.h"
#include <SoundRenderSource.h>
#include "ref_sound_data.h"

ref_sound_data::ref_sound_data()
{
    handle = 0;
    feedback = 0;
    g_type = 0;
    g_object = 0;
    s_type = st_Effect;
    dwBytesTotal = 0;
    fTimeTotal = 0.0f;
}

ref_sound_data::ref_sound_data(const char* name, esound_type soundType, int gameType)
{
    string256 id;
    strcpy_s(id, name);
    strlwr(id);

    if (strext(id))
    {
        *strext(id) = 0;
    }

    ISoundRenderSource* source = xr_new<SoundRenderSource>();
    source->Load(id);   

    handle = source;
    s_type = soundType;
    feedback = nullptr;
    g_object = nullptr;
    g_userdata = nullptr;
    dwBytesTotal = handle->BytesCount();
    fTimeTotal = handle->Length();

    g_type = (gameType == sg_SourceType)
        ? handle->Type()
        : gameType;
}

ref_sound_data::~ref_sound_data()
{
    if (feedback)
    {
        feedback->Stop(false);
    }
    feedback = nullptr;

    //R_ASSERT(nullptr == feedback);
    handle = nullptr;
}

float ref_sound_data::get_length_sec() const
{
    return fTimeTotal;
}
