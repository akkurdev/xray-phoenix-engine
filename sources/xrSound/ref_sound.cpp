#include "stdafx.h"
#include "ISoundRenderSource.h"
#include "ISoundEmitter.h"
#include "ref_sound.h"

CObject* ref_sound::_g_object()
{
    VERIFY(_p);
    return _p->g_object;
}

int ref_sound::_g_type()
{
    VERIFY(_p);
    return _p->g_type;
}

SoundType ref_sound::_sound_type()
{
    VERIFY(_p);
    return _p->s_type;
}

SoundUserDataPtr ref_sound::_g_userdata()
{
    VERIFY(_p);
    return _p->g_userdata;
}

void ref_sound::create(LPCSTR name, SoundType sound_type, int game_type)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->create(*this, name, sound_type, game_type);
}
void ref_sound::attach_tail(LPCSTR name)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->attach_tail(*this, name);
}

void ref_sound::clone(const ref_sound& from, SoundType sound_type, int game_type)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->clone(*this, from, sound_type, game_type);
}

void ref_sound::destroy()
{
    VERIFY(!::Sound->i_locked());
    ::Sound->destroy(*this);
}

void ref_sound::play(CObject* O, u32 flags, float d)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->play(*this, O, flags, d);
}

void ref_sound::play_at_pos(CObject* O, const Fvector& pos, u32 flags, float d)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->play_at_pos(*this, O, pos, flags, d);
}

void ref_sound::play_no_feedback(CObject* O, u32 flags, float d, Fvector* pos, float* vol, float* freq, Fvector2* range)
{
    VERIFY(!::Sound->i_locked());
    ::Sound->play_no_feedback(*this, O, flags, d, pos, vol, freq, range);
}

void ref_sound::set_position(const Fvector& pos)
{
    VERIFY(!::Sound->i_locked());
    VERIFY(_feedback());
    _feedback()->SetPosition(pos);
}

void ref_sound::set_frequency(float freq)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->SetFrequency(freq);
}

void ref_sound::set_range(float min, float max)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->SetRange(min, max);
}

void ref_sound::set_volume(float vol)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->SetVolume(vol);
}

void ref_sound::set_priority(float p)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->SetPriority(p);
}

void ref_sound::set_time(float t)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->SetTime(t);
}

void ref_sound::stop()
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->Stop(false);
}

void ref_sound::stop_deffered()
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        _feedback()->Stop(true);
}

SoundParams* ref_sound::get_params()
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
        return _feedback()->Params();
    else
        return NULL;
}

void ref_sound::set_params(SoundParams* p)
{
    VERIFY(!::Sound->i_locked());
    if (_feedback())
    {
        _feedback()->SetPosition(p->Position);
        _feedback()->SetFrequency(p->Frequency);
        _feedback()->SetRange(p->DistanceMin, p->DistanceMax);
        _feedback()->SetVolume(p->Volume);
    }
}