#include "stdafx.h"
#include "../xr_3da/xrLevel.h"
#include "soundrender_core.h"
#include "SoundRenderSource.h"
#include "SoundEmitter.h"

#pragma warning(push)
#pragma warning(disable : 4995)
#include <eax.h>
#pragma warning(pop)

#include <efx.h>
#include <SoundEnvironment.h>
#include <SoundEnvironmentLibrary.h>
#include <xrCDB/cl_intersect.h>

int psSoundTargets = 256; // 512; 
Flags32 psSoundFlags = {ss_Hardware};
float psSoundOcclusionScale = 0.5f;
float psSoundLinearFadeFactor = 0.4f;
float psSoundCull = 0.01f;
float psSoundRolloff = 0.75f;
u32 psSoundModel = 0;
float psSoundVEffects = 1.0f;
float psSoundVFactor = 1.0f;
float psSoundTimeFactor = 1.0f;
float psSoundVMusic = 1.f;
int psSoundCacheSizeMB = 32;

CSoundRender_Core* SoundRender = 0;
CSound_manager_interface* Sound = 0;


#define LOAD_PROC(x, type) ((x) = (type)alGetProcAddress(#x))
static LPALEFFECTF alEffectf;
static LPALEFFECTI alEffecti;
static LPALDELETEEFFECTS alDeleteEffects;
static LPALISEFFECT alIsEffect;
static LPALGENEFFECTS alGenEffects;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;

CSoundRender_Core::CSoundRender_Core()
{
    bPresent = FALSE;
    m_hasEax = false;
    m_hasDeferredEax = false;
    m_cacheLineSize = 0;
    geom_MODEL = NULL;
    geom_ENV = NULL;
    geom_SOM = NULL;
    m_environments = nullptr;
    Handler = NULL;
    m_emitterMarker = 0;
    m_currentEnvironment.SetIdentity();
    m_targetEnvironment.SetIdentity();
    m_isListenerMoved = false;
    m_isReady = false;
    m_isLocked = false;
    m_time = m_timer.GetElapsed_sec();
    m_deltaTime = 0.0f;
    m_reverberationProps = EFX_REVERB_PRESET_GENERIC;
    m_hasEfx = false;
    m_effect = 0;
    m_slot = 0;
}

CSoundRender_Core::~CSoundRender_Core()
{
    if (m_hasEfx)
    {
        if (m_effect)
            alDeleteEffects(1, &m_effect);

        if (m_slot)
            alDeleteAuxiliaryEffectSlots(1, &m_slot);
    }

    xr_delete(geom_ENV);
    xr_delete(geom_SOM);
}

float CSoundRender_Core::Time() const
{
    return m_time;
}

float CSoundRender_Core::ElapsedTime() const
{
    return m_timer.GetElapsed_sec();
}

float CSoundRender_Core::DeltaTime() const
{
    return m_deltaTime;
}

void CSoundRender_Core::SetTime(float time)
{
    m_time = time;
}

SoundRenderCache CSoundRender_Core::Cache() const
{
    return m_cache;
}

void CSoundRender_Core::_initialize(int stage)
{
    m_timer.Start();

    // load environment
    env_load();

    bPresent = TRUE;

    // Cache
    m_cacheLineSize = (sdef_target_block / 8) * 276400 / 1000;
    m_cache.Initialize(psSoundCacheSizeMB * 1024, m_cacheLineSize);

    m_isReady = true;
}

extern xr_vector<u8> g_target_temp_data;
void CSoundRender_Core::_clear()
{
    m_isReady = false;
    m_cache.Destroy();
    env_unload();

    // remove sources
    for (u32 sit = 0; sit < m_renderSources.size(); sit++)
        xr_delete(m_renderSources[sit]);
    m_renderSources.clear();

    // remove emmiters
    for (u32 eit = 0; eit < m_emitters.size(); eit++)
        xr_delete(m_emitters[eit]);
    m_emitters.clear();

    g_target_temp_data.clear();
}

void CSoundRender_Core::stop_emitters()
{
    for (u32 eit = 0; eit < m_emitters.size(); eit++)
        m_emitters[eit]->Stop(false);
}

int CSoundRender_Core::pause_emitters(bool val)
{
    int32_t pauseCounter = val ? +1 : -1;
    VERIFY(m_iPauseCounter >= 0);

    for (u32 it = 0; it < m_emitters.size(); it++)
        m_emitters[it]->Pause(val, val ? pauseCounter : pauseCounter + 1);

    return pauseCounter;
}

void CSoundRender_Core::env_load()
{
    string_path fn;

    if (FS.exist(fn, "$game_data$", SNDENV_FILENAME))
    {
        Msg("Loading of [%s]", SNDENV_FILENAME);

        m_environments = xr_new<SoundEnvironmentLibrary>();
        m_environments->Load(fn);

        for (u32 chunk = 0; chunk < m_environments->Library().size(); chunk++)
        {
            shared_str name = m_environments->Library()[chunk]->Name();

            Msg("~ env id=[%d] name=[%s]", chunk, name.c_str());
        }
    }
}

void CSoundRender_Core::env_unload()
{
    if (m_environments)
        m_environments->Unload();
    xr_delete(m_environments);
}

void CSoundRender_Core::_restart()
{
    m_cache.Destroy();
    m_cache.Initialize(psSoundCacheSizeMB * 1024, m_cacheLineSize);
    env_apply();
}

void CSoundRender_Core::set_handler(sound_event* E) { Handler = E; }

void CSoundRender_Core::set_geometry_occ(CDB::MODEL* M) { geom_MODEL = M; }

void CSoundRender_Core::set_geometry_som(IReader* I)
{
    xr_delete(geom_SOM);
    if (0 == I)
        return;

    // check version
    R_ASSERT(I->find_chunk(0));

#ifdef DEBUG
    u32 version =
#endif
        I->r_u32();
    VERIFY2(version == 0, "Invalid SOM version");
    // load geometry
    IReader* geom = I->open_chunk(1);
    VERIFY2(geom, "Corrupted SOM file");
    // Load tris and merge them
    struct SOM_poly
    {
        Fvector3 v1;
        Fvector3 v2;
        Fvector3 v3;
        u32 b2sided;
        float occ;
    };
    // Create AABB-tree
    CDB::Collector CL;
    while (!geom->eof())
    {
        SOM_poly P;
        geom->r(&P, sizeof(P));
        CL.add_face_packed_D(P.v1, P.v2, P.v3, *(size_t*)&P.occ, 0.01f);
        if (P.b2sided)
            CL.add_face_packed_D(P.v3, P.v2, P.v1, *(size_t*)&P.occ, 0.01f);
    }
    geom_SOM = xr_new<CDB::MODEL>();
    geom_SOM->build(CL.getV(), int(CL.getVS()), CL.getT(), int(CL.getTS()), nullptr, nullptr, false);

    geom->close();
}

void CSoundRender_Core::set_geometry_env(IReader* I)
{
    xr_delete(geom_ENV);
    m_environmentIds.clear();

    if (0 == I)
        return;
    if (0 == m_environments)
        return;

    // Assosiate names
    IReader* names = I->open_chunk(0);
    while (!names->eof())
    {
        string256 n;
        names->r_stringZ(n, sizeof(n));
        int id = m_environments->GetEnvironmentId(n);
        R_ASSERT(id >= 0);
        m_environmentIds.push_back(u16(id));
        Msg("~ set_geometry_env id=%d name[%s]=environment id[%d]", m_environmentIds.size() - 1, n, id);
    }
    names->close();

    // Load geometry
    IReader* geom_ch = I->open_chunk(1);

    u8* _data = (u8*)xr_malloc(geom_ch->length());

    Memory.mem_copy(_data, geom_ch->pointer(), geom_ch->length());

    IReader* geom = xr_new<IReader>(_data, geom_ch->length(), 0);

    hdrCFORM H;
    geom->r(&H, sizeof(hdrCFORM));
    R_ASSERT(H.version == CFORM_CURRENT_VERSION);
    Fvector* verts = (Fvector*)geom->pointer();
    CDB::TRI* tris = (CDB::TRI*)(verts + H.vertcount);

    geom_ENV = xr_new<CDB::MODEL>();
    geom_ENV->build(verts, H.vertcount, tris, H.facecount);
    geom_ch->close();
    geom->close();
    xr_free(_data);
}

void CSoundRender_Core::create(ref_sound& S, const char* fName, esound_type sound_type, int game_type)
{
    if (!bPresent)
        return;
    S._p = xr_new<ref_sound_data>(fName, sound_type, game_type);
}

void CSoundRender_Core::attach_tail(ref_sound& S, const char* fName)
{
    if (!bPresent)
        return;
    string_path fn;
    xr_strcpy(fn, fName);
    if (strext(fn))
        *strext(fn) = 0;
    if (S._p->fn_attached[0].size() && S._p->fn_attached[1].size())
    {
        return;
    }

    u32 idx = S._p->fn_attached[0].size() ? 1 : 0;

    S._p->fn_attached[idx] = fn;

    ISoundRenderSource* s = SoundRender->i_create_source(fn);
    S._p->dwBytesTotal += s->BytesCount();
    S._p->fTimeTotal += s->Length();
    if (S._feedback())
    {
        auto emitter = S._feedback();
        emitter->SetStopTime(emitter->StopTime() + s->Length());
    }

    SoundRender->i_destroy_source(s);
}

void CSoundRender_Core::clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int game_type)
{
    if (!bPresent)
        return;
    S._p = xr_new<ref_sound_data>();
    S._p->handle = from._p->handle;
    S._p->dwBytesTotal = from._p->dwBytesTotal;
    S._p->fTimeTotal = from._p->fTimeTotal;
    S._p->fn_attached[0] = from._p->fn_attached[0];
    S._p->fn_attached[1] = from._p->fn_attached[1];
    S._p->g_type = (game_type == sg_SourceType) ? S._p->handle->Type() : game_type;
    S._p->s_type = sound_type;
}

void CSoundRender_Core::play(ref_sound& S, CObject* O, u32 flags, float delay)
{
    if (!bPresent || 0 == S._handle())
        return;
    S._p->g_object = O;
    if (S._feedback())
        S._feedback()->Rewind();
    else
        i_play(&S, flags & sm_Looped, delay);

    if (flags & sm_2D || S._handle()->Format().nChannels == 2)
        S._feedback()->SwitchTo2D();
}

void CSoundRender_Core::play_no_feedback(ref_sound& S, CObject* O, u32 flags, float delay, Fvector* pos, float* vol, float* freq, Fvector2* range)
{
    if (!bPresent || 0 == S._handle())
        return;
    ref_sound_data_ptr orig = S._p;
    S._p = xr_new<ref_sound_data>();
    S._p->handle = orig->handle;
    S._p->g_type = orig->g_type;
    S._p->g_object = O;
    S._p->dwBytesTotal = orig->dwBytesTotal;
    S._p->fTimeTotal = orig->fTimeTotal;
    S._p->fn_attached[0] = orig->fn_attached[0];
    S._p->fn_attached[1] = orig->fn_attached[1];

    i_play(&S, flags & sm_Looped, delay);

    if (flags & sm_2D || S._handle()->Format().nChannels == 2)
        S._feedback()->SwitchTo2D();

    if (pos)
        S._feedback()->SetPosition(*pos);
    if (freq)
        S._feedback()->SetFrequency(*freq);
    if (range)
        S._feedback()->SetRange((*range)[0], (*range)[1]);
    if (vol)
        S._feedback()->SetVolume(*vol);
    S._p = orig;
}

void CSoundRender_Core::play_at_pos(ref_sound& S, CObject* O, const Fvector& pos, u32 flags, float delay)
{
    if (!bPresent || 0 == S._handle())
        return;
    S._p->g_object = O;
    if (S._feedback())
        S._feedback()->Rewind();
    else
        i_play(&S, flags & sm_Looped, delay);

    S._feedback()->SetPosition(pos);

    if (flags & sm_2D || S._handle()->Format().nChannels == 2)
        S._feedback()->SwitchTo2D();
}
void CSoundRender_Core::destroy(ref_sound& S)
{
    if (S._feedback())
    {
        ISoundEmitter* E = S._feedback();
        E->Stop(false);
    }
    S._p = 0;
}

void CSoundRender_Core::_create_data(ref_sound_data& S, LPCSTR fName, esound_type sound_type, int game_type)
{
    string_path fn;
    xr_strcpy(fn, fName);
    if (strext(fn))
        *strext(fn) = 0;
    S.handle = SoundRender->i_create_source(fn);
    S.g_type = (game_type == sg_SourceType) ? S.handle->Type() : game_type;
    S.s_type = sound_type;
    S.feedback = 0;
    S.g_object = 0;
    S.g_userdata = 0;
    S.dwBytesTotal = S.handle->BytesCount();
    S.fTimeTotal = S.handle->Length();
}
void CSoundRender_Core::_destroy_data(ref_sound_data& S)
{
    if (S.feedback)
    {
        ISoundEmitter* E = S.feedback;
        E->Stop(false);
    }
    R_ASSERT(0 == S.feedback);
    SoundRender->i_destroy_source(S.handle);

    S.handle = NULL;
}

SoundEnvironment* CSoundRender_Core::get_environment(const Fvector& P)
{
    static SoundEnvironment identity;

    if (geom_ENV)
    {
        Fvector dir = {0, -1, 0};

        // хитрый способ для проверки звуковых зон в 2х направлениях от камеры. но что то он хуже работает. часто не та зона выбираеться. пока убрал
        //CDB::COLLIDER geom_DB1;
        //geom_DB1.ray_query(CDB::OPT_ONLYNEAREST, geom_ENV, P, dir, 1000.f);
        //CDB::COLLIDER geom_DB2;
        //geom_DB2.ray_query(CDB::OPT_ONLYNEAREST, geom_ENV, P, Fvector(dir).invert(), 1000.f);

        geom_DB.ray_query(CDB::OPT_ONLYNEAREST, geom_ENV, P, dir, 1000.f);

        if (geom_DB.r_count())
        {
            CDB::RESULT* r = geom_DB.r_begin();

            CDB::TRI* T = geom_ENV->get_tris() + r->id;
            Fvector* V = geom_ENV->get_verts();

            Fvector tri_norm;
            tri_norm.mknormal(V[T->verts[0]], V[T->verts[1]], V[T->verts[2]]);
            float dot = dir.dotproduct(tri_norm);

            if (dot <= 0)
            {
                u16 id_front = (u16)((((u32)T->dummy) & 0x0000ffff) >> 0); //	front face

                return m_environments->GetById(m_environmentIds[id_front]);
            }
            else
            {
                u16 id_back = (u16)((((u32)T->dummy) & 0xffff0000) >> 16); //	back face

                return m_environments->GetById(m_environmentIds[id_back]);
            }
        }
    }

    identity.SetIdentity();
    return &identity;
}

void CSoundRender_Core::env_apply()
{
    m_isListenerMoved = true;
}

void CSoundRender_Core::update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt) {}

void CSoundRender_Core::InitAlEFXAPI()
{
    LOAD_PROC(alDeleteAuxiliaryEffectSlots, LPALDELETEAUXILIARYEFFECTSLOTS);
    LOAD_PROC(alGenEffects, LPALGENEFFECTS);
    LOAD_PROC(alDeleteEffects, LPALDELETEEFFECTS);
    LOAD_PROC(alIsEffect, LPALISEFFECT);
    LOAD_PROC(alEffecti, LPALEFFECTI);
    LOAD_PROC(alAuxiliaryEffectSloti, LPALAUXILIARYEFFECTSLOTI);
    LOAD_PROC(alGenAuxiliaryEffectSlots, LPALGENAUXILIARYEFFECTSLOTS);
    LOAD_PROC(alEffectf, LPALEFFECTF);
}

bool CSoundRender_Core::EFXTestSupport()
{
    alGenEffects(1, &m_effect);

    alEffecti(m_effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    alEffectf(m_effect, AL_REVERB_DENSITY, m_reverberationProps.flDensity);
    alEffectf(m_effect, AL_REVERB_DIFFUSION, m_reverberationProps.flDiffusion);
    alEffectf(m_effect, AL_REVERB_GAIN, m_reverberationProps.flGain);
    alEffectf(m_effect, AL_REVERB_GAINHF, m_reverberationProps.flGainHF);
    alEffectf(m_effect, AL_REVERB_DECAY_TIME, m_reverberationProps.flDecayTime);
    alEffectf(m_effect, AL_REVERB_DECAY_HFRATIO, m_reverberationProps.flDecayHFRatio);
    alEffectf(m_effect, AL_REVERB_REFLECTIONS_GAIN, m_reverberationProps.flReflectionsGain);
    alEffectf(m_effect, AL_REVERB_REFLECTIONS_DELAY, m_reverberationProps.flReflectionsDelay);
    alEffectf(m_effect, AL_REVERB_LATE_REVERB_GAIN, m_reverberationProps.flLateReverbGain);
    alEffectf(m_effect, AL_REVERB_LATE_REVERB_DELAY, m_reverberationProps.flLateReverbDelay);
    alEffectf(m_effect, AL_REVERB_AIR_ABSORPTION_GAINHF, m_reverberationProps.flAirAbsorptionGainHF);
    alEffectf(m_effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, m_reverberationProps.flRoomRolloffFactor);
    alEffecti(m_effect, AL_REVERB_DECAY_HFLIMIT, m_reverberationProps.iDecayHFLimit);

    /* Check if an error occured, and clean up if so. */
    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        Msg("!![%s] OpenAL error: %s", __FUNCTION__, alGetString(err));
        if (alIsEffect(m_effect))
            alDeleteEffects(1, &m_effect);
        return false;
    }

    alGenAuxiliaryEffectSlots(1, &m_slot);
    err = alGetError();
    ASSERT_FMT_DBG(err == AL_NO_ERROR, "!![%s] OpenAL EFX error: [%s]", __FUNCTION__, alGetString(err));

    return true;
}

inline static float mB_to_gain(float mb) { return powf(10.0f, mb / 2000.0f); }

void CSoundRender_Core::i_efx_listener_set(SoundEnvironment* _E)
{
    const auto E = static_cast<SoundEnvironment*>(_E);

    // http://openal.org/pipermail/openal/2014-March/000083.html
    float density = powf(E->EnvironmentSize(), 3.0f) / 16.0f;

    if (density > 1.0f)
        density = 1.0f;

    alEffectf(m_effect, AL_REVERB_DENSITY, density);
    alEffectf(m_effect, AL_REVERB_DIFFUSION, E->EnvironmentDiffusionFactor());
    alEffectf(m_effect, AL_REVERB_GAIN, mB_to_gain(E->RoomEffectFactor()));
    alEffectf(m_effect, AL_REVERB_GAINHF, mB_to_gain(E->RoomEffectHighFactor()));
    alEffectf(m_effect, AL_REVERB_DECAY_TIME, E->DecayTimeFactor());
    alEffectf(m_effect, AL_REVERB_DECAY_HFRATIO, E->DecayHighFrequencyFactor());
    alEffectf(m_effect, AL_REVERB_REFLECTIONS_GAIN, mB_to_gain(E->ReflectionFactor()));
    alEffectf(m_effect, AL_REVERB_REFLECTIONS_DELAY, E->ReflectionDelayFactor());
    alEffectf(m_effect, AL_REVERB_LATE_REVERB_DELAY, E->ReverberationDelayFactor());
    alEffectf(m_effect, AL_REVERB_LATE_REVERB_GAIN, mB_to_gain(E->ReverberationFactor()));
    alEffectf(m_effect, AL_REVERB_AIR_ABSORPTION_GAINHF, mB_to_gain(E->AirAbsorptionFactor()));
    alEffectf(m_effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, E->RoomRollOffFactor());
}

bool CSoundRender_Core::i_efx_commit_setting()
{
    alGetError();
    /* Tell the effect slot to use the loaded effect object. Note that the this
     * effectively copies the effect properties. You can modify or delete the
     * effect object afterward without affecting the effect slot.
     */
    alAuxiliaryEffectSloti(m_slot, AL_EFFECTSLOT_EFFECT, m_effect);
    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        Msg("!![%s] OpenAL EFX error: [%s]", __FUNCTION__, alGetString(err));
        return false;
    }

    return true;
}
//////////////////////////////////////////////////

void CSoundRender_Core::i_eax_listener_set(SoundEnvironment* _E)
{
    VERIFY(bEAX);
    SoundEnvironment* E = static_cast<SoundEnvironment*>(_E);

    EAXLISTENERPROPERTIES ep;
    ep.lRoom = iFloor(E->RoomEffectFactor()); // room effect level at low frequencies
    ep.lRoomHF = iFloor(E->RoomEffectHighFactor()); // room effect high-frequency level re. low frequency level
    ep.flRoomRolloffFactor = E->RoomRollOffFactor(); // like DS3D flRolloffFactor but for room effect
    ep.flDecayTime = E->DecayTimeFactor(); // reverberation decay time at low frequencies
    ep.flDecayHFRatio = E->DecayHighFrequencyFactor(); // high-frequency to low-frequency decay time ratio
    ep.lReflections = iFloor(E->ReflectionFactor()); // early reflections level relative to room effect
    ep.flReflectionsDelay = E->ReflectionDelayFactor(); // initial reflection delay time
    ep.lReverb = iFloor(E->ReverberationFactor()); // late reverberation level relative to room effect
    ep.flReverbDelay = E->ReverberationDelayFactor(); // late reverberation delay time relative to initial reflection
    ep.dwEnvironment = EAXLISTENER_DEFAULTENVIRONMENT; // sets all listener properties
    ep.flEnvironmentSize = E->EnvironmentSize(); // environment size in meters
    ep.flEnvironmentDiffusion = E->EnvironmentDiffusionFactor(); // environment diffusion
    ep.flAirAbsorptionHF = E->AirAbsorptionFactor(); // change in level per meter at 5 kHz
    ep.dwFlags = EAXLISTENER_DEFAULTFLAGS; // modifies the behavior of properties

    u32 deferred = m_hasDeferredEax ? DSPROPERTY_EAXLISTENER_DEFERRED : 0;

    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_ROOM, &ep.lRoom, sizeof(LONG));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_ROOMHF, &ep.lRoomHF, sizeof(LONG));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR, &ep.flRoomRolloffFactor, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_DECAYTIME, &ep.flDecayTime, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_DECAYHFRATIO, &ep.flDecayHFRatio, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_REFLECTIONS, &ep.lReflections, sizeof(LONG));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY, &ep.flReflectionsDelay, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_REVERB, &ep.lReverb, sizeof(LONG));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_REVERBDELAY, &ep.flReverbDelay, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION, &ep.flEnvironmentDiffusion, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF, &ep.flAirAbsorptionHF, sizeof(float));
    i_eax_set(&DSPROPSETID_EAX_ListenerProperties, deferred | DSPROPERTY_EAXLISTENER_FLAGS, &ep.dwFlags, sizeof(DWORD));
}

void CSoundRender_Core::i_eax_commit_setting()
{
    // commit eax
    if (m_hasDeferredEax)
        i_eax_set(&DSPROPSETID_EAX_ListenerProperties, DSPROPERTY_EAXLISTENER_COMMITDEFERREDSETTINGS, NULL, 0);
}

void CSoundRender_Core::object_relcase(CObject* obj)
{
    if (obj)
    {
        for (u32 eit = 0; eit < m_emitters.size(); eit++)
        {
            if (m_emitters[eit])
                if (m_emitters[eit]->SoundData())
                    if (obj == m_emitters[eit]->SoundData()->g_object)
                        m_emitters[eit]->SoundData()->g_object = 0;
        }
    }
}

XRSOUND_API float SoundRenderGetOcculution(Fvector& P, float R, Fvector* occ) { return SoundRender->get_occlusion(P, R, occ); }

ISoundEmitter* CSoundRender_Core::i_play(ref_sound* S, BOOL _loop, float delay)
{
    VERIFY(S->_p->feedback == 0);
    ISoundEmitter* E = xr_new<SoundEmitter>();
    S->_p->feedback = E;
    E->Start(S, _loop, delay);
    m_emitters.push_back(E);
    return E;
}

// TODO: alSourcePlay вызывается в Render(), Rewind() и Update() методах SoundRenderTarget - возможно, избыточно
void CSoundRender_Core::update(const Fvector& P, const Fvector& D, const Fvector& N)
{
    u32 it;

    if (0 == m_isReady)
        return;
    m_isLocked = true;
    m_timer.time_factor(psSoundTimeFactor); //--#SM+#--
    float new_tm = m_timer.GetElapsed_sec();
    m_deltaTime = new_tm - m_time;
    float dt_sec = m_deltaTime;
    m_time = new_tm;

    m_emitterMarker++;

    // Firstly update emitters, which are now being rendered
    for (it = 0; it < m_renderTargets.size(); it++)
    {
        ISoundRenderTarget* T = m_renderTargets[it];
        ISoundEmitter* E = T->Emitter();
        if (E)
        {
            E->Update(dt_sec);
            E->SetMarker(m_emitterMarker);
            E = T->Emitter(); // update can stop itself
            if (E)
                T->SetPriority(E->Priority());
            else
                T->SetPriority(-1);
        }
        else
        {
            T->SetPriority(-1);
        }
    }

    // Update emmitters
    for (it = 0; it < m_emitters.size(); it++)
    {
        ISoundEmitter* pEmitter = m_emitters[it];
        if (pEmitter->Marker() != m_emitterMarker)
        {
            pEmitter->Update(dt_sec);
            pEmitter->SetMarker(m_emitterMarker);
        }
        if (!pEmitter->IsPlaying())
        {
            // Stopped
            xr_delete(pEmitter);
            m_emitters.erase(m_emitters.begin() + it);
            it--;
        }
    }

    // Get currently rendering emitters
    m_deferredRenderTargets.clear();
    for (it = 0; it < m_renderTargets.size(); it++)
    {
        ISoundRenderTarget* T = m_renderTargets[it];
        if (T->Emitter())
        {
            // Has emmitter, maybe just not started rendering
            if (T->IsRendering())
            {
                T->Fill();
                T->Update();
            }
            else
                m_deferredRenderTargets.push_back(T);
        }
    }

    // Commit parameters from pending targets
    if (!m_deferredRenderTargets.empty())
    {
        m_deferredRenderTargets.erase(std::unique(m_deferredRenderTargets.begin(), m_deferredRenderTargets.end()), m_deferredRenderTargets.end());
        for (it = 0; it < m_deferredRenderTargets.size(); it++)
            m_deferredRenderTargets[it]->Fill();
    }

    // update EAX or EFX
    if (psSoundFlags.test(ss_EAX) && (m_hasEax || m_hasEfx))
    {
        static shared_str curr_env;

        if (m_isListenerMoved)
        {
            m_isListenerMoved = FALSE;
            m_targetEnvironment = *get_environment(P);

            if (!curr_env.size() || curr_env != m_targetEnvironment.Name())
            {
                curr_env = m_targetEnvironment.Name();
                Msg("~ current environment sound zone name [%s]", curr_env.c_str());
            }
        }

        m_currentEnvironment.Lerp(m_currentEnvironment, m_targetEnvironment, dt_sec);

        if (m_hasEax)
        {
            i_eax_listener_set(&m_currentEnvironment);
            i_eax_commit_setting();
        }
        else
        {
            i_efx_listener_set(&m_currentEnvironment);
            m_hasEfx = i_efx_commit_setting();
        }
    }

    // update listener
    update_listener(P, D, N, dt_sec);

    // Start rendering of pending targets
    if (!m_deferredRenderTargets.empty())
    {
        for (it = 0; it < m_deferredRenderTargets.size(); it++)
            m_deferredRenderTargets[it]->Render();
    }

    update_events();

    m_isLocked = false;
}

static u32 g_saved_event_count = 0;
void CSoundRender_Core::update_events()
{
    g_saved_event_count = s_events.size();
    for (u32 it = 0; it < s_events.size(); it++)
    {
        event& E = s_events[it];
        Handler(E.first, E.second);
    }
    s_events.clear();
}

void CSoundRender_Core::statistic(CSound_stats* dest, CSound_stats_ext* ext)
{
    if (dest)
    {
        dest->_rendered = 0;
        for (u32 it = 0; it < m_renderTargets.size(); it++)
        {
            ISoundRenderTarget* T = m_renderTargets[it];
            if (T->Emitter() && T->IsRendering())
                dest->_rendered++;
        }
        dest->_simulated = m_emitters.size();
        dest->_cache_hits = m_cache.StatsHits();
        dest->_cache_misses = m_cache.StatsMiss();
        dest->_events = g_saved_event_count;
        m_cache.ClearStatistic();
    }
    if (ext)
    {
        for (u32 it = 0; it < m_emitters.size(); it++)
        {
            ISoundEmitter* _E = m_emitters[it];
            CSound_stats_ext::SItem _I;
            _I._3D = !_E->Is2D();
            _I._rendered = !!_E->RenderTarget();
            _I.params = *_E->Params();
            _I.volume = _E->Volume();
            if (_E->SoundData())
            {
                _I.name = _E->RenderSource()->FileName();
                _I.game_object = _E->SoundData()->g_object;
                _I.game_type = _E->SoundData()->g_type;
                _I.type = _E->SoundData()->s_type;
            }
            else
            {
                _I.game_object = 0;
                _I.game_type = 0;
                _I.type = st_Effect;
            }
            ext->append(_I);
        }
    }
}

float CSoundRender_Core::get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion)
{
    float occ_value = 1.f;

    if (0 != geom_SOM)
    {
        // Calculate RAY params
        Fvector pos, dir;
        pos.random_dir();
        pos.mul(dispersion);
        pos.add(snd_pt);
        dir.sub(pos, hear_pt);
        float range = dir.magnitude();
        dir.div(range);

        geom_DB.ray_query(CDB::OPT_CULL, geom_SOM, hear_pt, dir, range);
        u32 r_cnt = u32(geom_DB.r_count());
        CDB::RESULT* _B = geom_DB.r_begin();

        if (0 != r_cnt)
        {
            for (u32 k = 0; k < r_cnt; k++)
            {
                CDB::RESULT* R = _B + k;
                occ_value *= *(float*)&R->dummy;
            }
        }
    }
    return occ_value;
}

float CSoundRender_Core::get_occlusion(Fvector& P, float R, Fvector* occ)
{
    float occ_value = 1.f;

    // Calculate RAY params
    Fvector base = listener_position();
    Fvector pos, dir;
    float range;
    pos.random_dir();
    pos.mul(R);
    pos.add(P);
    dir.sub(pos, base);
    range = dir.magnitude();
    dir.div(range);

    if (0 != geom_MODEL)
    {
        bool bNeedFullTest = true;
        // 1. Check cached polygon
        float _u, _v, _range;
        if (CDB::TestRayTri(base, dir, occ, _u, _v, _range, true))
            if (_range > 0 && _range < range)
            {
                occ_value = psSoundOcclusionScale;
                bNeedFullTest = false;
            }
        // 2. Polygon doesn't picked up - real database query
        if (bNeedFullTest)
        {
            geom_DB.ray_query(CDB::OPT_ONLYNEAREST, geom_MODEL, base, dir, range);
            if (0 != geom_DB.r_count())
            {
                // cache polygon
                const CDB::RESULT* R = geom_DB.r_begin();
                const CDB::TRI& T = geom_MODEL->get_tris()[R->id];
                const Fvector* V = geom_MODEL->get_verts();
                occ[0].set(V[T.verts[0]]);
                occ[1].set(V[T.verts[1]]);
                occ[2].set(V[T.verts[2]]);
                occ_value = psSoundOcclusionScale;
            }
        }
    }
    if (0 != geom_SOM)
    {
        geom_DB.ray_query(CDB::OPT_CULL, geom_SOM, base, dir, range);
        u32 r_cnt = u32(geom_DB.r_count());
        CDB::RESULT* _B = geom_DB.r_begin();

        if (0 != r_cnt)
        {
            for (u32 k = 0; k < r_cnt; k++)
            {
                CDB::RESULT* R = _B + k;
                occ_value *= *(float*)&R->dummy;
            }
        }
    }
    return occ_value;
}

ISoundRenderSource* CSoundRender_Core::i_create_source(LPCSTR name)
{
    // Search
    string256 id;
    xr_strcpy(id, name);
    strlwr(id);
    if (strext(id))
        *strext(id) = 0;

    for (u32 it = 0; it < m_renderSources.size(); it++)
    {
        if (0 == xr_strcmp(m_renderSources[it]->FileName(), id))
            return m_renderSources[it];
    }

    // Load a _new one
    ISoundRenderSource* S = xr_new<SoundRenderSource>();
    S->Load(id);
    m_renderSources.push_back(S);
    return S;
}

void CSoundRender_Core::i_destroy_source(ISoundRenderSource* S)
{
    // No actual destroy at all
}

void CSoundRender_Core::i_start(ISoundEmitter* E)
{
    R_ASSERT(E);

    // Search lowest-priority target
    float Ptest = E->Priority();
    float Ptarget = flt_max;
    ISoundRenderTarget* T = 0;

    for (u32 it = 0; it < m_renderTargets.size(); it++)
    {
        ISoundRenderTarget* Ttest = m_renderTargets[it];
        if (Ttest->Priority() < Ptarget)
        {
            T = Ttest;
            Ptarget = Ttest->Priority();
        }
    }

    // Stop currently playing
    if (T->Emitter())
        T->Emitter()->Cancel();

    // Associate
    E->SetRenderTarget(T);
    E->RenderTarget()->Start(E);
    T->SetPriority(Ptest);
}

void CSoundRender_Core::i_stop(ISoundEmitter* E)
{
    R_ASSERT(E);
    R_ASSERT(E == E->RenderTarget()->Emitter());
    E->RenderTarget()->Stop();
    E->SetRenderTarget(nullptr);
}

void CSoundRender_Core::i_rewind(ISoundEmitter* E)
{
    R_ASSERT(E);
    R_ASSERT(E == E->RenderTarget()->Emitter());
    E->RenderTarget()->Rewind();
}

BOOL CSoundRender_Core::i_allow_play(ISoundEmitter* E)
{
    // Search available target
    float Ptest = E->Priority();
    for (u32 it = 0; it < m_renderTargets.size(); it++)
    {
        ISoundRenderTarget* T = m_renderTargets[it];
        if (T->Priority() < Ptest)
            return TRUE;
    }
    return FALSE;
}
