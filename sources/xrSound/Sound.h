#pragma once
#include "dllexp.h"
#include "ref_sound.h"
#include "ref_sound_data.h"

#define SNDENV_FILENAME "sEnvironment.xr"
#define OGG_COMMENT_VERSION 0x0003

// refs
class CObject;
class XRSOUND_API CSound_stream_interface;

XRSOUND_API extern u32 psSoundModel;
XRSOUND_API extern float psSoundVEffects;
XRSOUND_API extern float psSoundVFactor;
XRSOUND_API extern float psSoundVMusic;
XRSOUND_API extern float psSoundRolloff;
XRSOUND_API extern float psSoundOcclusionScale;
XRSOUND_API extern float psSoundLinearFadeFactor;
XRSOUND_API extern Flags32 psSoundFlags;
XRSOUND_API extern int psSoundTargets;
XRSOUND_API extern int psSoundCacheSizeMB;
XRSOUND_API extern xr_token* snd_devices_token;
XRSOUND_API extern u32 snd_device_id;
XRSOUND_API extern float psSoundTimeFactor;

// Flags
enum
{
    ss_Hardware = (1ul << 1ul), //!< Use hardware mixing only
    ss_EAX = (1ul << 2ul), //!< Use EAX or EFX
    ss_forcedword = u32(-1)
};

enum
{
    sq_DEFAULT,
    sq_NOVIRT,
    sq_LIGHT,
    sq_HIGH,
    sq_forcedword = u32(-1)
};

enum
{
    sg_Undefined = 0,
    sg_SourceType = u32(-1),
    sg_forcedword = u32(-1),
};

enum
{
    sm_Looped = (1ul << 0ul), //!< Looped
    sm_2D = (1ul << 1ul), //!< 2D mode
    sm_forcedword = u32(-1),
};



/*! \class ref_sound
\brief Sound source + control

The main class respresenting source/emitter interface
This class infact just hides internals and redirect calls to
specific sub-systems
*/

/// definition (Sound Stream Interface)
class XRSOUND_API CSound_stream_interface
{
public:
};

/// definition (Sound Stream Interface)
class XRSOUND_API CSound_stats
{
public:
    u32 _rendered;
    u32 _simulated;
    u32 _cache_hits;
    u32 _cache_misses;
    u32 _events;
};

class XRSOUND_API CSound_stats_ext
{
public:
    struct SItem
    {
        shared_str name;
        CSound_params params;
        float volume;
        esound_type type;
        int game_type;
        CObject* game_object;
        struct
        {
            u32 _3D : 1;
            u32 _rendered : 1;
        };
    };
    DEFINE_VECTOR(SItem, item_vec, item_vec_it);
    item_vec items;

public:
    void clear() { items.clear(); }
    void append(const SItem& itm) { items.push_back(itm); }
};

/// definition (Sound Callback)
typedef void __stdcall sound_event(ref_sound_data_ptr S, float range);

/// definition (Sound Manager Interface)
class XRSOUND_API CSound_manager_interface
{
    virtual void _initialize(int stage) = 0;
    virtual void _clear() = 0;

protected:
    friend class ref_sound_data;
    virtual void _create_data(ref_sound_data& S, LPCSTR fName, esound_type sound_type, int game_type) = 0;
    virtual void _destroy_data(ref_sound_data& S) = 0;

public:
    virtual ~CSound_manager_interface() {}

    static void _create(int stage);
    static void _destroy();

    virtual void _restart() = 0;
    virtual BOOL i_locked() = 0;

    virtual void create(ref_sound& S, LPCSTR fName, esound_type sound_type, int game_type) = 0;
    virtual void attach_tail(ref_sound& S, LPCSTR fName) = 0;
    virtual void clone(ref_sound& S, const ref_sound& from, esound_type sound_type, int game_type) = 0;
    virtual void destroy(ref_sound& S) = 0;
    virtual void stop_emitters() = 0;
    virtual int pause_emitters(bool val) = 0;

    virtual void play(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f) = 0;
    virtual void play_at_pos(ref_sound& S, CObject* O, const Fvector& pos, u32 flags = 0, float delay = 0.f) = 0;
    virtual void play_no_feedback(ref_sound& S, CObject* O, u32 flags = 0, float delay = 0.f, Fvector* pos = 0, float* vol = 0, float* freq = 0, Fvector2* range = 0) = 0;

    virtual void set_master_volume(float f = 1.f) = 0;
    virtual void set_geometry_env(IReader* I) = 0;
    virtual void set_geometry_som(IReader* I) = 0;
    virtual void set_geometry_occ(CDB::MODEL* M) = 0;
    virtual void set_handler(sound_event* E) = 0;

    virtual void update(const Fvector& P, const Fvector& D, const Fvector& N) = 0;
    virtual void statistic(CSound_stats* s0, CSound_stats_ext* s1) = 0;

    virtual float get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion = 0.2f) = 0;

    virtual void object_relcase(CObject* obj) = 0;
    virtual const Fvector& listener_position() = 0;
};

class CSound_manager_interface;
extern XRSOUND_API CSound_manager_interface* Sound;

/// ********* Sound ********* (utils, accessors, helpers)
