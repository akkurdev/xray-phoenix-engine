#pragma once
#include <al.h>
#include <efx-presets.h>
#include "SoundRender.h"
#include "SoundRenderCache.h"
#include "ISoundrenderTarget.h"
#include <SoundEnvironmentLibrary.h>

class CSoundRender_Core : public CSound_manager_interface
{
public:
    typedef std::pair<ref_sound_data_ptr, float> event;
    xr_vector<event> s_events;
    BOOL bPresent;
    CTimer Timer;
    float fTimer_Value;
    float fTimer_Delta;
    sound_event* Handler;
    SoundRenderCache cache;    

public:
    CSoundRender_Core();
    virtual ~CSoundRender_Core();

    virtual void _create_data(ref_sound_data& S, LPCSTR fName, esound_type sound_type, int game_type);
    virtual void _destroy_data(ref_sound_data& S);
    virtual void create(ref_sound& S, const char* fName, esound_type sound_type, int game_type);
    virtual void attach_tail(ref_sound& S, LPCSTR fName);
    virtual void clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int game_type);
    virtual void destroy(ref_sound& S);    
    virtual void play(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f);
    virtual void play_at_pos(ref_sound& S, CObject* O, const Fvector& pos, u32 flags = 0, float delay = 0.f);
    virtual void play_no_feedback(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f, Fvector* pos = 0, float* vol = 0, float* freq = 0, Fvector2* range = 0); 
    virtual void _initialize(int stage) = 0;
    virtual void _clear() = 0;
    virtual void _restart();
    virtual void stop_emitters();
    virtual int pause_emitters(bool val);
    virtual void set_master_volume(float f) = 0;
    virtual void set_geometry_env(IReader* I);
    virtual void set_geometry_som(IReader* I);
    virtual void set_geometry_occ(CDB::MODEL* M);
    virtual void set_handler(sound_event* E);
    virtual void update(const Fvector& P, const Fvector& D, const Fvector& N);
    virtual void statistic(CSound_stats* dest, CSound_stats_ext* ext);
    virtual void object_relcase(CObject* obj);
    ISoundRenderSource* i_create_source(LPCSTR name);
    void i_destroy_source(ISoundRenderSource* S);    
    void i_start(ISoundEmitter* E);
    void i_stop(ISoundEmitter* E);
    void i_rewind(ISoundEmitter* E);
    BOOL i_allow_play(ISoundEmitter* E);
    virtual BOOL i_locked() { return m_isLocked; }
    float get_occlusion(Fvector& P, float R, Fvector* occ);  
    virtual float get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion = 0.2f);

protected:
    virtual void i_eax_set(const GUID* guid, u32 prop, void* val, u32 sz) = 0;
    virtual void i_eax_get(const GUID* guid, u32 prop, void* val, u32 sz) = 0;    
    bool EFXTestSupport();
    void InitAlEFXAPI();       
    virtual void update_events();    
    virtual void update_listener(const Fvector& P, const Fvector& D, const Fvector& N, float dt) = 0;
    void i_eax_listener_set(SoundEnvironment* E);
    void i_eax_commit_setting();
    void i_efx_listener_set(SoundEnvironment* E);
    bool i_efx_commit_setting();
    ISoundEmitter* i_play(ref_sound* S, BOOL _loop, float delay);    
    SoundEnvironment* get_environment(const Fvector& P);
    void env_load();
    void env_unload();
    void env_apply();

protected:
    bool m_isListenerMoved;
    SoundEnvironment m_currentEnvironment;
    SoundEnvironment m_targetEnvironment;
    CDB::COLLIDER geom_DB;
    CDB::MODEL* geom_SOM;
    CDB::MODEL* geom_MODEL;
    CDB::MODEL* geom_ENV;
    std::vector<ISoundRenderSource*> m_renderSources;
    std::vector<ISoundEmitter*> m_emitters;
    uint32_t m_emitterMarker;
    std::vector<ISoundRenderTarget*> m_renderTargets;
    std::vector<ISoundRenderTarget*> m_deferredRenderTargets;
    SoundEnvironmentLibrary* m_environments;
    std::vector<uint16_t> m_environmentIds;
    EFXEAXREVERBPROPERTIES m_reverberationProps;
    ALuint m_effect;
    ALuint m_slot;
    bool m_hasEax;
    bool m_hasDeferredEax;
    bool m_hasEfx;
    bool m_isReady;
    uint32_t m_cacheLineSize;
    volatile bool m_isLocked;
};

extern XRSOUND_API CSoundRender_Core* SoundRender;
