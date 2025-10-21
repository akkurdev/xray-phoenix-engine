#include "stdafx.h"
#include "SoundRender_Core.h"
#include "SoundEmitter.h"
#include "OalSoundRenderTarget.h"
#include "SoundRenderSource.h"

void CSoundRender_Core::i_start(ISoundEmitter* E)
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
    for (u32 it = 0; it < s_targets.size(); it++)
    {
        ISoundRenderTarget* T = s_targets[it];
        if (T->Priority() < Ptest)
            return TRUE;
    }
    return FALSE;
}
