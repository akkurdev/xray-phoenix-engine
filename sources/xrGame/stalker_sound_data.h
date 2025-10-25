////////////////////////////////////////////////////////////////////////////
//	Module 		: stalker_sound_data.h
//	Created 	: 02.02.2005
//  Modified 	: 02.02.2005
//	Author		: Dmitriy Iassenev
//	Description : Stalker sound data
////////////////////////////////////////////////////////////////////////////

#pragma once

class CAI_Stalker;

class CStalkerSoundData : public SoundUserData
{
private:
    CAI_Stalker* m_object;

public:
    IC CStalkerSoundData(CAI_Stalker* object);
    virtual ~CStalkerSoundData();
    virtual void Invalidate();
    virtual void Accept(CSound_UserDataVisitor* visitor);
    IC CAI_Stalker& object() const;
};

#include "stalker_sound_data_inline.h"