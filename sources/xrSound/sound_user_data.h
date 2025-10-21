#pragma once

class CSound_UserDataVisitor;

class CSound_UserData : public xr_resource
{
public:
    virtual ~CSound_UserData() {}
    virtual void accept(CSound_UserDataVisitor*) = 0;
    virtual void invalidate() = 0;
};
typedef resptr_core<CSound_UserData, resptr_base<CSound_UserData>> CSound_UserDataPtr;