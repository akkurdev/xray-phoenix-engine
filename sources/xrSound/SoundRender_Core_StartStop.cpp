#include "stdafx.h"
#include "SoundRender_Core.h"
#include "SoundRender_Emitter.h"
#include "OalSoundRenderTarget.h"
#include "SoundRenderSource.h"

void CSoundRender_Core::i_start(CSoundRender_Emitter* E)
{
    R_ASSERT(E);

    // Search lowest-priority target
    float Ptest = E->Priority();
    float Ptarget = flt_max;
    ISoundRenderTarget* T = 0;

    for (u32 it = 0; it < s_targets.size(); it++)
    {
        ISoundRenderTarget* Ttest = s_targets[it];
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

void CSoundRender_Core::i_stop(CSoundRender_Emitter* E)
{
    R_ASSERT(E);
    R_ASSERT(E == E->RenderTarget()->Emitter());
    E->RenderTarget()->Stop();
    E->SetRenderTarget(nullptr);
}

void CSoundRender_Core::i_rewind(CSoundRender_Emitter* E)
{
    R_ASSERT(E);
    R_ASSERT(E == E->RenderTarget()->Emitter());
    E->RenderTarget()->Rewind();
}

BOOL CSoundRender_Core::i_allow_play(CSoundRender_Emitter* E)
{
    // Search available target
    float Ptest = E->Priority();
    for (u32 it = 0; it < s_targets.size(); it++)
    {
        ISoundRenderTarget* T = s_targets[it];
        if (T->Priority() < Ptest)
            return TRUE;
    }
    return FALSE;
}
