#pragma once

class CSound_UserDataVisitor;

class SoundUserData : public xr_resource
{
public:
    virtual ~SoundUserData() {}
    virtual void Accept(CSound_UserDataVisitor* visitor) = 0;
    virtual void Invalidate() = 0;
};

typedef resptr_core<SoundUserData, resptr_base<SoundUserData>> SoundUserDataPtr;