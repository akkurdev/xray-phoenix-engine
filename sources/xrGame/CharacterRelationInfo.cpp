#include "stdafx.h"
#include "CharacterRelationInfo.h"
#include <object_loader.h>
#include <object_saver.h>

namespace Stalker::Mechanics::Relations
{
    CharacterRelationInfo::CharacterRelationInfo()
        :
        PersonalRelations(PersonalRelationMap()),
        CommunityRelations(CommunityRelationMap()),
        Foes(xr_vector<u16>())
    {
    }

    void CharacterRelationInfo::clear()
    {
        PersonalRelations.clear();
        CommunityRelations.clear();
    }

    void CharacterRelationInfo::load(IReader& reader)
    {
        load_data(PersonalRelations, reader);
        load_data(CommunityRelations, reader);
    }

    void CharacterRelationInfo::save(IWriter& writer)
    {
        save_data(PersonalRelations, writer);
        save_data(CommunityRelations, writer);
    }
}