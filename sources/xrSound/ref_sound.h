#pragma once
#include "ref_sound_data.h"
#include "SoundParams.h"

__interface ISoundRenderSource;
__interface ISoundEmitter;

struct ref_sound
{
    ref_sound_data_ptr _p;

public:
    ref_sound() {}
    ~ref_sound() {}

    ISoundRenderSource* _handle() const { return _p ? _p->handle : NULL; }
    ISoundEmitter* _feedback() { return _p ? _p->feedback : 0; }

    CObject* _g_object();
    int _g_type();
    esound_type _sound_type();
    CSound_UserDataPtr _g_userdata();

    void create(LPCSTR name, esound_type sound_type, int game_type);
    void attach_tail(LPCSTR name);
    void clone(const ref_sound& from, esound_type sound_type, int game_type);
    void destroy();
    void play(CObject* O, u32 flags = 0, float delay = 0.f);
    void play_at_pos(CObject* O, const Fvector& pos, u32 flags = 0, float delay = 0.f);
    void play_no_feedback(CObject* O, u32 flags = 0, float delay = 0.f, Fvector* pos = 0, float* vol = 0, float* freq = 0, Fvector2* range = 0);

    void stop();
    void stop_deffered();
    void set_position(const Fvector& pos);
    void set_frequency(float freq);
    void set_range(float min, float max);
    void set_volume(float vol);
    void set_priority(float vol);
    void set_time(float t);

    SoundParams* get_params();
    void set_params(SoundParams* p);
    float get_length_sec() const { return _p ? _p->get_length_sec() : 0.0f; };
};