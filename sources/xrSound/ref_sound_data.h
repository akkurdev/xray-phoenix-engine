#pragma once
#include "dllexp.h"
#include "SoundType.h"
#include "sound_user_data.h"

class CObject;
__interface ISoundRenderSource;
__interface ISoundEmitter;

class ref_sound_data : public xr_resource
{
public:
    //	shared_str						nm;
    ISoundRenderSource* handle; //!< Pointer to wave-source interface
    ISoundEmitter* feedback; //!< Pointer to emitter, automaticaly clears on emitter-stop
    esound_type s_type;
    int g_type; //!< Sound type, usually for AI
    CObject* g_object; //!< Game object that emitts ref_sound
    CSound_UserDataPtr g_userdata;
    shared_str fn_attached[2];

    u32 dwBytesTotal;
    float fTimeTotal;

public:
    ref_sound_data();
    ref_sound_data(LPCSTR fName, esound_type sound_type, int game_type);
    virtual ~ref_sound_data();
    float get_length_sec() const;
};

typedef resptr_core<ref_sound_data, resptr_base<ref_sound_data>> ref_sound_data_ptr;

