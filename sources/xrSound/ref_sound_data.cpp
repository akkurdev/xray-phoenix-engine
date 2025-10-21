#include "stdafx.h"
#include "ISoundRenderSource.h"
#include "ISoundEmitter.h"
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

ref_sound_data::ref_sound_data(LPCSTR fName, esound_type sound_type, int game_type)
{
    ::Sound->_create_data(*this, fName, sound_type, game_type);
}

ref_sound_data::~ref_sound_data()
{
    ::Sound->_destroy_data(*this);
}

float ref_sound_data::get_length_sec() const
{
    return fTimeTotal;
}
