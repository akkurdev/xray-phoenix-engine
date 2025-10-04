#pragma once
#include "../xrCore/xrCore.h"

class CGameFont;

class IFontRender
{
public:
    virtual ~IFontRender() { ; }

    virtual void Initialize(LPCSTR cShader, LPCSTR cTexture) = 0;
    virtual void OnRender(CGameFont& owner) = 0;
};
