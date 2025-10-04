#include "stdafx.h"

#include "SoundRender_Core.h"
#include "SoundRender_Source.h"

ISoundRenderSource* CSoundRender_Core::i_create_source(LPCSTR name)
{
    // Search
    string256 id;
    xr_strcpy(id, name);
    strlwr(id);
    if (strext(id))
        *strext(id) = 0;

    for (u32 it = 0; it < s_sources.size(); it++)
    {
        if (0 == xr_strcmp(s_sources[it]->FileName(), id))
            return s_sources[it];
    }

    // Load a _new one
    ISoundRenderSource* S = xr_new<SoundRenderSource>();
    S->Load(id);
    s_sources.push_back(S);
    return S;
}

void CSoundRender_Core::i_destroy_source(ISoundRenderSource* S)
{
    // No actual destroy at all
}
